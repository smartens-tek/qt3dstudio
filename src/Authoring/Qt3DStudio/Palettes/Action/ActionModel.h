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

#ifndef ACTIONMODEL_H
#define ACTIONMODEL_H

#include <QAbstractListModel>

#include "Qt3DSDMActionInfo.h"
#include "Qt3DSDMHandles.h"

namespace qt3dsdm {
    class IActionSystem;
}

class ActionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ActionModel(QObject *parent = nullptr);

    void setInstanceHandle(const qt3dsdm::Qt3DSDMInstanceHandle &handle);

    enum Roles {
        DescriptionRole = Qt::DisplayRole,
        VisibleRole = Qt::UserRole + 1,

    };

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &data, int role = Qt::EditRole) override;

    void addAction(const qt3dsdm::Qt3DSDMActionHandle &action);
    void removeAction(const qt3dsdm::Qt3DSDMActionHandle &action);
    void updateAction(const qt3dsdm::Qt3DSDMActionHandle &action);
    const qt3dsdm::Qt3DSDMActionHandle actionAt(int row);
    const qt3dsdm::SActionInfo actionInfoAt(int row);

private:
    qt3dsdm::IActionSystem *actionSystem() const;
    qt3dsdm::Qt3DSDMSlideHandle activeSlide() const;
    QString actionString(const qt3dsdm::Qt3DSDMActionHandle &action) const;

    qt3dsdm::Qt3DSDMInstanceHandle m_handle;
    qt3dsdm::TActionHandleList m_actions;
};

#endif // ACTIONMODEL_H
