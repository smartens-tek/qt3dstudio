/****************************************************************************
**
** Copyright (C) 2008 NVIDIA Corporation.
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
#include "Core.h"
#include "Doc.h"
#include "Dispatch.h"
#include "HotKeys.h"
#include "StudioProjectSettings.h"
#include "FileOutputStream.h"
#include "FormattedOutputStream.h"
#include "Cmd.h"
#include "StudioConst.h"
#include "StudioPreferences.h"
#include "StudioApp.h"
#include "Views/Views.h"
#include "MainFrm.h"
#include "IStudioRenderer.h"
#include <QtWidgets/qaction.h>

CCore::CCore()
    : m_Doc(NULL)
    , m_Dispatch(new CDispatch())
    , m_CmdStack(new CCmdStack())
    , m_HotKeys(new CHotKeys())
    , m_JustSaved(false)
{
    m_StudioProjectSettings = new CStudioProjectSettings(this);
    m_Dispatch->AddPresentationChangeListener(this);
    m_CmdStack->SetModificationListener(this);
    m_Doc = new CDoc(this);
}

CCore::~CCore()
{
    m_BuildConfigurations.Clear();
    m_CmdStack->Clear();
    delete m_Doc;
    delete m_Dispatch;
    delete m_CmdStack;
    delete m_HotKeys;
    delete m_StudioProjectSettings;
}

void CCore::Initialize()
{
    LoadBuildConfigurations();
}

CDoc *CCore::GetDoc() const
{
    return m_Doc;
}

CDispatch *CCore::GetDispatch()
{
    return m_Dispatch;
}

CCmdStack *CCore::GetCmdStack()
{
    return m_CmdStack;
}

CHotKeys *CCore::GetHotKeys()
{
    return m_HotKeys;
}

CStudioProjectSettings *CCore::GetStudioProjectSettings()
{
    return m_StudioProjectSettings;
}

Q3DStudio::CBuildConfigurations &CCore::GetBuildConfigurations()
{
    return m_BuildConfigurations;
}

/**
 *	Load all the build configurations
 */
bool CCore::LoadBuildConfigurations()
{
    using namespace Q3DStudio;
    // See if we can find the build configurations where they are located first
    QString configDirPath = QDir::cleanPath(Qt3DSFile::GetApplicationDirectory()
                                        + QStringLiteral("/../../../Studio/Build Configurations"));

    if (!QFileInfo(configDirPath).isDir()) {
        configDirPath = QDir::cleanPath(Qt3DSFile::GetApplicationDirectory()
                                    + QStringLiteral("/Build Configurations"));
    }

    Q3DStudio::CBuildConfigParser theParser(m_BuildConfigurations);
    bool theSuccess = theParser.LoadConfigurations(configDirPath);
    if (!theSuccess) {
        m_Dispatch->FireOnBuildConfigurationFileParseFail(theParser.GetErrorMessage());
    } else {
        InitAndValidateBuildConfiguration();
    }

    return theSuccess;
}

/**
 *	If the build configuration saved doesn't exist, use the first one
 *	also verify all the properties required existed, else use the first option available
 */
void CCore::InitAndValidateBuildConfiguration()
{
    Q3DStudio::CBuildConfiguration *theConfig =
            m_BuildConfigurations.GetConfiguration(CStudioPreferences::previewConfig());
    if (!theConfig) {
        Q3DStudio::CBuildConfigurations::TBuildConfigurations &theConfigurations =
                m_BuildConfigurations.GetConfigurations();
        if (theConfigurations.size()) {
            CStudioPreferences::setPreviewConfig(theConfigurations.begin()->first);
            theConfig = theConfigurations.begin()->second;
        }
    }
    if (theConfig) {
        Q3DStudio::CBuildConfiguration::TConfigProperties &theConfigProperties =
                theConfig->GetBuildProperties();
        Q3DStudio::CBuildConfiguration::TConfigProperties::iterator theConfigPropIter;
        for (theConfigPropIter = theConfigProperties.begin();
             theConfigPropIter != theConfigProperties.end(); ++theConfigPropIter) {
            const QString &thePropName = theConfigPropIter->GetName();
            QString theStoredValue = CStudioPreferences::previewProperty(thePropName);
            if (!theConfigPropIter->HasValue(theStoredValue)) {
                // add this property in
                if (theConfigPropIter->GetAcceptableValues().size()) {
                    CStudioPreferences::setPreviewProperty(
                                thePropName, theConfigPropIter->GetValue(0).GetName());
                }
            }
        }
    }
}

