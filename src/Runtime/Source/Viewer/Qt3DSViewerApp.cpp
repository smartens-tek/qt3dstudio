/****************************************************************************
**
** Copyright (C) 2013 - 2016 NVIDIA Corporation.
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

#include <algorithm>

#include "EnginePrefix.h"
#include "Qt3DSTegraApplication.h"
#include "Qt3DSViewerApp.h"
#include "Qt3DSTegraInputEngine.h"
#if !defined(Q_OS_MACOS)
#include "TCPPerfLogClientStub.h"
#endif
#include "Qt3DSInputFrame.h" // keyboard mapping
#include "foundation/Qt3DSTime.h"
#include "Qt3DSFNDTimer.h"
#include "Qt3DSAudioPlayer.h"

#include <QList>
#include <QFileInfo>
#include <QDebug>

#ifdef _MACOSX
#include <mach-o/dyld.h>
#endif

#if defined(_LINUX) || defined(_MACOSX) || defined(_LINUXPLATFORM)

namespace qt3ds {
void NVAssert(const char *exp, const char *file, int line, bool *igonore)
{
    qCritical() << "NVAssertion thrown: " << file << " " << line << " " << exp;
}
}

#ifndef EASTL_DEBUG_BREAK
void EASTL_DEBUG_BREAK()
{
    qFatal("EASTL_DEBUG_BREAK: Assertion blown");
}

#endif
#endif

#ifdef _WIN32
#include <Xinput.h>
#include <ShellAPI.h>
#include "Qt3DSInputFrame.h"

void HandleController(Q3DStudio::CTegraApplication &inApplication)
{
    static const class XInputLibrary
    {
    private:
        typedef DWORD(__stdcall *GetStateFunction)(DWORD, XINPUT_STATE *);

    public:
        XInputLibrary()
            : m_module(NULL)
            , GetState(NULL)
        {
            m_module = LoadLibraryA("XINPUT9_1_0.DLL");
            if (m_module != NULL) {
                GetState = reinterpret_cast<GetStateFunction>(GetProcAddress(m_module,
                                                                             "XInputGetState"));
            }
        }

        ~XInputLibrary()
        {
            if (m_module != NULL)
                FreeLibrary(m_module);
        }

        GetStateFunction GetState;

    private:
        HMODULE m_module;
    } library;

    static const DWORD MAX_USERS = 4;
    static const Q3DStudio::FLOAT STICK_RANGE = 32767;
    static const Q3DStudio::FLOAT TRIGGER_RANGE = 255;

    static const struct ButtonEntry
    {
        WORD XBoxButton;
        Q3DStudio::INT32 button;
    } buttonMap[] = {
    { XINPUT_GAMEPAD_START, Q3DStudio::BUTTON_START },
    { XINPUT_GAMEPAD_BACK, Q3DStudio::BUTTON_SELECT },
    { XINPUT_GAMEPAD_LEFT_THUMB, Q3DStudio::BUTTON_THUMBL },
    { XINPUT_GAMEPAD_RIGHT_THUMB, Q3DStudio::BUTTON_THUMBR },
    { XINPUT_GAMEPAD_LEFT_SHOULDER, Q3DStudio::BUTTON_L1 },
    { XINPUT_GAMEPAD_RIGHT_SHOULDER, Q3DStudio::BUTTON_R1 },
    { XINPUT_GAMEPAD_A, Q3DStudio::BUTTON_A },
    { XINPUT_GAMEPAD_B, Q3DStudio::BUTTON_B },
    { XINPUT_GAMEPAD_X, Q3DStudio::BUTTON_X },
    { XINPUT_GAMEPAD_Y, Q3DStudio::BUTTON_Y } };

    static DWORD userLastPacketNumber[MAX_USERS] = { 0 };
    static WORD userLastButtons[MAX_USERS] = { 0 };

    Q3DStudio::CInputEngine *input = inApplication.GetInputEngine();
    if (input != NULL && library.GetState) {
        // for each controller
        for (DWORD userIndex = 0; userIndex < MAX_USERS; ++userIndex) {
            DWORD &lastPacketNumber = userLastPacketNumber[userIndex];
            WORD &lastButtons = userLastButtons[userIndex];

            // get the state
            XINPUT_STATE state = { 0 };
            if (library.GetState(userIndex, &state) == ERROR_SUCCESS
                    && state.dwPacketNumber != lastPacketNumber) {
                using namespace Q3DStudio;
                const XINPUT_GAMEPAD &gamepad = state.Gamepad;
                const WORD &buttons = gamepad.wButtons;

                // handle button changes
                DWORD buttonChanges = buttons ^ lastButtons;
                for (size_t buttonMapIndex = 0;
                     buttonMapIndex < sizeof(buttonMap) / sizeof(buttonMap[0]); ++buttonMapIndex) {
                    const ButtonEntry &buttonEntry = buttonMap[buttonMapIndex];
                    if (buttonEntry.XBoxButton & buttonChanges) {
                        input->HandleButton(buttonEntry.button,
                                            buttons & buttonEntry.XBoxButton);
                    }
                }

                // handle gamepad
                if (buttons & XINPUT_GAMEPAD_DPAD_LEFT)
                    input->HandleAxis(AXIS_HAT_X, -1);
                else if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT)
                    input->HandleAxis(AXIS_HAT_X, 1);
                else
                    input->HandleAxis(AXIS_HAT_X, 0);
                if (buttons & XINPUT_GAMEPAD_DPAD_UP)
                    input->HandleAxis(AXIS_HAT_Y, -1);
                else if (buttons & XINPUT_GAMEPAD_DPAD_DOWN)
                    input->HandleAxis(AXIS_HAT_Y, 1);
                else
                    input->HandleAxis(AXIS_HAT_Y, 0);

                // handle sticks and triggers
                input->HandleAxis(AXIS_X, gamepad.sThumbLX / STICK_RANGE);
                input->HandleAxis(AXIS_Y, gamepad.sThumbLY / -STICK_RANGE);
                input->HandleAxis(AXIS_Z, gamepad.sThumbRX / STICK_RANGE);
                input->HandleAxis(AXIS_RZ, gamepad.sThumbRY / -STICK_RANGE);
                input->HandleAxis(AXIS_LTRIGGER, gamepad.bLeftTrigger / TRIGGER_RANGE);
                input->HandleAxis(AXIS_RTRIGGER, gamepad.bRightTrigger / TRIGGER_RANGE);

                // save the state
                lastButtons = buttons;
                lastPacketNumber = state.dwPacketNumber;
            }
        }
    }
}
#endif

using namespace Q3DStudio;

struct WindowRect
{
    int x;
    int y;
    int width;
    int height;
};

void PerfLogSetStringData(const char *)
{
} // Dummy defination when TCP perf logging isnt used

//-----------------------------------------------------------------------------
// KD overwrite
//-----------------------------------------------------------------------------
void kdLogMessage(const KDchar *string)
{
    qCInfo(qt3ds::TRACE_INFO) << string;
}

void kdHandleAssertion(const KDchar *condition, const KDchar *filename, KDint linenumber)
{
    qCCritical(qt3ds::TRACE_INFO) << "assertion failed: " << condition
                                  << " " << filename << " " << linenumber;
}

Q3DStudio::Qt3DSFNDTimer g_GlobalTimeProvider;

KDust GetTimeUST()
{
    // this needs to be nano seconds
    //	KDust kdTime = static_cast<KDust>( g_gobalTimer.getNanoSeconds() );
    Q3DStudio::ITimeProvider &theTimer = g_GlobalTimeProvider;
    KDust kdTime = static_cast<KDust>(theTimer.GetCurrentTimeMicroSeconds() * 1000);

    return kdTime;
}

void initResource() {
    // init runtime static resources
    Q_INIT_RESOURCE(res);
}

namespace Q3DSViewer {

struct SWindowSystemImpl : public Q3DStudio::IWindowSystem
{
    WindowRect m_Rect;
    int m_OffscreenID;
    int m_DepthBitCount;

    SSize GetWindowDimensions() override { return SSize(m_Rect.width, m_Rect.height); }
    void SetWindowDimensions(const SSize &inSize) override
    {
        m_Rect.width = inSize.m_Width;
        m_Rect.height = inSize.m_Height;
    }
    // For platforms that support it, we get the egl info for render plugins
    // Feel free to return NULL.
    SEGLInfo *GetEGLInfo() override { return NULL; }
    // on some systems we allow our default render target to be a offscreen buffer
    // otherwise return 0;
    int GetDefaultRenderTargetID() override { return m_OffscreenID; }
    // returns the depth buffer bit count for the render window
    int GetDepthBitCount() override { return m_DepthBitCount; }
};

class Q3DSViewerAppImpl
{
public:
    Q3DSViewerAppImpl(Q3DStudio::IAudioPlayer *inAudioPlayer)
        : m_tegraApp(0)
        , m_appInitSuccessful(false)
        , m_WatermarkX(-1.0f)
        , m_WatermarkY(-1.0f)
        , m_AudioPlayer(inAudioPlayer)
    {
#ifndef EMBEDDED_LINUX
        initResource();
#endif
    }
    Q3DStudio::CTegraApplication *m_tegraApp; ///< pointer to internal "tegra appliction"
    bool m_appInitSuccessful; ///< true if m_tegraApp is initialized successful
    std::vector<unsigned char> m_WatermarkData; ///< Watermark data discarded after upload
    float m_WatermarkX;
    float m_WatermarkY;

    std::vector<int> m_mouseButtons;
    Q3DStudio::IWindowSystem *m_WindowSystem;

    IAudioPlayer *m_AudioPlayer;
    QList<KDEvent *> m_pendingEvents;

    QString m_error;

    void queueMouseEvent(int index, int select, int x, int y)
    {
        KDEvent *e = new KDEvent;
        e->timestamp = GetTimeUST();
        e->type = KD_EVENT_INPUT_POINTER;
        e->userptr = reinterpret_cast<void*>(KD_NULL);
        KDEventInputPointer *dataPtr = reinterpret_cast<KDEventInputPointer *>(&(e->data));
        dataPtr->index = index;
        dataPtr->select = select;
        dataPtr->x = x;
        dataPtr->y = y;

        m_pendingEvents.append(e);
    }
};

///< @brief contructor
Q3DSViewerApp::Q3DSViewerApp(void *glContext, Q3DStudio::IAudioPlayer *inAudioPlayer)
    : m_Impl(*new Q3DSViewerAppImpl(inAudioPlayer))
{
    Q_UNUSED(glContext)
    m_Impl.m_WindowSystem = new SWindowSystemImpl();
}

///< @brief destructor
Q3DSViewerApp::~Q3DSViewerApp()
{
    qDeleteAll(m_Impl.m_pendingEvents);

    delete m_Impl.m_AudioPlayer;

    if (m_Impl.m_tegraApp) {
        disconnect(m_Impl.m_tegraApp->getNDDView()->signalProxy(), 0);

        m_Impl.m_tegraApp->Cleanup();
        Q3DStudio_virtual_delete(m_Impl.m_tegraApp, CTegraApplication);

        if (GetMemoryManager().GetLineTracker())
            GetMemoryManager().GetLineTracker()->Report();

        GetMemoryManager().Release();
    }
    delete static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem);
}

void Q3DSViewerApp::setOffscreenId(int offscreenID)
{
    static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem)->m_OffscreenID
            = offscreenID;
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine())
        m_Impl.m_tegraApp->GetTegraRenderEngine()->ensureRenderTarget();
}

bool Q3DSViewerApp::InitializeApp(int winWidth, int winHeight, const QSurfaceFormat &format,
                                 int offscreenID, const QString &source,
                                 qt3ds::Qt3DSAssetVisitor *assetVisitor)
{
    bool hasValidPresentationFile = !source.isEmpty();

    QFileInfo info(source);
    if (!info.exists()) {
        m_Impl.m_error
                = QObject::tr("Failed to initialize viewer, presentation doesn't exist");
        qCritical() << m_Impl.m_error;
        return false;
    }

    m_Impl.m_WindowSystem->SetWindowDimensions(SSize(winWidth, winHeight));

    // create our internal application
    if (hasValidPresentationFile && !m_Impl.m_tegraApp) {
        WindowRect &theWindowRect = static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem)->m_Rect;
        theWindowRect.x = 0;
        theWindowRect.y = 0;
        theWindowRect.width = winWidth;
        theWindowRect.height = winHeight;
        static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem)->m_OffscreenID = offscreenID;
        static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem)->m_DepthBitCount
                = format.depthBufferSize();

        // create memory manager
        // GetMemoryManager( ).Initialize( "GlobalManager", g_ChunkSize, g_ChunkCount );
        // create internal app
        m_Impl.m_tegraApp = Q3DStudio_virtual_new(CTegraApplication)
                CTegraApplication(g_GlobalTimeProvider, *m_Impl.m_WindowSystem,
                                  m_Impl.m_AudioPlayer);

        if (assetVisitor)
            m_Impl.m_tegraApp->getNDDView()->setAssetVisitor(assetVisitor);

        m_Impl.m_appInitSuccessful = m_Impl.m_tegraApp->BeginLoad(source) ? true : false;

        // Simulate killing the application during loading.  Useful for finding serious issues with
        // loading.
        /*for ( unsigned idx = 0; idx < 100; ++idx )
        {
                Sleep( 10*idx );
                m_Impl.m_tegraApp->Cleanup();
                Q3DStudio_virtual_delete( m_Impl.m_tegraApp, CTegraApplication );

                m_Impl.m_tegraApp =
        Q3DStudio_virtual_new(CTegraApplication)CTegraApplication(g_GlobalTimeProvider,
        *m_Impl.m_WindowSystem);
                m_Impl.m_appInitSuccessful = m_Impl.m_tegraApp->BeginLoad( viewerArgs ) ? true :
        false;
        }*/

        if (m_Impl.m_appInitSuccessful == false) {
            m_Impl.m_error = QObject::tr("Viewer launch failure! Failed to load: '%1'").arg(source);
            m_Impl.m_error.append("\n");
            qCritical() << m_Impl.m_error;
            return false;
        }

        bool success = m_Impl.m_tegraApp->InitializeGraphics(format);
        if (!success) {
            m_Impl.m_error = QObject::tr("Viewer launch failure! Failed to load: '%1'").arg(source);
            m_Impl.m_error.append("\n");
            qCritical() << m_Impl.m_error;
            return false;
        }

        SigPresentationReady();

        // Connect signals
        connect(m_Impl.m_tegraApp->getNDDView()->signalProxy(),
                &QINDDViewSignalProxy::SigSlideEntered, this, &Q3DSViewerApp::SigSlideEntered);
        connect(m_Impl.m_tegraApp->getNDDView()->signalProxy(),
                &QINDDViewSignalProxy::SigSlideExited, this, &Q3DSViewerApp::SigSlideExited);

        DoEnableWatermark();
        DoSetWatermarkLocation();

        Resize(winWidth, winHeight);
    }
    return true;
}

