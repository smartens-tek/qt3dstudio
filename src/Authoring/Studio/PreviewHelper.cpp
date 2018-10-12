/****************************************************************************
**
** Copyright (C) 2006 NVIDIA Corporation.
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

#include "Qt3DSCommonPrecompile.h"
#include "PreviewHelper.h"
#include "StudioApp.h"
#include "Dialogs.h"
#include "Dispatch.h"
#include "Doc.h"
#include "StudioPreferences.h"
#include "StudioProjectSettings.h"
#include "Core.h"
#include "Qt3DSFileTools.h"

#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtCore/qprocess.h>
#include <ProjectFile.h>

#include "remotedeploymentsender.h"

//=============================================================================
/**
 *	Callback for previewing a presentation.
 */
void CPreviewHelper::OnPreview(const QString &viewerExeName)
{
    Q3DStudio::CBuildConfigurations &theConfigurations =
            g_StudioApp.GetCore()->GetBuildConfigurations();
    Q3DStudio::CBuildConfiguration *theBuildConfiguration =
            theConfigurations.GetConfiguration(CStudioPreferences::GetPreviewConfig());
    if (theBuildConfiguration)
        PreviewViaConfig(theBuildConfiguration, EXECMODE_PREVIEW, viewerExeName);
}

//=============================================================================
/**
 *	Callback for deploying a presentation.
 */
void CPreviewHelper::OnDeploy(RemoteDeploymentSender &project)
{
    Q3DStudio::CBuildConfigurations &theConfigurations =
            g_StudioApp.GetCore()->GetBuildConfigurations();
    Q3DStudio::CBuildConfiguration *theBuildConfiguration =
            theConfigurations.GetConfiguration(CStudioPreferences::GetPreviewConfig());
    if (theBuildConfiguration) {
        // ItemDataPtr != nullptr ==> Build configurations specified NANT pipeline exporter
        PreviewViaConfig(theBuildConfiguration, EXECMODE_DEPLOY, QString(), &project);
    }
}

//=============================================================================
/**
 *	Previewing a presentation using the build configurations loaded.
 *	This involves 2 steps:
 *	1	Export the presentation using the specified exporter.
 *	2	Viewing the exported content following the command specified in the configuration.
 */
void CPreviewHelper::PreviewViaConfig(Q3DStudio::CBuildConfiguration *inSelectedConfig,
                                      EExecMode inMode, const QString &viewerExeName,
                                      RemoteDeploymentSender *project)
{
    CCore *theCore = g_StudioApp.GetCore();
    QString prvPath = theCore->getProjectFile().createPreview();
    try {
        DoPreviewViaConfig(inSelectedConfig, prvPath, inMode, viewerExeName, project);
    } catch (...) {
        theCore->GetDispatch()->FireOnProgressEnd();
        g_StudioApp.GetDialogs()->DisplaySaveReadOnlyFailed(prvPath);
    }
}

