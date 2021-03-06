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

#include "RowTreeLabel.h"
#include "TimelineConstants.h"
#include "TimelineItem.h"
#include "RowTree.h"
#include "StudioPreferences.h"

#include <QtWidgets/qstyleoption.h>
#include <QtGui/qevent.h>
#include <QtGui/qtextcursor.h>

RowTreeLabel::RowTreeLabel(QGraphicsItem *parent)
    : QGraphicsTextItem(parent)
    , m_locked(false)
    , m_master(false)
    , m_acceptOnFocusOut(true)
    , m_labelWidth(100)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setEnabled(false);
    updateLabelColor();
    QFont font;
    font.setPixelSize(12);
    setFont(font);
}

QString RowTreeLabel::label() const
{
    return m_label;
}

void RowTreeLabel::setLabel(const QString &label)
{
    setPlainText(label);
    if (m_label != label) {
        m_label = label;
        emit labelChanged(m_label);
    }
}

void RowTreeLabel::setMaster(bool isMaster) {
    if (m_master != isMaster) {
        m_master = isMaster;
        updateLabelColor();
    }
}

void RowTreeLabel::setLocked(bool isLocked) {
    if (m_locked != isLocked) {
        m_locked = isLocked;
        updateLabelColor();
    }
}

RowTree *RowTreeLabel::rowTree() const
{
    return m_rowTree;
}

void RowTreeLabel::setRowTree(RowTree *row)
{
    m_rowTree = row;
}

int RowTreeLabel::type() const
{
    // Enable the use of qgraphicsitem_cast with this item.
    return TimelineItem::TypeRowTreeLabel;
}

void RowTreeLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (!m_rowTree->y()) // prevents flickering when the row is just inserted to the layout
        return;

    // Limit the painted label width, see QT3DS-3913
    int labelWidth = m_rowTree->treeWidth() - int(TimelineConstants::TREE_ICONS_W) - 30;
    if (labelWidth != m_labelWidth) {
        m_labelWidth = labelWidth;
        setTextWidth(m_labelWidth);
    }

    // Remove the HasFocus style state, to prevent the dotted line from being drawn.
    QStyleOptionGraphicsItem *style = const_cast<QStyleOptionGraphicsItem *>(option);
    style->state &= ~QStyle::State_HasFocus;

    QGraphicsTextItem::paint(painter, option, widget);
}

void RowTreeLabel::focusOutEvent(QFocusEvent *event)
{
    if (m_acceptOnFocusOut)
        validateLabel();
    else
        setPlainText(m_label);

    // Remove possible selection and make disabled again
    QTextCursor cursor = textCursor();
    cursor.clearSelection();
    setTextCursor(cursor);
    setEnabled(false);
    QGraphicsTextItem::focusOutEvent(event);
    // Next time default to accepting
    m_acceptOnFocusOut = true;
}

void RowTreeLabel::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        m_acceptOnFocusOut = true;
        clearFocus();
        event->accept();
        return;
    } else if (key == Qt::Key_Escape) {
        m_acceptOnFocusOut = false;
        clearFocus();
        event->accept();
        return;
    }

    QGraphicsTextItem::keyPressEvent(event);
}

QRectF RowTreeLabel::boundingRect() const
{
    if (!m_rowTree)
        return QGraphicsTextItem::boundingRect();

    double w = m_rowTree->clipX() - x();
    // Bounding rect width must be at least 1
    w = std::max(w, 1.0);
    return QRectF(0, 0, w, TimelineConstants::ROW_H);
}

void RowTreeLabel::validateLabel()
{
    QString text = toPlainText().trimmed();
    if (text.isEmpty()) {
        // Inform label was empty and return previous label
        emit labelChanged("");
        setLabel(m_label);
        return;
    }

    setLabel(text);
}

void RowTreeLabel::updateLabelColor()
{
    if (m_locked)
        setDefaultTextColor(CStudioPreferences::disabledTextColor());
    else if (m_master)
        setDefaultTextColor(CStudioPreferences::masterColor());
    else
        setDefaultTextColor(CStudioPreferences::normalColor());
}
