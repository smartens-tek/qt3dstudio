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
#ifndef OBJECTBROWSERVIEW_H
#define OBJECTBROWSERVIEW_H

#include <QQuickWidget>

#include "RelativePathTools.h"
#include "Qt3DSDMHandles.h"

#include <QColor>

class ObjectListModel;
class FlatObjectListModel;

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

class ObjectBrowserView : public QQuickWidget
{
    Q_OBJECT
    Q_PROPERTY(bool focused READ isFocused NOTIFY focusChanged)
    Q_PROPERTY(QAbstractItemModel *model READ model NOTIFY modelChanged FINAL)
    Q_PROPERTY(int selection READ selection WRITE setSelection NOTIFY selectionChanged FINAL)
    Q_PROPERTY(PathType pathType READ pathType WRITE setPathType NOTIFY pathTypeChanged FINAL)

public:
    ObjectBrowserView(QWidget *parent = nullptr);


    enum PathType {
        Absolute = CRelativePathTools::EPATHTYPE_GUID,
        Relative = CRelativePathTools::EPATHTYPE_RELATIVE,
    };
    Q_ENUM(PathType)

    QAbstractItemModel *model() const;
    void setModel(ObjectListModel *model);

    Q_INVOKABLE QString absPath(int index) const;
    Q_INVOKABLE QString relPath(int index) const;
    Q_INVOKABLE bool selectable(int index) const;

    void selectAndExpand(const qt3dsdm::Qt3DSDMInstanceHandle &handle,
                         const qt3dsdm::Qt3DSDMInstanceHandle &owner);

    int selection() const { return m_selection; }
    void setSelection(int index);

    PathType pathType() const {return m_pathType;}
    void setPathType(PathType type);

    qt3dsdm::Qt3DSDMInstanceHandle selectedHandle() const;

    bool canCommit() const { return !m_blockCommit; }

Q_SIGNALS:
    void modelChanged();
    void pathTypeChanged();
    void selectionChanged();

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

Q_SIGNALS:
    void focusChanged();

private:
    void initialize();
    bool isFocused() const;

    FlatObjectListModel *m_model = nullptr;
    QHash<int, ObjectListModel *> m_subModels;
    QColor m_baseColor = QColor::fromRgb(75, 75, 75);
    QColor m_selectColor;
    int m_selection = -1;
    PathType m_pathType = Absolute;
    qt3dsdm::Qt3DSDMInstanceHandle m_ownerInstance = 0;
    bool m_blockCommit = false;
};

#endif // OBJECTBROWSERVIEW_H
