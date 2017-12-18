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

//==============================================================================
//	Prefix
//==============================================================================
#include "stdafx.h"

//==============================================================================
//	Includes
//==============================================================================
#include "Qt3DSDMTimelineItemBinding.h"
#include "TimelineTranslationManager.h"
#include "TimeEditDlg.h"
#include "EmptyTimelineTimebar.h"
#include "Qt3DSDMTimelineTimebar.h"
#include "BaseStateRow.h"
#include "BaseTimebarlessRow.h"
#include "PropertyTimebarRow.h"
#include "PropertyRow.h"
#include "KeyframesManager.h"
#include "StudioApp.h"
#include "Core.h"
#include "Dialogs.h"
#include "GraphUtils.h"
#include "Qt3DSDMDataCore.h"

// Data model specific
#include "IDoc.h"
#include "ClientDataModelBridge.h"
#include "Dispatch.h"
#include "DropSource.h"
#include "Qt3DSDMTimelineItemProperty.h"
#include "Qt3DSDMSlides.h"
#include "Qt3DSDMStudioSystem.h"
#include "Qt3DSDMSlideGraphCore.h"
#include "Qt3DSDMActionCore.h"
#include "Qt3DSDMAnimation.h"
#include "CmdDataModelChangeKeyframe.h"
#include "RelativePathTools.h"
#include "IDocumentEditor.h"
#include "Qt3DSFileTools.h"
#include "ImportUtils.h"

#include <QtWidgets/qmessagebox.h>

using namespace qt3dsdm;

Qt3DSDMTimelineItemBinding::Qt3DSDMTimelineItemBinding(CTimelineTranslationManager *inMgr,
                                                       Qt3DSDMInstanceHandle inDataHandle)
    : m_Row(nullptr)
    , m_TransMgr(inMgr)
    , m_DataHandle(inDataHandle)
    , m_Parent(nullptr)
    , m_TimelineTimebar(nullptr)

{
    m_StudioSystem = m_TransMgr->GetStudioSystem();
    m_TransMgr->GetDoc()->GetCore()->GetDispatch()->AddDataModelListener(this);
}

Qt3DSDMTimelineItemBinding::Qt3DSDMTimelineItemBinding(CTimelineTranslationManager *inMgr)
    : m_Row(nullptr)
    , m_TransMgr(inMgr)
    , m_DataHandle(0)
    , m_Parent(nullptr)
    , m_TimelineTimebar(nullptr)
{
    m_StudioSystem = m_TransMgr->GetStudioSystem();
    m_TransMgr->GetDoc()->GetCore()->GetDispatch()->AddDataModelListener(this);
}

Qt3DSDMTimelineItemBinding::~Qt3DSDMTimelineItemBinding()
{
    RemoveAllPropertyBindings();
    delete m_TimelineTimebar;
    m_TransMgr->GetDoc()->GetCore()->GetDispatch()->RemoveDataModelListener(this);
}

// helpers
bool Qt3DSDMTimelineItemBinding::GetBoolean(qt3dsdm::Qt3DSDMPropertyHandle inProperty) const
{
    qt3dsdm::IPropertySystem *thePropertySystem = m_StudioSystem->GetPropertySystem();
    SValue theValue;
    thePropertySystem->GetInstancePropertyValue(m_DataHandle, inProperty, theValue);
    return qt3dsdm::get<bool>(theValue);
}

void Qt3DSDMTimelineItemBinding::SetBoolean(qt3dsdm::Qt3DSDMPropertyHandle inProperty,
                                            bool inValue, const QString &inNiceText) const
{
    CDoc *theDoc = dynamic_cast<CDoc *>(g_StudioApp.GetCore()->GetDoc());
    Q3DStudio::SCOPED_DOCUMENT_EDITOR(*theDoc, inNiceText)
            ->SetInstancePropertyValue(m_DataHandle, inProperty, inValue);
}

void Qt3DSDMTimelineItemBinding::SetInstanceHandle(qt3dsdm::Qt3DSDMInstanceHandle inDataHandle)
{
    m_DataHandle = inDataHandle;
}

EStudioObjectType Qt3DSDMTimelineItemBinding::GetObjectType() const
{
    return m_StudioSystem->GetClientDataModelBridge()->GetObjectType(m_DataHandle);
}

bool Qt3DSDMTimelineItemBinding::IsMaster() const
{
    CDoc *theDoc = dynamic_cast<CDoc *>(g_StudioApp.GetCore()->GetDoc());
    Q3DStudio::IDocumentReader &theReader(theDoc->GetDocumentReader());
    if (GetObjectType() == OBJTYPE_IMAGE) {
        CClientDataModelBridge *theBridge = m_StudioSystem->GetClientDataModelBridge();
        Qt3DSDMInstanceHandle theParent;
        Qt3DSDMPropertyHandle theProperty;
        bool isPropertyLinked;

        theBridge->GetMaterialFromImageInstance(GetInstance(), theParent, theProperty);
        isPropertyLinked = theReader.IsPropertyLinked(theParent, theProperty);

        // Also check light probe
        if (!isPropertyLinked) {
            theBridge->GetLayerFromImageProbeInstance(GetInstance(), theParent, theProperty);
            isPropertyLinked = theReader.IsPropertyLinked(theParent, theProperty);
        }

        return isPropertyLinked;
    }
    Qt3DSDMInstanceHandle theQueryHandle(m_DataHandle);
    if (GetObjectType() == OBJTYPE_PATHANCHORPOINT)
        theQueryHandle = theReader.GetParent(m_DataHandle);

    // logic: you can't unlink name, so if name is linked then, this is master.
    Qt3DSDMPropertyHandle theNamePropHandle =
            m_StudioSystem->GetPropertySystem()->GetAggregateInstancePropertyByName(theQueryHandle,
                                                                                    L"name");
    return theReader.IsPropertyLinked(theQueryHandle, theNamePropHandle);
}

bool Qt3DSDMTimelineItemBinding::IsShy() const
{
    return GetBoolean(m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset().m_Shy);
}
void Qt3DSDMTimelineItemBinding::SetShy(bool inShy)
{
    SetBoolean(m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset().m_Shy, inShy,
               QObject::tr("Shy Toggle"));
}
bool Qt3DSDMTimelineItemBinding::IsLocked() const
{
    return GetBoolean(m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset().m_Locked);
}

