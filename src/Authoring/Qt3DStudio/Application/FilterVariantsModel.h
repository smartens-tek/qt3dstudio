/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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

#ifndef VARIANTSGROUPMODEL_H
#define VARIANTSGROUPMODEL_H

#include <QtCore/qabstractitemmodel.h>

class VariantsTagModel;

using VariantsFilterT = QHash<QString, QSet<QString> >;

class FilterVariantsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FilterVariantsModel(VariantsFilterT &variantsFilter, QObject *parent = nullptr);

    enum Roles {
        GroupTitleRole = Qt::UserRole + 1,
        GroupColorRole,
        TagsRole
    };

    struct VariantsGroupData
    {
        QString m_title;
        QString m_color;
        VariantsTagModel *m_tagsModel = nullptr;
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = GroupTitleRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void refresh();

    Q_INVOKABLE void setTagState(const QString &group, const QString &tag, bool selected);
    Q_INVOKABLE void toggleGroupState(const QString &group);
    Q_INVOKABLE void clearAll();

protected:
      QHash<int, QByteArray> roleNames() const override;

private:
      VariantsFilterT &m_variantsFilter;
      QVector<VariantsGroupData> m_data;
};

#endif // VARIANTSGROUPMODEL_H
