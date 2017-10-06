/****************************************************************************
**
** Copyright (C) 2002 NVIDIA Corporation.
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

#ifndef INCLUDED_TREE_BLANK_CONTROL_H
#define INCLUDED_TREE_BLANK_CONTROL_H 1

#pragma once

//==============================================================================
//	Includes
//==============================================================================
#include "BlankControl.h"

//==============================================================================
//	Forwards
//==============================================================================
class CRenderer;

//=============================================================================
/**
 * Extends the blank control to draw items specific to the tree view side of the
 * timeline palette.
 */
class CTreeBlankControl : public CBlankControl
{
public:
    CTreeBlankControl(CColor inColor = CStudioPreferences::GetBaseColor());
    virtual ~CTreeBlankControl();
    void Draw(CRenderer *inRenderer) override;
    void SetVisiblePositionX(long inPosition);

protected:
    long m_LeftEdgeOffset; ///< Visibility offset of the left edge in case timeline is scrolled
                           ///horizontally
};

#endif // INCLUDED_TREE_BLANK_CONTROL_H
