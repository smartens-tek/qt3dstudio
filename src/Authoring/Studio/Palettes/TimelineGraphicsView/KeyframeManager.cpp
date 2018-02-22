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

#include "KeyframeManager.h"
#include "RowTree.h"
#include "RowTimeline.h"
#include "Keyframe.h"
#include "RowTypes.h"
#include "TimelineConstants.h"
#include "Ruler.h"
#include "PlayHead.h"
#include "RowManager.h"
#include "TimelineGraphicsScene.h"

#include <qglobal.h>
#include <QtCore/qhash.h>
#include <QtCore/qdebug.h>

KeyframeManager::KeyframeManager(TimelineGraphicsScene *scene) : m_scene(scene)
{
}

QList<Keyframe *> KeyframeManager::insertKeyframe(RowTimeline *row, double time, double value,
                                                  bool selectInsertedKeyframes)
{
    QList<Keyframe *> addedKeyframes;
    QList<RowTimeline *> propRows;
    if (row->rowTree()->rowType() != RowType::Property) {
        const auto childRows = row->rowTree()->childRows();
        for (const auto r : childRows) {
            if (r->rowType() == RowType::Property)
                propRows.append(r->rowTimeline());
        }
    } else {
        propRows.append(row);
    }

    if (!propRows.empty()) {
        Keyframe *keyframe = nullptr;
        for (const auto &r : qAsConst(propRows)) {
            keyframe = new Keyframe(time, value, r);
            r->insertKeyframe(keyframe);
            r->parentRow()->insertKeyframe(keyframe);
            addedKeyframes.append(keyframe);
        }

        if (selectInsertedKeyframes && !addedKeyframes.empty()) {
            deselectAllKeyframes();
            selectKeyframes(addedKeyframes);
        }
    }

    return addedKeyframes;
}

void KeyframeManager::selectKeyframe(Keyframe *keyframe)
{
    if (!m_selectedKeyframes.contains(keyframe)) {
        keyframe->selected = true;
        m_selectedKeyframes.append(keyframe);

        if (!m_selectedKeyframesMasterRows.contains(keyframe->rowMaster))
            m_selectedKeyframesMasterRows.append(keyframe->rowMaster);

        keyframe->rowMaster->putSelectedKeyframesOnTop();
        keyframe->rowMaster->updateKeyframes();
    }
}

void KeyframeManager::selectKeyframes(const QList<Keyframe *> &keyframes)
{
    for (const auto keyframe : keyframes) {
        if (!m_selectedKeyframes.contains(keyframe)) {
            m_selectedKeyframes.append(keyframe);

            if (!m_selectedKeyframesMasterRows.contains(keyframe->rowMaster))
                m_selectedKeyframesMasterRows.append(keyframe->rowMaster);
        }
    }

    for (auto keyframe : qAsConst(m_selectedKeyframes))
        keyframe->selected = true;

    for (auto row : qAsConst(m_selectedKeyframesMasterRows)) {
        row->putSelectedKeyframesOnTop();
        row->updateKeyframes();
    }
}

void KeyframeManager::selectKeyframesInRect(const QRectF &rect)
{
    deselectAllKeyframes();

    int idx1 = (rect.top() + 4) / TimelineConstants::ROW_H;
    int idx2 = (rect.bottom() - 4) / TimelineConstants::ROW_H;

    m_scene->rowManager()->clampIndex(idx1);
    m_scene->rowManager()->clampIndex(idx2);

    // TODO: remove
    qDebug() << "idx1=" << idx1  << ", idx2=" << idx2;

    RowTimeline *rowTimeline;
    for (int i = idx1; i <= idx2; ++i) {
        rowTimeline = m_scene->rowManager()->rowTimelineAt(i);

        if (rowTimeline != nullptr) {
            const auto keyframes = rowTimeline->getKeyframesInRange(rect.left(), rect.right());
            for (auto keyframe : keyframes) {
                if (!m_selectedKeyframes.contains(keyframe)) {
                    m_selectedKeyframes.append(keyframe);

                    if (!m_selectedKeyframesMasterRows.contains(keyframe->rowMaster))
                        m_selectedKeyframesMasterRows.append(keyframe->rowMaster);
                }
            }
        }
    }

    for (auto keyframe : qAsConst(m_selectedKeyframes))
        keyframe->selected = true;

    for (auto row : qAsConst(m_selectedKeyframesMasterRows)) {
        row->putSelectedKeyframesOnTop();
        row->updateKeyframes();
    }
}