bool Q3DSViewerApp::IsInitialised(void)
{
    return m_Impl.m_tegraApp != NULL && m_Impl.m_appInitSuccessful == true;
}

int Q3DSViewerApp::GetWindowHeight()
{
    return m_Impl.m_WindowSystem->GetWindowDimensions().m_Height;
}

int Q3DSViewerApp::GetWindowWidth()
{
    return m_Impl.m_WindowSystem->GetWindowDimensions().m_Width;
}

void Q3DSViewerApp::setupSearchPath(std::vector<std::string> &cmdLineArgs)
{
    // setup some additional search path
    // index 0 contains the module directory
#ifdef _MACOSX
    char buf[2056];
    uint32_t bufsize = 2056;
    _NSGetExecutablePath(buf, &bufsize);
    std::string theModuleDirectory = buf;
#else
    std::string theModuleDirectory = cmdLineArgs[0];
#endif
    std::string::size_type pos = theModuleDirectory.rfind('\\');
    if (pos == std::string::npos)
        pos = theModuleDirectory.rfind('/');

    // Include the slash
    theModuleDirectory = theModuleDirectory.substr(0, pos + 1);

    // TODO: Search path needs to be set up properly
    //NvFAppendSearchPath(theModuleDirectory.c_str());
}

void Q3DSViewerApp::Render()
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        if (m_Impl.m_appInitSuccessful) {
            for (KDEvent *e : m_Impl.m_pendingEvents) {
                m_Impl.m_tegraApp->HandleMessage(e);
                delete e;
            }
            m_Impl.m_pendingEvents.clear();
#ifdef WIN32
            HandleController(*m_Impl.m_tegraApp);
#endif

            m_Impl.m_tegraApp->Render();
        }
    }
}

