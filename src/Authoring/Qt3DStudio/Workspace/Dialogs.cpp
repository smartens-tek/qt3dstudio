/****************************************************************************
**
** Copyright (C) 2002 NVIDIA Corporation.
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

#include "qtAuthoring-config.h"
#include "Dialogs.h"
#include "StudioApp.h"
#include "Core.h"
#include "Doc.h"
#include "MainFrm.h"
#include "InterpolationDlg.h"
#include "Qt3DSMessageBox.h"
#include "ProgressView.h"
#include "TimeEditDlg.h"
#include "DurationEditDlg.h"
#include "StudioPreferences.h"
#include "ResetKeyframeValuesDlg.h"
#include "GLVersionDlg.h"
#include "Qt3DSMacros.h"
#include "ImportUtils.h"

#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qcheckbox.h>
#include <QtCore/qstandardpaths.h>
#include <QtWidgets/qdesktopwidget.h>
#include <QtWidgets/qpushbutton.h>
#include <QtGui/qscreen.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qdialogbuttonbox.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qstyle.h>

namespace {

inline Q3DStudio::CString CreateExtensionsList(const char **extList)
{
    Q3DStudio::CString retval;
    for (const char **ext = extList; *ext != nullptr; ++ext) {
        if (retval.Length())
            retval += " ";
        retval += Q3DStudio::CString("*.") + *ext;
    }
    return retval;
}

struct SAllowedTypesEntry
{
    Q3DStudio::DocumentEditorFileType::Enum m_FileType;
    QString m_ResourceString; // Model Files, Image Files, etc
    const char **m_FileExtensions;
};

const char *imgExts[] = {
    "png", "jpg", "jpeg", "dds", "bmp", "gif", "hdr", "ktx", "astc", nullptr,
};

const wchar_t *wideImgExts[] = {
    L"png", L"jpg", L"jpeg", L"dds", L"bmp", L"gif", L"hdr", L"ktx", L"astc", nullptr,
};

const char *modelExts[] = {
    CDialogs::GetDAEFileExtension(),
    #ifdef QT_3DSTUDIO_FBX
    CDialogs::GetFbxFileExtension(),
    #endif
    nullptr,
};

const char *meshExts[] = {
    CDialogs::GetMeshFileExtension(), nullptr,
};

const char *importExts[] = {
    CDialogs::GetImportFileExtension(), nullptr,
};

const char *behaviorExts[] = {
    CDialogs::GetQmlFileExtension(), nullptr,
};

const char *presentationExts[] = {
    "uip", nullptr,
};

const wchar_t *widePresentationExts[] = {
    L"uip", nullptr,
};

const char *qmlStreamExts[] = {
    "qml", nullptr,
};

const wchar_t *wideQmlStreamExts[] = {
    L"qml", nullptr,
};

const char *projectExts[] = {
    "uia", nullptr,
};

const wchar_t *wideProjectExts[] = {
    L"uia", nullptr,
};

const char *fontExts[] = {
    "ttf", "otf", nullptr,
};
const wchar_t *wideFontExts[] = {
    L"ttf", L"otf", nullptr,
};

const char *effectExts[] = {
    "effect", nullptr,
};

const wchar_t *wideEffectExts[] = {
    L"effect", nullptr,
};

const char *materialExts[] = {
    "material", "shader", "materialdef", nullptr,
};

const char *shaderExts[] = {
    "material", "shader", nullptr,
};

const wchar_t *wideMaterialExts[] = {
    L"material", L"shader", L"materialdef", nullptr,
};

const char *soundExts[] = {
    "wav", nullptr,
};

const wchar_t *wideSoundExts[] = {
    L"wav", nullptr,
};

// List of file types allowed during import
// Note: Despite its name, Q3DStudio::DocumentEditorFileType::DAE type includes
// all supported model types
SAllowedTypesEntry g_AllowedImportTypes[] = {
    { Q3DStudio::DocumentEditorFileType::Presentation, QObject::tr("Presentations"),
      presentationExts },
    { Q3DStudio::DocumentEditorFileType::QmlStream, QObject::tr("Qml streams"), qmlStreamExts },
    { Q3DStudio::DocumentEditorFileType::DAE, QObject::tr("Model Files"), modelExts },
    { Q3DStudio::DocumentEditorFileType::Image, QObject::tr("Image Files"), imgExts },
    { Q3DStudio::DocumentEditorFileType::Behavior, QObject::tr("Behavior Scripts"), behaviorExts },
    { Q3DStudio::DocumentEditorFileType::Effect, QObject::tr("Effect Files"), effectExts },
    { Q3DStudio::DocumentEditorFileType::Font, QObject::tr("Font Files"), fontExts },
    { Q3DStudio::DocumentEditorFileType::Material, QObject::tr("Material Files"), materialExts },
};
int g_NumAllowedImportTypes = sizeof(g_AllowedImportTypes) / sizeof(*g_AllowedImportTypes);

// List of file types allowed for file references
SAllowedTypesEntry g_AllowedFileReferencesTypes[] = {
    { Q3DStudio::DocumentEditorFileType::Image, QObject::tr("Image Files"), imgExts },
    { Q3DStudio::DocumentEditorFileType::Behavior, QObject::tr("Behavior Scripts"), behaviorExts },
    { Q3DStudio::DocumentEditorFileType::Mesh, QObject::tr("Mesh Files"), meshExts },
    { Q3DStudio::DocumentEditorFileType::Import, QObject::tr("Import Files"), importExts },
    { Q3DStudio::DocumentEditorFileType::Effect, QObject::tr("Effect Files"), effectExts },
};
int g_NumAllowedFileReferencesTypes =
        sizeof(g_AllowedFileReferencesTypes) / sizeof(*g_AllowedFileReferencesTypes);
}

/**
 * @param inShowGUI true if dialogs should be displayed or piped to std:cout instead
 */
