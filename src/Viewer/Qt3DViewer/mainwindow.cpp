/****************************************************************************
**
** Copyright (C) 2013 - 2016 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QKeyEvent>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QMimeData>
#include <QMessageBox>
#include <QShortcut>
#include <QWindow>
#include <QVariant>
#include <QGLFormat>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QtStudio3D/private/q3dsviewersettings_p.h>
#include <QtGui/qguiapplication.h>
#include <QtCore/qtimer.h>

#include "Qt3DSView.h"
#include "q3dspresentationitem.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(bool generatorMode, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_studio3D(0)
    , m_connectionInfo(0)
    , m_remoteDeploymentReceiver(0)
    , m_generatorMode(generatorMode)
{
    ui->setupUi(this);

    if (!m_generatorMode) {
        resize(1280, 720);
        ui->actionOpen->setShortcut(QKeySequence::Open);
        QList<QKeySequence> shortcuts;
        shortcuts.push_back(QKeySequence(QKeySequence::Quit));
        shortcuts.push_back(QKeySequence("CTRL+Q"));
        ui->actionQuit->setShortcuts(shortcuts);
        ui->actionReload->setShortcut(QKeySequence::Refresh);

        QStringList strArg = QApplication::arguments();
        if (strArg.size() >= 2) {
            QFileInfo theFilePath(strArg[1]);
            if (theFilePath.exists()) {
                m_openFileDir = theFilePath.path();
                QSettings().setValue("DirectoryOfLastOpen", m_openFileDir);
            }
        }

#ifdef Q_OS_ANDROID
        m_openFileDir = QStringLiteral("/sdcard/qt3dviewer"); // Add default folder for Android
#else
        // Allow drops. Not usable for Android.
        setAcceptDrops(true);
#endif

        addAction(ui->actionFull_Screen);
        addAction(ui->actionShowOnScreenStats);
        addAction(ui->actionBorder);
        addAction(ui->actionToggle_Scale_Mode);
        addAction(ui->actionToggle_Shade_Mode);
    } else {
        ui->menuBar->clear();
        ui->menuBar->addAction(ui->actionQuit);
        resize(700, 100);
    }

    // Set import paths so that standalone installation works
    QString extraImportPath1(QStringLiteral("%1/qml"));
#ifdef Q_OS_MACOS
    QString extraImportPath2(QStringLiteral("%1/../../../../qml"));
#else
    QString extraImportPath2(QStringLiteral("%1/../qml"));
#endif
    ui->quickWidget->engine()->addImportPath(
                extraImportPath1.arg(QGuiApplication::applicationDirPath()));
    ui->quickWidget->engine()->addImportPath(
                extraImportPath2.arg(QGuiApplication::applicationDirPath()));

    ui->quickWidget->setSource(QUrl("qrc:/viewer.qml"));

    if (m_generatorMode)
        setupGeneratorUI();

    updateUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionToggle_Scale_Mode_triggered()
{
    switch (viewer()->viewerSettings()->scaleMode())
    {
    case Q3DSViewerSettings::ScaleModeCenter:
        viewer()->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeFill);
        break;
    case Q3DSViewerSettings::ScaleModeFill:
        viewer()->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeFit);
        break;
    case Q3DSViewerSettings::ScaleModeFit:
        viewer()->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeCenter);
        break;
    }
    updateUI();
}

void MainWindow::on_actionToggle_Shade_Mode_triggered()
{
    switch (viewer()->viewerSettings()->shadeMode())
    {
    case Q3DSViewerSettings::ShadeModeShaded:
        viewer()->viewerSettings()->setShadeMode(Q3DSViewerSettings::ShadeModeShadedWireframe);
        break;
    case Q3DSViewerSettings::ShadeModeShadedWireframe:
        viewer()->viewerSettings()->setShadeMode(Q3DSViewerSettings::ShadeModeShaded);
        break;
    }
    updateUI();
}

void MainWindow::on_actionBorder_triggered()
{
    Q3DSView *view = viewer();
    if (!view)
        return;

    QColor matte = view->viewerSettings()->matteColor();
    if (matte == QColor(Qt::black) || !matte.isValid())
        view->viewerSettings()->setMatteColor(QColor(50, 50, 50));
    else
        view->viewerSettings()->setMatteColor(Qt::black);

    updateUI();
}

void MainWindow::on_actionFull_Screen_triggered()
{
    if (ui->actionFull_Screen->isChecked()) {
        showFullScreen();
        ui->menuBar->hide();
    } else {
        showNormal();
        ui->menuBar->show();
    }
}

void MainWindow::on_actionShowOnScreenStats_triggered()
{
    Q3DSView *view = viewer();
    if (!view)
        return;

    view->viewerSettings()->setShowRenderStats(!view->viewerSettings()->isShowRenderStats());
    updateUI();
}

void MainWindow::on_actionQuit_triggered()
{
    delete m_studio3D;
    m_studio3D = 0;
    close();
}

void MainWindow::on_actionOpen_triggered()
{
    QSettings settings;
    if (m_openFileDir.size() == 0) {
        m_openFileDir
                = settings.value("DirectoryOfLastOpen", QString("")).toString();
    }

    QString filename = QFileDialog::getOpenFileName(
                this, tr("Open File or Project"), m_openFileDir,
                tr("All supported formats (*.uip *.uia *.uiab);;Studio UI Presentation "
                   "(*.uip);;Application file (*.uia);;Binary Application (*.uiab)"),
                NULL, QFileDialog::DontUseNativeDialog);

    if (filename.size() == 0)
        return;

    QFileInfo theInfo(filename);
    m_openFileDir = theInfo.path();
    settings.setValue("DirectoryOfLastOpen", m_openFileDir);

    loadFile(filename);
}

void MainWindow::on_actionConnect_triggered()
{
    if (m_remoteDeploymentReceiver) {
        delete m_remoteDeploymentReceiver;
        m_remoteDeploymentReceiver = 0;
        delete m_connectionInfo;
        m_connectionInfo = 0;
        if (m_studio3D)
            m_studio3D->setVisible(true);

        updateUI();
        return;
    }

    m_remoteDeploymentReceiver = new RemoteDeploymentReceiver(this);
    if (!m_remoteDeploymentReceiver->startServer()) {
        QString msg = "Unable to connect to remote project.\n";
        QMessageBox::warning(this, "Connection Error", msg, QMessageBox::Close);
        delete m_remoteDeploymentReceiver;
        m_remoteDeploymentReceiver = 0;
        updateUI();
        return;
    }

    int port = m_remoteDeploymentReceiver->serverPort();
    QString message;
    QTextStream stream(&message);
    stream << "Connection Info\n"
           << "Address: " << m_remoteDeploymentReceiver->hostAddress().toString() << "\n"
           << "Port: " + QString::number(port);

    QQmlEngine *engine = ui->quickWidget->engine();
    QQuickItem *root = ui->quickWidget->rootObject();

    QByteArray qml = "import QtQuick 2.7\n"
                     "import QtQuick.Controls 2.2\n"
                     "Label {\n"
                     "    color: \"White\"\n"
                     "    horizontalAlignment: Text.AlignHCenter\n"
                     "    verticalAlignment: Text.AlignVCenter\n"
                     "    anchors.fill: parent\n"
                     "    font.pixelSize: 42\n"
                     "}";

    QQmlComponent component(engine);
    component.setData(qml, QUrl());

    if (component.isError()) {
        qCritical() << "error" << component.errors();
        return;
    }

    m_connectionInfo = qobject_cast<QQuickItem *>(component.create());
    m_connectionInfo->setProperty("text", message);

    QQmlEngine::setObjectOwnership(m_connectionInfo, QQmlEngine::CppOwnership);
    m_connectionInfo->setParentItem(root);
    m_connectionInfo->setParent(engine);

    connect(m_remoteDeploymentReceiver, &RemoteDeploymentReceiver::remoteConnected,
            this, &MainWindow::remoteConnected);

    connect(m_remoteDeploymentReceiver, &RemoteDeploymentReceiver::remoteDisconnected,
            this, &MainWindow::remoteDisconnected);

    connect(m_remoteDeploymentReceiver, &RemoteDeploymentReceiver::projectChanging,
            this, &MainWindow::remoteProjectChanging);

    connect(m_remoteDeploymentReceiver, &RemoteDeploymentReceiver::projectChanged,
            this, &MainWindow::loadRemoteDeploymentReceiver);

    updateUI();
}

void MainWindow::on_actionReload_triggered()
{
    if (Q3DSView *view = viewer()) {
        view->reset();
        updateUI();
    }
}

Q3DSView *MainWindow::viewer() const
{
    return m_studio3D;
}

void MainWindow::loadFile(const QString &filename)
{
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists())
        return;

    QUrl sourceUrl = QUrl::fromLocalFile(fileInfo.absoluteFilePath());

    if (m_studio3D) {
        viewer()->presentation()->setSource(sourceUrl);
        viewer()->reset();
        return;
    }
    delete m_errorInfo;
    m_errorInfo = nullptr;

    QQmlEngine *engine = ui->quickWidget->engine();
    QQuickItem *root = ui->quickWidget->rootObject();

    QByteArray qml = "import QtStudio3D 1.0\n"
                     "Studio3D {\n"
                     "  id: studio3D\n"
                     "  anchors.fill: parent\n"
                     "  focus: true\n"
                     "}";

    QQmlComponent component(engine);
    component.setData(qml, QUrl());

    if (component.isError()) {
        qDebug() << "error" << component.errors();
        return;
    }

    m_studio3D = static_cast<Q3DSView *>(component.create());
    connect(m_studio3D, &Q3DSView::errorChanged, this, &MainWindow::onErrorChanged,
            Qt::QueuedConnection);
    viewer()->presentation()->setSource(sourceUrl);

    QQmlEngine::setObjectOwnership(m_studio3D, QQmlEngine::CppOwnership);
    m_studio3D->setParentItem(root);
    m_studio3D->setParent(engine);

#ifdef Q_OS_ANDROID
    // We have custom mouse event handling in android
    viewer()->setIgnoreEvents(true, false, false);
#endif

    updateUI();
}

void MainWindow::on_actionCenter_triggered()
{
    if (Q3DSView *view = viewer()) {
        view->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeCenter);
        updateUI();
    }
}

void MainWindow::on_actionScale_To_Fit_triggered()
{
    if (Q3DSView *view = viewer()) {
        view->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeFit);
        updateUI();
    }
}

void MainWindow::on_actionScale_To_Fill_triggered()
{
    if (Q3DSView *view = viewer()) {
        view->viewerSettings()->setScaleMode(Q3DSViewerSettings::ScaleModeFill);
        updateUI();
    }
}

QString MainWindow::convertMimeDataToFilename(const QMimeData *mimeData)
{
    if (mimeData->hasUrls()) {
        for (const QUrl &url : mimeData->urls()) {
            QString str = url.toLocalFile();
            if (str.isEmpty() == false) {
                if ((QFileInfo(str).suffix() == "uip")
                        || (QFileInfo(str).suffix() == "uia")
                        || (QFileInfo(str).suffix() == "uiab"))
                {
                    return str;
                }
            }
        }
    }
    return QString();
}

void MainWindow::setupGeneratorUI()
{
    QQmlEngine *engine = ui->quickWidget->engine();
    QQuickItem *root = ui->quickWidget->rootObject();

    QByteArray qml = "import QtQuick 2.7\n"
                     "import QtQuick.Controls 2.2\n"
                     "Item {\n"
                     "    property alias mainText: mainLabel.text\n"
                     "    property alias detailsText: detailsLabel.text\n"
                     "    anchors.fill: parent\n"
                     "    Label {\n"
                     "        id: mainLabel\n"
                     "        color: \"White\"\n"
                     "        horizontalAlignment: Text.AlignHCenter\n"
                     "        verticalAlignment: Text.AlignVCenter\n"
                     "        anchors.top: parent.top\n"
                     "        anchors.left: parent.left\n"
                     "        anchors.right: parent.right\n"
                     "        height: parent.height / 2\n"
                     "        font.pixelSize: width / 40\n"
                     "        text: \"Image sequence generation initializing...\"\n"
                     "    }\n"
                     "    Label {\n"
                     "        id: detailsLabel\n"
                     "        color: \"White\"\n"
                     "        horizontalAlignment: Text.AlignHCenter\n"
                     "        verticalAlignment: Text.AlignTop\n"
                     "        anchors.top: mainLabel.bottom\n"
                     "        anchors.left: parent.left\n"
                     "        anchors.right: parent.right\n"
                     "        height: parent.height / 2\n"
                     "        font.pixelSize: width / 50\n"
                     "    }\n"
                     "}";

    QQmlComponent component(engine);
    component.setData(qml, QUrl());

    if (component.isError()) {
        qCritical() << "Error setting up generator UI:" << component.errors();
        return;
    }

    m_generatorInfo = qobject_cast<QQuickItem *>(component.create());

    QQmlEngine::setObjectOwnership(m_generatorInfo, QQmlEngine::CppOwnership);
    m_generatorInfo->setParentItem(root);
    m_generatorInfo->setParent(engine);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (convertMimeDataToFilename(event->mimeData()).isEmpty() == false)
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString fileName = convertMimeDataToFilename(event->mimeData());
    if (fileName.isEmpty() == false)
        loadFile(fileName);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!m_generatorMode) {
        QSettings settings(QCoreApplication::organizationName(),
                           QCoreApplication::applicationName());
        restoreGeometry(settings.value("mainWindowGeometry").toByteArray());
        restoreState(settings.value("mainWindowState").toByteArray());
    }
    updateUI();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_generatorMode) {
        QSettings settings(QCoreApplication::organizationName(),
                           QCoreApplication::applicationName());
        settings.setValue("mainWindowGeometry", saveGeometry());
        settings.setValue("mainWindowState", saveState());
    }
    QMainWindow::closeEvent(event);
}

#ifdef Q_OS_ANDROID
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_swipeStart = event->pos();

    Q3DSView *view = viewer();
    if (view)
        view->presentation()->mousePressEvent(event);

    event->accept();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q3DSView *view = viewer();
    if (view)
        view->presentation()->mouseReleaseEvent(event);

    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // Fake swipe down event, as SwipeGesture doesn't work (unless you use 3 fingers..)
    int swipeLength = height() / 10;
    if (ui->actionFull_Screen->isChecked() && event->pos().y() > m_swipeStart.y() + swipeLength) {
        ui->actionFull_Screen->setChecked(false);
        on_actionFull_Screen_triggered();
    }

    Q3DSView *view = viewer();
    if (view)
        view->presentation()->mouseMoveEvent(event);

    event->accept();
}
#endif

void MainWindow::updateUI()
{
    ui->actionConnect->setChecked(m_remoteDeploymentReceiver);

    bool displayConnection = m_remoteDeploymentReceiver
        && !m_remoteDeploymentReceiver->isProjectDeployed();

    if (m_connectionInfo)
        m_connectionInfo->setVisible(displayConnection);

    if (m_studio3D)
        m_studio3D->setVisible(!displayConnection);

    Q3DSView *view = viewer();
    if (!view)
        return;

    Q3DSViewerSettings::ScaleMode scaleMode = view->viewerSettings()->scaleMode();
    ui->actionCenter->setChecked(scaleMode == Q3DSViewerSettings::ScaleModeCenter);
    ui->actionScale_To_Fit->setChecked(scaleMode == Q3DSViewerSettings::ScaleModeFit);
    ui->actionScale_To_Fill->setChecked(scaleMode == Q3DSViewerSettings::ScaleModeFill);

    QColor matte = view->viewerSettings()->matteColor();
    ui->actionBorder->setChecked(matte.isValid() && matte != QColor(Qt::black));
    ui->actionShowOnScreenStats->setChecked(view->viewerSettings()->isShowRenderStats());
}

void MainWindow::loadRemoteDeploymentReceiver()
{
    Q_ASSERT(m_remoteDeploymentReceiver);
    const QString remote = m_remoteDeploymentReceiver->fileName();
    loadFile(remote);
    updateUI();
}

void MainWindow::remoteProjectChanging()
{
    updateUI();
}

void MainWindow::remoteConnected()
{
    m_connectionInfo->setProperty("text", "Remote Connected");
    updateUI();
}

void MainWindow::remoteDisconnected()
{
    m_connectionInfo->setProperty("text", "Remote Disconnected");
    updateUI();
}

void MainWindow::onErrorChanged(const QString &error)
{
    if (error.isEmpty()) {
        delete m_errorInfo;
        m_errorInfo = nullptr;
    } else {
        if (!m_errorInfo) {
            QQmlEngine *engine = ui->quickWidget->engine();
            QQuickItem *root = ui->quickWidget->rootObject();

            QByteArray qml = "import QtQuick 2.7\n"
                             "import QtQuick.Controls 2.2\n"
                             "Label {\n"
                             "    color: \"White\"\n"
                             "    horizontalAlignment: Text.AlignHCenter\n"
                             "    verticalAlignment: Text.AlignVCenter\n"
                             "    anchors.fill: parent\n"
                             "    font.pixelSize: width / 80\n"
                             "}";

            QQmlComponent component(engine);
            component.setData(qml, QUrl());

            if (component.isError()) {
                qCritical() << "Error setting up error UI:" << component.errors();
                return;
            }

            m_errorInfo = qobject_cast<QQuickItem *>(component.create());
            m_errorInfo->setParentItem(root);
            m_errorInfo->setParent(engine);
            QQmlEngine::setObjectOwnership(m_errorInfo, QQmlEngine::CppOwnership);
        }
        m_errorInfo->setProperty("text", m_studio3D->error());

        delete m_studio3D;
        m_studio3D = nullptr;
    }
}

void MainWindow::generatorProgress(int totalFrames, int frameCount)
{
    QString progressString;
    if (frameCount >= totalFrames) {
        progressString =
                QCoreApplication::translate(
                    "main", "Image sequence generation done! (%2 frames generated)")
        .arg(totalFrames);
    } else {
        progressString =
                QCoreApplication::translate("main", "Image sequence generation progress: %1 / %2")
        .arg(frameCount).arg(totalFrames);
    }
    m_generatorInfo->setProperty("mainText", progressString);
}

void MainWindow::generatorFinished(bool success, const QString &details)
{
    if (success) {
        m_generatorInfo->setProperty("detailsText", details);
    } else {
        QString mainString =
                QCoreApplication::translate("main", "Image sequence generation failed:");
        m_generatorInfo->setProperty("mainText", mainString);
        m_generatorInfo->setProperty("detailsText", details);
    }
}

void MainWindow::updateProgress(int percent)
{
    QString progress = QStringLiteral("Receiving (");
    progress.append(QString::number(percent));
    progress.append("%)");
    // Don't wait for 100%, as it'll already start loading and text isn't updated anymore
    if (percent >= 99)
        m_connectionInfo->setProperty("text", QStringLiteral("Loading"));
    else
        m_connectionInfo->setProperty("text", progress);
    updateUI();
}

void MainWindow::setGeneratorDetails(const QString &filename)
{
    m_generatorInfo->setProperty("detailsText", filename);
}