void Q3DSViewerApp::SaveState()
{
    if (!m_Impl.m_tegraApp)
        return;

    if (m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        ITegraRenderStateManager &manager =
                m_Impl.m_tegraApp->GetTegraRenderEngine()->GetTegraRenderStateManager();

        manager.SaveAllState();
    }
}

void Q3DSViewerApp::RestoreState()
{
    if (!m_Impl.m_tegraApp)
        return;

    if (m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        ITegraRenderStateManager &manager =
                m_Impl.m_tegraApp->GetTegraRenderEngine()->GetTegraRenderStateManager();

        manager.RestoreAllState();
    }
}

void Q3DSViewerApp::EnableWatermarkDDS(const unsigned char *inData, size_t inDataSize)
{
    m_Impl.m_WatermarkData.assign(inData, inData + inDataSize);
    DoEnableWatermark();
}

void Q3DSViewerApp::SetWatermarkLocation(float x, float y)
{
    m_Impl.m_WatermarkX = x;
    m_Impl.m_WatermarkY = y;
    DoSetWatermarkLocation();
}

bool Q3DSViewerApp::WasLastFrameDirty()
{
    if (m_Impl.m_tegraApp)
        return m_Impl.m_tegraApp->WasLastFrameDirty();
    return false;
}

void Q3DSViewerApp::DoEnableWatermark()
{
    if (m_Impl.m_WatermarkData.size() && m_Impl.m_tegraApp
            && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        m_Impl.m_tegraApp->GetTegraRenderEngine()->SetWatermarkTextureDataDDS(
                    m_Impl.m_WatermarkData.data(), m_Impl.m_WatermarkData.size());
    }
}

