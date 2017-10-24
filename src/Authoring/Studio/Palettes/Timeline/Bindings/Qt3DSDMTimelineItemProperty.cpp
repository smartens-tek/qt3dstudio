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
#include "Qt3DSDMTimelineItemProperty.h"
#include "PropertyRow.h"
#include "TimelineTranslationManager.h"
#include "ITimelineItemBinding.h"
#include "PropertyTimebarRow.h"
#include "Qt3DSDMTimelineItemBinding.h"
#include "Qt3DSDMTimelineKeyframe.h"
#include "KeyframesManager.h"
#include "CmdDataModelChangeKeyframe.h"
#include "CmdDataModelRemoveKeyframe.h"
#include "StudioApp.h"
#include "Core.h"

// Link to data model
#include "TimeEditDlg.h"
#include "ClientDataModelBridge.h"
#include "Qt3DSDMSlides.h"
#include "Qt3DSDMStudioSystem.h"
#include "Qt3DSDMAnimation.h"
#include "Qt3DSDMMetaData.h"
#include "Qt3DSDMPropertyDefinition.h"
#include "Qt3DSDMDataCore.h"
#include "StudioFullSystem.h"
#include <boost/bind.hpp>
using namespace qt3dsdm;

bool SortKeyframeByTime(const Qt3DSDMTimelineKeyframe *inLHS, const Qt3DSDMTimelineKeyframe *inRHS)
{
    return inLHS->GetTime() < inRHS->GetTime();
}

// DataModel stores it from 0..1, UI expects 0..255
inline float DataModelToColor(float inValue)
{
    return inValue * 255;
}

Qt3DSDMTimelineItemProperty::Qt3DSDMTimelineItemProperty(CTimelineTranslationManager *inTransMgr,
                                                       Qt3DSDMPropertyHandle inPropertyHandle,
                                                       Qt3DSDMInstanceHandle inInstance)
    : m_Row(nullptr)
    , m_InstanceHandle(inInstance)
    , m_PropertyHandle(inPropertyHandle)
    , m_TransMgr(inTransMgr)
    , m_SetKeyframeValueCommand(nullptr)
{
    // Cache all the animation handles because we need them for any keyframes manipulation.
    // the assumption is that all associated handles are created all at once (i.e. we do not need to
    // add or delete from this list )
    CreateKeyframes();
    InitializeCachedVariables(inInstance);
    m_Signals.push_back(
        m_TransMgr->GetStudioSystem()->GetFullSystem()->GetSignalProvider()->ConnectPropertyLinked(
            boost::bind(&Qt3DSDMTimelineItemProperty::OnPropertyLinkStatusChanged, this, _1, _2,
                        _3)));

    m_Signals.push_back(
        m_TransMgr->GetStudioSystem()
            ->GetFullSystem()
            ->GetSignalProvider()
            ->ConnectPropertyUnlinked(boost::bind(
                &Qt3DSDMTimelineItemProperty::OnPropertyLinkStatusChanged, this, _1, _2, _3)));
}

Qt3DSDMTimelineItemProperty::~Qt3DSDMTimelineItemProperty()
{
    ReleaseKeyframes();
    Release();
}

void Qt3DSDMTimelineItemProperty::CreateKeyframes()
{
    // Cache all the animation handles because we need them for any keyframes manipulation.
    // the assumption is that all associated handles are created all at once (i.e. we do not need to
    // add or delete from this list )
    qt3dsdm::IPropertySystem *thePropertySystem = m_TransMgr->GetStudioSystem()->GetPropertySystem();
    DataModelDataType::Value theDataType = thePropertySystem->GetDataType(m_PropertyHandle);
    IStudioAnimationSystem *theAnimationSystem =
        m_TransMgr->GetStudioSystem()->GetAnimationSystem();
    std::tuple<bool, size_t> theArity = GetDatatypeAnimatableAndArity(theDataType);
    for (size_t i = 0; i < std::get<1>(theArity); ++i) {
        Qt3DSDMAnimationHandle theAnimationHandle =
            theAnimationSystem->GetControllingAnimation(m_InstanceHandle, m_PropertyHandle, i);
        if (theAnimationHandle.Valid())
            m_AnimationHandles.push_back(theAnimationHandle);
    }
    if (!m_AnimationHandles.empty()) { // update wrappers for keyframes
        IAnimationCore *theAnimationCore = m_TransMgr->GetStudioSystem()->GetAnimationCore();
        TKeyframeHandleList theKeyframes;
        // all channels have keyframes at the same time
        theAnimationCore->GetKeyframes(m_AnimationHandles[0], theKeyframes);
        for (size_t i = 0; i < theKeyframes.size(); ++i)
            CreateKeyframeIfNonExistent(theKeyframes[i], m_AnimationHandles[0]);
    }
}

