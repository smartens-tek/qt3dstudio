/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "TreeHeader.h"

#include <QtGui/qpainter.h>


TreeHeader::TreeHeader(TimelineItem *parent) : TimelineItem(parent)
{

}

void TreeHeader::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    m_rectShy    .setRect(m_treeWidth - 16 * 3.3, size().height() * .5 - 8, 16, 16);
    m_rectVisible.setRect(m_treeWidth - 16 * 2.2, size().height() * .5 - 8, 16, 16);
    m_rectLock   .setRect(m_treeWidth - 16 * 1.1, size().height() * .5 - 8, 16, 16);

    static const QPixmap pixShy     = QPixmap(":/images/Toggle-Shy.png");
    static const QPixmap pixVisible = QPixmap(":/images/Toggle-HideShow.png");
    static const QPixmap pixLock    = QPixmap(":/images/Toggle-Lock.png");

    QColor selectedColor = QColor(TimelineConstants::FILTER_BUTTON_SELECTED_COLOR);
    if (m_shy)
        painter->fillRect(m_rectShy, selectedColor);

    if (m_visible)
        painter->fillRect(m_rectVisible, selectedColor);

    if (m_lock)
        painter->fillRect(m_rectLock, selectedColor);

    painter->drawPixmap(m_rectShy    , pixShy);
    painter->drawPixmap(m_rectVisible, pixVisible);
    painter->drawPixmap(m_rectLock   , pixLock);
}

void TreeHeader::setWidth(double w)
{
    m_treeWidth = w;
    update();
}

TreeControlType TreeHeader::handleButtonsClick(const QPointF &scenePos)
{
    QPointF p = mapFromScene(scenePos.x(), scenePos.y());

    if (m_rectShy.contains(p.x(), p.y())) {
        m_shy = !m_shy;
        update();
        return TreeControlType::Shy;
    } else if (m_rectVisible.contains(p.x(), p.y())) {
        m_visible = !m_visible;
        update();
        return TreeControlType::Hide;
    } else if (m_rectLock.contains(p.x(), p.y())) {
        m_lock = !m_lock;
        update();
        return TreeControlType::Lock;
    }

    return TreeControlType::None;
}

bool TreeHeader::filterShy() const
{
    return m_shy;
}

bool TreeHeader::filterHidden() const
{
    return m_visible;
}

bool TreeHeader::filterLocked() const
{
    return m_lock;
}

int TreeHeader::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return TypeTreeHeader;
}
