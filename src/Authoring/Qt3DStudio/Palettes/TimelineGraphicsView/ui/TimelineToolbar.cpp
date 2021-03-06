/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "TimelineToolbar.h"
#include "StudioApp.h"
#include "Core.h"
#include "Doc.h"
#include "Dispatch.h"
#include "DataInputSelectView.h"
#include "DataInputDlg.h"
#include "Qt3DSDMStudioSystem.h"
#include "StudioPreferences.h"
#include "ClientDataModelBridge.h"
#include "IDocumentEditor.h"
#include "DocumentEditorEnumerations.h"
#include "Dialogs.h"

#include <QtWidgets/qslider.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qtimer.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/qshortcut.h>

class TimelineScaleEventfilter : public QObject
{
public:
    TimelineScaleEventfilter(QObject *parent) : QObject(parent) {}

    bool eventFilter(QObject *obj, QEvent *event) override
    {
        // reset scale upon double clicking the scale slider's head
        if (event->type() == QEvent::MouseButtonDblClick) {
            static_cast<QSlider *>(obj)->setValue(50);
            event->accept();
            return true;
        }

        return QObject::eventFilter(obj, event);
    }
};

TimelineToolbar::TimelineToolbar() : QToolBar()
{
    setContentsMargins(0, 0, 0, 0);
    setIconSize(QSize(16, 16));

    // create icons
    static const QIcon iconLayer = QIcon(":/images/Objects-Layer-Normal.png");
    static const QIcon iconDelete = QIcon(":/images/Action-Trash-Normal.png");
    static const QIcon iconFirst = QIcon(":/images/playback_tools_first.png");
    static const QIcon iconLast = QIcon(":/images/playback_tools_last.png");
    static const QIcon iconZoomIn = QIcon(":/images/zoom_in.png");
    static const QIcon iconZoomOut = QIcon(":/images/zoom_out.png");
    m_iconDiActive = QIcon(":/images/Objects-DataInput-Active.png");
    m_iconDiInactive = QIcon(":/images/Objects-DataInput-Inactive.png");
    m_iconStop = QIcon(":/images/playback_tools_stop.png");
    m_iconPlay = QIcon(":/images/playback_tools_play.png");

    // create actions
    QString ctrlKey(QStringLiteral("Ctrl+"));
    QString altKey(QStringLiteral("Alt+"));
#ifdef Q_OS_MACOS
    ctrlKey = "⌘";
    altKey = "⌥";
#endif
    QString newLayerString = tr("Add New Layer (%1L)").arg(ctrlKey);
    m_actionNewLayer = new QAction(iconLayer, newLayerString, this);
    QAction *actionFirst = new QAction(iconFirst, tr("Go to Timeline Start"), this);
    QAction *actionLast = new QAction(iconLast, tr("Go to Timeline End"), this);
    m_actionDataInput = new QAction(m_iconDiInactive, tr("No Controller"), this);
    m_actionDeleteRow = new QAction(iconDelete, tr("Delete Selected Object (Del)"), this);
    m_actionPlayStop = new QAction(this);
    m_timeLabel = new QPushButton(this);
    m_diLabel = new QLabel(this);
    m_actionZoomIn = new QAction(iconZoomIn, tr("Zoom In"), this);
    m_actionZoomOut = new QAction(iconZoomOut, tr("Zoom Out"), this);

    m_scaleSlider = new QSlider();
    m_scaleSlider->setOrientation(Qt::Horizontal);
    m_scaleSlider->setFixedWidth(100);
    m_scaleSlider->setMinimum(0);
    m_scaleSlider->setMaximum(100);
    m_scaleSlider->setValue(50);
    m_scaleSlider->installEventFilter(new TimelineScaleEventfilter(this));

    m_timeLabel->setObjectName(QLatin1String("timelineButton"));
    m_timeLabel->setFlat(true);
    m_timeLabel->setMinimumWidth(80);
    m_timeLabel->setToolTip(tr("Go To Time (%1%2T)").arg(ctrlKey).arg(altKey));

    m_diLabel->clear();
    m_diLabel->setMinimumWidth(100);
    m_diLabel->setAlignment(Qt::AlignCenter);
    QString styleString = "QLabel { background: transparent; color: "
                          + QString(CStudioPreferences::dataInputColor().name()) + "; }";
    m_diLabel->setStyleSheet(styleString);

    m_actionShowRowTexts = new QAction(tr("Toggle Timebars Text Visibility"), this);
    QIcon rowTextIcon { QPixmap(":/images/timeline_text_hidden.png") };
    rowTextIcon.addPixmap(QPixmap(":/images/timeline_text_shown.png"), QIcon::Normal, QIcon::On);
    m_actionShowRowTexts->setIcon(rowTextIcon);
    m_actionShowRowTexts->setCheckable(true);
    m_actionFilter = new QAction(tr("Filter Timeline Rows Visibility According to Variants Filter"),
                                 this);
    m_actionFilter->setCheckable(true);
    QIcon filterIcon { QPixmap(":/images/filter.png") };
    filterIcon.addPixmap(QPixmap(":/images/filter-colored.png"), QIcon::Normal, QIcon::On);
    m_actionFilter->setIcon(filterIcon);

    updatePlayButtonState(false);

    // connections
    connect(m_actionNewLayer, &QAction::triggered, this, &TimelineToolbar::newLayerTriggered);
    connect(m_actionDeleteRow, &QAction::triggered, this, &TimelineToolbar::deleteLayerTriggered);
    connect(m_timeLabel, &QPushButton::clicked, this, &TimelineToolbar::gotoTimeTriggered);
    connect(actionFirst, &QAction::triggered, this, &TimelineToolbar::firstFrameTriggered);
    connect(m_actionPlayStop, &QAction::triggered, this, &TimelineToolbar::onPlayButtonClicked);
    connect(actionLast, &QAction::triggered, this, &TimelineToolbar::lastFrameTriggered);
    connect(m_scaleSlider, &QSlider::valueChanged, this, &TimelineToolbar::onZoomLevelChanged);
    connect(m_actionZoomIn, &QAction::triggered, this, &TimelineToolbar::onZoomInButtonClicked);
    connect(m_actionZoomOut, &QAction::triggered, this, &TimelineToolbar::onZoomOutButtonClicked);
    connect(m_actionDataInput, &QAction::triggered, this, &TimelineToolbar::onDiButtonClicked);
    connect(m_actionShowRowTexts, &QAction::toggled, this, &TimelineToolbar::showRowTextsToggled);
    connect(m_actionFilter, &QAction::toggled, this, &TimelineToolbar::variantsFilterToggled);

    // add actions
    addAction(m_actionNewLayer);
    addAction(m_actionDeleteRow);
    addAction(m_actionDataInput);
    addSpacing(2);
    addAction(m_actionShowRowTexts);
    addAction(m_actionFilter);
    addWidget(m_diLabel);
    addSpacing(20);
    addWidget(m_timeLabel);
    addSpacing(20);
    addAction(actionFirst);
    addAction(m_actionPlayStop);
    addAction(actionLast);
    addSpacing(30);
    addAction(m_actionZoomOut);
    addWidget(m_scaleSlider);
    addAction(m_actionZoomIn);

    // add keyboard shortcuts
    m_actionZoomOut->setShortcut(Qt::Key_Minus);
    m_actionZoomOut->setShortcutContext(Qt::ApplicationShortcut);
    m_actionZoomIn->setShortcut(Qt::Key_Plus);
    m_actionZoomIn->setShortcutContext(Qt::ApplicationShortcut);
    m_actionNewLayer->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_L));
    m_actionNewLayer->setShortcutContext(Qt::ApplicationShortcut);

    QShortcut *gotoTimeShortcut = new QShortcut(this);
    gotoTimeShortcut->setKey(QKeySequence(Qt::ControlModifier | Qt::AltModifier | Qt::Key_T));
    gotoTimeShortcut->setContext(Qt::ApplicationShortcut);
    connect(gotoTimeShortcut, &QShortcut::activated, this, &TimelineToolbar::gotoTimeTriggered);

    m_connectSelectionChange = g_StudioApp.GetCore()->GetDispatch()->ConnectSelectionChange(
                std::bind(&TimelineToolbar::onSelectionChange, this, std::placeholders::_1));

    // make datainput indicator listen to selection dialog choice
    const QVector<EDataType> acceptedTypes = { EDataType::DataTypeRangedNumber };
    m_dataInputSelector = new DataInputSelectView(acceptedTypes, this);
    g_StudioApp.GetCore()->GetDispatch()->AddDataModelListener(this);
    connect(m_dataInputSelector, &DataInputSelectView::dataInputChanged,
            this, &TimelineToolbar::onDataInputChange);
}

