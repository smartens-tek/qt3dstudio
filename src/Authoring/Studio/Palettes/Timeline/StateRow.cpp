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

//==============================================================================
//	Prefix
//==============================================================================

#include "stdafx.h"

//==============================================================================
//	Includes
//==============================================================================
#include "StateRow.h"
#include "TimelineControl.h"
#include "TimelineTimelineLayout.h"
#include "ColorControl.h"
#include "ToggleControl.h"
#include "PropertyRow.h"
#include "StateTimebarRow.h"
#include "BaseTimebarlessRow.h"
#include "BaseTimelineTreeControl.h"
#include "Bindings/ITimelineItemBinding.h"

//=============================================================================
/**
 * Creates a new CStateRow for the Asset.
 * @param inParentRow the parent of this row.
 */
CStateRow::CStateRow(CBaseStateRow *inParentRow)
{
    m_ParentRow = inParentRow;
}

CStateRow::~CStateRow()
{
}

CBlankToggleControl *CStateRow::CreateToggleControl()
{
    return (m_TimelineItemBinding->ShowToggleControls())
        ? new CToggleControl(this, m_TimelineItemBinding)
        : CBaseStateRow::CreateToggleControl();
}

//=============================================================================
/**
 * Create a new CStateTimebarRow.
 * This is virtual and used for objects to return their type specific
 * timebar rows if they want to.
 * @return the created timebar row.
 */
CBaseTimebarlessRow *CStateRow::CreateTimebarRow()
{
    return new CStateTimebarRow(this);
}

//=============================================================================
/**
 * Initialize this object.
 * This must be called after construction and may only be called once.
 */
void CStateRow::Initialize(ITimelineItemBinding *inTimelineItemBinding,
                           ISnappingListProvider *inProvider)
{
    CBaseStateRow::Initialize(inTimelineItemBinding);

    // cache these numbers. I believe caching these numbers is to avoid having to incur expensive
    // recursive calculations on ever draw.
    CalculateActiveStartTime();
    CalculateActiveEndTime();

    SetSnappingListProvider(inProvider);

    if (GetTimelineItem()->IsExpanded()) // this is stored for the current opened presentation and
                                         // conveniently help you remember the last view before you
                                         // switch slides.
        Expand();
    // sk - Delay loading till this is expanded. I think it makes more sense to not have to incur
    // work till the UI needs to be displayed
    //		Plus it would not work now since the parent always needs to be 'fully' initialized for the
    //SnappingListProvider, prior to any child creation.
    // else
    //	LoadChildren( );
}

//=============================================================================
/**
 * Expand this node of the tree control.
 * This will display all children the fit the filter.
 */
void CStateRow::Expand(bool inExpandAll /*= false*/, bool inExpandUp)
{
    // Only RecalcLayout if loaded children or expanded.
    bool theDoRecalLayout = !m_Loaded;

    if (!m_Loaded)
        LoadChildren();

    bool theWasExpanded = m_IsExpanded;
    CBaseStateRow::Expand(inExpandAll, inExpandUp);
    // Check if this is expanded
    theDoRecalLayout |= (theWasExpanded != m_IsExpanded);
    GetTimelineItem()->SetExpanded(
        m_IsExpanded); // remember this setting so that it persist when this row is recreated

    if (theDoRecalLayout)
        DoTimelineRecalcLayout();
}

//=============================================================================
/**
 * Collapse this node of the tree control.
 * This will hide all children of this control.
 */
void CStateRow::Collapse(bool inCollapseAll /* = false */)
{
    bool theWasExpanded = m_IsExpanded;
    CBaseStateRow::Collapse(inCollapseAll);

    GetTimelineItem()->SetExpanded(
        m_IsExpanded); // remember this setting so that it persist when this row is recreated
    // only RecalcLayout if this is collapsed
    if (theWasExpanded != m_IsExpanded)
        DoTimelineRecalcLayout();
}

bool CStateRow::PerformFilter(const CFilter &inFilter)
{
    return inFilter.Filter(m_TimelineItemBinding->GetTimelineItem());
}

//=============================================================================
/**
 * Set the indent of this control.
 * This controls how far to the right the toggle and text display on this
 * control. The indent should be increased for every level of sub-controls.
 * @param inIndent how much this control should be indented.
 */