/**
 * Call by the mainframe to register the keyboard settings.
 * @param inShortcutHandler the handler for the keyboard actions.
 */
void CCore::RegisterGlobalKeyboardShortcuts(CHotKeys *inShortcutHandler, QWidget *actionParent)
{
    ADD_GLOBAL_SHORTCUT(actionParent,
                        QKeySequence(Qt::ControlModifier | Qt::Key_F6),
                        CCore::DumpCommandQueue);
    ADD_GLOBAL_SHORTCUT(actionParent,
                        QKeySequence(Qt::Key_BracketLeft),
                        CCore::SetTimebarStartAffectsChildren);
    ADD_GLOBAL_SHORTCUT(actionParent,
                        QKeySequence(Qt::Key_BracketRight),
                        CCore::SetTimebarEndAffectsChildren);
    ADD_GLOBAL_SHORTCUT(actionParent,
                        QKeySequence(Qt::ControlModifier | Qt::Key_BracketLeft),
                        CCore::SetTimebarStart);
    ADD_GLOBAL_SHORTCUT(actionParent,
                        QKeySequence(Qt::ControlModifier | Qt::Key_BracketRight),
                        CCore::SetTimebarEnd);
}

void CCore::GetCreateDirectoryFileName(const QString &inDocument,
                                       Q3DStudio::CFilePath &outFinalDir,
                                       Q3DStudio::CFilePath &outFinalDoc)
{
    using namespace Q3DStudio;
    CFilePath theOriginal(inDocument);
    CFilePath theName(theOriginal.GetFileName());
    CString theStem(theOriginal.GetFileStem());
    CFilePath theDir = theOriginal.GetDirectory();
    outFinalDir = CFilePath::CombineBaseAndRelative(theDir, theStem);
    outFinalDoc = CFilePath::CombineBaseAndRelative(outFinalDir, theName);
}

ProjectFile &CCore::getProjectFile()
{
    return m_projectFile;
}

/**
 * Call to create a new document.
 * This will clear out the current doc (if there is one) then create a new one.
 *
 * @param inDocument the document absolute path including the file name
 * @param isNewProject create a new project or create presentation under an existing project
 * @param silent don't show the presentation settings
 *
 * @return bool creation successful
 */
bool CCore::OnNewDocument(const QString &inDocument, bool isNewProject, bool silent)
{
    CDispatchDataModelNotificationScope __dispatchScope(*m_Dispatch);

    QFileInfo info(inDocument);
    if (!isNewProject) {
        // If document with given name exists, bail out
        if (info.exists())
            return false;

        // Ensure project file is created before current presentation is closed to make sure the
        // current presentation is added to the project.
        m_projectFile.ensureProjectFile();
    }

    g_StudioApp.getRenderer().MakeContextCurrent();
    m_Doc->CloseDocument();
    g_StudioApp.getRenderer().ReleaseContext();

    QString theDocument = inDocument;

    if (isNewProject) {
        QDir dir(info.absoluteDir());
        QString projName = info.completeBaseName(); // project name
        dir.mkdir(projName); // create project directory
        dir.cd(projName);    // go inside project directory

        // create asset folders
        dir.mkdir(QStringLiteral("effects"));
        dir.mkdir(QStringLiteral("fonts"));
        dir.mkdir(QStringLiteral("maps"));
        dir.mkdir(QStringLiteral("materials"));
        dir.mkdir(QStringLiteral("models"));
        dir.mkdir(QStringLiteral("presentations"));
        dir.mkdir(QStringLiteral("qml"));
        dir.mkdir(QStringLiteral("scripts"));

        // create the project .uia file
        QString uiaPath = info.absolutePath() + QStringLiteral("/") + projName
                          + QStringLiteral("/") + projName + QStringLiteral(".uia");
        m_projectFile.create(uiaPath);

        // set the default uip file path to be inside the presentations folder
        theDocument = dir.absolutePath() + QStringLiteral("/presentations/") + info.fileName();
    }

    if (!m_Doc->SetDocumentPath(theDocument)) {
        m_Doc->CreateNewDocument(); // Required to prevent a crash, as the old one is already closed
        return false;
    }

    m_Doc->CreateNewDocument();

    // Serialize the new document.
    m_Doc->SaveDocument(theDocument);

    // write a new presentation node to the uia file
    m_projectFile.addPresentationNode(theDocument);
    m_projectFile.updateDocPresentationId();
    m_projectFile.loadVariants();
    m_projectFile.loadSubpresentationsAndDatainputs(g_StudioApp.m_subpresentations,
                                                    g_StudioApp.m_dataInputDialogItems);
    g_StudioApp.getRenderer().RegisterSubpresentations(g_StudioApp.m_subpresentations);

    // show the presentation settings panel
    if (!silent)
        g_StudioApp.GetViews()->getMainFrame()->EditPreferences(PAGE_STUDIOPROJECTSETTINGS);

    return true;
}

