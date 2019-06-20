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

#ifndef PLAYHEAD_H
#define PLAYHEAD_H

#include "TimelineItem.h"

#include <QtWidgets/qgraphicsitem.h>

class Ruler;

class PlayHead : public QGraphicsRectItem
{

public:
    explicit PlayHead(Ruler *m_ruler);

    void setHeight(int height);
    void setPosition(double posX); // set x poisiotn
    void updatePosition(); // sync x poisiotn based on time value
    void setTime(long time); // set time (sets x based on time (ms) input)
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;
    long time() const;
    int type() const override;

private:
    long m_time = 0;
    Ruler *m_ruler;
};

#endif // PLAYHEAD_H