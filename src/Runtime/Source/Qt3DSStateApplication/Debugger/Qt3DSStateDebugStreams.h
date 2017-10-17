/****************************************************************************
**
** Copyright (C) 2013 NVIDIA Corporation.
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
#ifndef QT3DS_STATE_DEBUG_STREAMS_H
#define QT3DS_STATE_DEBUG_STREAMS_H
#pragma once
#include "Qt3DSState.h"
#include "foundation/Socket.h"
#include "foundation/Qt3DSTime.h"
#include "Qt3DSStateDebugger.h"

struct lua_State;

namespace qt3ds {
namespace state {
    namespace debugger {

        class IMultiProtocolSocketStream : public IDebugOutStream
        {
        public:
            virtual CRegisteredString GetProtocolName() = 0;
        };

        class IMultiProtocolSocketListener : public NVRefCounted
        {
        public:
            // If a listener is returned from on new protocol, the system creates a stream
            // with the returned listener
            virtual IDebugStreamListener *OnNewProtocol(CRegisteredString inProtocolName) = 0;
            // Created with the listener returned from OnNewProtocol.
            virtual void OnNewProtocolStream(CRegisteredString inProtocolName,
                                             IMultiProtocolSocketStream &inStream) = 0;
        };

        // Create a system of multiplexing multipler unrelated protocols
        // through a single network socket.
        class IMultiProtocolSocket : public NVRefCounted
        {
        public:
            virtual bool Initialize() = 0;
            virtual NVScopedRefCounted<IMultiProtocolSocketStream>
            CreateProtocol(const char *name, IDebugStreamListener *inListener) = 0;
            virtual NVScopedRefCounted<IMultiProtocolSocketStream>
            GetProtocol(const char *name) = 0;
            virtual bool Connected() = 0;

            // Upon connection the multi protocol system does a handshake where both sides send
            // their nanosecond
            // conversions across.  Note that the times sent on the multi protocol packets are in a
            // system-specific 64 bit quantity that
            // needs conversion to actual nanoseconds to be useful (identical to
            // QueryHighPerformanceFrequency, QueryHighPerformanceCounter
            virtual CounterFrequencyToTensOfNanos SourceConversion() = 0;

            // Manually do a nonblocking check on the network socket for any new information and
            // call the various stream listeners
            // with information packets if found.
            virtual void MessagePump() = 0;

            static NVScopedRefCounted<IMultiProtocolSocket>
            CreateProtocolSocket(NVFoundationBase &fnd, SocketStream &inStream,
                                 IStringTable &strTable,
                                 IMultiProtocolSocketListener *protocolListener);
        };

        struct LuaStreamError
        {
            enum _Enum {
                NoLuaError = 0,
                Timeout,
                Closed,
            };
        };

        class CProtocolNames
        {
        public:
            static const char *getMobdebugProtocolName() { return "mobdebug"; }
            static const char *getSCXMLProtocolName() { return "scxml"; }
        };

        // Maintains a queue of messages and runs through them when asked.
        // Mirrors the lua socket design and names so that systems built on lua
        // socket can be retrofitted to channel through a multiple protocol stream
        // as easily as possible.
        class IMultiProtocolStreamLuaCompat : public NVRefCounted
        {
        public:
            // timeout is just used to indicate if this should be
            // blocking or not.
            virtual void settimeout(QT3DSU32 value = 0) = 0;
            virtual QT3DSU32 gettimeout() = 0;
            // returns next line of data
            // or error.  If error, retval.second will be false.
            // reads a line at a time.
            virtual eastl::pair<const char *, LuaStreamError::_Enum> receive(QT3DSU32 numBytes) = 0;
            virtual void send(const char *data) = 0;
            virtual bool hasData() = 0;

            static const char *getProtocolName()
            {
                return CProtocolNames::getMobdebugProtocolName();
            }

            static NVScopedRefCounted<IMultiProtocolStreamLuaCompat>
            Create(NVFoundationBase &fnd, IMultiProtocolSocket &socket);

            // Puts a table at the top of the stack that binds all the functions of the stream.
            static void Bind(lua_State *state, IMultiProtocolStreamLuaCompat &stream);
        };
    }
}
}

#endif