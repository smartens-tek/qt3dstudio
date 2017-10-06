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
#include "GraphUtils.h"
#include "Doc.h"
#include "UICDMStudioSystem.h"
#include "ClientDataModelBridge.h"
#include "UICDMSlides.h"

//==============================================================================
//	Filters
//==============================================================================

//==============================================================================
/**
 *	Excludes those which are not inGraphableType
 */
bool FilterForGraphableType(CDoc * /*inDoc*/, EGraphableType inGraphableType,
                            Q3DStudio::TIdentifier /*inGraphable*/)
{
    EGraphableType theGraphableType = EGRAPHABLE_TYPE_ASSET;
    return theGraphableType != inGraphableType;
}

//==============================================================================
/**
 *	Excludes those which are not inGraphableType
 */
const Q3DStudio::TFilter FilterForGraphableType(CDoc *inDoc, EGraphableType inGraphableType)
{
    return boost::bind(FilterForGraphableType, inDoc, inGraphableType, _1);
}

//==============================================================================
/**
 *	Excludes those which are not inAssetType
 */
bool FilterForAssetType(CDoc *inDoc, EStudioObjectType inObjectType,
                        Q3DStudio::TIdentifier inGraphable)
{
    return inDoc->GetStudioSystem()->GetClientDataModelBridge()->GetObjectType(inGraphable)
        != inObjectType;
}

//==============================================================================
/**
 *	Excludes those which are not inAssetType
 */
const Q3DStudio::TFilter FilterForAssetType(CDoc *inDoc, EStudioObjectType inObjectType)
{
    return boost::bind(FilterForAssetType, inDoc, inObjectType, _1);
}

//==============================================================================
/**
 *	Excludes those which are inAssetType
 */
bool FilterAwayAssetType(CDoc *inDoc, EStudioObjectType inObjectType,
                         Q3DStudio::TIdentifier inGraphable)
{
    return inDoc->GetStudioSystem()->GetClientDataModelBridge()->GetObjectType(inGraphable)
        == inObjectType;
}

//==============================================================================
/**
 *	Excludes those which are inAssetType
 */
const Q3DStudio::TFilter FilterAwayAssetType(CDoc *inDoc, EStudioObjectType inObjectType)
{
    return boost::bind(FilterAwayAssetType, inDoc, inObjectType, _1);
}

//==============================================================================
/**
 *	Excludes those which are not node. This compares the high bit of the type ID
 */
bool FilterForNodeType(CDoc *inDoc, Q3DStudio::TIdentifier inGraphable)
{
    return !inDoc->GetStudioSystem()->GetClientDataModelBridge()->IsNodeType(inGraphable);
}

//==============================================================================
/**
 *	Excludes those which are not node. This compares the high bit of the type ID
 */
const Q3DStudio::TFilter FilterForNodeType(CDoc *inDoc)
{
    return boost::bind(FilterForNodeType, inDoc, _1);
}

//==============================================================================
/**
 *	Excludes those who doesn't exist in SlideIndex
 */
bool FilterForSlide(CDoc *inDoc, UICDM::CUICDMSlideHandle inSlide,
                    Q3DStudio::TIdentifier inGraphable)
{
    try {
        UICDM::ISlideSystem *theSlideSystem = inDoc->GetStudioSystem()->GetSlideSystem();
        UICDM::CUICDMSlideHandle theSlide = theSlideSystem->GetAssociatedSlide(inGraphable);
        return !(theSlideSystem->IsMasterSlide(theSlide) || inSlide == theSlide);
    } catch (UICDM::SlideNotFound &error) {
        Q_UNUSED(error);
        return false;
    }
}

//==============================================================================
/**
 *	Excludes those who doesn't exist in SlideIndex
 */
const Q3DStudio::TFilter FilterForSlide(CDoc *inDoc, UICDM::CUICDMSlideHandle inSlide)
{
    return boost::bind(FilterForSlide, inDoc, inSlide, _1);
}

//==============================================================================
/**
 *	Excludes those whose controlling timeparent are different from inControllingTimeParent
 */