void Qt3DSDMTimelineItemProperty::ReleaseKeyframes()
{
    // clear any selection from m_TransMgr
    TKeyframeList::iterator theIter = m_Keyframes.begin();
    for (; theIter != m_Keyframes.end(); ++theIter) {
        m_TransMgr->GetKeyframesManager()->SetKeyframeSelected(*theIter, false);

        SAFE_DELETE(*theIter);
    }
    m_Keyframes.clear();
    m_AnimationHandles.clear();
}

// Type doesn't change and due to the logic required to figure this out, cache it.
void Qt3DSDMTimelineItemProperty::InitializeCachedVariables(qt3dsdm::Qt3DSDMInstanceHandle inInstance)
{
    using namespace Q3DStudio;
    qt3dsdm::IPropertySystem *thePropertySystem = m_TransMgr->GetStudioSystem()->GetPropertySystem();

    m_Type.first = thePropertySystem->GetDataType(m_PropertyHandle);
    m_Type.second = thePropertySystem->GetAdditionalMetaDataType(inInstance, m_PropertyHandle);

    // Name doesn't change either.
    TCharStr theFormalName = thePropertySystem->GetFormalName(inInstance, m_PropertyHandle);

    if (theFormalName.empty()) // fallback on property name
        theFormalName = thePropertySystem->GetName(m_PropertyHandle);
    m_Name = theFormalName.c_str();
}

Q3DStudio::CString Qt3DSDMTimelineItemProperty::GetName() const
{
    return m_Name;
}

// Helper function to retrieve the parent binding class.
inline ITimelineItemBinding *GetParentBinding(CPropertyRow *inRow)
{
    ITimelineItemBinding *theParentBinding = nullptr;
    if (inRow) {
        CBaseStateRow *theParentRow = inRow->GetParentRow();
        if (theParentRow) {
            theParentBinding = theParentRow->GetTimelineItemBinding();
            ASSERT(theParentBinding); // TimelineItemBinding should be set properly during
                                      // CBaseStateRow::Initialize
        }
    }
    return theParentBinding;
}

bool Qt3DSDMTimelineItemProperty::IsMaster() const
{
    if (m_Row) {
        if (Qt3DSDMTimelineItemBinding *theParentBinding =
                dynamic_cast<Qt3DSDMTimelineItemBinding *>(GetParentBinding(m_Row)))
            return m_TransMgr->GetDoc()->GetDocumentReader().IsPropertyLinked(
                theParentBinding->GetInstanceHandle(), m_PropertyHandle);
    }
    return false;
}

qt3dsdm::TDataTypePair Qt3DSDMTimelineItemProperty::GetType() const
{
    return m_Type;
}

void CompareAndSet(const Qt3DSDMTimelineKeyframe *inKeyframe, float &outRetValue, bool inGreaterThan)
{
    float theValue = (inGreaterThan) ? inKeyframe->GetMaxValue() : inKeyframe->GetMinValue();
    if ((inGreaterThan && theValue > outRetValue) || (!inGreaterThan && theValue < outRetValue))
        outRetValue = theValue;
}