void ToggleChildrenLock(Q3DStudio::ScopedDocumentEditor &scopedDocEditor,
                        Qt3DSDMTimelineItemBinding *inTimelineItemBinding,
                        SDataModelSceneAsset inSceneAsset, bool inLocked)
{
    scopedDocEditor->SetInstancePropertyValue(inTimelineItemBinding->GetInstanceHandle(),
                                              inSceneAsset.m_Locked, inLocked);
    long childrenCount = inTimelineItemBinding->GetChildrenCount();
    if (childrenCount == 0)
        return;
    for (long i = 0; i < childrenCount; ++i) {
        Qt3DSDMTimelineItemBinding *child =
                static_cast<Qt3DSDMTimelineItemBinding *>(inTimelineItemBinding->GetChild(i));
        ToggleChildrenLock(scopedDocEditor, child, inSceneAsset, inLocked);
    }
}

void Qt3DSDMTimelineItemBinding::SetLocked(bool inLocked)
{
    CDoc *theDoc = dynamic_cast<CDoc *>(g_StudioApp.GetCore()->GetDoc());
    Q3DStudio::ScopedDocumentEditor scopedDocEditor(*theDoc, L"SetLock", __FILE__, __LINE__);

    SDataModelSceneAsset sceneAsset = m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset();
    ToggleChildrenLock(scopedDocEditor, this, sceneAsset, inLocked);

    if (inLocked)
        g_StudioApp.GetCore()->GetDoc()->NotifySelectionChanged();
}

bool Qt3DSDMTimelineItemBinding::IsVisible() const
{
    return GetBoolean(m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset().m_Eyeball);
}

void Qt3DSDMTimelineItemBinding::SetVisible(bool inVisible)
{
    SetBoolean(m_StudioSystem->GetClientDataModelBridge()->GetSceneAsset().m_Eyeball,
               inVisible, QObject::tr("Visibility Toggle"));
}

// remember the expanded state for the current presentation
bool Qt3DSDMTimelineItemBinding::IsExpanded() const
{
    return m_TransMgr->IsExpanded(m_DataHandle);
}
// remember the expanded state for the current presentation
void Qt3DSDMTimelineItemBinding::SetExpanded(bool inExpanded)
{
    m_TransMgr->SetExpanded(m_DataHandle, inExpanded);
}

bool Qt3DSDMTimelineItemBinding::HasAction(bool inMaster)
{
    TActionHandleList theActions;
    CDoc *theDoc = g_StudioApp.GetCore()->GetDoc();

    Qt3DSDMSlideHandle theSlide = theDoc->GetActiveSlide();
    qt3dsdm::ISlideCore &theSlideCore(*m_StudioSystem->GetSlideCore());
    if (theSlideCore.IsSlide(theSlide)) {
        if (inMaster) {
            theSlide =
                    m_StudioSystem->GetSlideSystem()->GetMasterSlide(theSlide); // use the master slide
        }

        m_StudioSystem->GetActionCore()->GetActions(theSlide, m_DataHandle, theActions);
    }
    return theActions.size() > 0;
}

bool Qt3DSDMTimelineItemBinding::ChildrenHasAction(bool inMaster)
{
    // Get all the instances in this slidegraph
    // check whehter it's an action instance and is in the slide of interst
    // check also it's owner is a descendent of the viewed instances
    CDoc *theDoc = g_StudioApp.GetCore()->GetDoc();
    IActionCore *theActionCore(m_StudioSystem->GetActionCore());
    CClientDataModelBridge *theBridge(m_StudioSystem->GetClientDataModelBridge());

    Qt3DSDMSlideHandle theSlide = theDoc->GetActiveSlide();
    qt3dsdm::ISlideCore &theSlideCore(*m_StudioSystem->GetSlideCore());
    if (theSlideCore.IsSlide(theSlide)) {
        if (inMaster) {
            theSlide =
                    m_StudioSystem->GetSlideSystem()->GetMasterSlide(theSlide); // use the master slide
        }

        TSlideInstancePairList theGraphInstances;
        m_StudioSystem->GetSlideSystem()->GetAssociatedInstances(theSlide, theGraphInstances);

        qt3dsdm::Qt3DSDMInstanceHandle theObservedInstance = GetInstance();
        if (theObservedInstance.Valid()) {
            for (TSlideInstancePairList::const_iterator theIter = theGraphInstances.begin();
                 theIter != theGraphInstances.end(); ++theIter) {
                if (theIter->first == theSlide && theBridge->IsActionInstance(theIter->second)) {
                    Qt3DSDMActionHandle theAction =
                            theActionCore->GetActionByInstance(theIter->second);
                    SActionInfo theActionInfo = theActionCore->GetActionInfo(theAction);
                    Qt3DSDMInstanceHandle theAcionOwner = theActionInfo.m_Owner;
                    if (theAcionOwner.Valid()
                            && IsAscendant(theAcionOwner, theObservedInstance, theDoc->GetAssetGraph()))
                        return true;
                }
            }
        }
    }

    return false;
}

bool Qt3DSDMTimelineItemBinding::ComponentHasAction(bool inMaster)
{
    // Get all the instances in this component slidegraph
    // check whether the instance is an action instance
    // if inMaster is true, we only interest with those that are in the master slide, else we want
    // those that are not in the master slide
    CClientDataModelBridge *theBridge(m_StudioSystem->GetClientDataModelBridge());
    if (!theBridge->IsComponentInstance(m_DataHandle))
        return false;

    Q3DStudio::CId theAssetId = theBridge->GetGUID(m_DataHandle);
    Qt3DSDMSlideHandle theMasterSlide =
            m_StudioSystem->GetSlideSystem()->GetMasterSlideByComponentGuid(GuidtoSLong4(theAssetId));

    TSlideInstancePairList theGraphInstances;
    m_StudioSystem->GetSlideSystem()->GetAssociatedInstances(theMasterSlide, theGraphInstances);

    for (TSlideInstancePairList::const_iterator theIter = theGraphInstances.begin();
         theIter != theGraphInstances.end(); ++theIter) {
        if (((inMaster && theIter->first == theMasterSlide)
             || (!inMaster && theIter->first != theMasterSlide))
                && theBridge->IsActionInstance(theIter->second)) {
            return true;
        }
    }
    return false;
}

ITimelineTimebar *Qt3DSDMTimelineItemBinding::GetTimebar()
{
    if (!m_TimelineTimebar)
        m_TimelineTimebar = CreateTimelineTimebar();
    return m_TimelineTimebar;
}

Q3DStudio::CString Qt3DSDMTimelineItemBinding::GetName() const
{
    if (m_StudioSystem->IsInstance(m_DataHandle) == false)
        return L"";
    Qt3DSDMPropertyHandle theNamePropHandle =
            m_StudioSystem->GetPropertySystem()->GetAggregateInstancePropertyByName(m_DataHandle,
                                                                                    L"name");
    SValue theNameValue;
    m_StudioSystem->GetPropertySystem()->GetInstancePropertyValue(m_DataHandle, theNamePropHandle,
                                                                  theNameValue);
    TDataStrPtr theName = qt3dsdm::get<TDataStrPtr>(theNameValue);

    return (theName) ? Q3DStudio::CString(theName->GetData()) : "";
}

