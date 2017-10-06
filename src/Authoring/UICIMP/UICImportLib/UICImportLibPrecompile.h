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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT                                                                               \
    0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#include "foundation/Qt3DS.h"
#include "foundation/Qt3DSDataRef.h"
#include "foundation/Qt3DSOption.h"
#include "render/Qt3DSRenderBaseTypes.h"
#include <map>
#include <vector>
#include <string>
#include <exception>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include "UICDMDataTypes.h"
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif
#include "foundation/Qt3DSIntrinsics.h"
#include "foundation/Qt3DSBasicTemplates.h"

namespace UICIMP {
using namespace qt3ds;
using namespace qt3ds::foundation;
using namespace qt3ds::render;
using namespace UICDM;
using namespace std;
}

// TODO: reference additional headers your program requires here
