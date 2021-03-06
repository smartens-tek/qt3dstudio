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
#pragma once

#include "CmdStack.h"
#include "DispatchListeners.h"
#include "BuildConfigParser.h"
#include "Qt3DSFileTools.h"
#include "Doc.h"
#include "ProjectFile.h"

class CDoc;
class CDispatch;
class CCmdStack;
class CHotKeys;
class CStudioProjectSettings;

QT_FORWARD_DECLARE_CLASS(QWidget)

class CCore : public QObject, public CModificationListener, public CPresentationChangeListener
{
    Q_OBJECT

public:
    CCore();
    ~CCore();

    void Initialize();

    CDoc *GetDoc() const;
    CDispatch *GetDispatch();
    CCmdStack *GetCmdStack();
    CHotKeys *GetHotKeys();
    CStudioProjectSettings *GetStudioProjectSettings();
    Q3DStudio::CBuildConfigurations &GetBuildConfigurations();

    bool LoadBuildConfigurations();
    void RegisterGlobalKeyboardShortcuts(CHotKeys *inShortcutHandler, QWidget *actionParent);

    bool OnNewDocument(const QString &inDocument, bool isNewProject, bool silent = false);
    void OnSaveDocument(const QString &inDocument, bool inSaveCopy = false);
    void OnSaveDocumentCatcher(const QString &inDocument, bool inSaveCopy = false);
    void SetCommandStackModifier(ICmdStackModifier *inModifier);
    bool ExecuteCommand(CCmd *inCommand, bool inIsSynchronous = true);
    void CommitCurrentCommand();
    void SetCommandModifiedFlag(bool inModified) override;
    void CommandUpdate(unsigned long inUpdateFlags) override;

    void GetCreateDirectoryFileName(const QString &inDocument, Q3DStudio::CFilePath &outFinalDir,
                                    Q3DStudio::CFilePath &outFinalDoc);

    // CPresentationListener
    void OnNewPresentation() override;
    void OnClosingPresentation() override;

    bool CanUndo();
    bool CanRedo();
    void RemoveInvalidCharsInComponentName(Q3DStudio::CString &ioComponentName);
    void DeselectAllKeyframes();
    void SetTimebarStart();
    void SetTimebarEnd();
    void SetTimebarStartAffectsChildren();
    void SetTimebarEndAffectsChildren();
    void DumpCommandQueue();
    bool HasJustSaved() { return m_JustSaved; }
    void SetJustSaved(bool inJustSaved) { m_JustSaved = inJustSaved; }
    ProjectFile &getProjectFile();

protected:
    CDoc *m_Doc;
    CDispatch *m_Dispatch;
    CCmdStack *m_CmdStack;
    CHotKeys *m_HotKeys;
    CStudioProjectSettings *m_StudioProjectSettings;
    Q3DStudio::CBuildConfigurations m_BuildConfigurations;
    bool m_JustSaved;

    void InitAndValidateBuildConfiguration();

private:
    ProjectFile m_projectFile;
};
