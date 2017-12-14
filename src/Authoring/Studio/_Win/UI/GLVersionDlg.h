/****************************************************************************
**
** Copyright (C) 2006 NVIDIA Corporation.
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

#ifndef INCLUDED_GL_VERSION_DLG
#define INCLUDED_GL_VERSION_DLG 1

#pragma once

//==============================================================================
//	 Includes
//==============================================================================
#include "Qt3DSString.h"

#include <QtWidgets/qdialog.h>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class GLVersionDlg;
}
QT_END_NAMESPACE

//==============================================================================
/**
 *	CGLVersionDlg: Dialog class for showing Open GL Version Warning
 */
//==============================================================================
class CGLVersionDlg : public QDialog
{
    Q_OBJECT
public:
    CGLVersionDlg(QWidget *pParent = nullptr); // standard constructor
    ~CGLVersionDlg();

    void Initialize(const QString &inTitle, const QString &inMessage, bool inErrorIcon);
    bool GetDontShowAgain();

private:
    QScopedPointer<QT_PREPEND_NAMESPACE(Ui::GLVersionDlg)> m_ui;
};

#endif // INCLUDED_GL_VERSION_DLG