void Q3DSViewerApp::DoSetWatermarkLocation()
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()
            && m_Impl.m_WatermarkX >= 0.0f) {
        m_Impl.m_tegraApp->GetTegraRenderEngine()->SetWatermarkLocation(m_Impl.m_WatermarkX,
                                                                        m_Impl.m_WatermarkY);
    }
}

QString Q3DSViewerApp::error()
{
    QString error = m_Impl.m_error;
    m_Impl.m_error.clear();
    return error;
}

void Q3DSViewerApp::Resize(int width, int height)
{
    WindowRect &theWindowRect = static_cast<SWindowSystemImpl *>(m_Impl.m_WindowSystem)->m_Rect;
    theWindowRect.width = width;
    theWindowRect.height = height;

#if !defined(Q_OS_MACOS)
    if (m_Impl.m_appInitSuccessful && m_Impl.m_tegraApp
            && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        KDEvent event;
        memset(&event, 0, sizeof(KDEvent));
        event.type = KD_EVENT_WINDOWPROPERTY_CHANGE;
        event.data.windowproperty.pname = KD_WINDOWPROPERTY_SIZE;
        m_Impl.m_tegraApp->HandleMessage(&event);
    }
#endif
}

void Q3DSViewerApp::HandleKeyInput(Q3DStudio::EKeyCode inKeyCode, bool isPressed)
{
    if (!m_Impl.m_tegraApp || inKeyCode == Q3DStudio::KEY_NOKEY)
        return;

    CInputEngine *input = m_Impl.m_tegraApp->GetInputEngine();
    if (input)
        input->HandleKeyboard(inKeyCode, isPressed);
}

