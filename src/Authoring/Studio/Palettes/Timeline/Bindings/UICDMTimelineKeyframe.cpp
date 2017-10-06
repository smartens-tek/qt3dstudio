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

#include "stdafx.h"
#include "UICDMTimelineKeyframe.h"
#include "UICDMAnimation.h"
#include "CmdDataModelChangeKeyframe.h"
#include "CmdBatch.h"
#include "UICDMStudioSystem.h"
#include "OffsetKeyframesCommandHelper.h"

#include "Doc.h"
#include "StudioApp.h"
#include "Core.h"

using namespace UICDM;

// TODO: figure out if we can just use IDoc instead of CDoc
CUICDMTimelineKeyframe::CUICDMTimelineKeyframe(IDoc *inDoc)
    : m_Doc(dynamic_cast<CDoc *>(inDoc))
    , m_Selected(false)
{
}

CUICDMTimelineKeyframe::~CUICDMTimelineKeyframe()
{
}

bool CUICDMTimelineKeyframe::IsSelected() const
{
    return m_Selected;
}

float my_roundf(float r)
{
    return (r > 0.0f) ? floorf(r + 0.5f) : ceilf(r - 0.5f);
}

long CUICDMTimelineKeyframe::GetTime() const
{
    if (!m_KeyframeHandles.empty()) {
        IAnimationCore *theAnimationCore = m_Doc->GetStudioSystem()->GetAnimationCore();
        CUICDMKeyframeHandle theKeyframeHandle = *m_KeyframeHandles.begin();
        if (theAnimationCore->KeyframeValid(theKeyframeHandle)) {
            float theTimeinSecs =
                KeyframeTime(theAnimationCore->GetKeyframeData(theKeyframeHandle));
            // We always convert back and forth between between long and float.
            // This causes especially issues when we do comparisons
            return (long)my_roundf(theTimeinSecs * 1000);
        }
    }
    return -1; // keyframe was deleted, and data cannot be retrieved.
}

float CUICDMTimelineKeyframe::GetTimeInSecs(long inTime)
{
    float theTimeinSecs = static_cast<float>(inTime) / 1000.f;
    // round off to 4 decimal place to workaround precision issues
    // TODO: fix this, either all talk float OR long. choose one.
    theTimeinSecs = (float)(((theTimeinSecs + 0.00005) * 10000.0) / 10000.0f);
    return theTimeinSecs;
}

void CUICDMTimelineKeyframe::SetTime(const long inNewTime)
{
    float theTimeinSecs = GetTimeInSecs(inNewTime);
    CCmd *theCmd = nullptr;
    if (m_KeyframeHandles.size() == 1) {
        theCmd = new CCmdDataModelSetKeyframeTime(m_Doc, m_KeyframeHandles.front(), theTimeinSecs);
    } else { // more than 1 channel
        CCmdBatch *theBatch = new CCmdBatch(m_Doc);
        TKeyframeHandleList::iterator theIter = m_KeyframeHandles.begin();
        for (; theIter != m_KeyframeHandles.end(); ++theIter)
            theBatch->AddCommand(new CCmdDataModelSetKeyframeTime(m_Doc, *theIter, theTimeinSecs));
        theCmd = theBatch;
    }
    if (theCmd)
        m_Doc->GetCore()->ExecuteCommand(theCmd);

#ifdef _DEBUG
    // we have a precision issue from converting from long to float..
    IAnimationCore *theAnimationCore = m_Doc->GetStudioSystem()->GetAnimationCore();
    long theTest = static_cast<long>(
        KeyframeTime(theAnimationCore->GetKeyframeData(*m_KeyframeHandles.begin())) * 1000);
    ASSERT(inNewTime == theTest);
#endif
}

inline CUICDMAnimationHandle GetAnimationHandle(UICDM::IAnimationCore *inAnimationCore,
                                                const TKeyframeHandleList &inKeyframes)
{
    if (!inKeyframes.empty())
        return inAnimationCore->GetAnimationForKeyframe(inKeyframes[0]);
    return 0;
}