// return the max value of the current set of keyframes
float Qt3DSDMTimelineItemProperty::GetMaximumValue() const
{
    float theRetVal = FLT_MIN;
    do_all(m_Keyframes, boost::bind(CompareAndSet, _1, boost::ref(theRetVal), true));
    if (m_Type.first == DataModelDataType::Float3 && m_Type.second == AdditionalMetaDataType::Color)
        theRetVal = DataModelToColor(theRetVal);
    return theRetVal;
}

// return the min value of the current set of keyframes
float Qt3DSDMTimelineItemProperty::GetMinimumValue() const
{
    float theRetVal = FLT_MAX;
    do_all(m_Keyframes, boost::bind(CompareAndSet, _1, boost::ref(theRetVal), false));
    if (m_Type.first == DataModelDataType::Float3 && m_Type.second == AdditionalMetaDataType::Color)
        theRetVal = DataModelToColor(theRetVal);
    return theRetVal;
}

void Qt3DSDMTimelineItemProperty::Bind(CPropertyRow *inRow)
{
    ASSERT(!m_Row);

    m_Row = inRow;
}

void Qt3DSDMTimelineItemProperty::Release()
{
    m_Row = nullptr;
}

// Ensures the object that owns this property is selected.
void Qt3DSDMTimelineItemProperty::SetSelected()
{
    if (m_Row) {
        ITimelineItemBinding *theParentBinding = GetParentBinding(m_Row);
        if (theParentBinding)
            theParentBinding->SetSelected(false);
    }
}

void Qt3DSDMTimelineItemProperty::ClearKeySelection()
{
    m_TransMgr->ClearKeyframeSelection();
}

void Qt3DSDMTimelineItemProperty::DeleteAllKeys()
{
    if (m_Keyframes.empty())
        return;

    using namespace Q3DStudio;

    ScopedDocumentEditor editor(*m_TransMgr->GetDoc(), L"Delete All Keyframes", __FILE__, __LINE__);
    for (size_t idx = 0, end = m_AnimationHandles.size(); idx < end; ++idx)
        editor->DeleteAllKeyframes(m_AnimationHandles[idx]);
}

ITimelineKeyframesManager *Qt3DSDMTimelineItemProperty::GetKeyframesManager() const
{
    return m_TransMgr->GetKeyframesManager();
}

IKeyframe *Qt3DSDMTimelineItemProperty::GetKeyframeByTime(long inTime) const
{
    std::vector<long> theTest;
    TKeyframeList::const_iterator theIter = m_Keyframes.begin();
    for (; theIter != m_Keyframes.end(); ++theIter) {
        if ((*theIter)->GetTime() == inTime)
            return (*theIter);

        theTest.push_back((*theIter)->GetTime());
    }
    // if key had been deleted, this returns nullptr
    return nullptr;
}

IKeyframe *Qt3DSDMTimelineItemProperty::GetKeyframeByIndex(long inIndex) const
{
    if (inIndex >= 0 && inIndex < (long)m_Keyframes.size())
        return m_Keyframes[inIndex];

    ASSERT(0); // should not happen
    return nullptr;
}

long Qt3DSDMTimelineItemProperty::GetKeyframeCount() const
{
    // this list is updated in constructor and when keyframes are added or deleted.
    return (long)m_Keyframes.size();
}

long Qt3DSDMTimelineItemProperty::GetChannelCount() const
{
    return (long)m_AnimationHandles.size();
}