void Q3DSViewerApp::HandleMouseMove(int x, int y, bool isPressed)
{
    if (!m_Impl.m_tegraApp)
        return;

    CInputEngine *input = m_Impl.m_tegraApp->GetInputEngine();
    if (input) {
        input->BeginPickInput();
        input->EndPickInput();
        input->SetPickInput(static_cast<Q3DStudio::FLOAT>(x), static_cast<Q3DStudio::FLOAT>(y),
                            isPressed);
    }

}

void Q3DSViewerApp::HandleMousePress(int x, int y, int mouseButton, bool isPressed)
{
    if (!m_Impl.m_tegraApp)
        return;

    bool hasButton
            = std::find(m_Impl.m_mouseButtons.begin(), m_Impl.m_mouseButtons.end(), mouseButton)
            != m_Impl.m_mouseButtons.end();

    if ((mouseButton == 1 || mouseButton == 2)) {
        // We keep track of the mouse presses because there are situations where the application
        // will not give us a mouse
        // up for each mouse down.  This ameleorates the effects of those slightly misbehaving
        // applications.
        if (isPressed == hasButton) {
            bool localIsPressed = !isPressed;
            // Slightly recursive call in order to handle situations where we aren't getting
            // the mouse up with the corresponding mouse down or vice versa.
            HandleMousePress(x, y, mouseButton, localIsPressed);
            HandleMousePress(x, y, mouseButton, isPressed);
        } else {
            if (isPressed) {
                m_Impl.m_mouseButtons.push_back(mouseButton);
                qCInfo(qt3ds::TRACE_INFO)
                        << "ViewerApp: Mouse down of frame "
                        << m_Impl.m_tegraApp->GetFrameCount();
            } else {
                m_Impl.m_mouseButtons.erase(std::remove(m_Impl.m_mouseButtons.begin(),
                                                        m_Impl.m_mouseButtons.end(), mouseButton),
                                            m_Impl.m_mouseButtons.end());
            }

            CInputEngine *input = m_Impl.m_tegraApp->GetInputEngine();

            if (input) {
                input->BeginPickInput();
                input->SetPickInput(static_cast<Q3DStudio::FLOAT>(x),
                                    static_cast<Q3DStudio::FLOAT>(y), isPressed);
                input->EndPickInput();

                m_Impl.queueMouseEvent(mouseButton, isPressed ? 1 : 0, x, y);
            }
        }
    }
}