TimelineToolbar::~TimelineToolbar()
{
    delete m_dataInputSelector;
}

void TimelineToolbar::onSelectionChange(Q3DStudio::SSelectedValue inNewSelectable)
{
    qt3dsdm::TInstanceHandleList selectedInstances = inNewSelectable.GetSelectedInstances();
    CDoc *doc = g_StudioApp.GetCore()->GetDoc();
    CClientDataModelBridge *theClientBridge = doc->GetStudioSystem()->GetClientDataModelBridge();
    bool canDelete = false;
    for (size_t idx = 0, end = selectedInstances.size(); idx < end; ++idx) {
        if (theClientBridge->CanDelete(selectedInstances[idx])) {
            canDelete = true;
            break;
        }
    }

    m_actionDeleteRow->setEnabled(canDelete);
}

// add a spacer widget
void TimelineToolbar::addSpacing(int width)
{
    auto *widget = new QWidget;
    widget->setStyleSheet("background:transparent;");
    widget->setFixedWidth(width);
    addWidget(widget);
}

void TimelineToolbar::setTime(long totalMillis)
{
    long mins = totalMillis % 3600000 / 60000;
    long secs = totalMillis % 60000 / 1000;
    long millis = totalMillis % 1000;

    m_timeLabel->setText(QString::asprintf("%01d:%02d.%03d", mins, secs, millis));
}

