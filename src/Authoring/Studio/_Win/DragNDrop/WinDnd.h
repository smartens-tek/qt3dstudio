/****************************************************************************
**
** Copyright (C) 1999-2003 NVIDIA Corporation.
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

//==============================================================================
//	Includes
//==============================================================================
#ifndef INCLUDED_WINDND
#define INCLUDED_WINDND

#include <map>

typedef struct _SDropItem
{
    long m_Flavor; // The Type of Data.
    void *m_Data; ///< A nullptr value will cause the Data to be created Later on ( Promise )
    unsigned long m_Size; //< The size of the Data.
} SDropItem;

//=============================================================================
/**
 * This class only supports dragging one item.
 */
class CWinDragManager
{
protected:
    SDropItem m_DragItem; ///< the dragged item

public:
    CWinDragManager();
    virtual ~CWinDragManager();

    void Track();
    void StartDrag();

    void AddData(long inFlavor, void *inData, unsigned long inSize);
};

#endif
