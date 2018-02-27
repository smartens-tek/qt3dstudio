/****************************************************************************
**
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

#include "ProjectContextMenu.h"
#include "ProjectView.h"

ProjectContextMenu::ProjectContextMenu(ProjectView *parent, int index)
    : QMenu(parent)
    , m_view(parent)
    , m_index(index)
{
    QAction *action = new QAction(tr("Show in Explorer"));
    connect(action, &QAction::triggered, this, &ProjectContextMenu::handleShowInExplorer);
    addAction(action);

    addSeparator();

    action = new QAction(tr("Copy Path"));
    connect(action, &QAction::triggered, this, &ProjectContextMenu::handleCopyPath);
    addAction(action);

    action = new QAction(tr("Copy Full Path"));
    connect(action, &QAction::triggered, this, &ProjectContextMenu::handleCopyFullPath);
    addAction(action);

    addSeparator();

    action = new QAction(tr("Import Assets..."));
    connect(action, &QAction::triggered, this, &ProjectContextMenu::handleImportAssets);
    addAction(action);

    if (m_view->isRefreshable(m_index)) {
        addSeparator();
        action = new QAction(tr("Refresh Import..."));
        connect(action, &QAction::triggered, this, &ProjectContextMenu::handleRefreshImport);
        addAction(action);
    }
}

ProjectContextMenu::~ProjectContextMenu()
{
}

void ProjectContextMenu::handleShowInExplorer()
{
    m_view->showInExplorer(m_index);
}

void ProjectContextMenu::handleCopyPath()
{
    m_view->copyPath(m_index);
}

void ProjectContextMenu::handleCopyFullPath()
{
    m_view->copyFullPath(m_index);
}

void ProjectContextMenu::handleRefreshImport()
{
    m_view->refreshImport(m_index);
}

void ProjectContextMenu::handleImportAssets()
{
    m_view->assetImportInContext(m_index);
}