QString TimelineToolbar::getCurrentController() const
{
    return m_currController;
}

QAction *TimelineToolbar::actionShowRowTexts() const
{
    return m_actionShowRowTexts;
}

void TimelineToolbar::setNewLayerEnabled(bool enable)
{
    m_actionNewLayer->setEnabled(enable);
}

void TimelineToolbar::updatePlayButtonState(bool started)
{
    if (started) {
        m_actionPlayStop->setIcon(m_iconStop);
        m_actionPlayStop->setText(tr("Stop Playing (Enter)"));
    } else {
        m_actionPlayStop->setIcon(m_iconPlay);
        m_actionPlayStop->setText(tr("Start Playing (Enter)"));
    }
}

void TimelineToolbar::onPlayButtonClicked()
{
    CDoc *doc = g_StudioApp.GetCore()->GetDoc();
    if (doc->IsPlaying())
        emit stopTriggered();
    else
        emit playTriggered();
}

void TimelineToolbar::onZoomLevelChanged(int scale)
{
    m_actionZoomIn->setEnabled(scale < m_scaleSlider->maximum());
    m_actionZoomOut->setEnabled(scale > m_scaleSlider->minimum());

    emit timelineScaleChanged(scale);
}

void TimelineToolbar::onZoomInButtonClicked()
{
    m_scaleSlider->setValue(m_scaleSlider->value() + 5);
}

void TimelineToolbar::onZoomOutButtonClicked()
{
    m_scaleSlider->setValue(m_scaleSlider->value() - 5);
}

void TimelineToolbar::onDiButtonClicked()
{
    QWidget *diButton = widgetForAction(m_actionDataInput);
    if (diButton) {
        QPoint chooserPos = diButton->pos() + QPoint(diButton->size().width(),
                                                     diButton->size().height());
        showDataInputChooser(mapToGlobal(chooserPos));
    }
}

bool TimelineToolbar::isVariantsFilterOn() const
{
    return m_actionFilter->isChecked();
}

