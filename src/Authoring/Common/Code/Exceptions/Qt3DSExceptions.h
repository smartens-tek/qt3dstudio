/****************************************************************************
**
** Copyright (C) 1999-2001 NVIDIA Corporation.
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

#ifndef _QT3DS_EXCEPTIONS_H__
#define _QT3DS_EXCEPTIONS_H__

//==============================================================================
//	Includes
//==============================================================================

#include "Qt3DSExceptionClass.h"

//==============================================================================
//	Constants
//==============================================================================

const unsigned long LEVEL_SUCCESS = 0x00000000;
const unsigned long LEVEL_INFORMATIONAL = 0x00000001;
const unsigned long LEVEL_WARNING = 0x00000002;
const unsigned long LEVEL_ERROR = 0x00000003;

//==============================================================================
//	Macros
//==============================================================================

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define QT3DS_DEFINE_THISFILE static wchar_t QT3DS_THIS_FILE[] = __WFILE__;

// The class
#define QT3DS_EXCEPTION Qt3DSExceptionClass

// An exception with no extra parameters.
#define QT3DS_THROW(inErrorCode) QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode);

// An exception with one extra paramter.
#define QT3DS_THROW1(inErrorCode, inParam1)                                                          \
    QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1);

// An exception with two extra paramters.
#define QT3DS_THROW2(inErrorCode, inParam1, inParam2)                                                \
    QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1, inParam2);

// An exception with no extra parameters, only if inHResult doesn't SUCCEED().
#define QT3DS_THROWFAIL(inHResult, inErrorCode)                                                      \
    if (FAILED(inHResult))                                                                         \
        QT3DS_EXCEPTION::ThrowFail(inHResult, __WFILE__, __LINE__, inErrorCode);

// An exception with one extra paramter, is thrown only if inHResult doesn't SUCCEED().
#define QT3DS_THROWFAIL1(inHResult, inErrorCode, inParam1)                                           \
    if (FAILED(inHResult))                                                                         \
        QT3DS_EXCEPTION::ThrowFail(inHResult, __WFILE__, __LINE__, inErrorCode, inParam1);

// An exception with two extra paramters, is thrown only if inHResult doesn't SUCCEED().
#define QT3DS_THROWFAIL2(inHResult, inErrorCode, inParam1, inParam2)                                 \
    if (FAILED(inHResult))                                                                         \
        QT3DS_EXCEPTION::ThrowFail(inHResult, __WFILE__, __LINE__, inErrorCode, inParam1, inParam2);

// An exception with no extra parameters, only if inCondition is false.
#define QT3DS_THROWFALSE(inCondition, inErrorCode)                                                   \
    if (!(inCondition))                                                                            \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode);

// An exception with one extra paramter, is thrown only if inCondition is false.
#define QT3DS_THROWFALSE1(inCondition, inErrorCode, inParam1)                                        \
    if (!(inCondition))                                                                            \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1);

// An exception with two extra paramters, is thrown only if inCondition is false.
#define QT3DS_THROWFALSE2(inCondition, inErrorCode, inParam1, inParam2)                              \
    if (!(inCondition))                                                                            \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1, inParam2);

// An exception with no extra parameters, only if inPointer is NULL.
#define QT3DS_THROWNULL(inPointer, inErrorCode)                                                      \
    if ((inPointer) == NULL)                                                                       \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode);

// An exception with one extra parameters, only if inPointer is NULL.
#define QT3DS_THROWNULL1(inPointer, inErrorCode, inParam1)                                           \
    if ((inPointer) == NULL)                                                                       \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1);

// An exception with two extra parameters, only if inPointer is NULL.
#define QT3DS_THROWNULL2(inPointer, inErrorCode, inParam1, inParam2)                                 \
    if ((inPointer) == NULL)                                                                       \
        QT3DS_EXCEPTION::Throw(__WFILE__, __LINE__, inErrorCode, inParam1, inParam2);

// Standard catch that transfers the HRESULT from the exception
#define QT3DS_CATCH(outResult)                                                                       \
    catch (QT3DS_EXCEPTION & inException) { outResult = inException.GetErrorCode(); }                \
    catch (...) { outResult = E_FAIL; }

// Catch that transfers the HRESULT and sets the resource string from the exception
// DEPRECATED: DO not use!!!!!!!
#define QT3DS_CATCHERRORINFO(outResult)                                                              \
    catch (QT3DS_EXCEPTION & inException) { outResult = inException.GetErrorCode(); }                \
    catch (...) { outResult = E_FAIL; }

// Wrapper around DOM calls to catch and translate exceptions resource strings to ErrorInfo
#define QT3DS_DOMTRYCATCH(inStatement)                                                               \
    {                                                                                              \
        HRESULT theResult;                                                                         \
                                                                                                   \
        try {                                                                                      \
            theResult = inStatement;                                                               \
        }                                                                                          \
        QT3DS_CATCHERRORINFO(theResult)                                                              \
                                                                                                   \
        return theResult;                                                                          \
    }

#ifndef _WIN32
#define E_FAIL 0x80004005L
#endif

#endif // #ifndef _QT3DS_EXCEPTIONS_H__