float Qt3DSDMTimelineItemProperty::GetChannelValueAtTime(long inChannelIndex, long inTime)
{
    // if no keyframes, get current property value.
    if (m_Keyframes.empty()) {
        Qt3DSDMTimelineItemBinding *theParentBinding =
            dynamic_cast<Qt3DSDMTimelineItemBinding *>(GetParentBinding(m_Row));
        if (theParentBinding) {

            SValue theValue;
            qt3dsdm::IPropertySystem *thePropertySystem =
                m_TransMgr->GetStudioSystem()->GetPropertySystem();
            thePropertySystem->GetInstancePropertyValue(theParentBinding->GetInstanceHandle(),
                                                        m_PropertyHandle, theValue);
            switch (m_Type.first) {
            case DataModelDataType::Float3: {
                if (m_Type.second == AdditionalMetaDataType::Color) {
                    SFloat3 theFloat3 = qt3dsdm::get<SFloat3>(theValue);
                    if (inChannelIndex >= 0 && inChannelIndex < 3)
                        return DataModelToColor(theFloat3[inChannelIndex]);
                } else {
                    SFloat3 theFloat3 = qt3dsdm::get<SFloat3>(theValue);
                    if (inChannelIndex >= 0 && inChannelIndex < 3)
                        return theFloat3[inChannelIndex];
                }
                break;
            }
            case DataModelDataType::Float2: {
                SFloat2 theFloat2 = qt3dsdm::get<SFloat2>(theValue);
                if (inChannelIndex >= 0 && inChannelIndex < 2)
                    return theFloat2[inChannelIndex];
                break;
            }
            case DataModelDataType::Float:
                return qt3dsdm::get<float>(theValue);
                break;
            default: // TODO: handle other types
                break;
            }
        }
    }
    IAnimationCore *theAnimationCore = m_TransMgr->GetStudioSystem()->GetAnimationCore();
    if (!m_AnimationHandles.empty() && inChannelIndex >= 0
        && inChannelIndex < (long)m_AnimationHandles.size()) {
        float theValue = theAnimationCore->EvaluateAnimation(
            m_AnimationHandles[inChannelIndex], Qt3DSDMTimelineKeyframe::GetTimeInSecs(inTime));
        if (m_Type.first == DataModelDataType::Float3
            && m_Type.second == AdditionalMetaDataType::Color)
            theValue = DataModelToColor(theValue);

        return theValue;
    }
    return 0.f;
}

void Qt3DSDMTimelineItemProperty::SetChannelValueAtTime(long inChannelIndex, long inTime,
                                                       float inValue)
{
    using namespace boost;
    Qt3DSDMTimelineKeyframe *theKeyframeWrapper =
        dynamic_cast<Qt3DSDMTimelineKeyframe *>(GetKeyframeByTime(inTime));
    if (theKeyframeWrapper) {
        Qt3DSDMTimelineKeyframe::TKeyframeHandleList theKeyframes;
        theKeyframeWrapper->GetKeyframeHandles(theKeyframes);
        if (!theKeyframes.empty() && inChannelIndex < (long)theKeyframes.size()) {
            inValue /= 255;
            if (!m_SetKeyframeValueCommand)
                m_SetKeyframeValueCommand = new CCmdDataModelSetKeyframeValue(
                    g_StudioApp.GetCore()->GetDoc(), theKeyframes[inChannelIndex], inValue);
            m_SetKeyframeValueCommand->Update(inValue);
        }
    }
}

long Qt3DSDMTimelineItemProperty::OffsetSelectedKeyframes(long inOffset)
{
    long theRetVal = m_TransMgr->GetKeyframesManager()->OffsetSelectedKeyframes(inOffset);
    if (m_Row) // UI update, since the data model sends no event while the change isn't commited.
    {
        m_Row->Refresh();
    }
    return theRetVal;
}

void Qt3DSDMTimelineItemProperty::CommitChangedKeyframes()
{
    if (m_SetKeyframeValueCommand) { // if this is moving a keyframe value
        g_StudioApp.GetCore()->ExecuteCommand(m_SetKeyframeValueCommand, false);
        m_SetKeyframeValueCommand = nullptr;
    } else // otherwise its changing keyframe times
        m_TransMgr->GetKeyframesManager()->CommitChangedKeyframes();
}

void Qt3DSDMTimelineItemProperty::OnEditKeyframeTime(long inCurrentTime, long inObjectAssociation)
{
    (void)inObjectAssociation;
    CTimeEditDlg theTimeEditDlg;
    theTimeEditDlg.SetKeyframesManager(m_TransMgr->GetKeyframesManager());
    theTimeEditDlg.ShowDialog(inCurrentTime, 0, g_StudioApp.GetCore()->GetDoc(), ASSETKEYFRAME);
}