void Q3DSViewerApp::HandleMouseWheel(int x, int y, int orientation, int numSteps)
{
    if (!m_Impl.m_tegraApp)
        return;

    CInputEngine *input = m_Impl.m_tegraApp->GetInputEngine();
    if (input) {
        input->SetPickInput(static_cast<Q3DStudio::FLOAT>(x), static_cast<Q3DStudio::FLOAT>(y), 0);
        input->SetScrollValue(orientation == 0 ? VSCROLLWHEEL : HSCROLLWHEEL, numSteps);
    }
}

void Q3DSViewerApp::GoToSlideByName(const char *elementPath, const char *slideName)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->GoToSlideByName(elementPath, slideName);
}

void Q3DSViewerApp::GoToSlideByIndex(const char *elementPath, const int slideIndex)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->GoToSlideByIndex(elementPath, slideIndex);
}

void Q3DSViewerApp::GoToSlideRelative(const char *elementPath, const bool next, const bool wrap)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->GoToSlideRelative(elementPath, next, wrap);
}

bool Q3DSViewerApp::GetSlideInfo(const char *elementPath, int &currentIndex, int &previousIndex,
                                QString &currentName, QString &previousName)
{
    if (!m_Impl.m_tegraApp)
        return false;

    return m_Impl.m_tegraApp->GetSlideInfo(elementPath, currentIndex, previousIndex,
                                           currentName, previousName);
}

void Q3DSViewerApp::SetPresentationActive(const char *presId, const bool active)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->SetPresentationAttribute(presId, nullptr, active ? "True" : "False");
}

void Q3DSViewerApp::GoToTime(const char *elementPath, const float time)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->GoToTime(elementPath, time);
}

void Q3DSViewerApp::PlaySoundFile(const char *soundPath)
{
    if (!m_Impl.m_AudioPlayer || !soundPath)
        return;

    m_Impl.m_AudioPlayer->PlaySoundFile(soundPath);
}

void Q3DSViewerApp::SetAttribute(const char *elementPath, const char *attributeName,
                                const char *value)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->SetAttribute(elementPath, attributeName, value);
}

