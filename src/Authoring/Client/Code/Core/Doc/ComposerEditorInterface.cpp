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
#include "stdafx.h"
#include "IComposerEditorInterface.h"
#include "UICImport.h"
#include "UICImportTranslation.h"
#include "Graph.h"
#include "StandardExtensions.h"
#include "EASTL/hash_map.h"

using namespace Q3DStudio::ComposerImport;
using namespace qt3ds::foundation;

namespace {
// base class between performing refresh and performing
// imports

struct STCharPtrHash
{
    size_t operator()(TCharPtr nm) const
    {
#ifdef KDAB_TEMPORARILY_REMOVED
        StaticAssert<sizeof(wchar_t) == sizeof(char16_t)>::valid_expression();
#endif
        return eastl::hash<const char16_t *>()(reinterpret_cast<const char16_t *>(nm));
    }
};
struct STCharPtrEqualTo
{
    bool operator()(TCharPtr lhs, TCharPtr rhs) const { return AreEqual(lhs, rhs); }
};

struct SComposerImportBase
{
    IDocumentEditor &m_Editor;
    CFilePath m_DocumentPath;
    CFilePath m_DestImportDir;
    CFilePath m_DestImportFile;
    Q3DStudio::CFilePath m_Relativeimportfile;
    qt3ds::QT3DSI32 m_StartTime;
    UICDM::IStringTable &m_StringTable;
    SComposerImportBase(
        IDocumentEditor &inEditor,
        const Q3DStudio::CFilePath &docPath /// Root directory where the studio file sits.
        ,
        const Q3DStudio::CFilePath &inFullPathToImportFile, long inStartTime,
        UICDM::IStringTable &inStringTable)
        : m_Editor(inEditor)
        , m_DocumentPath(docPath)
        , m_DestImportDir(inFullPathToImportFile
                              .GetDirectory()) // Directory where we are saving the import file
        , m_DestImportFile(inFullPathToImportFile)
        , m_Relativeimportfile(
              Q3DStudio::CFilePath::GetRelativePathFromBase(m_DocumentPath, inFullPathToImportFile))
        , m_StartTime(inStartTime)
        , m_StringTable(inStringTable)
    {
    }
};

struct SComposerImportInterface : public SComposerImportBase, public IComposerEditorInterface
{
    typedef eastl::hash_map<TImportId, CUICDMInstanceHandle, STCharPtrHash, STCharPtrEqualTo>
        TImportInstanceMap;
    CUICDMInstanceHandle m_Parent;
    CUICDMInstanceHandle m_Root;
    CUICDMSlideHandle m_Slide;
    Q3DStudio::CString m_TypeBuffer;
    qt3ds::QT3DSI32 m_StartTime;
    Import *m_ImportObj;
    TImportInstanceMap m_ImportToInstanceMap;

    // When we are refreshing, the root assets is the group we are refreshing.
    SComposerImportInterface(
        Q3DStudio::IDocumentEditor &editor, UICDM::CDataModelHandle parent // Parent object
        ,
        UICDM::CDataModelHandle root, UICDM::CUICDMSlideHandle slide,
        const Q3DStudio::CFilePath &docPath /// Root directory where the studio file sits.
        ,
        const Q3DStudio::CFilePath &inFullPathToImportFile, long inStartTime,
        UICDM::IStringTable &inStringTable)
        : SComposerImportBase(editor, docPath, inFullPathToImportFile, inStartTime, inStringTable)
        , m_Parent(parent)
        , m_Root(root)
        , m_Slide(slide)
        , m_StartTime(inStartTime)
        , m_ImportObj(NULL)
    {
        m_Editor.BeginAggregateOperation();
    }

    // Fires the 'do' notifications
    ~SComposerImportInterface() { m_Editor.EndAggregateOperation(); }

    // IComposerEditorInterface

    bool HasError() override { return m_Root.Valid() == false; }

    const wchar_t *GetRelativeimportfile() const {return nullptr;}

    void Finalize(const Q3DStudio::CFilePath &inFilePath) override
    {
        m_Editor.SetSpecificInstancePropertyValue(
            m_Slide, m_Root, L"sourcepath",
            std::make_shared<CDataStr>((const wchar_t *)m_Relativeimportfile));
        m_Editor.SetSpecificInstancePropertyValue(
            m_Slide, m_Root, L"importfile",
            std::make_shared<CDataStr>((const wchar_t *)m_Relativeimportfile));
    }

