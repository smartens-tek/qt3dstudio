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

#include "q3dspresentation_p.h"
#include "q3dssceneelement_p.h"
#include "q3dscommandqueue_p.h"
#include "viewerqmlstreamproxy_p.h"

#include <QtCore/qdebug.h>
#include <QtCore/qsettings.h>
#include <QtCore/qcoreapplication.h>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>

QT_BEGIN_NAMESPACE

Q3DSPresentation::Q3DSPresentation(QObject *parent)
    : QObject(parent)
    , d_ptr(new Q3DSPresentationPrivate(this))
{
}

Q3DSPresentation::~Q3DSPresentation()
{
}

QUrl Q3DSPresentation::source() const
{
    return d_ptr->m_source;
}

void Q3DSPresentation::registerElement(Q3DSElement *element)
{
    d_ptr->registerElement(element);
}

void Q3DSPresentation::unregisterElement(Q3DSElement *element)
{
    d_ptr->unregisterElement(element);
}

Q3DSElement *Q3DSPresentation::registeredElement(const QString &elementPath) const
{
    return d_ptr->m_elements.value(elementPath, nullptr);
}

void Q3DSPresentation::setSource(const QUrl &source)
{
    if (d_ptr->m_source != source) {
        d_ptr->setSource(source);
        Q_EMIT sourceChanged(source);
    }
}

void Q3DSPresentation::goToSlide(const QString &elementPath, unsigned int index)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        d_ptr->m_viewerApp->GoToSlideByIndex(path, index);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_GoToSlide, int(index));
    }
}

void Q3DSPresentation::goToSlide(const QString &elementPath, const QString &name)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        const QByteArray byteName(name.toUtf8());
        d_ptr->m_viewerApp->GoToSlideByName(path, byteName);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_GoToSlideByName, name);
    }
}

void Q3DSPresentation::goToSlide(const QString &elementPath, bool next, bool wrap)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        d_ptr->m_viewerApp->GoToSlideRelative(path, next, wrap);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_GoToSlideRelative,
                                            int(next), int(wrap));
    }
}

void Q3DSPresentation::goToTime(const QString &elementPath, float time)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        d_ptr->m_viewerApp->GoToTime(path, time);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_GoToTime, time);
    }
}

void Q3DSPresentation::setAttribute(const QString &elementPath, const QString &attributeName,
                                    const QVariant &value)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        const QByteArray name(attributeName.toUtf8());

        QByteArray valueStr;
        float valueFloat;

        const void *theValue = nullptr;
        switch (value.type()) {
        case QVariant::Bool:
        case QVariant::Int:
        case QVariant::Double:
        case QMetaType::Float:
            valueFloat = value.toFloat();
            theValue = &valueFloat;
            break;
        case QVariant::String:
        default: // Try string for other types
            valueStr = value.toString().toUtf8();
            theValue = valueStr.constData();
            break;
        }
        d_ptr->m_viewerApp->SetAttribute(path, name, (char *)theValue);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_SetAttribute,
                                            attributeName, value);
    }
}

void Q3DSPresentation::setPresentationActive(const QString &id, bool active)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray presId(id.toUtf8());
        d_ptr->m_viewerApp->SetPresentationActive(presId, active);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(id, CommandType_SetPresentationActive, active);
    }
}

void Q3DSPresentation::fireEvent(const QString &elementPath, const QString &eventName)
{
    if (d_ptr->m_viewerApp) {
        const QByteArray path(elementPath.toUtf8());
        const QByteArray name(eventName.toUtf8());
        d_ptr->m_viewerApp->FireEvent(path, name);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(elementPath, CommandType_FireEvent, eventName);
    }
}

void Q3DSPresentation::setGlobalAnimationTime(qint64 milliseconds)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->SetGlobalAnimationTime(milliseconds);
    } else {
        d_ptr->m_commandQueue->m_globalAnimationTimeChanged = true;
        d_ptr->m_commandQueue->m_globalAnimationTime = milliseconds;
    }
}

void Q3DSPresentation::mousePressEvent(QMouseEvent *e)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->HandleMousePress(e->x(), e->y(), e->button(), true);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(QString(), CommandType_MousePress,
                                           e->x(), e->y(), int(e->button()));
    }
}