void Qt3DSDMTimelineItemBinding::SetName(const Q3DStudio::CString &inName)
{
    // Display warning dialog if user tried to enter an empty string
    if (inName.IsEmpty()) {
        QString theTitle = QObject::tr("Rename Object Error");
        QString theString = QObject::tr("Object name cannot be an empty string.");
        g_StudioApp.GetDialogs()->DisplayMessageBox(theTitle, theString,
                                                    Qt3DSMessageBox::ICON_ERROR, false);

        return;
    }

    CClientDataModelBridge *theBridge = m_StudioSystem->GetClientDataModelBridge();
    if (!theBridge->CheckNameUnique(m_DataHandle, inName)) {
        QString theTitle = QObject::tr("Rename Object Error");
        QString theString = QObject::tr("The object name is duplicated under its parent, do you "
                                        "want to make it unique?");
        int theUserChoice = g_StudioApp.GetDialogs()->DisplayChoiceBox(
                    theTitle, theString, Qt3DSMessageBox::ICON_WARNING);
        if (theUserChoice == QMessageBox::Yes) {
            // Set with the unique name
            Q3DStudio::SCOPED_DOCUMENT_EDITOR(*m_TransMgr->GetDoc(), QObject::tr("Set Name"))
                    ->SetName(m_DataHandle, inName, true);
            return;
        }
    }
    // Set the name no matter it's unique or not
    Qt3DSDMPropertyHandle theNamePropHandle =
            m_StudioSystem->GetPropertySystem()->GetAggregateInstancePropertyByName(m_DataHandle,
                                                                                    L"name");
    Q3DStudio::SCOPED_DOCUMENT_EDITOR(*m_TransMgr->GetDoc(), QObject::tr("Set Name"))
            ->SetInstancePropertyValue(m_DataHandle, theNamePropHandle,
                                       std::make_shared<CDataStr>(inName));
}

ITimelineItem *Qt3DSDMTimelineItemBinding::GetTimelineItem()
{
    return this;
}

CBaseStateRow *Qt3DSDMTimelineItemBinding::GetRow()
{
    return m_Row;
}

void Qt3DSDMTimelineItemBinding::SetSelected(bool inMultiSelect)
{
    if (!inMultiSelect)
        g_StudioApp.GetCore()->GetDoc()->SelectDataModelObject(m_DataHandle);
    else
        g_StudioApp.GetCore()->GetDoc()->ToggleDataModelObjectToSelection(m_DataHandle);
}

void Qt3DSDMTimelineItemBinding::OnCollapsed()
{
    // Preserves legacy behavior where collapsing a tree will select that root, if any of its
    // descendant was selected
    // TODO: This won't work for Image (because Image is Material's property, not child)
    qt3dsdm::Qt3DSDMInstanceHandle theInstance = GetInstance();
    if (theInstance.Valid()) {
        CDoc *theDoc = m_TransMgr->GetDoc();
        qt3dsdm::Qt3DSDMInstanceHandle theSelectedInstance = theDoc->GetSelectedInstance();
        if (theSelectedInstance.Valid()
                && IsAscendant(theSelectedInstance, theInstance, theDoc->GetAssetGraph()))
            SetSelected(false);
    }
}

void Qt3DSDMTimelineItemBinding::ClearKeySelection()
{
    m_TransMgr->ClearKeyframeSelection();
}

bool Qt3DSDMTimelineItemBinding::OpenAssociatedEditor()
{
    return false; // nothing to do by default
}

void Qt3DSDMTimelineItemBinding::DoStartDrag(CControlWindowListener *inWndListener)
{
    inWndListener->DoStartDrag(this);
}

inline qt3dsdm::Qt3DSDMInstanceHandle Qt3DSDMTimelineItemBinding::GetInstance() const
{
    return m_DataHandle;
}

void Qt3DSDMTimelineItemBinding::SetDropTarget(CDropTarget *inTarget)
{
    qt3dsdm::Qt3DSDMInstanceHandle theInstance = GetInstance();
    inTarget->SetInstance(theInstance);
}

long Qt3DSDMTimelineItemBinding::GetChildrenCount()
{
    qt3dsdm::Qt3DSDMInstanceHandle theInstance = GetInstance();
    if (theInstance.Valid()) {
        Q3DStudio::CGraphIterator theChildren;
        Qt3DSDMSlideHandle theActiveSlide = m_TransMgr->GetDoc()->GetActiveSlide();
        GetAssetChildrenInTimeParent(theInstance, m_TransMgr->GetDoc(), AmITimeParent(),
                                     theChildren, theActiveSlide);
        return (long)theChildren.GetCount();
    }
    return 0;
}

ITimelineItemBinding *Qt3DSDMTimelineItemBinding::GetChild(long inIndex)
{
    qt3dsdm::Qt3DSDMInstanceHandle theInstance = GetInstance();
    if (theInstance.Valid()) {
        Q3DStudio::CGraphIterator theChildren;
        Qt3DSDMSlideHandle theActiveSlide = m_TransMgr->GetDoc()->GetActiveSlide();
        GetAssetChildrenInTimeParent(theInstance, m_TransMgr->GetDoc(), AmITimeParent(),
                                     theChildren, theActiveSlide);
        theChildren += inIndex;

        qt3dsdm::Qt3DSDMInstanceHandle theChildInstance = theChildren.GetCurrent();
        if (theChildInstance.Valid())
            return m_TransMgr->GetOrCreate(theChildInstance);
    }
    return nullptr;
}

ITimelineItemBinding *Qt3DSDMTimelineItemBinding::GetParent()
{
    return m_Parent;
}
void Qt3DSDMTimelineItemBinding::SetParent(ITimelineItemBinding *parent)
{
    if (parent != m_Parent) {
        ASSERT(parent == nullptr || m_Parent == nullptr);
        m_Parent = parent;
    }
}

long Qt3DSDMTimelineItemBinding::GetPropertyCount()
{
    long theCount = 0;
    if (m_StudioSystem->IsInstance(m_DataHandle)) {
        TPropertyHandleList theProperties;
        m_StudioSystem->GetPropertySystem()->GetAggregateInstanceProperties(m_DataHandle,
                                                                            theProperties);
        for (size_t thePropertyIndex = 0; thePropertyIndex < theProperties.size();
             ++thePropertyIndex) {
            if (m_StudioSystem->GetAnimationSystem()->IsPropertyAnimated(
                        m_DataHandle, theProperties[thePropertyIndex])) {
                ++theCount;
            }
        }
    }
    return theCount;
}