CDialogs::CDialogs(bool inShowGUI /*= true*/)
    : m_ProgressPalette(nullptr)
    , m_ShowGUI(inShowGUI)
    , m_LastSaveFile(QStringLiteral("./"))
{
    const auto effectExt = effectExtensions();
    const auto fontExt = fontExtensions();
    const auto mapExt = mapExtensions();
    const auto materialExt = materialExtensions();
    const auto modelExt = modelExtensions();
    const auto behaviorExt = behaviorExtensions();
    const auto presentationExt = presentationExtensions();

    for (const auto ext : effectExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("effects"));
    for (const auto ext : fontExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("fonts"));
    for (const auto ext : mapExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("maps"));
    for (const auto ext : materialExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("materials"));
    for (const auto ext : modelExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("models"));
    for (const auto ext : behaviorExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("scripts"));
    for (const auto ext : presentationExt)
        m_defaultDirForSuffixMap.insert(ext, QStringLiteral("presentations"));
}

CDialogs::~CDialogs()
{
}

/**
 * Displays a dialog asking the user to choose the keyframe interpolation.
 *
 * @param ioEaseIn value to be set as the ease in default - passes back the value chosen by the user
 * @param ioEaseOut value to be set as the ease out default - passes back the value chosen by the
 * @return true if the user clicked OK on the dialog (indicating that the values should be updated
 * on the track)
 */
bool CDialogs::displayKeyframeInterpolation(float &ioEaseIn, float &ioEaseOut)
{
    CInterpolationDlg interpolationDialog(ioEaseIn, ioEaseOut);

    if (interpolationDialog.exec() == QDialog::Accepted) {
        // Retrieve the new interpolation values
        ioEaseIn = interpolationDialog.easeIn();
        ioEaseOut = interpolationDialog.easeOut();
        return true;
    }

    return false;
}

/**
 *	Notify the user that the deletion of an asset has failed.
 */
void CDialogs::DisplayAssetDeleteFailed()
{
    QString theMessage = QObject::tr("Studio was unable to save your project data. Please close "
                                     "down Studio and try again.");
    QString theTitle = QObject::tr("General Error");

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theTitle, theMessage, Qt3DSMessageBox::ICON_ERROR, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theMessage;
    }
}

// Get the export choice.
Qt3DSFile CDialogs::GetExportChoice(const Q3DStudio::CString &, const Q3DStudio::CString &)
{
    // Need to fix this for windows if we decide to use it
    return Qt3DSFile("", false, false);
}

/**
 *	Notify that we are unable to refresh the resource.
 */
void CDialogs::DisplayRefreshResourceFailed(const QString &inResourceName,
                                            const QString &inDescription)
{
    QString theTitle = QObject::tr("Refresh File Error");
    QString theText = QObject::tr("Studio was unable to refresh the resource '%1'.\n")
            .arg(inResourceName);

    if (!inDescription.isEmpty())
        theText += inDescription;

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theTitle, theText, Qt3DSMessageBox::ICON_WARNING, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theText;
    }
}

/**
 *	Notify the user that the loading of the requested resource failed.
 *
 *  @param inURL				the URL for the asset that was to have been imported
 *	@param inDescription        description for the failure, if any
 *	@param inWarningsOnly		not a failure, just warnings
 */
void CDialogs::DisplayImportFailed(const QUrl &inURL, const QString &inDescription,
                                   bool inWarningsOnly)
{
    // Notify the user we couldn't load the resource.
    QString theTitle;
    QString theText;
    QString theMsgText;

    theTitle = QObject::tr("Studio Import Resource ");
    theTitle += !inWarningsOnly ? QObject::tr("Error") : QObject::tr("Warning");

    // Determine the asset type
    EStudioObjectType theAssetType =
            Q3DStudio::ImportUtils::GetObjectFileTypeForFile(inURL.path(), false)
            .m_ObjectType;

    bool theIsStudioObject = theAssetType != OBJTYPE_UNKNOWN;

    // Is this a behavior file, but perhaps incorrectly formatted?
    if (theAssetType == OBJTYPE_BEHAVIOR) {
        // Load the message about the behavior format
        if (inWarningsOnly) {
            theText = QObject::tr("Warnings were detected during import of the behavior script."
                                  "\nPlease check the file.\n");
        } else {
            theText = QObject::tr("Studio was unable to import the behavior script.\nPlease check "
                                  "the file and try again.\nNote that behavior files must be "
                                  "syntactically correct to be importable.");
        }
        if (!inDescription.isEmpty())
            theText += QStringLiteral("\n") + inDescription;
    } else if (theAssetType != OBJTYPE_UNKNOWN || theIsStudioObject) {
        // Valid registered file type, but invalid file

        bool theNoDescription = inDescription.isEmpty();
        // Load default text stating that the import resource failed.
        // descriptions if present are presented as "reasons" for failure.
        if (!inWarningsOnly || theNoDescription) {
            theText = QObject::tr("Studio was unable to import the resource%1")
                    .arg(theNoDescription ? QStringLiteral(".\n")
                                          : QObject::tr(" due to the following reason(s):\n"));
        }
        if (!theNoDescription)
            theText += inDescription;
        else
            theText += QObject::tr("Please check the above file and try again.");
    } else {
        // Display the warning messsage if we have one
        // instead of a meaningless message. This provides more feed back
        if (!inDescription.isEmpty()) {
            theText += inDescription;
        } else {
            theText = QObject::tr("Studio was unable to import the resource file.\nThis "
                                  "resource file type is not currently supported.\n");
        }
    }

    theMsgText = !inWarningsOnly ? QObject::tr("Import resource failed:")
                                 : QObject::tr("Import resource succeeded with warning(s):");
    theMsgText += QStringLiteral("\n%1\n\n").arg(inURL.toDisplayString()) + theText;

    // Display the failed import resource message.
    if (m_ShowGUI && !CStudioPreferences::doNotShowImportWarnings()) {
        bool doNotShowAgain = false;
        Qt3DSMessageBox::Show(theTitle, theMsgText, Qt3DSMessageBox::ICON_WARNING, doNotShowAgain,
                              false, true, g_StudioApp.m_pMainWnd);
        CStudioPreferences::setDoNotShowImportWarnings(doNotShowAgain);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theMsgText;
    }
}

