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

#ifndef VARIANTSTAGMODEL_H
#define VARIANTSTAGMODEL_H

#include <QtCore/qabstractitemmodel.h>

class VariantsTagModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit VariantsTagModel(const QVector<std::pair<QString, bool> > &data,
                              QObject *parent = nullptr);

    enum Roles {
        TagRole = Qt::UserRole + 1,
        SelectedRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = TagRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    void updateTagState(const QString &tag, bool selected);
    QString serialize(const QString &groupName) const;

protected:
      QHash<int, QByteArray> roleNames() const override;

private:
      QVector<std::pair<QString, bool> > m_data; // [{tagName, selectedState}, ...]
};

#endif // VARIANTSTAGMODEL_H