    CUICDMInstanceHandle FindInstance(TImportId inImportHdl) override
    {
        TImportInstanceMap::const_iterator entry(m_ImportToInstanceMap.find(inImportHdl));
        if (entry != m_ImportToInstanceMap.end())
            return entry->second;
        return 0;
    }

    void AddInstanceMap(CUICDMInstanceHandle instanceHandle, TImportId inImportId) override
    {
        if (inImportId == NULL || *inImportId == 0) {
            assert(0);
            return;
        }
        bool success =
            m_ImportToInstanceMap
                .insert(eastl::make_pair(m_StringTable.RegisterStr(inImportId), instanceHandle))
                .second;
        (void)success;
        assert(success);
    }
    CUICDMInstanceHandle GetRoot() override { return m_Root; }

    const Q3DStudio::CFilePath &GetDestImportFile() override { return m_DestImportFile; }

    // IComposerEditor
    // Object is stack created for now
    void Release() override {}

    void BeginImport(Import &importObj) override { m_ImportObj = &importObj; }

    void RemoveChild(TImportId parent, TImportId child) override
    {
        CUICDMInstanceHandle childHandle = FindInstance(child);
        CUICDMInstanceHandle parentHandle = FindInstance(parent);
        if (childHandle.Valid() && parentHandle.Valid()) {
            CUICDMInstanceHandle parentId = m_Editor.GetParent(childHandle);
            // If the child was moved, we don't remove it.  Only on the case where
            // it has it's original parent.
            if (parentId == parentHandle)
                m_Editor.RemoveChild(parentHandle, childHandle);
        }
    }

    void RemoveInstance(TImportId inInstance) override
    {
        CUICDMInstanceHandle instance = FindInstance(inInstance);
        if (instance.Valid())
            m_Editor.DeleteInstance(instance);
    }

    CUICDMInstanceHandle CreateInstance(ComposerObjectTypes::Enum type, CUICDMInstanceHandle parent,
                                        TImportId inImportId)
    {
        if (parent.Valid() == false) {
            assert(0);
            return 0;
        }

        // Map the type to the object type.
        CUICDMInstanceHandle retval = m_Editor.CreateSceneGraphInstance(type, parent, m_Slide);
        m_Editor.SetSpecificInstancePropertyValue(0, retval, L"importid",
                                                  std::make_shared<CDataStr>(inImportId));
        m_Editor.SetSpecificInstancePropertyValue(
            m_Slide, retval, L"importfile",
            std::make_shared<CDataStr>((const wchar_t *)m_Relativeimportfile));
        AddInstanceMap(retval, inImportId);
        return retval;
    }

    /**
     *	Note that instance properties that point to files (sourcepath generally) point to files
     *	relative to the import file.  You need to do combineBaseAndRelative with those files
     *	and the a new getRelativeFromBase with the final file in order to transfer data
     *	successfully.  The time to do this sort of thing is upon create or update instance.
     */
    void CreateRootInstance(TImportId inImportId, ComposerObjectTypes::Enum type) override
    {
        CUICDMInstanceHandle retval = m_Root;
        if (m_Root.Valid() == false) {
            retval = CreateInstance(type, m_Parent, inImportId);
            m_Root = retval;
            if (m_StartTime >= 0)
                m_Editor.SetStartTime(retval, m_StartTime);
        } else
            AddInstanceMap(m_Root, inImportId);

        QT3DS_ASSERT(m_Root.Valid());
    }

    void CreateInstance(TImportId inImportId, ComposerObjectTypes::Enum type,
                                TImportId inParent) override
    {
        CUICDMInstanceHandle theParent(FindInstance(inParent));
        if (theParent.Valid())
            CreateInstance(type, theParent, inImportId);
    }

