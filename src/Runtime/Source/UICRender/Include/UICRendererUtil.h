/****************************************************************************
**
** Copyright (C) 2008-2012 NVIDIA Corporation.
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
#pragma once
#ifndef UIC_RENDERER_UTIL_H
#define UIC_RENDERER_UTIL_H
#include "UICRender.h"
#include "render/Qt3DSRenderBaseTypes.h"
namespace uic {
namespace render {
    class CRendererUtil
    {
        static const QT3DSU32 MAX_SSAA_DIM = 8192; // max render traget size for SSAA mode

    public:
        static void ResolveMutisampleFBOColorOnly(IResourceManager &inManager,
                                                  CResourceTexture2D &ioResult,
                                                  NVRenderContext &inRenderContext, QT3DSU32 inWidth,
                                                  QT3DSU32 inHeight,
                                                  NVRenderTextureFormats::Enum inColorFormat,
                                                  NVRenderFrameBuffer &inSourceFBO);

        static void ResolveSSAAFBOColorOnly(IResourceManager &inManager,
                                            CResourceTexture2D &ioResult, QT3DSU32 outWidth,
                                            QT3DSU32 outHeight, NVRenderContext &inRenderContext,
                                            QT3DSU32 inWidth, QT3DSU32 inHeight,
                                            NVRenderTextureFormats::Enum inColorFormat,
                                            NVRenderFrameBuffer &inSourceFBO);

        static void GetSSAARenderSize(QT3DSU32 inWidth, QT3DSU32 inHeight, QT3DSU32 &outWidth,
                                      QT3DSU32 &outHeight);
    };
}
}

#endif