void Qt3DSDMTimelineItemProperty::SelectKeyframes(bool inSelected, long inTime /*= -1 */)
{
    Qt3DSDMTimelineItemBinding *theParent =
        dynamic_cast<Qt3DSDMTimelineItemBinding *>(GetParentBinding(m_Row));
    DoSelectKeyframes(inSelected, inTime, false, theParent);
}

CPropertyRow *Qt3DSDMTimelineItemProperty::GetRow()
{
    return m_Row;
}

bool Qt3DSDMTimelineItemProperty::IsDynamicAnimation()
{
    return m_Keyframes.size() > 0 && m_Keyframes[0]->IsDynamic();
}

//=============================================================================
/**
 * For updating the UI when keyframes are added/updated/deleted.
 */
bool Qt3DSDMTimelineItemProperty::RefreshKeyframe(qt3dsdm::Qt3DSDMKeyframeHandle inKeyframe,
                                                 ETimelineKeyframeTransaction inTransaction)
{
    bool theHandled = false;
    switch (inTransaction) {
    case ETimelineKeyframeTransaction_Delete: {
        TKeyframeList::iterator theIter = m_Keyframes.begin();
        for (; theIter != m_Keyframes.end(); ++theIter) {
            Qt3DSDMTimelineKeyframe *theKeyframe = *theIter;
            if (theKeyframe->HasKeyframeHandle(inKeyframe)) { // clear selection
                m_TransMgr->GetKeyframesManager()->SetKeyframeSelected(theKeyframe, false);
                m_Keyframes.erase(theIter);

                theHandled = true;
                break;
            }
        }
    } break;
    case ETimelineKeyframeTransaction_Add: {
        ASSERT(!m_AnimationHandles.empty());
        IAnimationCore *theAnimationCore = m_TransMgr->GetStudioSystem()->GetAnimationCore();
        Qt3DSDMAnimationHandle theAnimationHandle =
            theAnimationCore->GetAnimationForKeyframe(inKeyframe);
        // only create for the first animation handle.
        if (theAnimationHandle == m_AnimationHandles[0]) { // for undo/redo, the keyframes can be
                                                           // added in reverse, hence the need to
                                                           // sort
            if (CreateKeyframeIfNonExistent(inKeyframe, theAnimationHandle))
                std::stable_sort(m_Keyframes.begin(), m_Keyframes.end(), SortKeyframeByTime);
            theHandled = true;
        }
    } break;
    case ETimelineKeyframeTransaction_Update:
    case ETimelineKeyframeTransaction_DynamicChanged:
        theHandled = true;
        break;
    default:
        return false;
    }
    if (theHandled && m_Row)
        m_Row->Refresh();

    return theHandled;
}

IKeyframe *Qt3DSDMTimelineItemProperty::GetKeyframeByHandle(qt3dsdm::Qt3DSDMKeyframeHandle inKeyframe)
{
    TKeyframeList::iterator theIter = m_Keyframes.begin();
    for (; theIter != m_Keyframes.end(); ++theIter) {
        Qt3DSDMTimelineKeyframe *theKeyframe = *theIter;
        if (theKeyframe->HasKeyframeHandle(inKeyframe))
            return *theIter;
    }
    return nullptr;
}