bool FilterForControllingTimeParent(CDoc *inDoc,
                                    UICDM::CUICDMInstanceHandle inControllingTimeParent,
                                    Q3DStudio::TIdentifier inGraphable)
{
    return inDoc->GetStudioSystem()->GetClientDataModelBridge()->GetOwningComponentInstance(
               static_cast<UICDM::CUICDMInstanceHandle>(inGraphable))
        != inControllingTimeParent;
}

//==============================================================================
/**
 *	Excludes those whose controlling timeparent are different from inControllingTimeParent
 */
const Q3DStudio::TFilter
FilterForControllingTimeParent(CDoc *inDoc, UICDM::CUICDMInstanceHandle inControllingTimeParent)
{
    return boost::bind(FilterForControllingTimeParent, inDoc, inControllingTimeParent, _1);
}

//==============================================================================
/**
 *	Excludes those that are not in active component
 */
bool FilterForActiveComponent(CDoc *inDoc, UICDM::CUICDMSlideHandle inActiveComponentSlide,
                              Q3DStudio::TIdentifier inGraphable)
{
    CClientDataModelBridge *theBridge = inDoc->GetStudioSystem()->GetClientDataModelBridge();
    UICDM::ISlideSystem *theSlideSystem = inDoc->GetStudioSystem()->GetSlideSystem();

    UICDM::CUICDMSlideHandle theSlide = theSlideSystem->GetAssociatedSlide(inGraphable);
    if (theBridge->IsInActiveComponent(inGraphable)
        && (theSlideSystem->IsMasterSlide(theSlide) || theSlide == inActiveComponentSlide))
        return false; // I'm in the active component and on the right slide, don't filter me!
    return true;
}

//==============================================================================
/**
 *	Excludes those that are not in active component
 */
const Q3DStudio::TFilter FilterForActiveComponent(CDoc *inDoc,
                                                  UICDM::CUICDMSlideHandle inActiveComponentSlide)
{
    return boost::bind(FilterForActiveComponent, inDoc, inActiveComponentSlide, _1);
}

//==============================================================================
/**
 *	Excludes those that don't exist in current slide
 */
bool FilterForCurrentSlide(CDoc *inDoc, UICDM::CUICDMInstanceHandle inTimeParent,
                           Q3DStudio::TIdentifier inGraphable)
{
    UICDM::ISlideSystem *theSlideSystem = inDoc->GetStudioSystem()->GetSlideSystem();

    UICDM::CUICDMSlideHandle theSlideHandle = theSlideSystem->GetAssociatedSlide(inGraphable);

    // Get the current slide index
    UICDM::CUICDMSlideHandle theCurrentSlide =
        inDoc->GetStudioSystem()->GetClientDataModelBridge()->GetComponentActiveSlide(inTimeParent);
    if (theSlideSystem->IsMasterSlide(theSlideHandle) || (theCurrentSlide == theSlideHandle))
        return false;
    else
        return true;
}

//==============================================================================
/**
 *	Excludes those that don't exist in current slide
 */
const Q3DStudio::TFilter FilterForCurrentSlide(CDoc *inDoc,
                                               UICDM::CUICDMInstanceHandle inTimeParent)
{
    return boost::bind(FilterForCurrentSlide, inDoc, inTimeParent, _1);
}

//==============================================================================
//	Helper functions to get children from the graph
//==============================================================================

inline void ApplyDefaultAssetCriterias(CDoc *inDoc, Q3DStudio::CGraphIterator &outIterator,
                                       EStudioObjectType inObjectType)
{
    outIterator.ClearCriterias();
    outIterator += FilterForGraphableType(inDoc, EGRAPHABLE_TYPE_ASSET);

    if (inObjectType != OBJTYPE_UNKNOWN)
        outIterator += FilterForAssetType(inDoc, inObjectType); // compare the asset type id
}

inline void GetChildrenFromGraph(Q3DStudio::TGraphPtr inGraph, Q3DStudio::TIdentifier inIdentifier,
                                 Q3DStudio::CGraphIterator &outIterator)
{
    if (inGraph->IsExist(inIdentifier))
        inGraph->GetChildren(outIterator, inIdentifier);
}