ITimelineItemProperty *Qt3DSDMTimelineItemBinding::GetProperty(long inIndex)
{
    TPropertyHandleList theProperties;
    m_StudioSystem->GetPropertySystem()->GetAggregateInstanceProperties(m_DataHandle,
                                                                        theProperties);
    long theIndex = -1;
    size_t thePropertyIndex = 0;
    for (; thePropertyIndex < theProperties.size(); ++thePropertyIndex) {
        if (m_StudioSystem->GetAnimationSystem()->IsPropertyAnimated(
                    m_DataHandle, theProperties[thePropertyIndex])) {
            ++theIndex;
            if (theIndex == inIndex)
                break;
        }
    }
    ASSERT(thePropertyIndex < theProperties.size()); // no reason why this would be out of range!!
    return GetOrCreatePropertyBinding(theProperties[thePropertyIndex]);
}

bool Qt3DSDMTimelineItemBinding::ShowToggleControls() const
{
    return true;
}

bool Qt3DSDMTimelineItemBinding::IsLockedEnabled() const
{
    return IsLocked();
}

bool Qt3DSDMTimelineItemBinding::IsVisibleEnabled() const
{
    // You can only toggle visible if you aren't on the master slide.
    return m_StudioSystem->GetSlideSystem()->GetSlideIndex(m_TransMgr->GetDoc()->GetActiveSlide())
            != 0;
}

void Qt3DSDMTimelineItemBinding::Bind(CBaseStateRow *inRow)
{
    ASSERT(!m_Row);
    m_Row = inRow;

    // Because children(properties included) may only be loaded later, check if there are any
    // keyframes without having to have the UI created.
    TPropertyHandleList theProperties;
    m_StudioSystem->GetPropertySystem()->GetAggregateInstanceProperties(m_DataHandle,
                                                                        theProperties);
    for (size_t thePropertyIndex = 0; thePropertyIndex < theProperties.size(); ++thePropertyIndex) {
        if (m_StudioSystem->GetAnimationSystem()->IsPropertyAnimated(
                    m_DataHandle, theProperties[thePropertyIndex]))
            AddKeyframes(GetOrCreatePropertyBinding(theProperties[thePropertyIndex]));
    }

    // Set selection status
    Qt3DSDMInstanceHandle theSelectedInstance = m_TransMgr->GetDoc()->GetSelectedInstance();
    m_Row->OnSelected(m_DataHandle == theSelectedInstance);
}

void Qt3DSDMTimelineItemBinding::Release()
{
    m_Row = nullptr;
    RemoveAllPropertyBindings();
    m_TransMgr->Unregister(this);
}

bool Qt3DSDMTimelineItemBinding::IsValidTransaction(EUserTransaction inTransaction)
{
    qt3dsdm::Qt3DSDMInstanceHandle theInstance = GetInstance();
    switch (inTransaction) {
    case EUserTransaction_Rename:
        return (GetObjectType() != OBJTYPE_SCENE && GetObjectType() != OBJTYPE_IMAGE);

    case EUserTransaction_Duplicate:
        if (theInstance.Valid())
            return m_StudioSystem->GetClientDataModelBridge()->IsDuplicateable(theInstance);
        break;

    case EUserTransaction_Cut:
        return g_StudioApp.CanCut();

    case EUserTransaction_Copy:
        return g_StudioApp.CanCopy();

    case EUserTransaction_Paste:
        return m_TransMgr->GetDoc()->CanPasteObject();

    case EUserTransaction_Delete:
        if (theInstance.Valid())
            return m_StudioSystem->GetClientDataModelBridge()->CanDelete(theInstance);
        break;

    case EUserTransaction_MakeComponent: {
        bool theCanMakeFlag = false;
        if (theInstance.Valid()) {
            CClientDataModelBridge *theBridge = m_StudioSystem->GetClientDataModelBridge();
            EStudioObjectType theObjectType = theBridge->GetObjectType(theInstance);

            if (!IsLocked()) {
                // Any assets that are attached to the Scene directly must not be wrapped in a
                // component.
                // This may include behavior assets which may be directly attached to the Scene.
                // This is because by principal, components cannot exist on the Scene directly.
                qt3dsdm::Qt3DSDMInstanceHandle theParentInstance =
                        theBridge->GetParentInstance(theInstance);
                if (theObjectType != OBJTYPE_LAYER && theObjectType != OBJTYPE_SCENE
                        && theObjectType != OBJTYPE_MATERIAL && theObjectType != OBJTYPE_IMAGE
                        && theObjectType != OBJTYPE_EFFECT
                        && (theParentInstance.Valid()
                            && theBridge->GetObjectType(theParentInstance)
                            != OBJTYPE_SCENE)) // This checks if the object is
                    // AttachedToSceneDirectly
                {
                    theCanMakeFlag = true;
                }
            }
        }
        return theCanMakeFlag;
    }

    case EUserTransaction_EditComponent:
        return (GetObjectType() == OBJTYPE_COMPONENT);

    default: // not handled
        break;
    }

    return false;
}

using namespace Q3DStudio;

inline void DoCut(CDoc &inDoc, const qt3dsdm::TInstanceHandleList &inInstances)
{
    inDoc.DeselectAllKeyframes();
    inDoc.CutObject(inInstances);
}

inline void DoDelete(CDoc &inDoc, const qt3dsdm::TInstanceHandleList &inInstances)
{
    inDoc.DeselectAllKeyframes();
    inDoc.DeleteObject(inInstances);
}

inline void DoMakeComponent(CDoc &inDoc, const qt3dsdm::TInstanceHandleList &inInstances)
{
    SCOPED_DOCUMENT_EDITOR(inDoc, QObject::tr("Make Component"))->MakeComponent(inInstances);
}

void Qt3DSDMTimelineItemBinding::PerformTransaction(EUserTransaction inTransaction)
{
    CDoc *theDoc = m_TransMgr->GetDoc();
    qt3dsdm::TInstanceHandleList theInstances = theDoc->GetSelectedValue().GetSelectedInstances();
    if (theInstances.empty())
        return;
    CDispatch &theDispatch(*theDoc->GetCore()->GetDispatch());

    // Transactions that could result in *this* object being deleted need to be executed
    // via postmessage, not in this context because it could result in the currently
    // active timeline row being deleted while in its own mouse handler.
    switch (inTransaction) {
    case EUserTransaction_Duplicate: {
        theDoc->DeselectAllKeyframes();
        SCOPED_DOCUMENT_EDITOR(*theDoc, QObject::tr("Duplicate Object"))->DuplicateInstances(theInstances);
    } break;
    case EUserTransaction_Cut: {
        theDispatch.FireOnAsynchronousCommand(
                    std::bind(DoCut, std::ref(*theDoc), theInstances));
    } break;
    case EUserTransaction_Copy: {
        theDoc->DeselectAllKeyframes();
        theDoc->CopyObject(theInstances);
    } break;
    case EUserTransaction_Paste: {
        theDoc->DeselectAllKeyframes();
        theDoc->PasteObject(GetInstance());
    } break;
    case EUserTransaction_Delete: {
        theDispatch.FireOnAsynchronousCommand(
                    std::bind(DoDelete, std::ref(*theDoc), theInstances));
    } break;
    case EUserTransaction_MakeComponent: {
        theDispatch.FireOnAsynchronousCommand(
                    std::bind(DoMakeComponent, std::ref(*theDoc), theInstances));
    }
    default: // not handled
        break;
    }
}