    void UpdateInstanceProperties(TImportId inInstance, const PropertyValue *propertBuffer,
                                          QT3DSU32 propertyBufferSize) override
    {
        CUICDMInstanceHandle hdl(FindInstance(inInstance));
        if (hdl.Valid() == false)
            return;

        if (m_Editor.IsInstance(hdl) == false)
            return;

        for (QT3DSU32 idx = 0; idx < propertyBufferSize; ++idx) {
            const PropertyValue &value(propertBuffer[idx]);
            SValue theValue(value.m_Value);

            DataModelDataType::Value theType = GetValueType(theValue);
            if (value.m_Name == ComposerPropertyNames::sourcepath) {
                // re-work the path to be relative to where the main document
                // is saved instead of where the import result is saved
                TDataStrPtr value = UICDM::get<TDataStrPtr>(theValue);
                if (value->GetLength()) {
                    Q3DStudio::CString valueStr(value->GetData());
                    Q3DStudio::CFilePath fullPath =
                        Q3DStudio::CFilePath::CombineBaseAndRelative(m_DestImportDir, valueStr);
                    Q3DStudio::CString relativePath =
                        Q3DStudio::CFilePath::GetRelativePathFromBase(m_DocumentPath, fullPath);
                    theValue = std::make_shared<CDataStr>(relativePath.c_str());
                }
            } else if (theType == DataModelDataType::StringRef) {
                SStringRef theRef = get<SStringRef>(theValue);
                SLong4 theGuid;
                CUICDMInstanceHandle target = FindInstance(theRef.m_Id);
                if (target.Valid())
                    theGuid = m_Editor.GetGuidForInstance(target);
                theValue = theGuid;
            }

            // Note that we explicitly set the property values on the instance,
            // not on any given slide.
            m_Editor.SetSpecificInstancePropertyValue(
                0, hdl, ComposerPropertyNames::Convert(value.m_Name), theValue);
        }
    }
    void AddChild(TImportId parent, TImportId child, TImportId inSibling) override
    {
        CUICDMInstanceHandle theParent(FindInstance(parent));
        CUICDMInstanceHandle theChild(FindInstance(child));
        CUICDMInstanceHandle theSibling(FindInstance(inSibling));

        if (theParent.Valid() && theChild.Valid())
            m_Editor.AddChild(theParent, theChild, theSibling);
    }

    void RemoveAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex) override
    {
        CUICDMInstanceHandle hdl(FindInstance(inInstance));
        if (hdl.Valid())
            m_Editor.RemoveAnimation(m_Slide, hdl, propName, propSubIndex);
    }
    void UpdateAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex,
                                 EAnimationType animType, const float *animData, QT3DSU32 numFloats) override
    {
        CUICDMInstanceHandle hdl(FindInstance(inInstance));
        if (hdl.Valid()) {
            if (m_Editor.IsAnimationArtistEdited(m_Slide, hdl, propName, propSubIndex) == false) {
                CUICDMAnimationHandle anim = m_Editor.CreateOrSetAnimation(
                    m_Slide, hdl, propName, propSubIndex, animType, animData, numFloats, false);
                m_Editor.SetIsArtistEdited(anim, false);
            }
        }
    }
    void AddAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex,
                              EAnimationType animType, const float *animData, QT3DSU32 numFloats) override
    {
        UpdateAnimation(inInstance, propName, propSubIndex, animType, animData, numFloats);
    }

    void EndImport() override {}
};

struct SComposerRefreshInterface : public SComposerImportBase, public IComposerEditor
{
    TIdMultiMap &m_IdToSlideInstances;
    bool m_HasError;
    CGraph &m_AssetGraph;

    struct SSlideInstanceIdMapIterator
    {
        const vector<pair<CUICDMSlideHandle, CUICDMInstanceHandle>> *m_CurrentItems;
        size_t m_CurrentTreeIdx;
        size_t m_CurrentTreeEnd;
        TCharPtr m_Id;

        SSlideInstanceIdMapIterator(TImportId inImportId, TIdMultiMap &inItems,
                                    UICDM::IStringTable &inStringTable)
            : m_CurrentItems(NULL)
            , m_CurrentTreeIdx(0)
            , m_CurrentTreeEnd(0)
            , m_Id(inStringTable.RegisterStr(inImportId))
        {
            FindNextValidList(inItems);
        }
        void FindNextValidList(TIdMultiMap &inItems)
        {
            m_CurrentTreeIdx = 0;
            m_CurrentTreeEnd = 0;
            m_CurrentItems = NULL;
            TIdMultiMap::const_iterator theFind = inItems.find(m_Id);
            if (theFind != inItems.end()) {
                m_CurrentItems = &theFind->second;
                m_CurrentTreeIdx = 0;
                m_CurrentTreeEnd = theFind->second.size();
            }
        }
        bool IsDone() const
        {
            if (m_CurrentTreeIdx >= m_CurrentTreeEnd)
                return true;
            return false;
        }
        void Next()
        {
            if (m_CurrentTreeIdx < m_CurrentTreeEnd) {
                ++m_CurrentTreeIdx;
            }
        }
        CUICDMSlideHandle GetCurrentSlide() { return (*m_CurrentItems)[m_CurrentTreeIdx].first; }

