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

#include "SelectionRect.h"
#include "TimelineConstants.h"
#include "Ruler.h"

#include <QtGui/qpainter.h>

SelectionRect::SelectionRect()
{
    setZValue(100);
    setActive(false);
}

void SelectionRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_active)
        painter->drawRect(m_rect);
}

void SelectionRect::start(const QPointF &origin)
{
    m_rect.setTopLeft(origin);
    m_rect.setWidth(0);
    m_rect.setHeight(0);
    m_active = true;
}

void SelectionRect::updateSize(const QPointF &pos, const QRectF &visibleScene)
{
    QPointF newPos = pos;
    if (newPos.x() < visibleScene.left())
        newPos.setX(visibleScene.left());
    else if (newPos.x() > visibleScene.right())
        newPos.setX(visibleScene.right());
    if (newPos.y() < visibleScene.top())
        newPos.setY(visibleScene.top());
    else if (newPos.y() > visibleScene.bottom())
        newPos.setY(visibleScene.bottom());

    m_rect.setBottomRight(newPos);
    setRect(m_rect.normalized());
}

void SelectionRect::end()
{
    setRect(QRectF());
    m_active = false;
}

bool SelectionRect::isActive() const
{
    return m_active;
}