Q3DStudio::CString Qt3DSDMTimelineItemBinding::GetObjectPath()
{
    CDoc *theDoc = m_TransMgr->GetDoc();
    // Because we are getting absolute path, the base id doesn't matter.
    return CRelativePathTools::BuildAbsoluteReferenceString(m_DataHandle, theDoc);
}

ITimelineKeyframesManager *Qt3DSDMTimelineItemBinding::GetKeyframesManager() const
{
    return m_TransMgr->GetKeyframesManager();
}

void Qt3DSDMTimelineItemBinding::RemoveProperty(ITimelineItemProperty *inProperty)
{
    Q_UNUSED(inProperty);
    // TODO: This function has no use in DataModel world. This is replaced by RemovePropertyRow(
    // Qt3DSDMPropertyHandle inPropertyHandle ).
    // Decide if this function should be removed from ITimelineItemBinding.
}

void Qt3DSDMTimelineItemBinding::LoadProperties()
{
    TPropertyHandleList theProperties;
    m_StudioSystem->GetPropertySystem()->GetAggregateInstanceProperties(m_DataHandle,
                                                                        theProperties);
    for (size_t thePropertyIndex = 0; thePropertyIndex < theProperties.size(); ++thePropertyIndex) {
        if (m_StudioSystem->GetAnimationSystem()->IsPropertyAnimated(
                    m_DataHandle, theProperties[thePropertyIndex]))
            AddPropertyRow(theProperties[thePropertyIndex], true);
    }
}

void Qt3DSDMTimelineItemBinding::InsertKeyframe()
{
    if (m_PropertyBindingMap.empty())
        return;

    TPropertyBindingMap::const_iterator theIter = m_PropertyBindingMap.begin();
    ScopedDocumentEditor editor(*g_StudioApp.GetCore()->GetDoc(), L"Insert Keyframe", __FILE__,
                                __LINE__);
    for (; theIter != m_PropertyBindingMap.end(); ++theIter)
        editor->KeyframeProperty(m_DataHandle, theIter->first, false);
}

void Qt3DSDMTimelineItemBinding::DeleteAllChannelKeyframes()
{
    if (m_PropertyBindingMap.empty())
        return;

    CDoc *theDoc = m_TransMgr->GetDoc();
    Q3DStudio::ScopedDocumentEditor editor(*theDoc, L"Delete Channel Keyframes", __FILE__,
                                           __LINE__);
    for (TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.begin(),
         theEnd = m_PropertyBindingMap.end();
         theIter != theEnd; ++theIter) {
        theIter->second->DeleteAllKeys();
    }
}

long Qt3DSDMTimelineItemBinding::GetKeyframeCount() const
{
    // This list is updated when properties are loaded and when keyframes are added & deleted.
    return (long)m_Keyframes.size();
}

IKeyframe *Qt3DSDMTimelineItemBinding::GetKeyframeByTime(long inTime) const
{
    TAssetKeyframeList::const_iterator theIter = m_Keyframes.begin();
    for (; theIter != m_Keyframes.end(); ++theIter) {
        if ((*theIter).GetTime() == inTime)
            return const_cast<Qt3DSDMAssetTimelineKeyframe *>(&(*theIter));
    }
    return nullptr;
}

IKeyframe *Qt3DSDMTimelineItemBinding::GetKeyframeByIndex(long inIndex) const
{
    if (inIndex >= 0 && inIndex < (long)m_Keyframes.size())
        return const_cast<Qt3DSDMAssetTimelineKeyframe *>(&m_Keyframes[inIndex]);

    ASSERT(0); // should not happen
    return nullptr;
}

long Qt3DSDMTimelineItemBinding::OffsetSelectedKeyframes(long inOffset)
{
    return m_TransMgr->GetKeyframesManager()->OffsetSelectedKeyframes(inOffset);
}

void Qt3DSDMTimelineItemBinding::CommitChangedKeyframes()
{
    m_TransMgr->GetKeyframesManager()->CommitChangedKeyframes();
}

void Qt3DSDMTimelineItemBinding::OnEditKeyframeTime(long inCurrentTime, long inObjectAssociation)
{
    CTimeEditDlg theTimeEditDlg;
    theTimeEditDlg.SetKeyframesManager(m_TransMgr->GetKeyframesManager());
    theTimeEditDlg.ShowDialog(inCurrentTime, 0, g_StudioApp.GetCore()->GetDoc(),
                              inObjectAssociation);
}

void Qt3DSDMTimelineItemBinding::SelectKeyframes(bool inSelected, long inTime /*= -1 */)
{
    // Callback from UI, hence skip the UI update
    DoSelectKeyframes(inSelected, inTime, false);
}

Qt3DSDMInstanceHandle Qt3DSDMTimelineItemBinding::GetInstanceHandle() const
{
    return m_DataHandle;
}

long Qt3DSDMTimelineItemBinding::GetFlavor() const
{
    return QT3DS_FLAVOR_ASSET_TL;
}

void Qt3DSDMTimelineItemBinding::OnBeginDataModelNotifications()
{
}
void Qt3DSDMTimelineItemBinding::OnEndDataModelNotifications()
{
    RefreshStateRow();
}
void Qt3DSDMTimelineItemBinding::OnImmediateRefreshInstanceSingle(
        qt3dsdm::Qt3DSDMInstanceHandle inInstance)
{
    if (inInstance == m_DataHandle)
        RefreshStateRow(true);
}
void Qt3DSDMTimelineItemBinding::OnImmediateRefreshInstanceMultiple(
        qt3dsdm::Qt3DSDMInstanceHandle *inInstance, long inInstanceCount)
{
    for (long idx = 0; idx < inInstanceCount; ++idx)
        if (inInstance[idx] == m_DataHandle) {
            RefreshStateRow();
            break;
        }
}