        CUICDMInstanceHandle GetCurrentInstance()
        {
            return (*m_CurrentItems)[m_CurrentTreeIdx].second;
        }
    };

    SComposerRefreshInterface(Q3DStudio::IDocumentEditor &editor, TIdMultiMap &inIdToInstanceMap,
                              const Q3DStudio::CFilePath &docPath,
                              const Q3DStudio::CFilePath &inDestimportfile, long inStartTime,
                              UICDM::IStringTable &inStringTable, CGraph &inAssetGraph)
        : SComposerImportBase(editor, docPath, inDestimportfile, inStartTime, inStringTable)
        , m_IdToSlideInstances(inIdToInstanceMap)
        , m_HasError(false)
        , m_AssetGraph(inAssetGraph)
    {
    }

    void Release() override {}
    void BeginImport(Import &) override {}

    void RemoveChild(TImportId inParentId, TImportId inChildId) override
    {
        for (SSlideInstanceIdMapIterator theIterator(inParentId, m_IdToSlideInstances,
                                                     m_StringTable);
             theIterator.IsDone() == false; theIterator.Next()) {
            CUICDMInstanceHandle theParent = theIterator.GetCurrentInstance();
            for (long idx = 0; idx < m_AssetGraph.GetChildCount(theParent); ++idx) {
                CUICDMInstanceHandle theChild = m_AssetGraph.GetChild(theParent, idx);
                CString theImportId = m_Editor.GetImportId(theChild);
                if (m_Editor.GetAssociatedSlide(theChild) == theIterator.GetCurrentSlide()
                    && theImportId == inChildId) {
                    m_Editor.RemoveChild(theParent, theChild);
                    --idx;
                }
            }
        }
    }

    void RemoveInstance(TImportId inParentId) override
    {
        SSlideInstanceIdMapIterator theIterator(inParentId, m_IdToSlideInstances, m_StringTable);
        if (!theIterator.IsDone()) {
            for (size_t parentIdx = 0, parentEnd = theIterator.m_CurrentTreeEnd;
                 parentIdx < parentEnd; ++parentIdx) {
                if (m_Editor.IsInstance(theIterator.GetCurrentInstance()))
                    m_Editor.DeleteInstance(theIterator.GetCurrentInstance());
            }
            m_IdToSlideInstances.erase(theIterator.m_Id);
        }
    }
    /**
     *	Note that instance properties that point to files (sourcepath generally) point to files
     *	relative to the import file.  You need to do combineBaseAndRelative with those files
     *	and the a new getRelativeFromBase with the final file in order to transfer data
     *	successfully.  The time to do this sort of thing is upon create or update instance.
     */
    void CreateRootInstance(TImportId /*inImportId*/, ComposerObjectTypes::Enum /*type*/) override {}
    // inParent may be null (or an invalid handle) if the instance doesn't have a parent (images)
    void CreateInstance(TImportId inImportId, ComposerObjectTypes::Enum type,
                                TImportId inParent) override
    {
        const wchar_t *theInsertId(m_StringTable.GetWideStr(inImportId));
        pair<TIdMultiMap::iterator, bool> theInserter(m_IdToSlideInstances.insert(
            make_pair(theInsertId, vector<pair<CUICDMSlideHandle, CUICDMInstanceHandle>>())));

        for (SSlideInstanceIdMapIterator theIterator(inParent, m_IdToSlideInstances, m_StringTable);
             theIterator.IsDone() == false; theIterator.Next()) {
            CUICDMInstanceHandle theParent = theIterator.GetCurrentInstance();
            CUICDMInstanceHandle newInstance =
                m_Editor.CreateSceneGraphInstance(type, theParent, theIterator.GetCurrentSlide());
            if (m_StartTime >= 0)
                m_Editor.SetSpecificInstancePropertyValue(0, newInstance, L"starttime",
                                                          m_StartTime);
            m_Editor.SetSpecificInstancePropertyValue(0, newInstance, L"importid",
                                                      std::make_shared<CDataStr>(inImportId));
            m_Editor.SetSpecificInstancePropertyValue(
                0, newInstance, L"importfile", std::make_shared<CDataStr>(m_Relativeimportfile));
            insert_unique(theInserter.first->second,
                          make_pair(theIterator.GetCurrentSlide(), newInstance));
        }
    }