void CUICDMTimelineKeyframe::SetDynamic(bool inIsDynamic)
{
    if (!m_KeyframeHandles.empty()) {
        CUICDMAnimationHandle theAnimation =
            GetAnimationHandle(m_Doc->GetStudioSystem()->GetAnimationCore(), m_KeyframeHandles);
        if (theAnimation.Valid())
            m_Doc->GetCore()->ExecuteCommand(
                new CCmdDataModelChangeDynamicKeyframe(m_Doc, theAnimation, inIsDynamic));
    }
}

// Only the first key of a track can be dynamic.
bool CUICDMTimelineKeyframe::IsDynamic() const
{
    UICDM::IAnimationCore *theAnimationCore = m_Doc->GetStudioSystem()->GetAnimationCore();
    CUICDMAnimationHandle theAnimation = GetAnimationHandle(theAnimationCore, m_KeyframeHandles);
    if (theAnimation.Valid()) {
        SAnimationInfo theInfo = theAnimationCore->GetAnimationInfo(theAnimation);
        if (theInfo.m_DynamicFirstKeyframe) {
            TKeyframeHandleList theKeyframes;
            theAnimationCore->GetKeyframes(theAnimation, theKeyframes);
            if (!theKeyframes.empty()) // only true if track is dynamic and this is the first
                                       // keyframe. Might have to optimize because this is so
                                       // clunky.
                return (theKeyframes[0] == m_KeyframeHandles[0]);
        }
    }
    return false;
}

void CUICDMTimelineKeyframe::AddKeyframeHandle(UICDM::CUICDMKeyframeHandle inHandle)
{
    m_KeyframeHandles.push_back(inHandle);
}

bool CUICDMTimelineKeyframe::HasKeyframeHandle(UICDM::CUICDMKeyframeHandle inHandle) const
{
    TKeyframeHandleList::const_iterator theIter = m_KeyframeHandles.begin();
    for (; theIter != m_KeyframeHandles.end(); ++theIter) {
        if (*theIter == inHandle)
            return true;
    }
    return false;
}

void CUICDMTimelineKeyframe::SetSelected(bool inSelected)
{
    m_Selected = inSelected;
}

// For colors, there would be 3 keyframe handles
void CUICDMTimelineKeyframe::UpdateKeyframesTime(COffsetKeyframesCommandHelper *inCommandHelper,
                                                 long inTime)
{
    for (size_t i = 0; i < m_KeyframeHandles.size(); ++i)
        inCommandHelper->SetCommandTime(m_KeyframeHandles[i], inTime);
}

void CUICDMTimelineKeyframe::GetKeyframeHandles(TKeyframeHandleList &outList) const
{
    outList = m_KeyframeHandles;
}

void CompareAndSet(CUICDMKeyframeHandle inKeyframe, IAnimationCore *inAnimationCore,
                   float &outRetValue, bool inGreaterThan)
{
    TKeyframe theKeyframeData = inAnimationCore->GetKeyframeData(inKeyframe);
    float theValue = KeyframeValueValue(theKeyframeData);
    if ((inGreaterThan && theValue > outRetValue) || (!inGreaterThan && theValue < outRetValue))
        outRetValue = theValue;
}

float CUICDMTimelineKeyframe::GetMaxValue() const
{
    IAnimationCore *theAnimationCore = m_Doc->GetStudioSystem()->GetAnimationCore();
    float theRetVal = FLT_MIN;
    do_all(m_KeyframeHandles,
           std::bind(CompareAndSet, std::placeholders::_1, theAnimationCore,
                     std::ref(theRetVal), true));
    return theRetVal;
}

float CUICDMTimelineKeyframe::GetMinValue() const
{
    IAnimationCore *theAnimationCore = m_Doc->GetStudioSystem()->GetAnimationCore();
    float theRetVal = FLT_MAX;
    do_all(m_KeyframeHandles,
           std::bind(CompareAndSet, std::placeholders::_1, theAnimationCore,
                     std::ref(theRetVal), false));
    return theRetVal;
}