// Update datainput button state according to this timecontext control state.
void TimelineToolbar::updateDataInputStatus()
{
    CDoc *doc = g_StudioApp.GetCore()->GetDoc();
    if (!doc->isValid())
        return;
    qt3dsdm::Qt3DSDMPropertyHandle ctrldProp;
    qt3dsdm::Qt3DSDMInstanceHandle timeCtxRoot = doc->GetActiveRootInstance();
    CClientDataModelBridge *theClientBridge = doc->GetStudioSystem()->GetClientDataModelBridge();
    if (theClientBridge->GetObjectType(timeCtxRoot) == EStudioObjectType::OBJTYPE_SCENE) {
        ctrldProp = theClientBridge->GetObjectDefinitions().m_Scene.m_ControlledProperty;
    } else if (theClientBridge->GetObjectType(timeCtxRoot) ==
               EStudioObjectType::OBJTYPE_COMPONENT) {
        ctrldProp = theClientBridge->GetObjectDefinitions().m_Component.m_ControlledProperty;
    } else {
        Q_ASSERT(false);
    }

    qt3dsdm::SValue controlledPropertyVal;
    doc->GetStudioSystem()->GetPropertySystem()->GetInstancePropertyValue(
                timeCtxRoot, ctrldProp, controlledPropertyVal);
    auto existingCtrl = qt3dsdm::get<QString>(controlledPropertyVal);

    QString newController;
    int timelineStrPos = existingCtrl.indexOf("@timeline");
    if (timelineStrPos != -1) {
        int ctrStrPos = existingCtrl.lastIndexOf("$", timelineStrPos - 2);
        newController = existingCtrl.mid(ctrStrPos + 1, timelineStrPos - ctrStrPos - 2);
    }
    if (newController != m_currController) {
        m_currController = newController;
        // Toggle if we changed to a controlled time context, or if icon current state
        // differs from the control state of current time context
        if (!m_currController.isEmpty()) {
            m_actionDataInput->setToolTip(
                tr("Timeline Controller:\n%1").arg(m_currController));
            m_actionDataInput->setIcon(m_iconDiActive);
            updateTimelineTitleColor(true);
        } else {
            // TODO actually delete the entire property instead of setting it as empty string
            m_actionDataInput->setIcon(m_iconDiInactive);
            m_actionDataInput->setToolTip(tr("No Controller"));
            updateTimelineTitleColor(false);
        }
        m_diLabel->setText(m_currController);
        emit controllerChanged(m_currController);
        if (m_dataInputSelector && m_dataInputSelector->isVisible())
            m_dataInputSelector->setCurrentController(m_currController);
    }
}

void TimelineToolbar::showDataInputChooser(const QPoint &point)
{
    QString currCtr = m_currController.size() ?
        m_currController : m_dataInputSelector->getNoneString();
    QVector<QPair<QString, int>> dataInputList;

    for (auto &it : qAsConst(g_StudioApp.m_dataInputDialogItems))
            dataInputList.append({it->name, it->type});

    m_dataInputSelector->setData(dataInputList, currCtr);

    CDialogs::showWidgetBrowser(this, m_dataInputSelector, point,
                                CDialogs::WidgetBrowserAlign::ToolButton);
}