void KeyframeManager::deselectKeyframe(Keyframe *keyframe)
{
    if (m_selectedKeyframes.contains(keyframe)) {
        keyframe->selected = false;
        m_selectedKeyframes.removeAll(keyframe);
        keyframe->rowMaster->updateKeyframes();
        m_selectedKeyframesMasterRows.removeAll(keyframe->rowMaster);
    }
}

void KeyframeManager::deselectAllKeyframes()
{
    for (auto keyframe : qAsConst(m_selectedKeyframes))
        keyframe->selected = false;

    for (auto row : qAsConst(m_selectedKeyframesMasterRows))
        row->updateKeyframes();

    m_selectedKeyframes.clear();
    m_selectedKeyframesMasterRows.clear();
}

void KeyframeManager::deleteSelectedKeyframes()
{
    if (!m_selectedKeyframes.empty()) {
        for (auto keyframe : qAsConst(m_selectedKeyframes)) {
            keyframe->rowMaster->removeKeyframe(keyframe);
            keyframe->rowProperty->removeKeyframe(keyframe);

            delete keyframe;
        }

        for (auto row : qAsConst(m_selectedKeyframesMasterRows))
            row->updateKeyframes();

        m_selectedKeyframes.clear();
        m_selectedKeyframesMasterRows.clear();
    }
}

// delete all keyframes on a row
void KeyframeManager::deleteKeyframes(RowTimeline *row)
{
    const auto keyframes = row->keyframes();
    for (auto keyframe : keyframes) {
        keyframe->rowMaster->removeKeyframe(keyframe);
        keyframe->rowProperty->removeKeyframe(keyframe);

        if (m_selectedKeyframes.contains(keyframe))
            m_selectedKeyframes.removeAll(keyframe);

        delete keyframe;
    }

    if (m_selectedKeyframesMasterRows.contains(row))
        m_selectedKeyframesMasterRows.removeAll(row);

    row->updateKeyframes();
}

void KeyframeManager::copySelectedKeyframes()
{
    if (!m_selectedKeyframes.empty()
        && m_selectedKeyframesMasterRows.count() == 1) {
        // delete old copies
        for (auto keyframe : qAsConst(m_copiedKeyframes))
            delete keyframe;

        m_copiedKeyframes.clear();

        Keyframe *copyKeyframe;
        for (auto keyframe : qAsConst(m_selectedKeyframes)) {
            copyKeyframe = new Keyframe(*keyframe);
            copyKeyframe->rowMaster = nullptr;
            copyKeyframe->rowProperty = nullptr;
            m_copiedKeyframes.append(copyKeyframe);
        }
    }
}

void KeyframeManager::pasteKeyframes(RowTimeline *row)
{
    if (row == nullptr)
        return;

    if (row->rowTree()->rowType() == RowType::Property)
        row = row->parentRow();

    if (!m_copiedKeyframes.empty()) {
        // filter copied keyframes to the row supported properties
        const QList<Keyframe *> filteredKeyframes = filterKeyframesForRow(row, m_copiedKeyframes);

        // calc min/max copied frames time
        double minTime = 999999; // seconds (~277.78 hrs)
        double maxTime = 0;
        for (auto keyframe : filteredKeyframes) {
            if (keyframe->time < minTime)
                minTime = keyframe->time;

            if (keyframe->time > maxTime)
                maxTime = keyframe->time;
        }

        double dt = m_scene->playHead()->time() - minTime;

        if (maxTime + dt > m_scene->ruler()->duration())
            dt = m_scene->ruler()->duration() - maxTime;

        RowTree *propRow;
        QList<Keyframe *> addedKeyframes;
        for (auto keyframe : filteredKeyframes) {
            propRow = m_scene->rowManager()->getOrCreatePropertyRow(keyframe->propertyType,
                                                                    row->rowTree());
            addedKeyframes.append(insertKeyframe(propRow->rowTimeline(), keyframe->time + dt,
                                                 keyframe->value, false));
        }

        if (!addedKeyframes.empty()) {
            deselectAllKeyframes();
            selectKeyframes(addedKeyframes);
        }
    }
}