    // We guarantee that all instances will be created before their properties are updated thus you
    // can resolve references during this updateInstanceProperties call if necessary.
    void UpdateInstanceProperties(TImportId inInstance, const PropertyValue *propertBuffer,
                                          QT3DSU32 propertyBufferSize) override
    {
        for (SSlideInstanceIdMapIterator theIterator(inInstance, m_IdToSlideInstances,
                                                     m_StringTable);
             theIterator.IsDone() == false; theIterator.Next()) {
            CUICDMInstanceHandle hdl = theIterator.GetCurrentInstance();
            for (QT3DSU32 idx = 0; idx < propertyBufferSize; ++idx) {
                const PropertyValue &value(propertBuffer[idx]);
                SValue theValue(value.m_Value);

                DataModelDataType::Value theType = GetValueType(theValue);
                if (value.m_Name == ComposerPropertyNames::sourcepath) {
                    // re-work the path to be relative to where the main document
                    // is saved instead of where the import result is saved
                    TDataStrPtr value = UICDM::get<TDataStrPtr>(theValue);
                    if (value->GetLength()) {
                        Q3DStudio::CString valueStr(value->GetData());
                        Q3DStudio::CFilePath fullPath =
                            Q3DStudio::CFilePath::CombineBaseAndRelative(m_DestImportDir, valueStr);
                        Q3DStudio::CString relativePath =
                            Q3DStudio::CFilePath::GetRelativePathFromBase(m_DocumentPath, fullPath);
                        theValue = std::make_shared<CDataStr>(relativePath.c_str());
                    }
                } else if (theType == DataModelDataType::StringRef) {
                    SStringRef theRef = get<SStringRef>(theValue);
                    SLong4 theGuid;
                    // We are going to cheat here and look for a child of the current instance
                    // in the current slide who has the same import id;
                    for (long childIdx = 0, childCount = m_AssetGraph.GetChildCount(hdl);
                         childIdx < childCount; ++childIdx) {
                        CUICDMInstanceHandle target = m_AssetGraph.GetChild(hdl, childIdx);
                        if (m_Editor.GetAssociatedSlide(hdl) == theIterator.GetCurrentSlide()
                            && m_Editor.GetImportId(target).Compare(theRef.m_Id)) {
                            theGuid = m_Editor.GetGuidForInstance(target);
                            theValue = theGuid;
                        }
                    }
                }
                // Note that we explicitly set the property values on the instance,
                // not on any given slide.
                m_Editor.SetSpecificInstancePropertyValue(
                    0, hdl, ComposerPropertyNames::Convert(value.m_Name), theValue);
            }
        }
    }
    // This is called even for new instances where we told you the parent because
    // they may be out of order so if the child already has this parent relationship you need
    // to check the order and ensure that is also (somewhat) correct.
    void AddChild(TImportId parent, TImportId child, TImportId nextSiblingId) override
    {
        TIdMultiMap::iterator theParentList =
            m_IdToSlideInstances.find(m_StringTable.RegisterStr(parent));
        TIdMultiMap::iterator theChildList =
            m_IdToSlideInstances.find(m_StringTable.RegisterStr(child));
        if (theParentList == m_IdToSlideInstances.end()
            || theChildList == m_IdToSlideInstances.end())
            return;
        size_t numItems = NVMin(theParentList->second.size(), theChildList->second.size());
        for (size_t idx = 0; idx < numItems; ++idx) {
            CUICDMSlideHandle theParentSlide = theParentList->second[idx].first;
            CUICDMInstanceHandle theParent(theParentList->second[idx].second);
            CUICDMInstanceHandle theChild(theChildList->second[idx].second);
            CUICDMInstanceHandle nextSibling;
            if (!IsTrivial(nextSiblingId)) {
                for (long childIdx = 0, childCount = m_AssetGraph.GetChildCount(theParent);
                     childIdx < childCount; ++childIdx) {
                    CUICDMInstanceHandle theSibling = m_AssetGraph.GetChild(theParent, childIdx);
                    if (m_Editor.GetAssociatedSlide(theSibling) == theParentSlide
                        && m_Editor.GetImportId(theSibling).Compare(nextSiblingId)) {
                        nextSibling = theSibling;
                        break;
                    }
                }
            }
            if (nextSibling.Valid())
                m_AssetGraph.MoveBefore(theChild, nextSibling);
            else
                m_AssetGraph.MoveTo(theChild, theParent, COpaquePosition::LAST);
        }
    }

