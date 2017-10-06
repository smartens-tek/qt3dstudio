/****************************************************************************
**
** Copyright (C) 2008 NVIDIA Corporation.
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

#ifndef INCLUDED_IKEYFRAMES_MANAGER_H
#define INCLUDED_IKEYFRAMES_MANAGER_H 1

#pragma once

//=============================================================================
/**
 * Interface to manage keyframes related actions
 */
//=============================================================================
class IKeyframesManager
{
public:
    virtual ~IKeyframesManager() {}

    virtual bool HasSelectedKeyframes(bool inOnlyDynamic = false) = 0;
    virtual bool HasDynamicKeyframes() = 0;
    virtual bool CanPerformKeyframeCopy() = 0;
    virtual bool CanPerformKeyframePaste() = 0;
    virtual void CopyKeyframes() = 0;
    virtual bool RemoveKeyframes(bool inPerformCopy) = 0;
    virtual void PasteKeyframes() = 0;
    virtual void SetKeyframeInterpolation() = 0;
    virtual void SelectAllKeyframes() = 0;
    virtual void DeselectAllKeyframes() = 0;
    virtual void SetChangedKeyframes() = 0;
};

#endif // INCLUDED_IKEYFRAMES_MANAGER_H