bool Q3DSViewerApp::GetAttribute(const char *elementPath, const char *attributeName, void *value)
{
    if (!m_Impl.m_tegraApp)
        return false;

    return m_Impl.m_tegraApp->GetAttribute(elementPath, attributeName, value);
}

void Q3DSViewerApp::FireEvent(const char *elementPath, const char *evtName)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->FireEvent(elementPath, evtName);
}

bool Q3DSViewerApp::PeekCustomAction(std::string &outElementPath, std::string &outActionName)
{
    if (!m_Impl.m_tegraApp)
        return false;

    char *theElementPath = NULL;
    char *theActioName = NULL;
    bool retVal = m_Impl.m_tegraApp->PeekCustomAction(theElementPath, theActioName);

    if (theElementPath)
        outElementPath = theElementPath;
    if (theActioName)
        outActionName = theActioName;

    return retVal;
}

bool Q3DSViewerApp::RegisterScriptCallback(ViewerCallbackType::Enum inCallbackType,
                                          const qml_Function inCallback, void *inUserData)
{
    if (!m_Impl.m_tegraApp)
        return false;

    // convert to int
    int callbackType = 0;
    if (inCallbackType == Q3DSViewer::ViewerCallbackType::CALLBACK_ON_INIT)
        callbackType = 1;
    else if (inCallbackType == Q3DSViewer::ViewerCallbackType::CALLBACK_ON_UPDATE)
        callbackType = 2;
    else
        return false;

    bool retVal = m_Impl.m_tegraApp->RegisterScriptCallback(callbackType, inCallback, inUserData);

    return retVal;
}

void Q3DSViewerApp::SetScaleMode(ViewerScaleModes::Enum inScale)
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        m_Impl.m_tegraApp->GetTegraRenderEngine()->SetScaleMode(
                    static_cast<TegraRenderScaleModes::Enum>(inScale));
    }
}

ViewerScaleModes::Enum Q3DSViewerApp::GetScaleMode()
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        return static_cast<ViewerScaleModes::Enum>(
                    m_Impl.m_tegraApp->GetTegraRenderEngine()->GetScaleMode());
    }

    return ViewerScaleModes::ExactSize;
}

void Q3DSViewerApp::setMatteColor(const QColor &color)
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        m_Impl.m_tegraApp->GetTegraRenderEngine()->SetMatteColor(
                    qt3ds::QT3DSVec4(color.redF(), color.greenF(),
                                     color.blueF(), color.alphaF()));
    }
}

void Q3DSViewerApp::setShowOnScreenStats(bool inShow)
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine())
        m_Impl.m_tegraApp->getNDDView()->showOnScreenStats(inShow);
}

void Q3DSViewerApp::SetShadeMode(ViewerShadeModes::Enum inShadeMode)
{
    if (m_Impl.m_tegraApp && m_Impl.m_tegraApp->GetTegraRenderEngine()) {
        StaticAssert<ViewerShadeModes::Shaded == TegraRenderShadeModes::Shaded>::valid_expression();
        StaticAssert<ViewerShadeModes::ShadedWireframe
                == TegraRenderShadeModes::ShadedWireframe>::valid_expression();
        StaticAssert<ViewerShadeModes::Wireframe
                == TegraRenderShadeModes::Wireframe>::valid_expression();

        m_Impl.m_tegraApp->GetTegraRenderEngine()->SetShadeMode(
                    static_cast<TegraRenderShadeModes::Enum>(inShadeMode));
    }
}

void Q3DSViewerApp::SetGlobalAnimationTime(qint64 inMilliSecs)
{
    if (!m_Impl.m_tegraApp)
        return;

    m_Impl.m_tegraApp->SetGlobalAnimationTime(inMilliSecs);
}

Q3DSViewerApp &Q3DSViewerApp::Create(void *glContext, Q3DStudio::IAudioPlayer *inAudioPlayer)
{
    return *Q3DStudio_virtual_new(Q3DSViewerApp) Q3DSViewerApp(glContext, inAudioPlayer);
}

void Q3DSViewerApp::Release()
{
    Q3DStudio_virtual_delete(this, Q3DSViewerApp);
}

} // end namespace
