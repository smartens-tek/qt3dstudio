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

//==============================================================================
//	Prefix
//==============================================================================
#ifndef INCLUDED_OFFSCREEN_RENDERER_H
#define INCLUDED_OFFSCREEN_RENDERER_H 1

//==============================================================================
//	Includes
//==============================================================================
#include "WinRenderer.h"

//=============================================================================
/**
 * Class for creating a renderer compatible with the current display, but that
 * does not actually draw to the display.  Provided so that you controls can
 * query text size outside of their draw functions.  This class could be
 * further extended to provide offscreen drawing and blitting, though it's
 * not currently set up to do so.
 */
class COffscreenRenderer : public CWinRenderer
{
public:
    COffscreenRenderer(const QRect &inClippingRect);
    virtual ~COffscreenRenderer();

    QPixmap pixmap() const override { return m_pixmap;}

protected:
    QPixmap m_pixmap;
};

#endif // INCLUDED_OFFSCREEN_RENDERER_H