/**
 * Call to save the current document.
 * This will do all the prompting, directory stuff necessary and perform the
 * saving of the document.
 */
void CCore::OnSaveDocument(const QString &inDocument, bool inSaveCopy /*= false*/)
{
    m_JustSaved = true;
    GetDispatch()->FireOnSavingPresentation(inDocument);
    bool isSuccess = false;
    try {
        OnSaveDocumentCatcher(inDocument, inSaveCopy);
        m_Dispatch->FireOnSaveDocument(inDocument, true, inSaveCopy);
        isSuccess = true;
    } catch (...) { /** TODO: implement stacktrace*/
    }

    if (!isSuccess)
        m_Dispatch->FireOnSaveDocument(inDocument, false, inSaveCopy);
}

/**
 * Called by OnSaveDocument, to allow the error reporting to be inserted.
 * Because of the nature of the error reporting, OnSaveDocument has to have
 * a certain structure that limits it (C type variables, no object destructors).
 * If we are saving a copy, then set the m_SaveCopy flag to true.  This will
 * leave the document in a dirty state and not update it to point to the new
 * file path.
*/
void CCore::OnSaveDocumentCatcher(const QString &inDocument, bool inSaveCopy /*= false*/)
{
    QFileInfo fileInfo(inDocument);
    m_Dispatch->FireOnProgressBegin(QObject::tr("Saving "), fileInfo.fileName());

    bool theDisplaySaveFailDialog = false;
    bool theFileExists = fileInfo.exists();
    Qt3DSFile theTempFile(fileInfo);

    // Test for readonly files
    if (theFileExists && !fileInfo.isWritable())
        theDisplaySaveFailDialog = true;
    else {
        try {
            // if file already exists, write to a temp file first, to prevent corrupting the
            // original file.
            if (theFileExists) {
                theTempFile = Qt3DSFile::GetTemporaryFile();
                // sanity check: if we fail to get a temporary file
                if (theTempFile.GetAbsolutePosixPath().IsEmpty()) { // too bad, we'll have to use
                    // the original (which might be
                    // the only writeable file)
                    theTempFile = inDocument;
                    theFileExists = false;
                }
            }

            m_Doc->SaveDocument(theTempFile.GetAbsolutePath().toQString());

            // update the original file
            if (theFileExists)
                theTempFile.CopyTo(inDocument);

            // If we are saving normally and not a copy, then we need to update the current document
            // to make sure it points to the saved file and make it not-dirty.  If we are saving a
            // copy
            // then we will leave the document with the original file path dirty state
            if (!inSaveCopy) {
                m_Doc->SetDocumentPath(inDocument);
                m_Doc->SetModifiedFlag(false);
            }
        } catch (CStudioException &) // one of our exceptions, show the standard save error.
        {
            theDisplaySaveFailDialog = true;
        } catch (...) {
            m_Dispatch->FireOnSaveFail(false);
        }
    }
    // clean up
    if (theFileExists)
        theTempFile.DeleteFile();

    if (theDisplaySaveFailDialog) {
        m_Dispatch->FireOnSaveFail(true);
    }

    m_Dispatch->FireOnProgressEnd();
}

