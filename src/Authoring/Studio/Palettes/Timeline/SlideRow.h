/****************************************************************************
**
** Copyright (C) 2016 NVIDIA Corporation.
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

#ifndef INCLUDED_TIME_CONTEXT_ROW_H
#define INCLUDED_TIME_CONTEXT_ROW_H 1

#pragma once

#include "BaseStateRow.h"

class ISnappingListProvider;
class ITimelineControl;

class CSlideRow : public CBaseStateRow
{
public:
    CSlideRow(ITimelineItemBinding *inTimelineItem);
    virtual ~CSlideRow();

    void SetTimelineControl(ITimelineControl *inTimelineControl);

    // BaseStateRow
    void Expand(bool inExpandAll = false, bool inExpandUp = false) override;
    bool CalculateActiveStartTime() override;
    bool CalculateActiveEndTime() override;
    ISnappingListProvider *GetSnappingListProvider() const override;
    void SetSnappingListProvider(ISnappingListProvider *inProvider) override;
    ITimelineControl *GetTopControl() const override;

protected:
    // CBaseStateRow
    CBaseTimebarlessRow *CreateTimebarRow() override;
    bool PerformFilter(const CFilter &inFilter) override;

    ITimelineControl *m_TimelineControl;
};
#endif // INCLUDED_TIME_CONTEXT_ROW_H