    void RemoveAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex) override
    {
        for (SSlideInstanceIdMapIterator theIterator(inInstance, m_IdToSlideInstances,
                                                     m_StringTable);
             theIterator.IsDone() == false; theIterator.Next())
            m_Editor.RemoveAnimation(theIterator.GetCurrentSlide(),
                                     theIterator.GetCurrentInstance(), propName, propSubIndex);
    }
    void UpdateAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex,
                                 EAnimationType animType, const float *animData, QT3DSU32 numFloats) override
    {
        for (SSlideInstanceIdMapIterator theIterator(inInstance, m_IdToSlideInstances,
                                                     m_StringTable);
             theIterator.IsDone() == false; theIterator.Next()) {
            if (m_Editor.AnimationExists(theIterator.GetCurrentSlide(),
                                         theIterator.GetCurrentInstance(), propName, propSubIndex)
                && m_Editor.IsAnimationArtistEdited(theIterator.GetCurrentSlide(),
                                                    theIterator.GetCurrentInstance(), propName,
                                                    propSubIndex)
                    == false) {
                CUICDMAnimationHandle anim = m_Editor.CreateOrSetAnimation(
                    theIterator.GetCurrentSlide(), theIterator.GetCurrentInstance(), propName,
                    propSubIndex, animType, animData, numFloats, false);
                m_Editor.SetIsArtistEdited(anim, false);
            }
        }
    }

    void AddAnimation(TImportId inInstance, const wchar_t *propName, long propSubIndex,
                              EAnimationType animType, const float *animData, QT3DSU32 numFloats) override
    {
        for (SSlideInstanceIdMapIterator theIterator(inInstance, m_IdToSlideInstances,
                                                     m_StringTable);
             theIterator.IsDone() == false; theIterator.Next()) {
            if (!m_Editor.AnimationExists(theIterator.GetCurrentSlide(),
                                          theIterator.GetCurrentInstance(), propName,
                                          propSubIndex)) {
                CUICDMAnimationHandle anim = m_Editor.CreateOrSetAnimation(
                    theIterator.GetCurrentSlide(), theIterator.GetCurrentInstance(), propName,
                    propSubIndex, animType, animData, numFloats, false);
                m_Editor.SetIsArtistEdited(anim, false);
            }
        }
    }

    void EndImport() override {}
};
}

std::shared_ptr<IComposerEditorInterface> IComposerEditorInterface::CreateEditorInterface(
    Q3DStudio::IDocumentEditor &editor, UICDM::CDataModelHandle parent // Parent object
    ,
    UICDM::CDataModelHandle root, UICDM::CUICDMSlideHandle slide,
    const Q3DStudio::CFilePath &docPath, const Q3DStudio::CFilePath &destimportfile,
    long inStartTime, UICDM::IStringTable &inStringTable)
{
    return std::make_shared<SComposerImportInterface>(std::ref(editor), parent, root, slide,
                                                        docPath, destimportfile, inStartTime,
                                                        std::ref(inStringTable));
}

// The refresh interface is setup to refresh multiple trees automatically
std::shared_ptr<IComposerEditor> IComposerEditorInterface::CreateEditorInterface(
    Q3DStudio::IDocumentEditor &editor, TIdMultiMap &inRoots, const Q3DStudio::CFilePath &docPath,
    const Q3DStudio::CFilePath &destimportfile, long inStartTime,
    UICDM::IStringTable &inStringTable, CGraph &inAssetGraph)
{
    return std::make_shared<SComposerRefreshInterface>(
        std::ref(editor), std::ref(inRoots), docPath, destimportfile, inStartTime,
        std::ref(inStringTable), std::ref(inAssetGraph));
}