void CCore::SetCommandStackModifier(ICmdStackModifier *inModifier)
{
    m_CmdStack->SetCommandStackModifier(inModifier);
}

/**
 * This is used for do/undo/redo and handles the execution of the command.
 * The command must have been new'd up, this will take responsibility for
 * deleting the command.
 * @param inCommand the command to be executed.
 * @return bool true if inCommand was deleted, false otherwise
 * @see CCmdStack::ExecuteCommand.
 */
bool CCore::ExecuteCommand(CCmd *inCommand, bool inIsSynchronous)
{
    if (!inIsSynchronous) {
        m_Dispatch->FireOnAsynchronousCommand(inCommand);
        return false;
    } else
        return m_CmdStack->ExecuteCommand(inCommand);
}

/**
 * Commit the last command to be executed.
 * @see CCmdStack::CommitLastCommand.
 */
void CCore::CommitCurrentCommand()
{
    m_CmdStack->CommitLastCommand();
}

/**
 * Callback from the CommandStack to change whether or not this is modified.
 * This is used when the execution of commands changes the modified state of
 * the project.
 */
void CCore::SetCommandModifiedFlag(bool inIsModified)
{
    if (inIsModified)
        m_Doc->SetModifiedFlag();
}

/**
 * Callback from the CommandStack for the change flags after a command was
 * executed.
 */
void CCore::CommandUpdate(unsigned long /*inFlags*/)
{
}

/**
 * Notification from the dispatch that a new presentation is being created.
 */
void CCore::OnNewPresentation()
{
    m_CmdStack->Clear();
}

/**
 * Notification from the dispatch that the current presentation is closing.
 */
void CCore::OnClosingPresentation()
{
    m_StudioProjectSettings->reset();
    m_CmdStack->Clear();
}

/**
 *	Helper function to remove invalid characters not accepted by windows when
 *	exporting a component.
 *	Invalid characters include: \/:*?"<>|
 */
void CCore::RemoveInvalidCharsInComponentName(Q3DStudio::CString &ioComponentName)
{
    if (ioComponentName.Find('\\') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("\\", "");

    if (ioComponentName.Find('/') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("/", "");

    if (ioComponentName.Find(':') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace(":", "");

    if (ioComponentName.Find('*') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("*", "");

    if (ioComponentName.Find('?') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("?", "");

    if (ioComponentName.Find('"') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("\"", "");

    if (ioComponentName.Find('<') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("<", "");

    if (ioComponentName.Find('>') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace(">", "");

    if (ioComponentName.Find('|') != Q3DStudio::CString::ENDOFSTRING)
        ioComponentName.Replace("|", "");
}

void CCore::DeselectAllKeyframes()
{
    m_Doc->DeselectAllKeyframes();
}

/**
 * Called by hotkeys to set the start time of the currently selected object.
 */
void CCore::SetTimebarStart()
{
    m_Doc->TruncateTimebar(true, false);
}

/**
 * Called by hotkeys to set the end time of the currently selected object.
 */
void CCore::SetTimebarEnd()
{
    m_Doc->TruncateTimebar(false, false);
}

/**
 * Called by hotkeys to set the start time of the currently selected object.
 * Affects children objects
 */
void CCore::SetTimebarStartAffectsChildren()
{
    m_Doc->TruncateTimebar(true, true);
}

/**
 * Called by hotkeys to set the end time of the currently selected object.
 * Affects children objects
 */
void CCore::SetTimebarEndAffectsChildren()
{
    m_Doc->TruncateTimebar(false, true);
}

/**
 * Debugging utility to dump the command queue to a file.
 */
void CCore::DumpCommandQueue()
{
    CFileOutputStream theOutputStream("CommandStack.txt");
    CCmdStack::TCmdList theUndoCommands = m_CmdStack->GetUndoStack();
    CCmdStack::TCmdList::iterator thePos = theUndoCommands.begin();
    for (; thePos != theUndoCommands.end(); ++thePos) {
        Q3DStudio::CString theText = Q3DStudio::CString::fromQString((*thePos)->ToString());
        theOutputStream.Write(theText.GetCharStar(), theText.Length() + 1);
        theOutputStream.Write("\r\n", 2);
    }
}