void Q3DSPresentation::mouseReleaseEvent(QMouseEvent *e)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->HandleMousePress(e->x(), e->y(), e->button(), false);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(QString(), CommandType_MouseRelease,
                                           e->x(), e->y(), int(e->button()));
    }
}

void Q3DSPresentation::mouseMoveEvent(QMouseEvent *e)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->HandleMouseMove(e->x(), e->y(), true);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(QString(), CommandType_MouseMove,
                                            e->x(), e->y());
    }
}

void Q3DSPresentation::wheelEvent(QWheelEvent *e)
{
    QPoint pixelData = e->pixelDelta();
    int numSteps = 0;
    if (pixelData.isNull()) {
        if (e->orientation() == Qt::Vertical)
            numSteps = e->angleDelta().y() / 8;
        else
            numSteps = e->angleDelta().x() / 8;
    } else {
        // trackpad, pixel = one step in scroll wheel.
        if (e->orientation() == Qt::Vertical)
            numSteps = pixelData.y();
        else
            numSteps = pixelData.x();
    }
    if (numSteps != 0) {
        if (d_ptr->m_viewerApp) {
            d_ptr->m_viewerApp->HandleMouseWheel(e->x(), e->y(),
                                                 e->orientation() == Qt::Vertical ? 0 : 1,
                                                 numSteps);
        } else if (d_ptr->m_commandQueue) {
            d_ptr->m_commandQueue->queueCommand(QString(), CommandType_MouseWheel,
                                                e->x(), e->y(),
                                                int(e->orientation() == Qt::Vertical), numSteps);
        }
    }
}

void Q3DSPresentation::keyPressEvent(QKeyEvent *e)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->HandleKeyInput(d_ptr->getScanCode(e), true);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(QString(), CommandType_KeyPress,
                                            d_ptr->getScanCode(e));
    }
}

void Q3DSPresentation::keyReleaseEvent(QKeyEvent *e)
{
    if (d_ptr->m_viewerApp) {
        d_ptr->m_viewerApp->HandleKeyInput(d_ptr->getScanCode(e), false);
    } else if (d_ptr->m_commandQueue) {
        d_ptr->m_commandQueue->queueCommand(QString(), CommandType_KeyRelease,
                                            d_ptr->getScanCode(e));
    }
}

Q3DSPresentationPrivate::Q3DSPresentationPrivate(Q3DSPresentation *q)
    : QObject(q)
    , q_ptr(q)
    , m_viewerApp(nullptr)
    , m_commandQueue(nullptr)
    , m_streamProxy(nullptr)
{
}

Q3DSPresentationPrivate::~Q3DSPresentationPrivate()
{
    unregisterAllElements();
    delete m_streamProxy;
}

void Q3DSPresentationPrivate::setSource(const QUrl &source)
{
    m_source = source;
    if (m_commandQueue) {
        m_commandQueue->m_sourceChanged = true;
        m_commandQueue->m_source = source;
    }
}

void Q3DSPresentationPrivate::setViewerApp(UICViewer::UICViewerApp *app, bool connectApp)
{

    for (Q3DSElement *element : m_elements.values())
        element->d_ptr->setViewerApp(app);

    if (connectApp) {
        if (app) {
            connect(app, &UICViewer::UICViewerApp::SigSlideEntered,
                    this, &Q3DSPresentationPrivate::handleSlideEntered);
            connect(app, &UICViewer::UICViewerApp::SigSlideExited,
                    q_ptr, &Q3DSPresentation::slideExited);
        }
        if (m_viewerApp) {
            disconnect(m_viewerApp, &UICViewer::UICViewerApp::SigSlideEntered,
                       this, &Q3DSPresentationPrivate::handleSlideEntered);
            disconnect(m_viewerApp, &UICViewer::UICViewerApp::SigSlideExited,
                       q_ptr, &Q3DSPresentation::slideExited);
        }
    }

    m_viewerApp = app;
}

void Q3DSPresentationPrivate::setCommandQueue(CommandQueue *queue)
{
    for (Q3DSElement *element : m_elements.values())
        element->d_ptr->setCommandQueue(queue);

    m_commandQueue = queue;

    if (m_commandQueue)
        setSource(m_source);
}

