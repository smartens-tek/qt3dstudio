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

#ifndef QT3DS_FOUNDATION_PSSYNC_H
#define QT3DS_FOUNDATION_PSSYNC_H

#include "foundation/Qt3DS.h"
#include "foundation/Qt3DSAllocatorCallback.h"

namespace qt3ds {
namespace foundation {
    class QT3DS_FOUNDATION_API Sync
    {
    public:
        static const QT3DSU32 waitForever = 0xffffffff;

        Sync(NVAllocatorCallback &alloc);

        ~Sync();

        /** Wait on the object for at most the given number of ms. Returns
        *  true if the object is signaled. Sync::waitForever will block forever
        *  or until the object is signaled.
        */

        bool wait(QT3DSU32 milliseconds = waitForever);

        /** Signal the synchronization object, waking all threads waiting on it */

        void set();

        /** Reset the synchronization object */

        void reset();

    private:
        class SyncImpl *mImpl;
    };

} // namespace foundation
} // namespace qt3ds

#endif