void Qt3DSDMTimelineItemBinding::RefreshStateRow(bool inRefreshChildren)
{
    CStateRow *theRow = dynamic_cast<CStateRow *>(m_Row);
    if (theRow) {
        theRow->OnTimeChange();
        theRow->ClearDirty();
        if (inRefreshChildren) {
            long theChildrenCount = GetChildrenCount();
            for (long theIndex = 0; theIndex < theChildrenCount; ++theIndex) {
                ITimelineItemBinding *theChild = GetChild(theIndex);
                Qt3DSDMTimelineItemBinding *theBinding =
                        dynamic_cast<Qt3DSDMTimelineItemBinding *>(theChild);
                if (theBinding)
                    theBinding->RefreshStateRow(inRefreshChildren);
            }
        }
    }
}

ITimelineTimebar *Qt3DSDMTimelineItemBinding::CreateTimelineTimebar()
{
    return new Qt3DSDMTimelineTimebar(m_TransMgr, m_DataHandle);
}

ITimelineItemProperty *
Qt3DSDMTimelineItemBinding::GetPropertyBinding(Qt3DSDMPropertyHandle inPropertyHandle)
{
    TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.find(inPropertyHandle);
    // check if it already exists
    if (theIter != m_PropertyBindingMap.end())
        return theIter->second;
    return nullptr;
}

ITimelineItemProperty *
Qt3DSDMTimelineItemBinding::GetOrCreatePropertyBinding(Qt3DSDMPropertyHandle inPropertyHandle)
{
    ITimelineItemProperty *theProperty = GetPropertyBinding(inPropertyHandle);
    // check if it already exists
    if (theProperty)
        return theProperty;

    // Create
    Qt3DSDMTimelineItemProperty *theTimelineProperty =
            new Qt3DSDMTimelineItemProperty(m_TransMgr, inPropertyHandle, m_DataHandle);
    m_PropertyBindingMap.insert(std::make_pair(inPropertyHandle, theTimelineProperty));

    return theTimelineProperty;
}

//=============================================================================
/**
 * Add a new property row for this property.
 * @param inAppend true to skip the check to find where to insert. ( true if this is a
 * loading/initializing step, where the call is already done in order )
 */
void Qt3DSDMTimelineItemBinding::AddPropertyRow(Qt3DSDMPropertyHandle inPropertyHandle,
                                                bool inAppend /*= false */)
{
    ITimelineItemProperty *theTimelineProperty = GetPropertyBinding(inPropertyHandle);
    if (theTimelineProperty && theTimelineProperty->GetRow()) // if created, bail
        return;

    if (!theTimelineProperty)
        theTimelineProperty = GetOrCreatePropertyBinding(inPropertyHandle);

    // Find the row to insert this new property, if any, this preserves the order the property rows
    // is displayed in the timeline.
    ITimelineItemProperty *theNextProperty = nullptr;
    if (!inAppend) {
        TPropertyHandleList theProperties;
        m_StudioSystem->GetPropertySystem()->GetAggregateInstanceProperties(m_DataHandle,
                                                                            theProperties);
        size_t thePropertyIndex = 0;
        size_t thePropertyCount = theProperties.size();
        for (; thePropertyIndex < thePropertyCount; ++thePropertyIndex) {
            if (theProperties[thePropertyIndex] == inPropertyHandle) {
                ++thePropertyIndex;
                break;
            }
        }
        // Not all properties are displayed, so another loop to search for the first one that maps
        // to a existing propertyrow
        for (; thePropertyIndex < thePropertyCount; ++thePropertyIndex) {
            TPropertyBindingMap::iterator theNextPropIter =
                    m_PropertyBindingMap.find(theProperties[thePropertyIndex]);
            if (theNextPropIter != m_PropertyBindingMap.end()) {
                theNextProperty = theNextPropIter->second;
                break;
            }
        }
    }
    // Create a new property row
    m_TransMgr->CreateNewPropertyRow(theTimelineProperty, m_Row,
                                     theNextProperty ? theNextProperty->GetRow() : nullptr);

    // Update keyframes
    AddKeyframes(theTimelineProperty);
}

void Qt3DSDMTimelineItemBinding::RemovePropertyRow(Qt3DSDMPropertyHandle inPropertyHandle)
{
    TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.find(inPropertyHandle);
    if (theIter != m_PropertyBindingMap.end()) {
        ITimelineItemProperty *thePropertyBinding = theIter->second;

        bool theUpdateUI = DeleteAssetKeyframesWhereApplicable(thePropertyBinding);

        m_TransMgr->RemovePropertyRow(thePropertyBinding);
        m_PropertyBindingMap.erase(theIter);

        // UI must update
        if (m_Row && theUpdateUI) {
            m_Row->OnChildVisibilityChanged();
            m_Row->GetTimebar()->SetDirty(true);
        }
    }
}

// called when a keyframe is inserted, deleted or updated in the data model
void Qt3DSDMTimelineItemBinding::RefreshPropertyKeyframe(
        qt3dsdm::Qt3DSDMPropertyHandle inPropertyHandle, qt3dsdm::Qt3DSDMKeyframeHandle inKeyframe,
        ETimelineKeyframeTransaction inTransaction)
{
    TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.find(inPropertyHandle);
    if (theIter != m_PropertyBindingMap.end()) {
        Qt3DSDMTimelineItemProperty *theProperty = theIter->second;
        if (theProperty) {
            if (theProperty->RefreshKeyframe(inKeyframe, inTransaction)) {
                // Update asset keyframes
                UpdateKeyframe(theProperty->GetKeyframeByHandle(inKeyframe), inTransaction);
                if (m_Row) // UI update
                    m_Row->GetTimebar()->SetDirty(true);
            }
        }
    }
}

// called when the keyframes are updated in the UI and data model hasn't committed the change, ie no
// event callback from DataModel
void Qt3DSDMTimelineItemBinding::UIRefreshPropertyKeyframe(long inOffset)
{
    if (!m_Row)
        return;

    // TODO: figure out a better way to sync m_Keyframes
    TAssetKeyframeList::iterator theKeyIter = m_Keyframes.begin();
    for (; theKeyIter != m_Keyframes.end(); ++theKeyIter) {
        if (theKeyIter->IsSelected())
            theKeyIter->UpdateTime(theKeyIter->GetTime() + inOffset);
    }
    // If a asset keyframe was "shared" by several properties' keyframes
    // we need to 'break' this sharing and create for the remaining unmoved keyframes.
    TPropertyBindingMap::const_iterator theIter = m_PropertyBindingMap.begin();
    for (; theIter != m_PropertyBindingMap.end(); ++theIter) {
        (*theIter).second->RefreshKeyFrames();

        for (long i = 0; i < theIter->second->GetKeyframeCount(); ++i) {
            IKeyframe *theKeyframe = theIter->second->GetKeyframeByIndex(i);
            UpdateKeyframe(theKeyframe, ETimelineKeyframeTransaction_Add);

            // Unfortunately, this is the way we can propagate UI updates to ALL selected keyframes
            if (theKeyframe->IsSelected()) {
                CPropertyRow *thePropertyRow = theIter->second->GetRow();
                if (thePropertyRow)
                    thePropertyRow->GetTimebar()->SetDirty(true);
            }
        }
    }
    m_Row->GetTimebar()->SetDirty(true);
}