// Inform user that UIP file contained datainput bindings for datainput names not found
// from UIA file. Returns true if user wants to delete invalid datainput bindings
// automatically
bool CDialogs::DisplayUndefinedDatainputDlg(
        const QMultiMap<QString,
                        QPair<qt3dsdm::Qt3DSDMInstanceHandle,
                              qt3dsdm::Qt3DSDMPropertyHandle>> *map)
{
    const auto keys = map->uniqueKeys();
    QString theTitle = QObject::tr("Missing Data Input");
    QLabel *theText = new QLabel;
    int keysSize = keys.size();
    if (keysSize > 1) {
        theText->setText(QObject::tr("Could not find Data Inputs. "
                                     "%1 Data Inputs used as controllers are undefined.")
                         .arg(keysSize));
    } else {
        theText->setText(QObject::tr("Could not find Data Input. "
                                     "%1 Data Input used as controller is undefined.")
                         .arg(keysSize));
    }

    QString theSmallText;
    for (auto it : keys)
        theSmallText.append(QStringLiteral("\n") + it);

    theSmallText.append(QStringLiteral("\n"));

    QLabel *diList = new QLabel(theSmallText);

    QLabel *noUndoText = new QLabel;
    noUndoText->setText(QObject::tr("Clear cannot be undone.\n"));
    theText->setIndent(10);
    diList->setIndent(20);
    noUndoText->setIndent(10);


    QIcon warn(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
    QLabel *warnLab = new QLabel();
    const int size = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
    warnLab->setPixmap(warn.pixmap(QSize(size, size)));

    QDialog msgBox(g_StudioApp.m_pMainWnd, Qt::WindowCloseButtonHint | Qt::WindowTitleHint
                   | Qt::MSWindowsFixedSizeDialogHint);
    QGridLayout *layout = new QGridLayout();

    layout->addWidget(warnLab, 1, 1);
    layout->addWidget(theText, 1, 2);

    QPushButton *ok = new QPushButton(QObject::tr("Clear invalid controllers now"));
    QPushButton *cancel = new QPushButton(QObject::tr("I'll fix controllers manually"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(ok, QDialogButtonBox::AcceptRole);
    buttonBox->addButton(cancel, QDialogButtonBox::RejectRole);
    layout->addWidget(buttonBox, 4, 1, 1, 3, Qt::AlignCenter);
    layout->addWidget(diList, 2, 2, Qt::AlignVCenter);
    layout->addWidget(noUndoText, 3, 2);

    msgBox.setLayout(layout);

    msgBox.setWindowTitle(theTitle);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &msgBox, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &msgBox, &QDialog::reject);

    auto res = msgBox.exec();

    return res == QDialog::Accepted ? true : false;
}

QString CDialogs::ConfirmRefreshModelFile(const QString &inFile)
{
    QString prompt = QObject::tr("Refreshing an import is not undoable and resets the undo stack.\n"
                                 "Are you sure you want to refresh '%1'?")
                     .arg(QFileInfo(inFile).fileName());

    int choice = QMessageBox::warning(nullptr, QObject::tr("Refresh import"),
                                      prompt, QMessageBox::Yes | QMessageBox::Cancel);

    if (choice == QMessageBox::Cancel)
        return {};

    // this produces an extension string which contains all allowed formats specified in
    // g_AllowedImportTypes
    // currently DAE and FBX
    QString initialFilter;
    QString theFileFilter =
            CreateAllowedTypesString(Q3DStudio::DocumentEditorFileType::DAE, initialFilter,
                                     true, true);

    return QFileDialog::getOpenFileName(g_StudioApp.m_pMainWnd, QObject::tr("Open"),
                                        inFile, theFileFilter, nullptr);
}

QList<QUrl> CDialogs::SelectAssets(QString &outPath,
                                   Q3DStudio::DocumentEditorFileType::Enum assetType)
{
    QFileDialog fd(g_StudioApp.m_pMainWnd);
    fd.setDirectory(outPath);
    fd.setFileMode(QFileDialog::ExistingFiles);
    QString initialFilter;
    fd.setNameFilter(CreateAllowedTypesString(
                         assetType, initialFilter, true,
                         assetType != Q3DStudio::DocumentEditorFileType::Unknown));
    fd.selectNameFilter(initialFilter);
    fd.setWindowTitle(QObject::tr("Import Assets"));

    QList<QUrl> files;
    if (fd.exec()) {
        files = fd.selectedUrls();
        QString newOutPath = fd.directory().absolutePath();
        QString contentPath = QDir::fromNativeSeparators(
                    Qt3DSFile::GetApplicationDirectory() + QStringLiteral("/Content"));

        if (assetType != Q3DStudio::DocumentEditorFileType::Unknown
                || (assetType == Q3DStudio::DocumentEditorFileType::Unknown
                    && !newOutPath.startsWith(contentPath))) {
            // Return the new path if we are browsing a specific asset type, or we are browsing
            // outside the Content folder.
            outPath = newOutPath;
        }
    }

    return files;
}

QString CDialogs::defaultDirForUrl(const QUrl &url)
{
    QString defaultDir;
    if (!url.isLocalFile())
        return defaultDir;

    const QFileInfo fi(url.toLocalFile());
    const QString suffix = fi.suffix();

    defaultDir = m_defaultDirForSuffixMap.value(suffix.toLower());

    return defaultDir;
}

/**
 * Notify the user that the presentation we tried to load has failed.
 * @param loadFileInfo QFileInfo for the failing file
 * @param errrorText error message
 */
void CDialogs::DisplayLoadingPresentationFailed(const QFileInfo &loadFileInfo,
                                                const QString &loadFileName,
                                                const QString &errorText)
{
    QString theErrorMessage = loadFileInfo.isFile() ? loadFileInfo.fileName() : loadFileName;

    if (errorText.isEmpty())
        theErrorMessage +=  QObject::tr(" failed to load.");
    else
        theErrorMessage += errorText;

    QString theErrorTitle = QObject::tr("Open File Error");

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theErrorTitle, theErrorMessage, Qt3DSMessageBox::ICON_WARNING, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theErrorTitle << ": " << theErrorMessage;
    }
}

/**
 *	Notify the user that the presentation we tried to save has failed.
 *
 *	@param	inSavedLocation	The AKFile that we failed to save.
 */
void CDialogs::DisplaySavingPresentationFailed()
{
    QString theErrorMessage = QObject::tr("Unable to save presentation. Please ensure that the "
                                          "file is not set as read-only.");
    QString theErrorTitle = QObject::tr("Qt 3D Studio");

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theErrorTitle, theErrorMessage, Qt3DSMessageBox::ICON_WARNING, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theErrorTitle << ": " << theErrorMessage;
    }
}

