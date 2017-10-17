/****************************************************************************
**
** Copyright (C) 1993-2009 NVIDIA Corporation.
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt 3D Studio.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#include "RuntimePrefix.h"

//==============================================================================
//	Includes
//==============================================================================
#include "SystemPrefix.h"
#include "Qt3DSSyncPrimitive.h"

//==============================================================================
//	Namespace
//==============================================================================
namespace Q3DStudio {

//==============================================================================
/**
 *	Constructor
 */
CSyncPrimitive::CSyncPrimitive()
    : m_LockHandle(0)
{
    AppCreateLockPrimitive(&m_LockHandle);
}

//==============================================================================
/**
 *	Destructor
 */
CSyncPrimitive::~CSyncPrimitive()
{
    AppDestroyLockPrimitive(&m_LockHandle);
}

//==============================================================================
/**
 *
 */
int CSyncPrimitive::BeginSync()
{
    AppBeginSync(m_LockHandle);
    return 1;
}

//==============================================================================
/**
 *
 */
int CSyncPrimitive::EndSync()
{
    AppEndSync(m_LockHandle);
    return 1;
}

} // namespace Q3DStudio