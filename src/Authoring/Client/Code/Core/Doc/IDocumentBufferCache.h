/****************************************************************************
**
** Copyright (C) 1999-2005 Anark Corporation.
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
#ifndef INCLUDED_IDOCUMENTBUFFERCACHE_H
#define INCLUDED_IDOCUMENTBUFFERCACHE_H
#include "Qt3DSFileTools.h"
#include "StudioObjectTypes.h"
#include "Q3DSImageTextureData.h"
#include "Q3DSRenderMesh.h"

#include <QtCore/qfileinfo.h>

class CDoc;

namespace Q3DStudio {

struct SModelBufferAndPath
{
    Q3DSRenderMesh *m_modelBuffer;
    QFileInfo m_filePath;
    QString m_identifier;

    SModelBufferAndPath(Q3DSRenderMesh *inBuffer, const QFileInfo &inPath, const QString &identifier)
        : m_modelBuffer(inBuffer)
        , m_filePath(inPath)
        , m_identifier(identifier)
    {
    }

    SModelBufferAndPath()
        : m_modelBuffer(NULL)
    {
    }

    operator Q3DSRenderMesh *() { return m_modelBuffer; }
    Q3DSRenderMesh *operator->() { return m_modelBuffer; }
};

class IDocumentBufferCache
{
protected:
    virtual ~IDocumentBufferCache() {}
public:
    friend class std::shared_ptr<IDocumentBufferCache>;
    virtual QStringList GetPrimitiveNames() = 0;
    virtual QString GetPrimitiveName(EPrimitiveType inPrimitiveType) = 0;
    // Get or create the model buffer.  May return NULL if the sourcepath doesn't
    // map to a loadable model buffer.
    // Takes a *relative* path from the document
    virtual SModelBufferAndPath GetOrCreateModelBuffer(const QString &inSourcePath) = 0;
    //
    // Get or create the image buffer.  May return NULL if the sourcepath doesn't
    // map to a loadable image buffer.
    // Takes a *relative* path from the document
    virtual Q3DSImageTextureData GetOrCreateImageBuffer(const QString &inSourcePath) = 0;

    virtual void
    GetImageBuffers(std::vector<std::pair<QString, Q3DSImageTextureData>> &outBuffers) = 0;

    // Takes a *relative* path from the document
    virtual void InvalidateBuffer(const QFileInfo &inSourcePath) = 0;

    // Don't send events but just clear everything out.  Used on document::close
    virtual void Clear() = 0;

    static std::shared_ptr<IDocumentBufferCache> CreateBufferCache(CDoc &inDoc);
};
}

#endif