QString CPreviewHelper::getViewerFilePath(const QString &exeName)
{
    using namespace Q3DStudio;
    CFilePath currentPath(Qt3DSFile::GetApplicationDirectory());
    CFilePath viewerDir(QApplication::applicationDirPath());

    QString viewerFile;
#ifdef Q_OS_WIN
    if (!viewerDir.IsDirectory())
        viewerDir = currentPath.GetDirectory(); // Developing directory
    viewerFile = QStringLiteral("%1.exe").arg(exeName);

    QString viewer = viewerDir.filePath() + QStringLiteral("/") + viewerFile;
    if (!QFileInfo(viewer).exists()
            && exeName == QLatin1String("q3dsviewer")) {
        viewer = viewerDir.filePath() + QStringLiteral("/../src/Runtime/qt3d-runtime/bin/")
                + viewerFile;
    }
#else
#ifdef Q_OS_MACOS
    // Check if we're looking for Viewer 2.x that has a different development
    // time path for the executable
    QString viewerDevPath;
    if (exeName == QLatin1String("q3dsviewer"))
        viewerDevPath = QStringLiteral("../src/Runtime/qt3d-runtime/bin/");

    // Name of the executable file on macOS
    viewerFile = QStringLiteral("%1.app/Contents/MacOS/%1").arg(exeName);

    // Executable directory is three steps above the directory of studio executable
    QString executableDir = viewerDir.filePath() + QStringLiteral("/../../../");

    // Formulate the expected path to the viewer in development environment
    QString viewer = executableDir + viewerDevPath + viewerFile;

    // If not in development environment, expect viewer to be in same directory
    if (!QFileInfo(viewer).exists())
        viewer = executableDir + viewerFile;

#else
    if (!viewerDir.IsDirectory())
        viewerDir = currentPath.GetDirectory(); // Developing directory

    viewerFile = exeName;

    QString viewer = viewerDir.filePath() + QStringLiteral("/") + viewerFile;
    if (!QFileInfo(viewer).exists()
            && exeName == QLatin1String("q3dsviewer")) {
        viewer = viewerDir.filePath() + QStringLiteral("/../src/Runtime/qt3d-runtime/bin/")
                + viewerFile;
    }
#endif
#endif

    return viewer;
}

void CPreviewHelper::cleanupProcess(QProcess *p, QString *docPath)
{
    p->disconnect();
    if (docPath->endsWith(QLatin1String("_@preview@.uia"))) {
        QString uipPreviewPath = g_StudioApp.GetCore()->GetDoc()->GetDocumentPath()
                                 .replace(QLatin1String(".uip"), QLatin1String("_@preview@.uip"));
        QFile(uipPreviewPath).remove(); // remove uip preview (if exists)
        QFile(*docPath).remove(); // remove uia preview
    } else if (docPath->endsWith(QLatin1String("_@preview@.uip"))) {
        QFile(*docPath).remove();  // remove uip preview (if exists)
    }
    if (p->state() == QProcess::Running) {
        p->terminate();
        p->waitForFinished(5000); // To avoid warning about deleting a running process
    }
    p->deleteLater();
    delete docPath;
}

void CPreviewHelper::DoPreviewViaConfig(Q3DStudio::CBuildConfiguration * /*inSelectedConfig*/,
                                        const QString &inDocumentFile,
                                        EExecMode inMode, const QString &viewerExeName,
                                        RemoteDeploymentSender *project)
{
    using namespace Q3DStudio;

    if (inMode == EXECMODE_DEPLOY) {
        Q_ASSERT(project);
        project->streamProject(inDocumentFile);
    } else if (inMode == EXECMODE_PREVIEW
               && CStudioPreferences::GetPreviewProperty("PLATFORM") == "PC") {
        // Quick Preview on PC without going via NANT
        QString theCommandStr = getViewerFilePath(viewerExeName);
        QString *pDocStr = new QString(inDocumentFile);

        QProcess *p = new QProcess;
        QMetaObject::Connection *connection = new QMetaObject::Connection(
                    QObject::connect(qApp, &QApplication::aboutToQuit, [p, pDocStr](){
            // connection object is never destroyed, but it doesn't matter as application is
            // quitting anyway.
            cleanupProcess(p, pDocStr);
        }));
        auto finished
                = static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished);
        QObject::connect(p, finished, [p, pDocStr, connection](){
            // Disconnect the other connection to avoid duplicate cleanup
            QObject::disconnect(*connection);
            delete connection;
            cleanupProcess(p, pDocStr);
        });
        p->start(theCommandStr, { *pDocStr });

        if (!p->waitForStarted()) {
            QMessageBox::critical(nullptr, QObject::tr("Error Launching Viewer"),
                                  QObject::tr("'%1' failed with error: '%2'")
                                  .arg(theCommandStr).arg(p->errorString()));
            delete p;
            return;
        }
    }
}