/**
 *	Display a message box to indicate failure to overwrite a read-only file
 *
 *	@param	inSavedLocation
 *			the file location to be saved
 *
 *	@return	void
 */
void CDialogs::DisplaySaveReadOnlyFailed(const QString &inSavedLocation)
{
    QString theMsg = QObject::tr("Studio cannot save the file '%1'. The file is marked Read-Only."
                                 "\nSave the file with another file name or to a different "
                                 "location.").arg(inSavedLocation);
    QString theTitle = QObject::tr("Qt 3D Studio");

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theTitle, theMsg, Qt3DSMessageBox::ICON_WARNING, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theMsg;
    }
}

void CDialogs::DisplayObjectRenamed(const QString &origName, const QString &newName, bool async)
{
    QString title = QObject::tr("Warning");
    QString message = QObject::tr("Object %1 was renamed to %2 because "
                                  "original name was duplicated "
                                  "under its parent.").arg(origName).arg(newName);
    if (async) {
        QTimer::singleShot(0, [this, title, message]() {
            DisplayMessageBox(title, message, Qt3DSMessageBox::ICON_WARNING, false);
        });
    } else {
        DisplayMessageBox(title, message, Qt3DSMessageBox::ICON_WARNING, false);
    }
}

/**
 *	Displays a Qt3DSMessageBox using the specified parameters.  The message box
 *	is modal to the main frame.  This provides an easy way to place modal dialogs
 * 	to the user, without requiring your class to know about the main frame or
 *	window refs.
 *	@param inTitle Title of the message box (not used on Mac)
 *	@param inText Text of the message
 *	@param inIcon Icon to be displayed next to the text
 *	@param inShowCancel true to show a Cancel button, false only show an OK button
 *	@return Indication of which button was pressed to dismiss the dialog
 */
Qt3DSMessageBox::EMessageBoxReturn
CDialogs::DisplayMessageBox(const QString &inTitle, const QString &inText,
                            Qt3DSMessageBox::EMessageBoxIcon inIcon, bool inShowCancel,
                            QWidget *parent)
{
    Qt3DSMessageBox::EMessageBoxReturn theUserChoice;

    if (m_ShowGUI) {
        if (parent == nullptr)
            parent = g_StudioApp.m_pMainWnd;
        theUserChoice =
                Qt3DSMessageBox::Show(inTitle, inText, inIcon,
                                      inShowCancel, parent);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << inTitle << ": " << inText;
        theUserChoice = Qt3DSMessageBox::MSGBX_OK;
    }

    return theUserChoice;
}

void CDialogs::asyncDisplayMessageBox(const QString &title, const QString &text,
                                      Qt3DSMessageBox::EMessageBoxIcon icon, QWidget *parent)
{
    if (m_ShowGUI) {
        if (parent == nullptr)
            parent = g_StudioApp.m_pMainWnd;
        QTimer::singleShot(0, [title, text, icon, parent]() {
            Qt3DSMessageBox::Show(title, text, icon, false, parent);
        });
    } else {
        qCDebug(qt3ds::TRACE_INFO) << title << ": " << text;
    }
}

int CDialogs::DisplayChoiceBox(const QString &inTitle, const QString &inText, int inIcon)
{
    if (m_ShowGUI) {
        QMessageBox box;
        box.setWindowTitle(inTitle);
        box.setText(inText);
        switch (inIcon) {
        case Qt3DSMessageBox::ICON_WARNING:
            box.setIcon(QMessageBox::Warning);
            break;
        case Qt3DSMessageBox::ICON_ERROR:
            box.setIcon(QMessageBox::Critical);
            break;
        case Qt3DSMessageBox::ICON_INFO:
            box.setIcon(QMessageBox::Information);
            break;
        default:
            break;
        }
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        switch (box.exec()) {
        case QMessageBox::Yes:
            return IDYES;
        case QMessageBox::No:
            return IDNO;
        default:
            Q_UNREACHABLE();
        }
    } else {
        qCDebug(qt3ds::TRACE_INFO) << inTitle << ": " << inText;
        return IDYES;
    }
}

/**
 * Display a box to choose whether to override or skip an existing asset during import
 *
 * @return  user choice (Yes, No, YesToAll, NoToAll)
 */
int CDialogs::displayOverrideAssetBox(const QString &assetPath)
{
    if (m_ShowGUI) {
        QMessageBox box;
        box.setWindowTitle(QObject::tr("Asset Exists"));
        box.setText(QObject::tr("The following asset already exists, do you want to override it?"));
        box.setInformativeText(assetPath);
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setButtonText(QMessageBox::Yes, QObject::tr("Override"));
        box.setButtonText(QMessageBox::No, QObject::tr("Skip"));
        box.setDefaultButton(QMessageBox::No);
        box.setCheckBox(new QCheckBox(QObject::tr("Do this action for all and don't ask again."),
                                      &box));
        int choice = box.exec();
        if (box.checkBox()->isChecked())
            choice = choice == QMessageBox::Yes ? QMessageBox::YesToAll : QMessageBox::NoToAll;
        return choice;
    } else {
        qCDebug(qt3ds::TRACE_INFO) << "Asset Override: " << assetPath;
        return QMessageBox::No;
    }
}

const char *CDialogs::GetDAEFileExtension()
{
    return "dae";
}

const char *CDialogs::GetFbxFileExtension()
{
    return "fbx";
}

// Null terminated list
const char **CDialogs::GetImgFileExtensions()
{
    return imgExts;
}

const char *CDialogs::GetImportFileExtension()
{
    return "import";
}

const char *CDialogs::GetMeshFileExtension()
{
    return "mesh";
}

const char *CDialogs::GetQmlFileExtension()
{
    return "qml";
}

const char *CDialogs::GetMaterialDataFileExtension()
{
    return "materialdef";
}

const char **CDialogs::GetFontFileExtensions()
{
    return fontExts;
}

const char **CDialogs::GetEffectFileExtensions()
{
    return effectExts;
}