// This is either triggered from this property's keyframe selection OR from a parent's keyframe
// selection.
void Qt3DSDMTimelineItemProperty::DoSelectKeyframes(bool inSelected, long inTime,
                                                   bool inParentTriggered,
                                                   Qt3DSDMTimelineItemBinding *inParent)
{
    // this is what it used to do before the refactor. selecting a keyframe always selects the
    // asset.
    if (inSelected)
        SetSelected();

    if (!inParent)
        return;

    if (inTime == -1) // all keyframes
    {
        TKeyframeList::iterator theIter = m_Keyframes.begin();
        for (; theIter != m_Keyframes.end(); ++theIter) {
            (*theIter)->SetSelected(inSelected);
            m_TransMgr->GetKeyframesManager()->SetKeyframeSelected(*theIter, inSelected, inParent);
        }
    } else {
        Qt3DSDMTimelineKeyframe *theKeyframe =
            dynamic_cast<Qt3DSDMTimelineKeyframe *>(GetKeyframeByTime(inTime));
        if (theKeyframe) {
            theKeyframe->SetSelected(inSelected);
            m_TransMgr->GetKeyframesManager()->SetKeyframeSelected(theKeyframe, inSelected,
                                                                   inParent);
        }
    }
    // Requires UI to be updated explicitly
    if (inParentTriggered && m_Row)
        m_Row->GetTimebar()->SelectKeysByTime(inTime, inSelected);

    // Support existing feature, selection by mouse-drag a rect, when all property keyframes are
    // selected, the asset keyframe is automatically selected as well.
    // and the mouse drags 'outside' a keyframe and de-selecting it, the asset keyframe has to be
    // deselected as well.
    if (!inParentTriggered && inTime != -1)
        inParent->OnPropertySelection(inTime);
}

//=============================================================================
/**
 * Create a wrapper for this keyframe if doesn't exists.
 * @return true if created, false if already exists.
 */
bool Qt3DSDMTimelineItemProperty::CreateKeyframeIfNonExistent(
    qt3dsdm::Qt3DSDMKeyframeHandle inKeyframeHandle, Qt3DSDMAnimationHandle inOwningAnimation)
{
    TKeyframeList::iterator theIter = m_Keyframes.begin();
    for (; theIter != m_Keyframes.end(); ++theIter) {
        Qt3DSDMTimelineKeyframe *theKeyframe = *theIter;
        if (theKeyframe->HasKeyframeHandle(inKeyframeHandle))
            return false;
    }
    // check for multiple channels => only create 1 Qt3DSDMTimelineKeyframe
    Qt3DSDMTimelineKeyframe *theNewKeyframe =
        new Qt3DSDMTimelineKeyframe(g_StudioApp.GetCore()->GetDoc());
    theNewKeyframe->AddKeyframeHandle(inKeyframeHandle);
    if (m_AnimationHandles.size()
        > 1) { // assert assumption that is only called for the first handle
        ASSERT(m_AnimationHandles[0] == inOwningAnimation);
        IAnimationCore *theAnimationCore = m_TransMgr->GetStudioSystem()->GetAnimationCore();
        float theKeyframeTime = KeyframeTime(theAnimationCore->GetKeyframeData(inKeyframeHandle));
        for (size_t i = 1; i < m_AnimationHandles.size(); ++i) {
            TKeyframeHandleList theKeyframes;
            theAnimationCore->GetKeyframes(m_AnimationHandles[i], theKeyframes);
            // the data model ensures that there is only 1 keyframe created for a given time
            for (size_t theKeyIndex = 0; theKeyIndex < theKeyframes.size(); ++theKeyIndex) {
                float theValue =
                    KeyframeTime(theAnimationCore->GetKeyframeData(theKeyframes[theKeyIndex]));
                if (theValue == theKeyframeTime) {
                    theNewKeyframe->AddKeyframeHandle(theKeyframes[theKeyIndex]);
                    break;
                }
            }
        }
    }
    m_Keyframes.push_back(theNewKeyframe);
    return true;
}

void Qt3DSDMTimelineItemProperty::OnPropertyLinkStatusChanged(qt3dsdm::Qt3DSDMSlideHandle inSlide,
                                                             qt3dsdm::Qt3DSDMInstanceHandle inInstance,
                                                             qt3dsdm::Qt3DSDMPropertyHandle inProperty)
{
    if (inInstance == m_InstanceHandle && inProperty == m_PropertyHandle) {
        // Re-bind to keyframes because the ones we should be pointing to will have changed.
        ReleaseKeyframes();
        CreateKeyframes();
    }
}

void Qt3DSDMTimelineItemProperty::RefreshKeyFrames(void)
{
    std::stable_sort(m_Keyframes.begin(), m_Keyframes.end(), SortKeyframeByTime);
}