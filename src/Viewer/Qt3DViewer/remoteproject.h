/****************************************************************************
**
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


#ifndef REMOTEPROJECT_H
#define REMOTEPROJECT_H

#include <QtCore/qobject.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qtemporarydir.h>
#include <QtNetwork/qtcpserver.h>
#include <QtWidgets/qwidget.h>

class RemoteProject : public QObject
{
    Q_OBJECT
public:
    explicit RemoteProject(QWidget *parent);
    ~RemoteProject();

    bool startServer();

    QHostAddress hostAddress() const { return m_hostAddress; }
    int serverPort() const { return m_serverPort; }
    void setServerPort(int port) { m_serverPort = port; }
    bool isConnected() const { return m_connection; }
    QString fileName() const { return m_projectFile; }

Q_SIGNALS:
    void projectChanged();
    void remoteConnected();
    void remoteDisconnected();

private Q_SLOTS:
    void acceptRemoteConnection();
    void acceptRemoteDisconnection();
    void readProject();

private:
    QWidget *m_mainWindow = nullptr;
    QTcpServer *m_tcpServer = nullptr;
    QTcpSocket *m_connection = nullptr;
    QHostAddress m_hostAddress;
    QDataStream m_incoming;
    QTemporaryDir *m_temporaryDir = nullptr;
    QString m_projectFile;
    int m_serverPort;
};

#endif // REMOTEPROJECT_H
