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

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <QQuickWidget>
#include <QScopedPointer>

#include "DispatchListeners.h"
#include "ObjectListModel.h"
#include "Qt3DSDMHandles.h"
#include "Qt3DSDMSignals.h"
#include "SelectedValueImpl.h"
#include "TimelineObjectModel.h"

class CDoc;
class CTimelineTranslationManager;

QT_FORWARD_DECLARE_CLASS(QAbstractItemModel)

class TimelineView : public QQuickWidget, public CPresentationChangeListener
{
    Q_OBJECT
    Q_PROPERTY(QAbstractItemModel *objectModel READ objectModel NOTIFY objectModelChanged FINAL)
    Q_PROPERTY(int selection READ selection WRITE setSelection NOTIFY selectionChanged FINAL)
public:
    explicit TimelineView(QWidget *parent = nullptr);

    QAbstractItemModel *objectModel() const;

    int selection() const { return m_selection; }
    void setSelection(int index);

    // Presentation Change Listener
    void OnNewPresentation() override;
    void OnClosingPresentation() override;
    void OnSelectionChange(Q3DStudio::SSelectedValue inNewSelectable);

    Q_INVOKABLE void select(int index, Qt::KeyboardModifiers modifiers);


Q_SIGNALS:
    void objectModelChanged();
    void selectionChanged();

protected:
    // DataModel callbacks
    virtual void OnActiveSlide(const qt3dsdm::Qt3DSDMSlideHandle &inMaster, int inIndex,
                               const qt3dsdm::Qt3DSDMSlideHandle &inSlide);
    void OnAnimationDeleted(qt3dsdm::Qt3DSDMInstanceHandle parentInstance,
                            qt3dsdm::Qt3DSDMPropertyHandle property);
    void OnAnimationCreated(qt3dsdm::Qt3DSDMInstanceHandle parentInstance,
                            qt3dsdm::Qt3DSDMPropertyHandle property);

private:
    void initialize();
    inline CDoc *GetDoc();

    std::vector<std::shared_ptr<qt3dsdm::ISignalConnection>>
        m_Connections; /// connections to the DataModel
    QScopedPointer<FlatObjectListModel> m_model;
    QScopedPointer<TimelineObjectModel> m_objectListModel;
    qt3dsdm::Qt3DSDMSlideHandle m_activeSlide;

    CTimelineTranslationManager *m_translationManager = nullptr;

    int m_selection = -1;
    QColor m_BaseColor = QColor::fromRgb(75, 75, 75);
};

#endif // TIMELINEVIEW_H