void Qt3DSDMTimelineItemBinding::OnPropertyChanged(Qt3DSDMPropertyHandle inPropertyHandle)
{ // Refresh UI
    CClientDataModelBridge *theBridge = m_StudioSystem->GetClientDataModelBridge();
    if (m_Row && (inPropertyHandle == theBridge->GetNameProperty()
                  || inPropertyHandle == theBridge->GetSceneAsset().m_Eyeball
                  || inPropertyHandle == theBridge->GetSceneAsset().m_Locked
                  || inPropertyHandle == theBridge->GetSceneAsset().m_Shy
                  || inPropertyHandle == theBridge->GetSceneAsset().m_StartTime
                  || inPropertyHandle == theBridge->GetSceneAsset().m_EndTime))
        m_Row->OnDirty();
}

void Qt3DSDMTimelineItemBinding::OnPropertyLinked(Qt3DSDMPropertyHandle inPropertyHandle)
{
    if (m_StudioSystem->GetAnimationSystem()->IsPropertyAnimated(m_DataHandle, inPropertyHandle)) {
        // Refresh property row by delete and recreate
        RemovePropertyRow(inPropertyHandle);
        AddPropertyRow(inPropertyHandle);
    }
}

bool Qt3DSDMTimelineItemBinding::HasDynamicKeyframes(long inTime)
{
    if (inTime == -1) {
        if (GetPropertyCount() == 0)
            return false;

        for (long i = 0; i < GetPropertyCount(); ++i) {
            ITimelineItemProperty *theTimelineItemProperty = GetProperty(i);
            if (!theTimelineItemProperty->IsDynamicAnimation())
                return false;
        }
        return true;
    } else {
        TPropertyBindingMap::const_iterator theIter = m_PropertyBindingMap.begin();
        for (; theIter != m_PropertyBindingMap.end(); ++theIter) {
            IKeyframe *theKeyframe = theIter->second->GetKeyframeByTime(inTime);
            if (theKeyframe && theKeyframe->IsDynamic())
                return true;
        }
    }
    return false;
}

void Qt3DSDMTimelineItemBinding::SetDynamicKeyframes(long inTime, bool inDynamic)
{
    TPropertyBindingMap::const_iterator theIter = m_PropertyBindingMap.begin();
    for (; theIter != m_PropertyBindingMap.end(); ++theIter) {
        IKeyframe *theKeyframe = theIter->second->GetKeyframeByTime(inTime);
        if (theKeyframe)
            theKeyframe->SetDynamic(inDynamic); // TODO: we want this in 1 batch command
    }
}

// Update UI on the selection state of all keyframes on this row and all its properties' keyframes.
void Qt3DSDMTimelineItemBinding::DoSelectKeyframes(bool inSelected, long inTime, bool inUpdateUI)
{
    if (inTime == -1) // all keyframes
    {
        TAssetKeyframeList::iterator theKeyIter = m_Keyframes.begin();
        for (; theKeyIter != m_Keyframes.end(); ++theKeyIter)
            theKeyIter->SetSelected(inSelected);
    } else {
        Qt3DSDMAssetTimelineKeyframe *theKeyframe =
                dynamic_cast<Qt3DSDMAssetTimelineKeyframe *>(GetKeyframeByTime(inTime));
        if (theKeyframe)
            theKeyframe->SetSelected(inSelected);
    }
    if (inUpdateUI && m_Row)
        m_Row->GetTimebar()->SelectKeysByTime(-1, inSelected);

    // legacy feature: all properties with keyframes at inTime or all if inTime is -1 are selected
    // as well.
    TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.begin();
    for (; theIter != m_PropertyBindingMap.end(); ++theIter)
        theIter->second->DoSelectKeyframes(inSelected, inTime, true, this);
}

// When selecting by mouse-drag, if all properties are selected, select the asset keyframe. And if
// one gets de-selected, de-select. Legacy feature.
// Note that if only 1 property has a keyframe at time t, the asset keyframe gets selected
// automatically when that keyframe is selected. Its odd to me but
// that's how it has always behaved.
void Qt3DSDMTimelineItemBinding::OnPropertySelection(long inTime)
{
    IKeyframe *theAssetKeyframe = GetKeyframeByTime(inTime);
    if (theAssetKeyframe) {
        bool theAllSelectedFlag = true;
        TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.begin();
        for (; theIter != m_PropertyBindingMap.end(); ++theIter) {
            IKeyframe *theKeyframe = theIter->second->GetKeyframeByTime(inTime);
            if (theKeyframe && !theKeyframe->IsSelected()) {
                // done, i.e selection remain unchanged.
                theAllSelectedFlag = false;
                break;
            }
        }
        if (theAssetKeyframe->IsSelected() != theAllSelectedFlag) {
            dynamic_cast<Qt3DSDMAssetTimelineKeyframe *>(theAssetKeyframe)
                    ->SetSelected(theAllSelectedFlag);
            // Update UI
            if (m_Row)
                m_Row->GetTimebar()->SelectKeysByTime(inTime, theAllSelectedFlag);
        }
    }
}

Q3DStudio::CId Qt3DSDMTimelineItemBinding::GetGuid() const
{
    CClientDataModelBridge *theClientBridge = m_StudioSystem->GetClientDataModelBridge();
    qt3dsdm::IPropertySystem *thePropertySystem = m_StudioSystem->GetPropertySystem();
    SValue theValue;
    if (thePropertySystem->GetInstancePropertyValue(m_DataHandle, theClientBridge->GetIdProperty(),
                                                    theValue)) {
        SLong4 theLong4 = qt3dsdm::get<SLong4>(theValue);
        return Q3DStudio::CId(theLong4.m_Longs[0], theLong4.m_Longs[1], theLong4.m_Longs[2],
                theLong4.m_Longs[3]);
    }
    return Q3DStudio::CId();
}

