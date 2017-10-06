/****************************************************************************
**
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
//						CommonLib Precompiled Header
//==============================================================================
#ifdef __cplusplus
#pragma once
#include "UICMacros.h"

#ifdef WIN32
//==============================================================================
//	Disable certain warnings since warnings are errors
#pragma warning(disable : 4702) // Unreachable code
#pragma warning(disable : 4290) // C++ Exception Specification ignored
#pragma warning(disable : 4514) // unreferenced inline function
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4819)
#endif

//==============================================================================
//	Common Includes
#include <stdio.h> // Standard includes MUST come first
#include <stdlib.h>
#include <wchar.h>

//==============================================================================
//	STL Includes
#ifdef WIN32
#pragma warning(push, 3) // Temporarily pop to warning level 3 while including standard headers
#pragma warning(disable : 4018) // Disable mismatched < STL warning
#endif

#include <vector>
#include <map>
#include <deque>
#include <string>
#include <stack>
#include <set>
#include <list>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <limits>
#ifdef WIN32
#pragma warning(pop) // Pop out to previous warning level (probably level 4)
#endif

//==============================================================================
//	Common Player Includes
#include "PlatformTypes.h"
#include "PlatformStrings.h"
#include "UICMath.h"
#include "UICExceptions.h"
#include "UICObjectCounter.h"
#include "UICString.h"
#include "STLHelpers.h"

#include <QtGlobal>
#endif