// Doc note: The ownership of the registered scenes remains with the caller, who needs to
// ensure that registered scenes are alive as long as the presentation is alive.
void Q3DSPresentationPrivate::registerElement(Q3DSElement *element)
{
    Q_ASSERT(!element->elementPath().isEmpty());

    // Allow only single registration for each element path and scene object
    QMutableHashIterator<QString, Q3DSElement *> i(m_elements);
    while (i.hasNext()) {
        i.next();
        if (i.value() == element) {
            // If the same scene object is already registered with different path,
            // remove it from the map to avoid duplication.
            if (i.key() != element->elementPath())
                i.remove();
        } else if (i.key() == element->elementPath()) {
            // If the same element path is registered by another scene object, the old
            // scene object is unregistered.
            i.value()->d_ptr->setViewerApp(nullptr);
            i.value()->d_ptr->setPresentation(nullptr);
            i.remove();
        }
    }

    element->d_ptr->setViewerApp(m_viewerApp);
    element->d_ptr->setCommandQueue(m_commandQueue);
    element->d_ptr->setPresentation(this);

    m_elements.insert(element->elementPath(), element);
}

void Q3DSPresentationPrivate::unregisterElement(Q3DSElement *element)
{
    Q3DSElement *oldScene = m_elements.value(element->elementPath());
    if (oldScene == element) {
        element->d_ptr->setViewerApp(nullptr);
        element->d_ptr->setCommandQueue(nullptr);
        element->d_ptr->setPresentation(nullptr);
        m_elements.remove(element->elementPath());
    }
}

void Q3DSPresentationPrivate::unregisterAllElements()
{
    for (Q3DSElement *element : m_elements.values()) {
        element->d_ptr->setViewerApp(nullptr);
        element->d_ptr->setCommandQueue(nullptr);
        element->d_ptr->setPresentation(nullptr);
    }
    m_elements.clear();
}

