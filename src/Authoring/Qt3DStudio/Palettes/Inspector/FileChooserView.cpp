/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "FileChooserView.h"
#include "FileChooserModel.h"
#include "Literals.h"
#include "StudioUtils.h"
#include "IDocumentEditor.h"
#include "Qt3DSDMStudioSystem.h"
#include "Qt3DSDMValue.h"
#include "Core.h"
#include "Doc.h"
#include "StudioApp.h"
#include "StudioPreferences.h"

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtCore/qtimer.h>

FileChooserView::FileChooserView(QWidget *parent)
    : QQuickWidget(parent)
    , m_model(new FileChooserModel(this))
{
    setWindowTitle(tr("Imports"));
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    QTimer::singleShot(0, this, &FileChooserView::initialize);
}

void FileChooserView::initialize()
{
    CStudioPreferences::setQmlContextProperties(rootContext());
    rootContext()->setContextProperty(QStringLiteral("_resDir"),
                                      StudioUtils::resourceImageUrl());
    rootContext()->setContextProperty(QStringLiteral("_fileChooserView"), this);
    rootContext()->setContextProperty(QStringLiteral("_fileChooserModel"), m_model);
    engine()->addImportPath(StudioUtils::qmlImportPath());
    setSource(QUrl(QStringLiteral("qrc:/Palettes/Inspector/FileChooser.qml")));
}

void FileChooserView::setHandle(int handle)
{
    m_handle = handle;
}

int FileChooserView::handle() const
{
    return m_handle;
}

void FileChooserView::setInstance(int instance)
{
    m_instance = instance;
}

int FileChooserView::instance() const
{
    return m_instance;
}

void FileChooserView::updateSelection()
{
    const auto doc = g_StudioApp.GetCore()->GetDoc();
    const auto propertySystem = doc->GetStudioSystem()->GetPropertySystem();

    qt3dsdm::SValue value;
    propertySystem->GetInstancePropertyValue(m_instance, m_handle, value);

    QString valueStr = qt3dsdm::get<QString>(value);
    if (valueStr.isEmpty())
        valueStr = ChooserModelBase::noneString();

    m_model->setCurrentFile(valueStr);
}

void FileChooserView::focusOutEvent(QFocusEvent *event)
{
    QQuickWidget::focusOutEvent(event);
    // Don't lose focus because of progress dialog pops up which happens e.g. when importing mesh
    // in response to file selection
    if (g_StudioApp.isOnProgress()) {
        if (!m_focusOutTimer) {
            m_focusOutTimer = new QTimer(this);
            connect(m_focusOutTimer, &QTimer::timeout, [this]() {
                // Periodically check if progress is done to refocus the chooser view
                if (!g_StudioApp.isOnProgress()) {
                    m_focusOutTimer->stop();
                    m_focusOutTimer->deleteLater();
                    m_focusOutTimer = nullptr;
                    this->activateWindow();
                }
            });
            m_focusOutTimer->start(250);
        }
    } else {
        QTimer::singleShot(0, this, &FileChooserView::close);
    }
}

void FileChooserView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
        QTimer::singleShot(0, this, &FileChooserView::close);

    QQuickWidget::keyPressEvent(event);
}

void FileChooserView::showEvent(QShowEvent *event)
{
    updateSelection();
    QQuickWidget::showEvent(event);
}