const char **CDialogs::GetMaterialFileExtensions()
{
    return materialExts;
}
const char **CDialogs::GetSoundFileExtensions()
{
    return soundExts;
}

bool IsFileExtension(const char *inExt, const char **inExts)
{
    if (inExt == nullptr)
        return false;
    for (const char **ext = inExts; *ext != nullptr; ++ext) {
        if (QString::compare(inExt, *ext, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

bool CDialogs::IsImageFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, imgExts);
}

bool CDialogs::IsFontFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, fontExts);
}

bool CDialogs::IsEffectFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, effectExts);
}

bool CDialogs::IsMaterialFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, materialExts);
}

bool CDialogs::IsSoundFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, soundExts);
}

bool CDialogs::isMeshFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, meshExts);
}

bool CDialogs::isImportFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, importExts);
}

bool CDialogs::isPresentationFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, presentationExts);
}

bool CDialogs::isProjectFileExtension(const char *inExt)
{
    return IsFileExtension(inExt, projectExts);
}

const wchar_t **CDialogs::GetWideImgFileExtensions()
{
    return wideImgExts;
}

const wchar_t *CDialogs::GetWideDAEFileExtension()
{
    return L"dae";
}

const wchar_t *CDialogs::GetWideFbxFileExtension()
{
    return L"fbx";
}

const wchar_t *CDialogs::GetWideImportFileExtension()
{
    return L"import";
}

const wchar_t *CDialogs::GetWideMeshFileExtension()
{
    return L"mesh";
}

const wchar_t **CDialogs::GetWideFontFileExtensions()
{
    return wideFontExts;
}

const wchar_t **CDialogs::GetWideEffectFileExtensions()
{
    return wideEffectExts;
}

const wchar_t **CDialogs::GetWideMaterialFileExtensions()
{
    return wideMaterialExts;
}

bool IsFileExtension(const wchar_t *inExt, const wchar_t **inExts)
{
    if (inExt == nullptr)
        return false;
    for (const wchar_t **ext = inExts; *ext != nullptr; ++ext) {
        if (QString::compare(QString::fromWCharArray(inExt),
                             QString::fromWCharArray(*ext), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool CDialogs::IsImageFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideImgExts);
}

bool CDialogs::IsFontFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideFontExts);
}

bool CDialogs::IsEffectFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideEffectExts);
}

bool CDialogs::IsMaterialFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideMaterialExts);
}

bool CDialogs::IsPathFileExtension(const wchar_t *inExt)
{
    return QString::compare(QString::fromWCharArray(inExt), "svg", Qt::CaseInsensitive) == 0;
}

bool CDialogs::IsPathBufferExtension(const wchar_t *inExt)
{
    return QString::compare(QString::fromWCharArray(inExt), "path", Qt::CaseInsensitive) == 0;
}

bool CDialogs::IsSoundFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideSoundExts);
}

bool CDialogs::isPresentationFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, widePresentationExts);
}

bool CDialogs::isProjectFileExtension(const wchar_t *inExt)
{
    return IsFileExtension(inExt, wideProjectExts);
}

/**
 *  CreateAllowedTypesString: Creates the string used to determine allowable types
 *  for import or for filereferences
 *  @return the string that dynamically created with the extensions supported.
 */
QString CDialogs::CreateAllowedTypesString(Q3DStudio::DocumentEditorFileType::Enum fileTypeFilter,
                                           QString &outInitialFilter, bool forImport,
                                           bool exclusive)
{
    QString theReturnString;
    QString combinedFilter;
    int theCount = forImport ? g_NumAllowedImportTypes : g_NumAllowedFileReferencesTypes;
    for (int idx = 0; idx < theCount; ++idx) {
        const SAllowedTypesEntry &entry =
                forImport ? g_AllowedImportTypes[idx] : g_AllowedFileReferencesTypes[idx];
        if (!exclusive || fileTypeFilter == entry.m_FileType) {
            QString theTypeString(entry.m_ResourceString);
            QString theExtensions(CreateExtensionsList(entry.m_FileExtensions).toQString());
            const QString filterString = theTypeString + " (" + theExtensions + ");;";
            theReturnString += filterString;
            if (exclusive)
                outInitialFilter = filterString;
            else
                combinedFilter += theExtensions + " ";
        }
    }
    if (!combinedFilter.isEmpty()) {
        combinedFilter.chop(1); // Remove last separator
        theReturnString.prepend(QObject::tr("All Supported Asset types")
                                + " (" + combinedFilter + ");;");
        outInitialFilter = QObject::tr("All Supported Asset types")
                + " (" + combinedFilter + ")";
    }
    theReturnString.chop(2);
    if (exclusive)
        outInitialFilter.chop(2);

    return theReturnString;
}

/**
 *	Display a error dialog box with the given text string that describes the error.
 */
void CDialogs::DisplayKnownErrorDialog(const QString &inErrorText)
{
    // make sure this is valid
    if (!inErrorText.isEmpty()) {
        QString theTitle = QObject::tr("Qt 3D Studio");
        if (m_ShowGUI) {
            Qt3DSMessageBox::Show(theTitle, inErrorText, Qt3DSMessageBox::ICON_ERROR,
                                  false, g_StudioApp.m_pMainWnd);
        } else {
            qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << inErrorText;
        }
    }
}

/**
 *	Prompt the user to save the document before losing their changes.
 *	This is used when closing, loading or newing up a document when the current
 *	one has modifications.
 *	@return	the user's choice.
 */
CDialogs::ESavePromptResult CDialogs::PromptForSave()
{
    QString theDocTitle;

    Qt3DSFile theCurrentDoc = g_StudioApp.GetCore()->GetDoc()->GetDocumentPath();
    if (theCurrentDoc.IsFile())
        theDocTitle = theCurrentDoc.GetName().toQString();
    else // if the current doc has not been saved then use the default title.
        theDocTitle = QObject::tr("Untitled");

    QString thePrompt = QObject::tr("Save changes to %1?").arg(theDocTitle);

    int theChoice = QMessageBox::warning(nullptr, QObject::tr("Qt 3D Studio"),
                                         thePrompt, QMessageBox::Yes | QMessageBox::No
                                         | QMessageBox::Cancel);

    ESavePromptResult theResult = CANCEL_OPERATION;

    switch (theChoice) {
    case QMessageBox::Yes:
        theResult = SAVE_FIRST;
        break;
    case QMessageBox::No:
        theResult = CONTINUE_NO_SAVE;
        break;
    case QMessageBox::Cancel:
        theResult = CANCEL_OPERATION;
        break;
    default:
        break;
    }

    return theResult;
}