// Delete asset keyframes at time t if no property keyframes exist at time t
//@param inSkipPropertyBinding property that to skip, e.g. in cases where property is deleted
//@return true if there are asset keyframes deleted.
bool Qt3DSDMTimelineItemBinding::DeleteAssetKeyframesWhereApplicable(
        ITimelineItemProperty *inSkipPropertyBinding /*= nullptr */)
{
    // iterate through m_Keyframes because we cannot obtain time information from the Animation
    // keyframes anymore, since they are deleted.
    std::vector<long> theDeleteIndicesList;
    for (size_t theIndex = 0; theIndex < m_Keyframes.size(); ++theIndex) {
        TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.begin();
        for (; theIter != m_PropertyBindingMap.end(); ++theIter) {
            if ((!inSkipPropertyBinding || theIter->second != inSkipPropertyBinding)
                    && theIter->second->GetKeyframeByTime(m_Keyframes[theIndex].GetTime())) {
                // done!
                break;
            }
        }
        if (theIter == m_PropertyBindingMap.end())
            theDeleteIndicesList.push_back((long)theIndex);
    }
    // start with the last item, so that the indices remain valid.
    for (long i = (long)theDeleteIndicesList.size() - 1; i >= 0; --i) {
        TAssetKeyframeList::iterator theKeyIter = m_Keyframes.begin();
        std::advance(theKeyIter, theDeleteIndicesList[i]);
        m_Keyframes.erase(theKeyIter);
    }

    return !theDeleteIndicesList.empty();
}

void Qt3DSDMTimelineItemBinding::RemoveAllPropertyBindings()
{
    TPropertyBindingMap::iterator theIter = m_PropertyBindingMap.begin();
    for (; theIter != m_PropertyBindingMap.end(); ++theIter)
        delete theIter->second;
    m_PropertyBindingMap.clear();
}

void Qt3DSDMTimelineItemBinding::AddKeyframes(ITimelineItemProperty *inPropertyBinding)
{
    for (long i = 0; i < inPropertyBinding->GetKeyframeCount(); ++i)
        UpdateKeyframe(inPropertyBinding->GetKeyframeByIndex(i), ETimelineKeyframeTransaction_Add);
}

// Update the asset keyframes based on the properties' keyframes.
void Qt3DSDMTimelineItemBinding::UpdateKeyframe(IKeyframe *inKeyframe,
                                                ETimelineKeyframeTransaction inTransaction)
{
    bool theDoAddFlag = (inTransaction == ETimelineKeyframeTransaction_Add);
    bool theDoDeleteFlag = (inTransaction == ETimelineKeyframeTransaction_Delete);

    // For update, if there isn't already a asset keyframe at the associated time, create one
    if (inTransaction == ETimelineKeyframeTransaction_Update) {
        theDoAddFlag = inKeyframe && !GetKeyframeByTime(inKeyframe->GetTime());
        theDoDeleteFlag = true; // plus, since we don't keep track of indiviual property keyframes
        // here, iterate and make sure list is correct.
    }

    if (theDoDeleteFlag)
        DeleteAssetKeyframesWhereApplicable();

    // Add when a new keyframe is added or MAYBE when a keyframe is moved
    if (theDoAddFlag && inKeyframe) {
        long theKeyframeTime = inKeyframe->GetTime();
        if (theKeyframeTime >= 0) {
            bool theAppend = true;
            // insert this in the order that it should be. and we trust the
            TAssetKeyframeList::iterator theIter = m_Keyframes.begin();
            for (; theIter != m_Keyframes.end(); ++theIter) {
                long theTime = (*theIter).GetTime();
                if (theTime == theKeyframeTime) {
                    theAppend = false;
                    break; // already exists, we are done. Because we only need 1 to represent ALL
                    // properties
                }
            }
            if (theAppend)
                m_Keyframes.push_back(Qt3DSDMAssetTimelineKeyframe(this, theKeyframeTime));
        }
    }
    if (m_Row && (theDoAddFlag
                  || inTransaction == ETimelineKeyframeTransaction_DynamicChanged)) {
        // dynamic => only UI needs to refresh
        m_Row->GetTimebar()->SetDirty(true);
    }
}

void Qt3DSDMTimelineItemBinding::OnAddChild(Qt3DSDMInstanceHandle inInstance)
{
    CDoc *theDoc = m_TransMgr->GetDoc();
    CClientDataModelBridge *theBridge = m_StudioSystem->GetClientDataModelBridge();
    ISlideSystem *theSlideSystem = m_StudioSystem->GetSlideSystem();

    qt3dsdm::Qt3DSDMSlideHandle theSlide = theSlideSystem->GetAssociatedSlide(inInstance);
    if (theBridge->IsInActiveComponent(inInstance)
            && (theSlideSystem->IsMasterSlide(theSlide) || theSlide == theDoc->GetActiveSlide())) {
        // Only add if the asset is in the current active component, and it's a master asset or in
        // the current slide
        ITimelineItemBinding *theNextItem = nullptr;
        qt3dsdm::Qt3DSDMInstanceHandle theParentInstance = GetInstance();
        // Figure out where to insert this row, if applicable.
        // CAsset has a list of children, and not necessarily all are active in this slide (e.g.
        // non-master children)
        Q3DStudio::TIdentifier theNextChild = 0;
        if (theParentInstance.Valid()) {
            // Get the next prioritized child in the same slide
            Q3DStudio::CGraphIterator theChildren;
            GetAssetChildrenInSlide(theDoc, theParentInstance, theDoc->GetActiveSlide(),
                                    theChildren);
            theNextChild = GetSibling(inInstance, true, theChildren);
        }

        if (theNextChild != 0)
            theNextItem = m_TransMgr->GetOrCreate(theNextChild);

        m_Row->AddChildRow(m_TransMgr->GetOrCreate(inInstance), theNextItem);
    }
}

void Qt3DSDMTimelineItemBinding::OnDeleteChild(Qt3DSDMInstanceHandle inInstance)
{
    ITimelineItemBinding *theChild = m_TransMgr->GetOrCreate(inInstance);
    if (theChild) {
        m_Row->RemoveChildRow(theChild);
    }
}

void Qt3DSDMTimelineItemBinding::UpdateActionStatus()
{
    if (m_Row)
        m_Row->UpdateActionStatus();
}

//=============================================================================
/**
 *	Open the associated item as though it was double-clicked in explorer
 *	Respective subclasses (for example Image and Behavior) can call this function
 */
bool Qt3DSDMTimelineItemBinding::OpenSourcePathFile()
{
    // Get source path property value
    CClientDataModelBridge *theClientBridge = m_StudioSystem->GetClientDataModelBridge();
    qt3dsdm::IPropertySystem *thePropertySystem = m_StudioSystem->GetPropertySystem();
    SValue theValue;
    if (thePropertySystem->GetInstancePropertyValue(
                m_DataHandle, theClientBridge->GetSourcePathProperty(), theValue)) {
        // Open the respective file
        Q3DStudio::CFilePath theSourcePath(qt3dsdm::get<qt3dsdm::TDataStrPtr>(theValue)->GetData());
        Qt3DSFile theFile(m_TransMgr->GetDoc()->GetResolvedPathToDoc(theSourcePath));
        theFile.Execute();
        return true;
    }
    return false;
}