inline void GetReverseChildrenFromGraph(Q3DStudio::TGraphPtr inGraph,
                                        Q3DStudio::TIdentifier inIdentifier,
                                        Q3DStudio::CGraphIterator &outIterator)
{
    if (inGraph->IsExist(inIdentifier))
        inGraph->GetReverseChildren(outIterator, inIdentifier);
}

//==============================================================================
/**
 *	Returns an iterator for Asset Children
 *	This queries AssetGraph and applies filter to get only EGRAPHABLE_TYPE_ASSET children
 *	An optional filter is applied to get asset of a certain type
 */
void GetAssetChildren(CDoc *inDoc, UICDM::CUICDMInstanceHandle inInstance,
                      Q3DStudio::CGraphIterator &outIterator, EStudioObjectType inObjectType)
{
    TAssetGraphPtr theGraph = inDoc->GetAssetGraph();
    ApplyDefaultAssetCriterias(inDoc, outIterator, inObjectType);
    GetChildrenFromGraph(theGraph, inInstance, outIterator);
}

//==============================================================================
/**
 *	Returns an iterator for Asset Children from a certain slide.
 *	This is similar to GetAssetChildren with additional filter:
 *	check if it exists in inSlide or Master.
 *	Used primarily for displaying objects in the timeline in the correct order.
 */
void GetAssetChildrenInSlide(CDoc *inDoc, UICDM::CUICDMInstanceHandle inInstance,
                             UICDM::CUICDMSlideHandle inSlide,
                             Q3DStudio::CGraphIterator &outIterator, EStudioObjectType inObjectType)
{
    TAssetGraphPtr theGraph = inDoc->GetAssetGraph();
    ApplyDefaultAssetCriterias(inDoc, outIterator, inObjectType);
    outIterator += FilterForSlide(inDoc, inSlide);
    GetChildrenFromGraph(theGraph, inInstance, outIterator);
}

//==============================================================================
/**
 *	Returns an iterator for Asset Children that are in same Time Context
 *	This is similar to GetAssetChildren with additional filter:
 *	check if the controlling timeparent is the same
 */
void GetAssetChildrenInTimeParent(UICDM::CUICDMInstanceHandle inInstance, CDoc *inDoc,
                                  bool inIsAssetTimeParent, Q3DStudio::CGraphIterator &outIterator,
                                  UICDM::CUICDMSlideHandle inActiveComponentSlide)
{
    // TODO: for inFilterActiveComponentSlideIndex, just pass in the SlideHandle
    CClientDataModelBridge *theBridge = inDoc->GetStudioSystem()->GetClientDataModelBridge();
    TAssetGraphPtr theGraph = inDoc->GetAssetGraph();
    ApplyDefaultAssetCriterias(inDoc, outIterator, OBJTYPE_UNKNOWN);
    outIterator += FilterForControllingTimeParent(
        inDoc,
        inIsAssetTimeParent ? inInstance : theBridge->GetOwningComponentInstance(inInstance));
    if (inActiveComponentSlide.Valid())
        outIterator += FilterForActiveComponent(inDoc, inActiveComponentSlide);
    GetChildrenFromGraph(theGraph, inInstance, outIterator);
}

//==============================================================================
/**
 *	Returns an iterator for Asset Children in current slide. This is used to find which asset to
 *be rendered
 *	The children are returned in reverse order
 */
void GetAssetChildrenInCurrentSlide(CDoc *inDoc, UICDM::CUICDMInstanceHandle inInstance,
                                    Q3DStudio::CGraphIterator &outIterator,
                                    EStudioObjectType inObjectType)
{
    CClientDataModelBridge *theBridge = inDoc->GetStudioSystem()->GetClientDataModelBridge();
    TAssetGraphPtr theGraph = inDoc->GetAssetGraph();
    ApplyDefaultAssetCriterias(inDoc, outIterator, inObjectType);
    // get the asset from the active slide of the containing Component
    UICDM::CUICDMInstanceHandle theTimeParentInstance =
        (theBridge->IsSceneInstance(inInstance) || theBridge->IsComponentInstance(inInstance))
        ? inInstance
        : theBridge->GetOwningComponentInstance(inInstance);
    if (theTimeParentInstance.Valid()) // theComponent should never be NULL. But there are some
                                       // exceptional cases such as Edit Camera & Edit Layer
    {
        outIterator += FilterForCurrentSlide(inDoc, theTimeParentInstance);
        // for rendering, we do in reverse order
        GetReverseChildrenFromGraph(theGraph, inInstance, outIterator);
    }
}