/**
 * Prompt the user for a file to save to.
 * If browsing for location for presentation (isProject == false), then
 * allow browsing only inside current project.
 * If browsing for a location for a project, then disallow browsing inside current project.
 * If isCopy is true, then create a copy of the existing project rather than a new one.
 * isCopy is ignored if isProject is false;
 * Return an invalid file if the user cancels the save dialog.
 */
QString CDialogs::GetSaveAsChoice(const QString &inDialogTitle, bool isProject, bool isCopy)
{
    QString projPath(QDir::cleanPath(g_StudioApp.GetCore()->getProjectFile().getProjectPath()));
    QString previousFolder;
    QString theFilename = g_StudioApp.GetCore()->GetDoc()->GetDocumentPath();

    if (theFilename.isEmpty() || isProject)
        theFilename = QObject::tr("Untitled");

    QString theFileExt = QStringLiteral(".uip");

    QFileDialog theFileDlg;
    theFileDlg.setOption(QFileDialog::DontConfirmOverwrite);

    const QFileInfo fi(m_LastSaveFile);
    // TODO: introduce a workspace concept?
    theFileDlg.setDirectory(
                fi.path() == QLatin1String(".")
                ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                : fi.path());
    theFileDlg.setAcceptMode(QFileDialog::AcceptSave);
    theFileDlg.setDefaultSuffix(theFileExt);
    if (!inDialogTitle.isEmpty())
        theFileDlg.setWindowTitle(inDialogTitle);

    if (isProject) {
        theFileDlg.setLabelText(QFileDialog::FileName, QObject::tr("Project Name"));
        if (!isCopy)
            theFileDlg.setLabelText(QFileDialog::Accept, QObject::tr("Create"));
        // Limit browsing to outside project directory
        if (!projPath.isEmpty()) {
            QDir projDir(projPath);
            projDir.cdUp();
            previousFolder = projDir.absolutePath();
            if (isCopy)
                theFileDlg.setDirectory(previousFolder);
            connect(&theFileDlg, &QFileDialog::directoryEntered,
                    [&](const QString &dir) {
                QFileInfo fi(QDir::cleanPath(dir));
                const QString absPath = fi.absoluteFilePath();
                if (absPath.startsWith(projPath))
                    theFileDlg.setDirectory(previousFolder);
                else
                    previousFolder = absPath;
            });
        }
    } else {
        // Limit browsing to project directory
        connect(&theFileDlg, &QFileDialog::directoryEntered,
                [&theFileDlg, &projPath](const QString &dir) {
            QFileInfo fi(QDir::cleanPath(dir));
            const QString absPath = fi.absoluteFilePath();
            if (!absPath.startsWith(projPath))
                theFileDlg.setDirectory(projPath);
        });
        // Note that since we are using native file dialog, we cannot change the sidebar or
        // "look in" combo contents of the file dialog, which may cause bit of confusion.
    }

    bool theShowDialog = true;
    QString theFile;
    while (theShowDialog && theFileDlg.exec()) {
        theShowDialog = false;
        QString selectedName = theFileDlg.selectedFiles().front();

        // Make sure file name has correct extension
        if (isProject && isCopy && selectedName.endsWith(theFileExt))
            selectedName.chop(theFileExt.length());
        else if (!selectedName.endsWith(theFileExt))
            selectedName.append(theFileExt);

        if (!isProject) {
            // If user somehow manages to select a file path outside project directory, save to
            // default presentations directory
            QFileInfo fi(QDir::cleanPath(selectedName));
            const QString absPath = fi.absoluteFilePath();
            if (!absPath.startsWith(projPath))
                selectedName = projPath + QStringLiteral("/presentations/") + fi.fileName();
        }

        theFile = selectedName;
        m_LastSaveFile = selectedName;
        // New directory is only created when creating a new project.
        if (isProject) {
            Q3DStudio::CFilePath theFinalDir;
            Q3DStudio::CFilePath theFinalDoc;
            g_StudioApp.GetCore()->GetCreateDirectoryFileName(selectedName,
                                                              theFinalDir, theFinalDoc);

            // Update last save file to final doc
            m_LastSaveFile = theFinalDoc.absoluteFilePath();

            // File dialog shouldn't allow choosing existing folder for project, but since we
            // are using native dialog, let's play it safe and check.
            if (theFinalDir.Exists()) {
                const QString title(QObject::tr("Existing directory"));
                const QString warning = QObject::tr("%1 already exists.")
                        .arg(theFinalDir.GetFileName().toQString());
                QMessageBox::warning(nullptr, title, warning);
                // Reset the file and show the file dialog again
                theFile.clear();
                theShowDialog = true;
                continue;
            }
        }
    }

    return theFile;
}

QString CDialogs::getImportVariantsDlg()
{
    QString docDir = QFileInfo(g_StudioApp.GetCore()->GetDoc()->GetDocumentPath()).absolutePath();

    QFileDialog dlg;
    dlg.setDirectory(docDir);
    dlg.setWindowTitle(tr("Import variants"));
    dlg.setDefaultSuffix(QStringLiteral(".variants"));
    dlg.setNameFilters({tr("All supported files (*.variants *.uia)"),
                        tr("Variants files (*.variants)"), tr("Project files (*.uia)")});
    auto result = dlg.exec();

    if (result == QDialog::Accepted && !dlg.selectedFiles().empty())
        return dlg.selectedFiles().front();

    return {};
}

QString CDialogs::getExportVariantsDlg()
{
    QString docDir = QFileInfo(g_StudioApp.GetCore()->GetDoc()->GetDocumentPath()).absolutePath();

    QFileDialog dlg;
    dlg.setDirectory(docDir);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setWindowTitle(tr("Export variants"));
    dlg.setDefaultSuffix(QStringLiteral(".variants"));
    dlg.setNameFilters({QObject::tr("Variants files (*.variants)")});
    dlg.exec();

    if (!dlg.selectedFiles().empty())
        return dlg.selectedFiles().front();

    return {};
}

