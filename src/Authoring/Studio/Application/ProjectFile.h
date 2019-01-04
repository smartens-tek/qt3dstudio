/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#ifndef PROJECTFILE_H
#define PROJECTFILE_H

#include "Qt3DSFileTools.h"
#include <QtXml/qdom.h>

namespace Q3DStudio {
class CFilePath;
class CString;
}
struct SubPresentationRecord;
class CDataInputDialogItem;

class ProjectFile : public QObject
{
    Q_OBJECT

public:
    ProjectFile();

    void create(const QString &uiaPath);
    void ensureProjectFile();
    void initProjectFile(const QString &presPath);
    static void parseDataInputElem(const QDomElement &elem,
                                   QMap<QString, CDataInputDialogItem *> &dataInputs);
    static void loadDataInputs(const QString &projFile,
                               QMap<QString, CDataInputDialogItem *> &dataInputs);
    void loadSubpresentationsAndDatainputs(
            QVector<SubPresentationRecord> &subpresentations,
            QMap<QString, CDataInputDialogItem *> &datainputs);
    void writePresentationId(const QString &id, const QString &src = {});
    void updateDocPresentationId();
    void addPresentationNode(const QString &uip, const QString &pId = {});
    void addPresentationNodes(const QHash<QString, QString> &nodeList);
    bool isUniquePresentationId(const QString &id, const QString &src = {}) const;
    QString getUniquePresentationName(const QString &presSrc) const;
    QString getProjectPath() const;
    QString getProjectFilePath() const;
    QString getProjectName() const;
    QString getPresentationId(const QString &src) const;
    QString getAbsoluteFilePathTo(const QString &relFilePath) const;
    QString getRelativeFilePathTo(const QString &absFilePath) const;
    QString createPreview();
    QMultiMap<QString, QPair<QString, QString>> getDiBindingtypesFromSubpresentations() const;

    static QString getInitialPresentationSrc(const QString &uiaPath);
    static void getPresentations(const QString &inUiaPath,
                                 QVector<SubPresentationRecord> &outSubpresentations,
                                 const QString &excludePresentationSrc = {});
    QString initialPresentation() const { return m_initialPresentation; }
    void setInitialPresentation(const QString &initialId);
    bool renamePresentationFile(const QString &oldName, const QString &newName);
    void deletePresentationFile(const QString &filePath);
    void renameMaterial(const QString &oldName, const QString &newName);
    bool duplicatePresentation(const QString &oldPres, const QString &newPres);

Q_SIGNALS:
    void presentationIdChanged(const QString &path, const QString &id);
    void assetNameChanged();

private:
    QString ensureUniquePresentationId(const QString &id) const;

    QFileInfo m_fileInfo; // uia file info
    QString m_initialPresentation;
};

#endif // PROJECTFILE_H
