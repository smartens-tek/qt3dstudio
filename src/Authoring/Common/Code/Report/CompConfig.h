/****************************************************************************
**
** Copyright (C) 1999-2002 NVIDIA Corporation.
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

#ifndef INCLUDED_COMP_CONFIG_H
#define INCLUDED_COMP_CONFIG_H 1

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//==============================================================================
//	Includes
//==============================================================================

#include <iostream>
#include <string>
#include "Qt3DSString.h"
//#include <string>
//#include <vector>
//#include <Wbemidl.h>

// using namespace std;
//==============================================================================
//	Constants
//==============================================================================

//==============================================================================
//	Class
//==============================================================================
//==============================================================================
/**
 *	@class	CCompConfig
 *
 */
//==============================================================================
class CCompConfig
{
    //==============================================================================
    //	Fields
    //==============================================================================

protected:
    unsigned long m_TabCount; ///< Number of tabs to print when writing to XML output

    //==============================================================================
    //	Methods
    //==============================================================================

    // Construction

public:
    CCompConfig();
    virtual ~CCompConfig();

    // Access

public:
    virtual void DetermineConfig() = 0;
    void SetTabCount(unsigned long inTabCount);

    virtual void WriteXML(std::ostream &inStream) = 0;
};

#endif // INCLUDED_COMP_CONFIG_H
