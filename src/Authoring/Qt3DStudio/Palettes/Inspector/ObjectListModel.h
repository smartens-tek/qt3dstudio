/****************************************************************************
**
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

#ifndef OBJECTLISTMODEL_H
#define OBJECTLISTMODEL_H

#include <QAbstractItemModel>
#include <QAbstractListModel>

#include "Qt3DSDMHandles.h"
#include "StudioObjectTypes.h"

class IObjectReferenceHelper;
class CCore;

class ObjectListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ObjectListModel(CCore *core, const qt3dsdm::Qt3DSDMInstanceHandle &baseHandle,
                    QObject *parent = nullptr,
                    bool isAliasSelectList = false);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index,
                  const QModelIndex &startingIndex = {},
                  int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    enum Roles {
        NameRole = Qt::DisplayRole,
        AbsolutePathRole = Qt::UserRole + 1,
        PathReferenceRole,
        IconRole,
        TextColorRole,
        HandleRole,
        LastRole = HandleRole
    };

    QHash<int, QByteArray> roleNames() const override;

    qt3dsdm::Qt3DSDMInstanceHandle baseHandle() const {return m_baseHandle;}

    QModelIndex indexForHandle(const qt3dsdm::Qt3DSDMInstanceHandle &handle,
                               const QModelIndex &startIndex = {}) const;

    bool selectable(const qt3dsdm::Qt3DSDMInstanceHandle &handle) const;

    void excludeObjectTypes(const QVector<EStudioObjectType> &types);

Q_SIGNALS:
    void roleUpdated(int role);
    void rolesUpdated(const QVector<int> &roles = QVector<int> ());

protected:
    qt3dsdm::Qt3DSDMInstanceHandle handleForIndex(const QModelIndex &index) const;

    virtual qt3dsdm::TInstanceHandleList childrenList(const qt3dsdm::Qt3DSDMSlideHandle &slideHandle,
                                                      const qt3dsdm::Qt3DSDMInstanceHandle &handle) const;

    QString nameForHandle(const qt3dsdm::Qt3DSDMInstanceHandle &handle) const;

    CCore *m_core;
    qt3dsdm::Qt3DSDMSlideHandle m_slideHandle;
    qt3dsdm::Qt3DSDMInstanceHandle m_baseHandle;
    IObjectReferenceHelper *m_objRefHelper;
    QVector<EStudioObjectType> m_excludeTypes;
    bool m_AliasSelectList;
};

class FlatObjectListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // TODO Make FlatObjectList take a shared pointer
    FlatObjectListModel(ObjectListModel *sourceModel, QObject *parent = nullptr);

    enum Roles {
        DepthRole = ObjectListModel::LastRole + 1,
        ExpandedRole,
        ParentExpandedRole,
        HasChildrenRole
    };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index,
                  const QModelIndex &startingIndex = {},
                  int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void setSourceModel(ObjectListModel *sourceModel);
    ObjectListModel *sourceModel() const { return m_sourceModel; }
    bool expandTo(const QModelIndex &rootIndex, const QModelIndex &searchIndex);
    int rowForSourceIndex(const QModelIndex &sourceIndex) const;
    QModelIndex sourceIndexForHandle(const qt3dsdm::Qt3DSDMInstanceHandle &handle);

private:
    int rowForSourceIndex(const QModelIndex &parentIndex, int row) const;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    struct SourceInfo {
        bool expanded = false;
        int depth = 0;
        QPersistentModelIndex index;
    };

    QVector<SourceInfo> collectSourceIndexes(const QModelIndex &sourceIndex, int depth) const;

    QVector<SourceInfo> m_sourceInfo;
    ObjectListModel *m_sourceModel = nullptr;
};


#endif // OBJECTLISTMODEL_H