/**
 * Prompt the user for a file to create.
 * @param  isProject   true: new project, false: new presentation
 * @return an invalid file if the user cancels the save dialog.
 */
QString CDialogs::GetNewDocumentChoice(const QString &inInitialDirectory, bool isProject)
{
    if (inInitialDirectory.size())
        m_LastSaveFile = inInitialDirectory + QStringLiteral("/");
    QString title = isProject ? QObject::tr("Create New Project")
                              : QObject::tr("Create New Presentation");
    return GetSaveAsChoice(title, isProject);
}

/**
 * Prompt the user for a file to open.
 * This will return an invalid file if the user cancels the save dialog.
 */
QString CDialogs::GetFileOpenChoice(const QString &inInitialDirectory)
{
    QFileInfo theFile;
    QString theImportFilter = QObject::tr("Studio UI Presentation (*.uip *.uia)");

    QFileDialog theFileDlg(g_StudioApp.m_pMainWnd, QString(),
                           (inInitialDirectory == QLatin1String("."))
                           ? QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                           : inInitialDirectory,
                           theImportFilter);
    theFileDlg.setAcceptMode(QFileDialog::AcceptOpen);

    if (theFileDlg.exec() == QDialog::Accepted) {
        theFile.setFile(theFileDlg.selectedFiles().first());
        m_LastSaveFile = theFile.absoluteFilePath();
    }

    return theFile.absoluteFilePath();
}

/**
 *	Prompt the user to make sure they want to revert the current project.
 *	@return true if they do want to continue with the revert.
 */
bool CDialogs::ConfirmRevert()
{
    bool theConfirmation = false;
    QString thePrompt = QObject::tr("All changes that have been made to your project since your "
                                    "last save will be lost.\n\nDo you want to continue?");
    QString theTitle = QObject::tr("Qt 3D Studio");

    Qt3DSMessageBox::EMessageBoxReturn theChoice;

    if (m_ShowGUI) {
        theChoice = Qt3DSMessageBox::Show(theTitle, thePrompt, Qt3DSMessageBox::ICON_WARNING, true,
                                          g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << thePrompt;
        theChoice = Qt3DSMessageBox::MSGBX_OK;
    }

    // user decided to go ahead and delete all unused resources
    if (theChoice == Qt3DSMessageBox::MSGBX_OK)
        theConfirmation = true;

    return theConfirmation;
}

/**
 * Displays a progress screen, if there is not one aleady being shown.  The
 * progress screen doesn't get dismissed until you call
 * CDialogs::DestroyProgressScreen().
 * @param inActionText text to be displayed as the action
 * @param inAdditionalText additional text, for example a file name
 */
void CDialogs::DisplayProgressScreen(const QString &inActionText,
                                     const QString &inAdditionalText)
{
    if (m_ShowGUI && !m_ProgressPalette) {
        m_ProgressPalette = new CProgressView(g_StudioApp.m_pMainWnd);
        m_ProgressPalette->SetActionText(inActionText);
        m_ProgressPalette->SetAdditionalText(inAdditionalText);
        m_ProgressPalette->show();
        qApp->processEvents();
    }
}

/**
 * If a loading screen is currently being shown, this function destroys it.  You
 * can show the loading screen again with another call to
 * CDialogs::DisplayLoadingScreen().
 */
void CDialogs::DestroyProgressScreen()
{
    if (m_ShowGUI && m_ProgressPalette) {
        delete m_ProgressPalette;
        m_ProgressPalette = nullptr;
    }
}

/**
 *	Inform the user that the environment variables entered does not match the format
 *	expected, listing down all those settings that are wrong.
 *	@param	inErrorMessage the listing of all those errors.
 */
void CDialogs::DisplayEnvironmentVariablesError(const Q3DStudio::CString &inErrorMessage)
{
    QString theTitle = QObject::tr("Unable to accept all Environment Variables");
    QString theMessage = QObject::tr("The following variables will not be saved:\n")
            + inErrorMessage.toQString()
            + QObject::tr("\nVariables must be listed in the following format:"
                          "\n{Variable} = Value\n");
    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theTitle, theMessage, Qt3DSMessageBox::ICON_ERROR, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theMessage;
    }
}

/**
 *	Reset settings.
 *	Typically inCurrentDocPath is only set when Studio is first launched.
 *	@param inCurrentDocPath	the current document path, if any. Application directory if
 *there is none.
 */
void CDialogs::ResetSettings(const QString &inCurrentDocPath)
{
    // Initialize the default dir/paths to the current document path if specified, otherwise leave
    // everything as it is.
    if (!inCurrentDocPath.isEmpty())
        m_LastSaveFile = inCurrentDocPath;
}

bool CDialogs::DisplayResetKeyframeValuesDlg()
{
    CResetKeyframeValuesDlg theDialog;
    return theDialog.exec() == QDialog::Accepted;
}

/**
 *	User trying to do a pathological paste, such as pasting a component copied from a different
 *instance
 *	of Studio into an instance of the same component that already exists in the current instance
 *of Studio, and
 *	is potentially replaced and deleted.
 */
void CDialogs::DisplayPasteFailed()
{
    QString theTitle = QObject::tr("Paste Error");
    QString theMessage = QObject::tr("Sorry, the attempted paste operation cannot be completed,"
                                     " the destination is invalid.");

    if (m_ShowGUI) {
        Qt3DSMessageBox::Show(theTitle, theMessage, Qt3DSMessageBox::ICON_ERROR, false,
                              g_StudioApp.m_pMainWnd);
    } else {
        qCDebug(qt3ds::TRACE_INFO) << theTitle << ": " << theMessage;
    }
}

/**
 *	Video card OpenGL version is too low to be supported.
 */
void CDialogs::DisplayGLVersionError(const Q3DStudio::CString &inGLVersion,
                                     const Q3DStudio::CString &inMinVersion)
{
    DisplayGLVersionDialog(inGLVersion, inMinVersion, true);
}