QList<Keyframe *> KeyframeManager::filterKeyframesForRow(RowTimeline *row,
                                                         const QList<Keyframe *> &keyframes)
{
    QList<Keyframe *> result;

    for (auto keyframe : keyframes) {
        if (SUPPORTED_ROW_PROPS[row->rowTree()->rowType()].contains(keyframe->propertyType))
            result.append(keyframe);
    }

    return result;
}

void KeyframeManager::moveSelectedKeyframes(double dx)
{
    double dt = m_scene->ruler()->distanceToTime(dx);

    if (dt > 0) { // check max limit
        double maxTime = 0;
        for (auto keyframe : qAsConst(m_selectedKeyframes)) {
            if (keyframe->time > maxTime)
                maxTime = keyframe->time;
        }

        if (maxTime + dt > m_scene->ruler()->duration())
            dt = m_scene->ruler()->duration() - maxTime;
    } else if (dt < 0) { // check min limit
        double minTime = 999999; // seconds (~277.78 hrs)
        for (auto keyframe : qAsConst(m_selectedKeyframes)) {
            if (keyframe->time < minTime)
                minTime = keyframe->time;
        }

        if (minTime + dt < 0)
            dt = -minTime;
    }

    for (auto keyframe : qAsConst(m_selectedKeyframes))
        keyframe->time += dt;

    for (auto row : qAsConst(m_selectedKeyframesMasterRows))
        row->updateKeyframes();
}

// selected keyframes belong to only one master row
bool KeyframeManager::oneMasterRowSelected() const
{
    return m_selectedKeyframesMasterRows.count() == 1;
}

bool KeyframeManager::hasSelectedKeyframes() const
{
    return !m_selectedKeyframes.empty();
}

bool KeyframeManager::hasCopiedKeyframes() const
{
    return !m_copiedKeyframes.empty();
}

const QHash<RowType, QList<PropertyType>> KeyframeManager::SUPPORTED_ROW_PROPS {
    { RowType::Layer, {
          PropertyType::Left,
          PropertyType::Width,
          PropertyType::Top,
          PropertyType::Height,
          PropertyType::AO,
          PropertyType::AODistance,
          PropertyType::AOSoftness,
          PropertyType::AOThreshold,
          PropertyType::AOSamplingRate,
          PropertyType::IBLBrightness,
          PropertyType::IBLHorizonCutoff,
          PropertyType::IBLFOVAngle }
    },
    { RowType::Camera, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::FieldOfView,
          PropertyType::ClippingStart,
          PropertyType::ClippingEnd }
    },
    { RowType::Light, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::LightColor,
          PropertyType::SpecularColor,
          PropertyType::AmbientColor,
          PropertyType::Brightness,
          PropertyType::ShadowDarkness,
          PropertyType::ShadowSoftness,
          PropertyType::ShadowDepthBias,
          PropertyType::ShadowFarClip,
          PropertyType::ShadowFOV }
    },
    { RowType::Object, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::Opacity,
          PropertyType::EdgeTessellation,
          PropertyType::InnerTessellation }
    },
    { RowType::Text, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::Opacity,
          PropertyType::TextColor,
          PropertyType::Leading,
          PropertyType::Tracking }
    },
    { RowType::Alias, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::Opacity }
    },
    { RowType::Group, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::Opacity }
    },
    { RowType::Component, {
          PropertyType::Position,
          PropertyType::Rotation,
          PropertyType::Scale,
          PropertyType::Pivot,
          PropertyType::Opacity }
    }
};