void CStateRow::SetIndent(long inIndent)
{
    CTimelineRow::SetIndent(inIndent);

    m_TreeControl->SetIndent(inIndent);

    TStateRowList::iterator thePos = m_StateRows.begin();
    for (; thePos != m_StateRows.end(); ++thePos)
        (*thePos)->SetIndent(inIndent + CTimelineRow::TREE_INDENT);

    // For each property on this object
    TPropertyRowList::iterator thePropPos = m_PropertyRows.begin();
    for (; thePropPos != m_PropertyRows.end(); ++thePropPos) {
        CPropertyRow *thePropRow = (*thePropPos);
        if (thePropRow)
            thePropRow->SetIndent(inIndent + CTimelineRow::TREE_INDENT);
    }
}

bool CStateRow::HasVisibleChildren()
{
    if (!m_Loaded) {
        CTimelineItemOrderedIterator theChildren(m_TimelineItemBinding);
        // Return true if has children but do not load the children.
        if (!theChildren.IsDone()) {
            return true;
        }
        CTimelineItemPropertyIterator theProperties(m_TimelineItemBinding);
        if (!theProperties.IsDone()) {
            return true;
        }
    }
    return CBaseStateRow::HasVisibleChildren();
}

ISnappingListProvider *CStateRow::GetSnappingListProvider() const
{
    CStateTimebarRow *theTimebarControl = dynamic_cast<CStateTimebarRow *>(m_TimebarControl);
    return (theTimebarControl) ? &theTimebarControl->GetSnappingListProvider() : nullptr;
}

void CStateRow::SetSnappingListProvider(ISnappingListProvider *inProvider)
{
    CStateTimebarRow *theTimebarControl = dynamic_cast<CStateTimebarRow *>(m_TimebarControl);
    if (theTimebarControl)
        theTimebarControl->SetSnappingListProvider(inProvider);
}

//=============================================================================
/**
 * Trigger any external applications where applicable.
 */
void CStateRow::OnMouseDoubleClick(CPt inPoint, Qt::KeyboardModifiers inFlags)
{
    Q_UNUSED(inPoint);
    Q_UNUSED(inFlags);

    if (!m_TimelineItemBinding
             ->OpenAssociatedEditor()) // if not handled, fall backon the base class
        CBaseStateRow::OnMouseDoubleClick(inPoint, inFlags);
}

void CStateRow::OnTimeChange()
{
    CalculateActiveStartTime();
    CalculateActiveEndTime();

    // sk - I don't see the need to DoTimelineRecalcLayout here, because that is usually when height
    // of the control change
    //		this should just change width.. but maybe I am missing something, so I am leaving this
    //here for 'easy' debugging
    // DoTimelineRecalcLayout( );

    m_TimebarControl->UpdateTime(GetStartTime(), GetEndTime());

    GetTopControl()->OnLayoutChanged();
}

//=============================================================================
/**
 * calculate the active start time... this function set the active start to its
 * parent's start time if it comes after the objects start time
 */
bool CStateRow::CalculateActiveStartTime()
{
    long theRetVal = GetStartTime();

    if (m_ParentRow) {
        if (m_ParentRow->CalculateActiveStartTime()) {
            long theParentActiveStart = m_ParentRow->GetActiveStart();
            if (theParentActiveStart > theRetVal)
                theRetVal = theParentActiveStart;
        }
    }
    m_ActiveStart = theRetVal;
    return true;
}

//=============================================================================
/**
 * calculate the active end time... this function set the active end to its
 * parent's end time if it comes before the objects end time
 */
bool CStateRow::CalculateActiveEndTime()
{
    long theRetVal = GetEndTime();
    if (m_ParentRow) {
        if (m_ParentRow->CalculateActiveEndTime()) {
            long theParentActiveEnd = m_ParentRow->GetActiveEnd();
            if (theParentActiveEnd < theRetVal)
                theRetVal = theParentActiveEnd;
        }
    }
    m_ActiveEnd = theRetVal;
    return true;
}

//==============================================================================
/**
 *
 */
long CStateRow::GetLatestEndTime()
{
    if (m_IsExpanded) // if its children are not visible, they do not have any affect
        return CBaseStateRow::GetLatestEndTime();

    return GetActiveEnd();
}

//=============================================================================
/**
 * Load all the properties on this object.
 */
void CStateRow::LoadProperties()
{
    m_TimelineItemBinding->LoadProperties();
}

//==============================================================================
/**
 *	Tells the timeline timeline layout to recalc its layout.  Should only be called
 *  from this class, that's why it's protected.
 */
void CStateRow::DoTimelineRecalcLayout()
{
    GetTopControl()->OnLayoutChanged();
}