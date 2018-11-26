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
#ifndef INCLUDED_QT3DS_FILE_H
#define INCLUDED_QT3DS_FILE_H 1

#pragma once

#include "Qt3DSString.h"

#include <set>
#include "Qt3DSString.h"
#include "FileIterator.h"
#include "PlatformTypes.h"
#include <QtCore/QFileInfo>

//=========================================================================================
//	Typedefs
//=========================================================================================
typedef std::set<QString> TFilePathList;
typedef TFilePathList::iterator TFilePathListIterator;

class Qt3DSFile
{

public:
    typedef void OSErr; // this may be better typedefed to HRESULT

    Qt3DSFile(const QString &inPathName, bool inIsPosix, bool inAddBase);
    Qt3DSFile(const QString &inPathName, const QString &inName);
    Qt3DSFile(const Qt3DSFile &inBasePath, const QString &inPathname, bool inIsPosix);
    Qt3DSFile(const Qt3DSFile &inFile);
    Qt3DSFile(const QString &inFile);
    Qt3DSFile(const char *inFile);
    Qt3DSFile(const QFileInfo &inFile);

    CFileIterator GetSubItems() const;

    ~Qt3DSFile();

    bool operator==(const Qt3DSFile &inRHS) const;

    bool CanRead() const;
    bool CanWrite() const;
    bool DeleteFile() const;
    bool Exists() const;

    QString GetAbsolutePath() const;
    QString GetAbsolutePosixPath() const;

    QString GetName() const;
    QString GetStem() const;
    QString GetExtension() const;
    QString GetPath() const;

    bool IsFile(bool inCheckForAlias = true) const;
    bool IsHidden() const;
    long Length() const;

    OSErr MoveTo(const Qt3DSFile &inDestination);
    void CopyTo(const Qt3DSFile &inDestination);

    void Execute() const;

    OSErr SetReadOnly(bool inReadOnlyFlag);

    static QString GetApplicationDirectory();
    static Qt3DSFile GetTemporaryFile(const QString &inExtension);
    static Qt3DSFile GetTemporaryFile();
    static bool IsPathRelative(const QString &inPath);

    QUrl GetURL() const;

    static Qt3DSFile Combine(const Qt3DSFile &inFile, const QString &inRelPath);
    HANDLE OpenFileReadHandle() const;
    HANDLE OpenFileWriteHandle() const;
    void RenameTo(const Qt3DSFile &inDestination);

    static void ClearCurrentTempCache();

    static void AddTempFile(const QString &inFile);

    // protected functions
protected:
    static TFilePathList s_TempFilePathList; ///< List of temporary files that gets created; this
                                             ///should be cleared at end of program execution by
                                             ///calling ClearTempCache
    QString m_Path;
};

#endif // INCLUDED_QT3DS_FILE_H