Q3DStudio::EKeyCode Q3DSPresentationPrivate::getScanCode(QKeyEvent *e)
{
    enum {
        RIGHT_SHIFT = 0x036,
        RIGHT_CTRL = 0x11d,
        RIGHT_ALT = 0x138,
    };

    Qt::Key keyScanCode = static_cast<Qt::Key>(e->key());

    Q3DStudio::EKeyCode newScanCode = Q3DStudio::KEY_NOKEY;
    switch (keyScanCode) {
    case Qt::Key_Down:
        newScanCode = Q3DStudio::KEY_DOWN;
        break;
    case Qt::Key_Up:
        newScanCode = Q3DStudio::KEY_UP;
        break;
    case Qt::Key_Left:
        newScanCode = Q3DStudio::KEY_LEFT;
        break;
    case Qt::Key_Right:
        newScanCode = Q3DStudio::KEY_RIGHT;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        newScanCode = e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPADENTER
                                                           : Q3DStudio::KEY_RETURN;
        break;
    case Qt::Key_Backspace:
        newScanCode = Q3DStudio::KEY_BACK;
        break;
    case Qt::Key_Tab:
        newScanCode = Q3DStudio::KEY_TAB;
        break;
    case Qt::Key_Escape:
        newScanCode = Q3DStudio::KEY_ESCAPE;
        break;
    case Qt::Key_A:
        newScanCode = Q3DStudio::KEY_A;
        break;
    case Qt::Key_B:
        newScanCode = Q3DStudio::KEY_B;
        break;
    case Qt::Key_C:
        newScanCode = Q3DStudio::KEY_C;
        break;
    case Qt::Key_D:
        newScanCode = Q3DStudio::KEY_D;
        break;
    case Qt::Key_E:
        newScanCode = Q3DStudio::KEY_E;
        break;
    case Qt::Key_F:
        newScanCode = Q3DStudio::KEY_F;
        break;
    case Qt::Key_G:
        newScanCode = Q3DStudio::KEY_G;
        break;
    case Qt::Key_H:
        newScanCode = Q3DStudio::KEY_H;
        break;
    case Qt::Key_I:
        newScanCode = Q3DStudio::KEY_I;
        break;
    case Qt::Key_J:
        newScanCode = Q3DStudio::KEY_J;
        break;
    case Qt::Key_K:
        newScanCode = Q3DStudio::KEY_K;
        break;
    case Qt::Key_L:
        newScanCode = Q3DStudio::KEY_L;
        break;
    case Qt::Key_M:
        newScanCode = Q3DStudio::KEY_M;
        break;
    case Qt::Key_N:
        newScanCode = Q3DStudio::KEY_N;
        break;
    case Qt::Key_O:
        newScanCode = Q3DStudio::KEY_O;
        break;
    case Qt::Key_P:
        newScanCode = Q3DStudio::KEY_P;
        break;
    case Qt::Key_Q:
        newScanCode = Q3DStudio::KEY_Q;
        break;
    case Qt::Key_R:
        newScanCode = Q3DStudio::KEY_R;
        break;
    case Qt::Key_S:
        newScanCode = Q3DStudio::KEY_S;
        break;
    case Qt::Key_T:
        newScanCode = Q3DStudio::KEY_T;
        break;
    case Qt::Key_U:
        newScanCode = Q3DStudio::KEY_U;
        break;
    case Qt::Key_V:
        newScanCode = Q3DStudio::KEY_V;
        break;
    case Qt::Key_W:
        newScanCode = Q3DStudio::KEY_W;
        break;
    case Qt::Key_X:
        newScanCode = Q3DStudio::KEY_X;
        break;
    case Qt::Key_Y:
        newScanCode = Q3DStudio::KEY_Y;
        break;
    case Qt::Key_Z:
        newScanCode = Q3DStudio::KEY_Z;
        break;
    case Qt::Key_0:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD0 : Q3DStudio::KEY_0;
        break;
    case Qt::Key_1:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD1 : Q3DStudio::KEY_1;
        break;
    case Qt::Key_2:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD2 : Q3DStudio::KEY_2;
        break;
    case Qt::Key_3:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD3 : Q3DStudio::KEY_3;
        break;
    case Qt::Key_4:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD4 : Q3DStudio::KEY_4;
        break;
    case Qt::Key_5:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD5 : Q3DStudio::KEY_5;
        break;
    case Qt::Key_6:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD6 : Q3DStudio::KEY_6;
        break;
    case Qt::Key_7:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD7 : Q3DStudio::KEY_7;
        break;
    case Qt::Key_8:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD8 : Q3DStudio::KEY_8;
        break;
    case Qt::Key_9:
        newScanCode =
                e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPAD9 : Q3DStudio::KEY_9;
        break;
    case Qt::Key_Minus:
        newScanCode = e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPADSUBTRACT
                                                           : Q3DStudio::KEY_SUBTRACT;
        break;
    case Qt::Key_Plus:
        newScanCode = e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPADADD
                                                           : Q3DStudio::KEY_EQUALS;
        break;
    case Qt::Key_NumLock:
        newScanCode = Q3DStudio::KEY_NUMLOCK;
        break;
    case Qt::Key_ScrollLock:
        newScanCode = Q3DStudio::KEY_SCROLL;
        break;
    case Qt::Key_CapsLock:
        newScanCode = Q3DStudio::KEY_CAPITAL;
        break;
    case Qt::Key_Pause:
        newScanCode = Q3DStudio::KEY_PAUSE;
        break;
    case Qt::Key_Print:
        newScanCode = Q3DStudio::KEY_PRINTSCREEN;
        break;
    case Qt::Key_Insert:
        newScanCode = Q3DStudio::KEY_INSERT;
        break;
    case Qt::Key_Delete:
        newScanCode = Q3DStudio::KEY_DELETE;
        break;
    case Qt::Key_Home:
        newScanCode = Q3DStudio::KEY_HOME;
        break;
    case Qt::Key_End:
        newScanCode = Q3DStudio::KEY_END;
        break;
    case Qt::Key_PageUp:
        newScanCode = Q3DStudio::KEY_PGUP;
        break;
    case Qt::Key_PageDown:
        newScanCode = Q3DStudio::KEY_PGDN;
        break;
    case Qt::Key_F1:
        newScanCode = Q3DStudio::KEY_F1;
        break;
    case Qt::Key_F2:
        newScanCode = Q3DStudio::KEY_F2;
        break;
    case Qt::Key_F3:
        newScanCode = Q3DStudio::KEY_F3;
        break;
    case Qt::Key_F4:
        newScanCode = Q3DStudio::KEY_F4;
        break;
    case Qt::Key_F5:
        newScanCode = Q3DStudio::KEY_F5;
        break;
    case Qt::Key_F6:
        newScanCode = Q3DStudio::KEY_F6;
        break;
    case Qt::Key_F7:
        newScanCode = Q3DStudio::KEY_F7;
        break;
    case Qt::Key_F8:
        newScanCode = Q3DStudio::KEY_F8;
        break;
    case Qt::Key_F9:
        newScanCode = Q3DStudio::KEY_F9;
        break;
    case Qt::Key_F10:
        newScanCode = Q3DStudio::KEY_F10;
        break;
    case Qt::Key_F11:
        newScanCode = Q3DStudio::KEY_F11;
        break;
    case Qt::Key_F12:
        newScanCode = Q3DStudio::KEY_F12;
        break;
    case Qt::Key_F13:
        newScanCode = Q3DStudio::KEY_F13;
        break;
    case Qt::Key_F14:
        newScanCode = Q3DStudio::KEY_F14;
        break;
    case Qt::Key_QuoteLeft:
        newScanCode = Q3DStudio::KEY_GRAVE;
        break;
    case Qt::Key_Asterisk:
        newScanCode = Q3DStudio::KEY_MULTIPLY;
        break;
    case Qt::Key_BracketRight:
        newScanCode = Q3DStudio::KEY_RBRACKET;
        break;
    case Qt::Key_BracketLeft:
        newScanCode = Q3DStudio::KEY_LBRACKET;
        break;
    case Qt::Key_Semicolon:
        newScanCode = Q3DStudio::KEY_SEMICOLON;
        break;
    case Qt::Key_Comma:
        newScanCode = Q3DStudio::KEY_COMMA;
        break;
    case Qt::Key_Period:
        newScanCode = e->modifiers() == Qt::KeypadModifier ? Q3DStudio::KEY_NUMPADDECIMAL
                                                           : Q3DStudio::KEY_PERIOD;
        break;
    case Qt::Key_Apostrophe:
        newScanCode = Q3DStudio::KEY_APOSTROPHE;
        break;
    case Qt::Key_Slash:
        newScanCode = Q3DStudio::KEY_SLASH;
        break;
    case Qt::Key_Backslash:
        newScanCode = Q3DStudio::KEY_BACKSLASH;
        break;
    case Qt::Key_Equal:
        newScanCode = Q3DStudio::KEY_EQUALS;
        break;
    case Qt::Key_Space:
        newScanCode = Q3DStudio::KEY_SPACE;
        break;
    case Qt::Key_Shift:
        newScanCode =
                e->nativeScanCode() == RIGHT_SHIFT ? Q3DStudio::KEY_RSHIFT : Q3DStudio::KEY_LSHIFT;
        break;
    case Qt::Key_Control:
        newScanCode = e->nativeScanCode() == RIGHT_CTRL ? Q3DStudio::KEY_RCONTROL
                                                        : Q3DStudio::KEY_LCONTROL;
        break;
    case Qt::Key_Alt:
        newScanCode =
                e->nativeScanCode() == RIGHT_ALT ? Q3DStudio::KEY_RALT : Q3DStudio::KEY_LALT;
        break;
    default:
        break;
    }

    return newScanCode;
}

ViewerQmlStreamProxy *Q3DSPresentationPrivate::streamProxy()
{
    if (!m_streamProxy)
        m_streamProxy = new ViewerQmlStreamProxy();
    return m_streamProxy;
}

void Q3DSPresentationPrivate::handleSlideEntered(const QString &elementPath, unsigned int index,
                                                 const QString &name)
{
    Q3DSSceneElement *scene = qobject_cast<Q3DSSceneElement *>(m_elements.value(elementPath));
    if (scene)
        scene->d_func()->handleSlideEntered(index, name);
    Q_EMIT q_ptr->slideEntered(elementPath, index, name);
}

QT_END_NAMESPACE
