/****************************************************************************
**
** Copyright (C) 1999-2001 NVIDIA Corporation.
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

//=============================================================================
// Prefix
//=============================================================================
#ifndef INCLUDED_DIALOGS_H
#define INCLUDED_DIALOGS_H 1

#pragma once

//=============================================================================
// Includes
//=============================================================================
#include "Qt3DSFile.h"
#include "StudioObjectTypes.h"
#include "Qt3DSMessageBox.h"
#include "Qt3DSFileTools.h"
#include <QtGui/qcolor.h>
#include "DocumentEditorEnumerations.h"

#include <QtWidgets/qmessagebox.h>

//=============================================================================
// Forwards
//=============================================================================
class IDoc;
class CStudioApp;
class CControl;
class CDialogControl;
class CProgressView;

class CDialogs : public QObject
{
    Q_OBJECT
public:
    enum ESavePromptResult {
        CANCEL_OPERATION,
        CONTINUE_NO_SAVE,
        SAVE_FIRST,
    };

    CDialogs(bool inShowGUI = true);
    virtual ~CDialogs();

    void DisplayAssetDeleteFailed();
    void DisplayRefreshResourceFailed(const QString &inResourceName, const QString &inDescription);
    QString ConfirmRefreshModelFile(const QString &inOriginalPath);
    QList<QUrl> SelectAssets(QString &outPath, Q3DStudio::DocumentEditorFileType::Enum assetType);

    QString defaultDirForUrl(const QUrl &url);

    static QStringList effectExtensions();
    static QStringList fontExtensions();
    static QStringList mapExtensions();
    static QStringList materialExtensions();
    static QStringList modelExtensions();
    static QStringList behaviorExtensions();
    static QStringList presentationExtensions();

    // This is not an appropriate place for these, but better
    // in an inappropriate place than duplicated
    static QString GetDAEFileExtension();
    static QString GetFbxFileExtension();
    // Null terminated list
    static QStringList GetImgFileExtensions();
    static QString GetImportFileExtension();
    static QString GetMeshFileExtension();
    static QString GetQmlFileExtension();
    static QString GetMaterialDataFileExtension();
    static QStringList GetFontFileExtensions();
    static QStringList GetEffectFileExtensions();
    static QStringList GetMaterialFileExtensions();
    static QStringList GetSoundFileExtensions();
    static bool IsImageFileExtension(const QString &inExt);
    static bool IsFontFileExtension(const QString &inExt);
    static bool IsEffectFileExtension(const QString &inExt);
    static bool IsMaterialFileExtension(const QString &inExt);
    static bool IsSoundFileExtension(const QString &inExt);
    static bool isPresentationFileExtension(const QString &inExt);
    static bool isMeshFileExtension(const QString &inExt);
    static bool isProjectFileExtension(const QString &inExt);
    static bool IsPathFileExtension(const QString &inExt);
    static bool IsPathBufferExtension(const QString &inExt);

    Qt3DSFile GetExportChoice(const QString &inExtension,
                              const QString &inDefaultName);

    QString GetSaveAsChoice(const QString &inDialogTitle = {}, bool isProject = false);
    QString GetNewDocumentChoice(const QString &inInitialDirectory = {}, bool isProject = true);
    QString GetFileOpenChoice(const QString &inInitialDirectory = {});

    void DisplayImportFailed(const QUrl &inURL, const QString &inDescription, bool inWarningsOnly);
    void DisplayLoadingPresentationFailed(const QFileInfo &loadFileInfo, const QString &errorText);
    void DisplaySavingPresentationFailed();
    void DisplaySaveReadOnlyFailed(const QString &inSavedLocation);
    void DisplayObjectRenamed(const QString &origName, const QString &newName, bool async = false);
    Qt3DSMessageBox::EMessageBoxReturn DisplayMessageBox(const QString &inTitle,
                                                         const QString &inText,
                                                         Qt3DSMessageBox::EMessageBoxIcon inIcon,
                                                         bool inShowCancel,
                                                         QWidget *parent = nullptr);
    int displayOverrideAssetBox(const QString &assetPath);
    int DisplayChoiceBox(const QString &inTitle, const QString &inText, int inIcon);
    void DisplayKnownErrorDialog(const QString &inErrorText);
    QColor displayColorDialog(const QColor &color) const;

    ESavePromptResult PromptForSave();
    bool PromptForKeyframeInterpolation(float &ioEaseIn, float &ioEaseOut);

    bool ConfirmRevert();

    void DisplayProgressScreen(const QString &inActionText,
                               const QString &inAdditionalText);
    void DestroyProgressScreen();

    void DisplayEnvironmentVariablesError(const Q3DStudio::CString &inErrorMessage);

    void ResetSettings(const QString &inCurrentDocPath = {});

    bool DisplayResetKeyframeValuesDlg();
    void DisplayPasteFailed();

    bool DisplayUndefinedDatainputDlg(
            const QMultiMap<QString,
                            QPair<qt3dsdm::Qt3DSDMInstanceHandle,
                                  qt3dsdm::Qt3DSDMPropertyHandle>> *map);

    static void DisplayGLVersionError(const Q3DStudio::CString &inGLVersion,
                                      const Q3DStudio::CString &inMinVersion);
    static void DisplayGLVersionWarning(const Q3DStudio::CString &inGLVersion,
                                        const Q3DStudio::CString &inRecommendedVersion);
    static void showWidgetBrowser(QWidget *screenWidget, QWidget *browser, const QPoint &point,
                                  QSize customSize = {});

Q_SIGNALS:
    void onColorChanged(const QColor &color);

protected:
    QString CreateAllowedTypesString(Q3DStudio::DocumentEditorFileType::Enum fileTypeFilter,
                                     QString &outInitialFilter, bool forImport, bool exclusive);
    static void DisplayGLVersionDialog(const Q3DStudio::CString &inGLVersion,
                                       const Q3DStudio::CString &inRecommendedVersion,
                                       bool inError);

    CProgressView *m_ProgressPalette;
    bool m_ShowGUI;

    QString m_LastSaveFile; ///< Path to the file was previously saved

    QHash<QString, QString> m_defaultDirForSuffixMap;
};
#endif // INCLUDED_DIALOGS_H
