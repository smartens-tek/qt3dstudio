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
#ifndef INCLUDED_QT3DSDM_INSPECTORGROUP_H
#define INCLUDED_QT3DSDM_INSPECTORGROUP_H 1

#pragma once

//==============================================================================
//	Includes
//==============================================================================
#include "EasyInspectorGroup.h"
#include "Qt3DSDMHandles.h"
#include "StudioApp.h"

class CDoc;
namespace Q3DStudio {
class Qt3DSDMInspectorRow;
};

class Qt3DSDMInspectable;

//==============================================================================
/**
 *
 */
class Qt3DSDMInspectorGroup: public CEasyInspectorGroup
{
protected: // Members
    CStudioApp &m_App;
    std::vector<Q3DStudio::Qt3DSDMInspectorRow *> m_DMInspectorRows;
    Qt3DSDMInspectable &m_Inspectable;
    long m_Index;

public: // Construction
    Qt3DSDMInspectorGroup(CStudioApp &inApp, const QString &inName,
                         Qt3DSDMInspectable &inInspectable, long inIndex);
    ~Qt3DSDMInspectorGroup();

    const std::vector<Q3DStudio::Qt3DSDMInspectorRow *> &GetRows() const
    {
        return m_DMInspectorRows;
    }

public: // Use
    void CreateRow(CDoc *inDoc, qt3dsdm::Qt3DSDMMetaDataPropertyHandle inProperty);
};

#endif