void TimelineToolbar::onDataInputChange(int handle, int instance, const QString &dataInputName)
{
    Q_UNUSED(handle)
    Q_UNUSED(instance)

    if (dataInputName == m_currController)
        return;

    CDoc *doc = g_StudioApp.GetCore()->GetDoc();
    CClientDataModelBridge *bridge = doc->GetStudioSystem()->GetClientDataModelBridge();
    QString fullTimeControlStr;

    if (dataInputName != m_dataInputSelector->getNoneString()) {
        m_actionDataInput->setToolTip(tr("Timeline Controller:\n%1").arg(dataInputName));
        fullTimeControlStr = "$" + dataInputName + " @timeline";
        m_actionDataInput->setIcon(m_iconDiActive);
        m_currController = dataInputName;
        updateTimelineTitleColor(true);
    } else {
        m_actionDataInput->setToolTip(tr("No Controller"));
        // TODO actually delete the entire property instead of setting it as empty string
        m_actionDataInput->setIcon(m_iconDiInactive);
        m_currController.clear();
        updateTimelineTitleColor(false);
    }

    emit controllerChanged(m_currController);

    // To indicate that this presentation timeline is controlled by data input,
    // we set "controlled property" of this time context root (scene or component)
    // to contain the name of controller followed by special indicator "@timeline".
    // Either replace existing timeline control indicator string or append new one
    // but do not touch @slide indicator string as scene can have both
    qt3dsdm::Qt3DSDMPropertyHandle ctrldPropertyHandle;
    qt3dsdm::Qt3DSDMInstanceHandle timeCtxRoot = doc->GetActiveRootInstance();
    // Time context root is either scene or component
    if (bridge->GetObjectType(timeCtxRoot) == EStudioObjectType::OBJTYPE_SCENE)
        ctrldPropertyHandle = bridge->GetObjectDefinitions().m_Scene.m_ControlledProperty;
    else if (bridge->GetObjectType(timeCtxRoot) == EStudioObjectType::OBJTYPE_COMPONENT)
        ctrldPropertyHandle = bridge->GetObjectDefinitions().m_Component.m_ControlledProperty;
    else
        Q_ASSERT(false);

    qt3dsdm::SValue controlledPropertyVal;
    doc->GetStudioSystem()->GetPropertySystem()->GetInstancePropertyValue(
                timeCtxRoot, ctrldPropertyHandle, controlledPropertyVal);

    auto existingCtrl = qt3dsdm::get<QString>(controlledPropertyVal);
    int slideStrPos = existingCtrl.indexOf(QLatin1String("@timeline"));
    if (slideStrPos != -1) {
        // find the controlling datainput name and build the string to replace
        int ctrStrPos = existingCtrl.lastIndexOf(QLatin1Char('$'), slideStrPos - 2);
        QString prevCtrler = existingCtrl.mid(ctrStrPos, slideStrPos - ctrStrPos);
        existingCtrl.replace(prevCtrler + "@timeline", fullTimeControlStr);
    } else {
        if (!existingCtrl.isEmpty() && m_currController.size())
            existingCtrl.append(QLatin1Char(' '));
        existingCtrl.append(fullTimeControlStr);
    }

    if (existingCtrl.endsWith(QLatin1Char(' ')))
        existingCtrl.chop(1);

    if (existingCtrl.startsWith(QLatin1Char(' ')))
        existingCtrl.remove(0, 1);

    m_diLabel->setText(m_currController);
    qt3dsdm::SValue fullCtrlPropVal
        = std::make_shared<qt3dsdm::CDataStr>(
            Q3DStudio::CString::fromQString(existingCtrl));
    Q3DStudio::SCOPED_DOCUMENT_EDITOR(*doc, tr("Set Timeline control"))
        ->SetInstancePropertyValue(timeCtxRoot, ctrldPropertyHandle, fullCtrlPropVal);
}

void TimelineToolbar::OnBeginDataModelNotifications()
{
}

void TimelineToolbar::OnEndDataModelNotifications()
{
    updateDataInputStatus();
}

void TimelineToolbar::OnImmediateRefreshInstanceSingle(qt3dsdm::Qt3DSDMInstanceHandle inInstance)
{
    Q_UNUSED(inInstance)
}

void TimelineToolbar::OnImmediateRefreshInstanceMultiple(qt3dsdm::Qt3DSDMInstanceHandle *inInstance,
                                                         long inInstanceCount)
{
    Q_UNUSED(inInstance)
    Q_UNUSED(inInstanceCount)
}

// Notify the user about control state change also with timeline dock title color change.
void TimelineToolbar::updateTimelineTitleColor(bool controlled)
{
    QString styleString = QStringLiteral("QDockWidget#timeline { color: %1; }")
                          .arg(controlled ? CStudioPreferences::dataInputColor().name()
                                          : CStudioPreferences::textColor().name());

    QWidget *timelineDock = parentWidget()->parentWidget()->parentWidget();
    timelineDock->setStyleSheet(styleString);
}