//==============================================================================
//	Other useful functions related to graph traversal
//==============================================================================

//==============================================================================
/**
 *	Returns true if inAncestor is parent, grandparent, etc of inDescendant
 */
bool IsAscendant(Q3DStudio::TIdentifier inDescendant, Q3DStudio::TIdentifier inAncestor,
                 Q3DStudio::TGraphPtr inGraph)
{
    // We get the parent of inDescendant and compare it with inAncestor
    Q3DStudio::TIdentifier theDescendantParent = inGraph->GetParent(inDescendant);
    if (theDescendantParent) {
        if (theDescendantParent == inAncestor)
            return true;
        else
            return IsAscendant(theDescendantParent, inAncestor, inGraph);
    } else
        return false;
}

//==============================================================================
/**
 *	Get the sibling node (get the node before or after) inNode from CGraphIterator
 *	This will not change the iterator
 *	Note that if this function is used frequently, consider moving it to inside CGraphIterator
 *	so we can use std::find on m_Results
 */
Q3DStudio::TIdentifier GetSibling(const Q3DStudio::TIdentifier inNode, bool inAfter,
                                  const Q3DStudio::CGraphIterator &inIterator)
{
    size_t theCount = inIterator.GetCount();
    size_t theIndex = 0;
    Q3DStudio::TIdentifier theSiblingNode = 0;

    for (theIndex = 0; theIndex < theCount; ++theIndex) {
        if (inIterator.GetResult(theIndex) == inNode)
            break;
    }

    if (inAfter)
        ++theIndex; // Get next node
    else
        --theIndex; // Get previous node

    if (theIndex < theCount && theIndex >= 0)
        theSiblingNode = inIterator.GetResult(theIndex);

    return theSiblingNode;
}

#include <iostream>
//==============================================================================
/**
 *	Prints the subtree of the asset
 */
void PrintAssetSubTree(UICDM::CUICDMInstanceHandle inInstance, CDoc *inDoc, char *prefix)
{
    CClientDataModelBridge *theBridge = inDoc->GetStudioSystem()->GetClientDataModelBridge();
    std::cout << prefix << theBridge->GetName(inInstance).GetCharStar() << std::endl;

    // Go through each asset and pass update to each child
    Q3DStudio::CGraphIterator theChildren;
    GetAssetChildrenInCurrentSlide(inDoc, inInstance, theChildren);
    for (; !theChildren.IsDone(); ++theChildren) {
        PrintAssetSubTree(theChildren.GetCurrent(), inDoc, prefix);
    }
}

//==============================================================================
/**
 *	Prints all the slides under a master slide
 */
void PrintSlideInfo(CDoc *m_Doc, char *prefix, UICDM::CUICDMSlideHandle theMasterSlide)
{
    size_t slideCountBefore =
        m_Doc->GetStudioSystem()->GetSlideSystem()->GetSlideCount(theMasterSlide);
    for (size_t i = 0; i < slideCountBefore; ++i) {
        UICDM::CUICDMSlideHandle slideHdl =
            m_Doc->GetStudioSystem()->GetSlideSystem()->GetSlideByIndex(theMasterSlide, i);
        Q3DStudio::CString name = m_Doc->GetStudioSystem()->GetClientDataModelBridge()->GetName(
            m_Doc->GetStudioSystem()->GetSlideSystem()->GetSlideInstance(slideHdl));
        std::cout << "    : " << name.GetCharStar() << std::endl;

        UICDM::CUICDMInstanceHandle slideInstance =
            m_Doc->GetStudioSystem()->GetClientDataModelBridge()->GetOwningComponentInstance(
                slideHdl);
        PrintAssetSubTree(slideInstance, m_Doc, prefix);
    }
}