/**
 *	Video card OpenGL version is outdated, but could be usable.
 */
void CDialogs::DisplayGLVersionWarning(const Q3DStudio::CString &inGLVersion,
                                       const Q3DStudio::CString &inRecommendedVersion)
{
    DisplayGLVersionDialog(inGLVersion, inRecommendedVersion, false);
}

void CDialogs::asyncDisplayTimeEditDialog(long time, IDoc *doc, long objectAssociation,
                                          KeyframeManager *keyframesManager) const
{
    QTimer::singleShot(0, [time, doc, objectAssociation, keyframesManager]() {
        CTimeEditDlg timeEditDlg(keyframesManager);
        timeEditDlg.showDialog(time, doc, objectAssociation);
    });
}

void CDialogs::asyncDisplayDurationEditDialog(long startTime, long endTime,
                                              ITimeChangeCallback *callback) const
{
    QTimer::singleShot(0, [startTime, endTime, callback]() {
        CDurationEditDlg durationEditDlg;
        durationEditDlg.showDialog(startTime, endTime, callback);
    });
}

void CDialogs::showWidgetBrowser(QWidget *screenWidget, QWidget *browser, const QPoint &point,
                                 WidgetBrowserAlign align, QSize customSize)
{
    QSize popupSize = customSize.isEmpty() ? CStudioPreferences::browserPopupSize() : customSize;
    browser->resize(popupSize);
    QPoint newPos = point;

    // Make sure the popup doesn't go outside the screen
    int screenNum = QApplication::desktop()->screenNumber(screenWidget);
    QScreen *screen = nullptr;

    // If we are somehow not on any screen, just show the browser at upper left corner of the
    // primary screen.
    if (screenNum < 0) {
        screen = QGuiApplication::primaryScreen();
        newPos = QPoint(25, 25) + QPoint(popupSize.width(), popupSize.height());
    } else {
        screen = QGuiApplication::screens().at(screenNum);
    }
    QRect screenRect = screen->availableGeometry();

    const int controlH = CStudioPreferences::controlBaseHeight();
    if (align == WidgetBrowserAlign::ComboBox) {
        // position the popup below the combobox
        newPos -= QPoint(popupSize.width(), -controlH) + screenRect.topLeft();
        // if no space below the combobox, move it above it
        if (newPos.y() + popupSize.height() > screenRect.height())
            newPos.setY(newPos.y() - popupSize.height() - controlH);
    } else if (align == WidgetBrowserAlign::ToolButton) {
        // The point is assumed to be the lower right corner of the button
        newPos -= QPoint(popupSize.width(), popupSize.height()) + screenRect.topLeft();
        if (newPos.y() < 0)
            newPos.setY(newPos.y() + popupSize.height() - controlH);
    } else { // WidgetBrowserAlign::Center
        newPos -= QPoint(popupSize.width() / 2, popupSize.height() / 2) + screenRect.topLeft();
    }

    if (newPos.y() < 0)
        newPos.setY(0);

    if (newPos.x() + popupSize.width() > screenRect.width())
        newPos.setX(screenRect.width() - popupSize.width());
    else if (newPos.x() < 0)
        newPos.setX(0);

    newPos += screenRect.topLeft();

    browser->move(newPos);

    // Show asynchronously to avoid flashing blank window on first show
    QTimer::singleShot(0, screenWidget, [browser, popupSize, screenWidget] {
        browser->show();
        browser->activateWindow();
        browser->setFocus();
        // Make sure we are the desired size after show, as showing on a different screen
        // can cause incorrectly sized popup.
        QTimer::singleShot(0, screenWidget, [browser, popupSize] {
            browser->resize(popupSize);
        });
    });
}

/**
 *	Display the error dialog or warning dialog that OpenGL version is lower than what is
 *expected
 */
void CDialogs::DisplayGLVersionDialog(const Q3DStudio::CString &inGLVersion,
                                      const Q3DStudio::CString &inRecommendedVersion, bool inError)
{
    QString theTitle;
    QString theMessage;

    if (inError) {
        theTitle = QObject::tr("Error");
        theMessage = QObject::tr("OpenGL version %1 is unsupported.\nPlease use a video card and "
                                 "driver that supports at least OpenGL %2 or higher.").arg(
                    inGLVersion.toQString()).arg(inRecommendedVersion.toQString());
    } else {
        theTitle = QObject::tr("Warning");
        theMessage = QObject::tr("OpenGL version %1 detected.\nA video card with an updated driver "
                                 "capable of OpenGL %2 is recommended or there may be rendering "
                                 "errors.").arg(inGLVersion.toQString()).arg(
                    inRecommendedVersion.toQString());
    }

    CGLVersionDlg theGLVersionDlg;
    theGLVersionDlg.Initialize(theTitle, theMessage, inError);
    theGLVersionDlg.exec();

    if (theGLVersionDlg.GetDontShowAgain())
        CStudioPreferences::setDontShowGLVersionDialog(true);
}

QStringList CDialogs::effectExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : effectExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::fontExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : fontExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::mapExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : imgExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::materialExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : materialExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::shaderExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : shaderExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::modelExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : modelExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::behaviorExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : behaviorExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::presentationExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : presentationExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QStringList CDialogs::qmlStreamExtensions()
{
    static QStringList exts;
    if (exts.isEmpty()) {
        for (const char *ext : qmlStreamExts) {
            if (ext)
                exts << QString::fromLatin1(ext);
        }
    }
    return exts;
}

QColor CDialogs::displayColorDialog(const QColor &color, bool showAlpha) const
{
    QColorDialog theColorDlg;
    theColorDlg.setOption(QColorDialog::DontUseNativeDialog);

    if (showAlpha)
        theColorDlg.setOption(QColorDialog::ShowAlphaChannel);

    theColorDlg.setCurrentColor(color);
    connect(&theColorDlg, &QColorDialog::currentColorChanged, this, &CDialogs::onColorChanged);
    int result = theColorDlg.exec();
    disconnect(&theColorDlg, &QColorDialog::currentColorChanged, this, &CDialogs::onColorChanged);
    if (result == QDialog::Accepted)
        return theColorDlg.selectedColor();
    else
        return color;
}
