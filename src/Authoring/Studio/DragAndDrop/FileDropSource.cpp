/****************************************************************************
**
** Copyright (C) 1999-2002 NVIDIA Corporation.
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

//==============================================================================
//	Prefix
//==============================================================================

//==============================================================================
//	Includes
//==============================================================================
#include "stdafx.h"
#include "Dialogs.h"
#include "FileDropSource.h"
#include "DropTarget.h"
#include "StudioObjectTypes.h"
#include "HotKeys.h"
#include "Doc.h"
#include "StudioApp.h"
#include "Core.h"
#include "UICDMStudioSystem.h"
#include "ClientDataModelBridge.h"
#include "IDocumentEditor.h"
#include "UICFileTools.h"
#include "ImportUtils.h"

bool CFileDropSource::s_FileHasValidTarget = false;

//===============================================================================
/**
 *
 */
bool CFileDropSource::ValidateTarget(CDropTarget *inTarget)
{
    using namespace Q3DStudio;

    EStudioObjectType targetType = (EStudioObjectType)inTarget->GetObjectType();
    bool theValidTarget = false;

    // the only thing we want to do from here is check the type.
    theValidTarget = CStudioObjectTypes::AcceptableParent(
        (EStudioObjectType)m_ObjectType, (EStudioObjectType)inTarget->GetObjectType());

    if (!theValidTarget) {
        SetHasValidTarget(theValidTarget);
        return theValidTarget;
    } else {
        if (CHotKeys::IsKeyDown(Qt::AltModifier) && targetType != OBJTYPE_SCENE
            && targetType != OBJTYPE_COMPONENT) {
            UICDM::CUICDMInstanceHandle theTarget = inTarget->GetInstance();
            CDoc *theDoc = g_StudioApp.GetCore()->GetDoc();
            IDocumentReader &theReader(theDoc->GetDocumentReader());
            UICDM::CUICDMSlideHandle toSlide = theReader.GetAssociatedSlide(theTarget);
            ;

            if (!theReader.IsMasterSlide(toSlide))
                theValidTarget = false;
        }

        SetHasValidTarget(theValidTarget);
        return theValidTarget;
    }
}

//===============================================================================
/**
 *
 */
CFileDropSource::CFileDropSource(long inFlavor, void *inData, unsigned long inSize)
    : CDropSource(inFlavor, inSize)
    , m_File("")
{
    // Pull out all of the SDropItemData and build a file.
    m_File = *(CUICFile *)inData;
    m_ObjectType =
        Q3DStudio::ImportUtils::GetObjectFileTypeForFile(m_File.GetAbsolutePath()).m_ObjectType;
}

//===============================================================================
/**
 *
 */
void CFileDropSource::SetHasValidTarget(bool inValid)
{
    m_HasValidTarget = inValid;
    CFileDropSource::s_FileHasValidTarget = inValid;
}

//===============================================================================
/**
 *
 */
bool CFileDropSource::GetHasValidTarget()
{
    return CFileDropSource::s_FileHasValidTarget;
}

//===============================================================================
/**
 *
 */
bool CFileDropSource::CanMove()
{
    return false;
}

//===============================================================================
/**
 *
 */
bool CFileDropSource::CanCopy()
{
    return true;
}

CCmd *CFileDropSource::GenerateAssetCommand(UICDM::CUICDMInstanceHandle inTarget,
                                            EDROPDESTINATION inDestType,
                                            UICDM::CUICDMSlideHandle inSlide)
{
    UICDM::CUICDMInstanceHandle theTarget = inTarget;

    CDoc &theDoc(*g_StudioApp.GetCore()->GetDoc());
    Q3DStudio::CFilePath theFilePath(m_File.GetAbsolutePath());
    CPt thePoint;
    //  if ( CHotKeys::IsKeyDown( Qt::AltModifier ) )
    //      thePoint = GetCurrentPoint();

    long theStartTime = -1;
    if (CHotKeys::IsKeyDown(Qt::ControlModifier))
        theStartTime = theDoc.GetCurrentViewTime();

    if (theFilePath.IsFile()) {
        Q3DStudio::DocumentEditorFileType::Enum theDocType(
            Q3DStudio::ImportUtils::GetObjectFileTypeForFile(theFilePath).m_FileType);
        QString theCommandName;
        // TODO: internationalize
        switch (theDocType) {
        case Q3DStudio::DocumentEditorFileType::DAE:
            theCommandName = QObject::tr("File Drop DAE File");
            break;
        case Q3DStudio::DocumentEditorFileType::Import:
            theCommandName = QObject::tr("File Drop Import File");
            break;
        case Q3DStudio::DocumentEditorFileType::Image:
            theCommandName = QObject::tr("File Drop Image File");
            break;
        case Q3DStudio::DocumentEditorFileType::Behavior:
            theCommandName = QObject::tr("File Drop Behavior File");
            break;
        case Q3DStudio::DocumentEditorFileType::Mesh:
            theCommandName = QObject::tr("File Drop Mesh File");
            break;
        case Q3DStudio::DocumentEditorFileType::Font:
            theCommandName = QObject::tr("File Drop Font File");
            break;
        case Q3DStudio::DocumentEditorFileType::Effect:
            theCommandName = QObject::tr("File Drop Effect File");
            break;
        case Q3DStudio::DocumentEditorFileType::Material:
            theCommandName = QObject::tr("File Drop Material File");
            break;
        case Q3DStudio::DocumentEditorFileType::Path:
            theCommandName = QObject::tr("File Drop Path File");
            break;
        }

        Q3DStudio::SCOPED_DOCUMENT_EDITOR(theDoc, theCommandName)
            ->ImportFile(theDocType, theFilePath, theTarget, inSlide,
                         CDialogs::GetImportFileExtension(),
                         Q3DStudio::ImportUtils::GetInsertTypeForDropType(inDestType), thePoint,
                         theStartTime);
    }
    return nullptr;
}
