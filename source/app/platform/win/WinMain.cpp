/*
* This file contains WinMain and related functions
*/

#include "StdInc.h"

#include "LoadingScreen.h"
#include "ControllerConfigManager.h"
#include "Gamma.h"

#include "VideoPlayer.h"
#include "VideoMode.h"
#include "Input.h"
#include "Platform.h"
#include "WndProc.h"

#include "extensions/Configs/FastLoader.hpp"

constexpr auto NO_FOREGROUND_PAUSE = true;

// 0x747300
char* getDvdGamePath() {
    return plugin::CallAndReturn<char*, 0x747300>();
}

// 0x746870
void MessageLoop() {
    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD)) {
        if (msg.message == WM_QUIT) {
            RsGlobal.quit = true;
        } else {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
}

// 0x7486A0
bool InitApplication(HINSTANCE hInstance) {
    WNDCLASS windowClass      = { 0 };
    windowClass.style         = CS_BYTEALIGNWINDOW;
    windowClass.lpfnWndProc   = __MainWndProc;
    windowClass.hInstance     = hInstance;
    windowClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    windowClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    windowClass.lpszClassName = APP_CLASS;
    return RegisterClass(&windowClass);
}

// 0x745560
HWND InitInstance(HINSTANCE hInstance) {
    RECT rect = { 0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);

    return CreateWindowEx(
        0,
        APP_CLASS,
        RsGlobal.appName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
}

// 0x7468E0
bool IsAlreadyRunning() {
    CreateEvent(nullptr, false, true, APP_CLASS);
    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        return false;
    }

    const auto fgwnd = FindWindow(APP_CLASS, RsGlobal.appName);
    SetForegroundWindow(fgwnd != INVALID_HANDLE_VALUE ? fgwnd : PSGLOBAL(window));
    
    return true;
};

// 0x746060
bool IsForegroundApp() {
    return ForegroundApp;
}

// Code from winmain, 0x748DCF
bool ProcessGameLogic(INT nCmdShow, MSG& Msg) {
    if (RsGlobal.quit || FrontEndMenuManager.m_bStartGameLoading) {
        return false;
    }

    // Process Window messages
    if (PeekMessage(&Msg, NULL, NULL, NULL, PM_REMOVE | PM_NOYIELD)) {
        if (Msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        return true;
    }

    // Game is in background
    if (!NO_FOREGROUND_PAUSE && !ForegroundApp) {
        if (isForeground) {
            isForeground = false;
        }
        Sleep(100);
        return true;
    }
    
    FrameMark;

    // TODO: Move this out from here (It's not platform specific at all)
    switch (gGameState) {
    case GAME_STATE_INITIAL: {
        const auto ProcessSplash = [](bool isNVidia) {
            CLoadingScreen::LoadSplashes(true, isNVidia);
            CLoadingScreen::Init(true, true);
            CLoadingScreen::DoPCTitleFadeOut();
            CLoadingScreen::DoPCTitleFadeIn();
            CLoadingScreen::Shutdown();
        };
        if (!g_FastLoaderConfig.NoEAX) {
            ProcessSplash(false);
        }
        if (!g_FastLoaderConfig.NoNVidia) {
            ProcessSplash(true);
        }
        ChangeGameStateTo(GAME_STATE_LOGO);
        break;
    }
    case GAME_STATE_LOGO: {
        if (!g_FastLoaderConfig.NoLogo) {
            if (!Windowed) {
                VideoPlayer::Play(nCmdShow, "movies\\Logo.mpg");
            }
        }
        ChangeGameStateTo(g_FastLoaderConfig.NoLogo ? GAME_STATE_TITLE : GAME_STATE_PLAYING_LOGO);
        break;
    }
    case GAME_STATE_PLAYING_LOGO:
    case GAME_STATE_PLAYING_INTRO: { // 0x748B17
        CPad::UpdatePads();
        auto* pad = CPad::GetPad();
        if (   Windowed
            || ControlsManager.GetJoyButtonJustDown()
            || pad->NewState.CheckForInput()
            || CPad::IsMouseLButtonPressed()
            || CPad::IsEnterJustPressed()
            || pad->IsStandardKeyJustPressed(VK_SPACE)
            || CPad::IsMenuKeyJustPressed()
            || CPad::IsTabJustPressed()
        ) {
            ChangeGameStateTo([] {
                switch (gGameState) {
                case GAME_STATE_PLAYING_LOGO:  return GAME_STATE_TITLE;
                case GAME_STATE_PLAYING_INTRO: return GAME_STATE_FRONTEND_LOADING;
                default:                       NOTSA_UNREACHABLE();
                }
            }());
        }
        break;
    }
    case GAME_STATE_TITLE: {
        if (!g_FastLoaderConfig.NoTitleOrIntro) {
            VideoPlayer::Shutdown();
            VideoPlayer::Play(nCmdShow, FrontEndMenuManager.GetMovieFileName());
        }
        ChangeGameStateTo(g_FastLoaderConfig.NoTitleOrIntro ? GAME_STATE_FRONTEND_LOADING : GAME_STATE_PLAYING_INTRO);
        break;
    }
    case GAME_STATE_FRONTEND_LOADING: {
        VideoPlayer::Shutdown();
        CLoadingScreen::Init(true, false);
        if (!g_FastLoaderConfig.NoCopyright) {
            CLoadingScreen::DoPCTitleFadeOut();
        }
        if (!CGame::InitialiseEssentialsAfterRW()) {
            RsGlobal.quit = true;
        }
        CGame::InitialiseCoreDataAfterRW();
        ChangeGameStateTo(GAME_STATE_FRONTEND_LOADED);
        anisotropySupportedByGFX = (RwD3D9GetCaps()->RasterCaps & D3DPRASTERCAPS_ANISOTROPY) != 0; // todo: func
        break;
    }
    case GAME_STATE_FRONTEND_LOADED: {
        FrontEndMenuManager.m_bActivateMenuNextFrame = true;
        FrontEndMenuManager.m_bMainMenuSwitch = true;
        if (IsVMNotSelected) {
            FrontEndMenuManager.m_nPrefsVideoMode = FrontEndMenuManager.m_nDisplayVideoMode = gCurrentVideoMode;
            IsVMNotSelected = false;
        }
        ChangeGameStateTo(GAME_STATE_FRONTEND_IDLE);
        if (g_FastLoaderConfig.NoCopyright) {
            CLoadingScreen::SkipCopyrightSplash();
        } else {
            CLoadingScreen::DoPCTitleFadeIn();
        }
        break;
    }
    case GAME_STATE_FRONTEND_IDLE: { // 0x748CB2
        WINDOWPLACEMENT wndpl{ .length = sizeof(WINDOWPLACEMENT) };
        VERIFY(GetWindowPlacement(PSGLOBAL(window), &wndpl));
        if (g_FastLoaderConfig.ShouldLoadSaveGame()) {
            RsEventHandler(rsFRONTENDIDLE, nullptr); // We need to still run the frontend processing once because it has some important stuff
            if ((GetAsyncKeyState(g_FastLoaderConfig.SkipSaveGameLoadKey) & 0xF000) == 0) {
                g_FastLoaderConfig.StartGame(g_FastLoaderConfig.SaveGameToLoad); // Load game
            }
            g_FastLoaderConfig.TriedLoadingSaveGame = true;
        } else if (g_FastLoaderConfig.RenderAtAllTimes || wndpl.showCmd != SW_SHOWMINIMIZED) {
            RsEventHandler(rsFRONTENDIDLE, nullptr);
        }
        if (FrontEndMenuManager.m_bMenuActive && !FrontEndMenuManager.m_bLoadingData) {
            break;
        }
        ChangeGameStateTo(GAME_STATE_LOADING_STARTED);
        if (!FrontEndMenuManager.m_bLoadingData) {
            break;
        }
        NOTSA_SWCFALLTHRU; // Fall down and start loading
    }
    case GAME_STATE_LOADING_STARTED: {
        if (!g_FastLoaderConfig.NoLoadingTune) {
            AudioEngine.StartLoadingTune();
        }

        InitialiseGame();
        ChangeGameStateTo(GAME_STATE_IDLE);
        FrontEndMenuManager.m_bMainMenuSwitch = false;

        AudioEngine.InitialisePostLoading();
        break;
    }
    case GAME_STATE_IDLE: {
        if (!RwInitialized)
            break;

        auto v9_1 = 1000.0f / (float)RsGlobal.frameLimit;
        auto v9_2 = (float)CTimer::GetCurrentTimeInCycles() / (float)CTimer::GetCyclesPerMillisecond();
        if (!FrontEndMenuManager.m_bPrefsFrameLimiter && CReplay::Mode != eReplayMode::MODE_PLAYBACK && !AudioEngine.IsBeatInfoPresent() || v9_1 < v9_2) {
            RsEventHandler(rsIDLE, (void*)true);
        }
        break;
    }
    }

    if (!isForeground) {
        isForeground = true;
    }

    return true;
}

// Code from winmain, 0x7489FB
void MainLoop(INT nCmdShow, MSG& Msg) {
    bool bNewGameFirstTime = false;
    while (true) {
        RwInitialized = true;

        // Useless, and annoying
        //RwV2d pos{ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        //RsMouseSetPos(&pos);

        gamma.Init();

        // Game logic main loop
        while (ProcessGameLogic(nCmdShow, Msg));

        // 0x748DDA
        RwInitialized = false;
        FrontEndMenuManager.UnloadTextures();
        if (!FrontEndMenuManager.m_bStartGameLoading) {
            break;
        }

        // load game
        CCheat::ResetCheats();
        CTimer::Stop();

        if (FrontEndMenuManager.m_bLoadingData) {
            CGame::ShutDownForRestart();
            CGame::InitialiseWhenRestarting();
            FrontEndMenuManager.m_bLoadingData = false;
        } else if (bNewGameFirstTime) {
            CTimer::Stop();
            ChangeGameStateTo(
                FrontEndMenuManager.m_nGameState != 1
                    ? GAME_STATE_LOADING_STARTED
                    : GAME_STATE_FRONTEND_LOADED
            );
        } else {
            CCheat::ResetCheats();
            CGame::ShutDownForRestart();
            CTimer::Stop();
            CGame::InitialiseWhenRestarting();
        }

        bNewGameFirstTime = false;
        FrontEndMenuManager.m_nGameState = 0;
        FrontEndMenuManager.m_bStartGameLoading = false;
    }

}

// 0x748710
INT WINAPI NOTSA_WinMain(HINSTANCE instance, HINSTANCE hPrevInstance, LPSTR cmdLine, INT nCmdShow) {
    SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0u, nullptr, 2);
    if (IsAlreadyRunning()) {
        return false;
    }

    auto initializeEvent = RsEventHandler(rsINITIALIZE, nullptr);
    if (rsEVENTERROR == initializeEvent) {
        return false;
    }

    if (!InitApplication(instance)) {
        return false;
    }

    char** argv = __argv;
    int    argc = __argc;
    for (int i = 0; i < argc; i++) {
        RsEventHandler(rsPREINITCOMMANDLINE, argv[i]);
    }

    PSGLOBAL(window) = InitInstance(instance);
    if (!PSGLOBAL(window)) {
        return false;
    }
    PSGLOBAL(instance) = instance;

    // 0x7487CF
    VERIFY(WinInput::Initialise());

    ControlsManager.InitDefaultControlConfigMouse(WinInput::GetMouseState(), !FrontEndMenuManager.m_nController);

    // 0x748847
    if (RsEventHandler(rsRWINITIALIZE, PSGLOBAL(window)) == rsEVENTERROR) {
        DestroyWindow(PSGLOBAL(window));
        RsEventHandler(rsTERMINATE, nullptr);
        return false;
    }

    // 0x7488EE
    for (auto i = 0; i < argc; i++) {
        RsEventHandler(rsCOMMANDLINE, argv[i]);
    }

    if (MultipleSubSystems || PSGLOBAL(fullScreen)) {
        SetWindowLongPtr(PSGLOBAL(window), GWL_STYLE, (LONG_PTR)WS_POPUP);
        SetWindowPos(PSGLOBAL(window), nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
    }

    RwRect rect{ 0, 0, RsGlobal.maximumWidth, RsGlobal.maximumHeight };
    RsEventHandler(rsCAMERASIZE, &rect);

    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 0u, nullptr, 2u);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 0u, nullptr, 2u);
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 0u, nullptr, 2u);
    STICKYKEYS pvParam { .cbSize = sizeof(STICKYKEYS) };
    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &pvParam, 2u);
    STICKYKEYS pvParam1 = { .cbSize = sizeof(STICKYKEYS), .dwFlags = SKF_TWOKEYSOFF };
    SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &pvParam1, 2u);

    ShowWindow(PSGLOBAL(window), nCmdShow);
    UpdateWindow(PSGLOBAL(window));

    // 0x748995
    CFileMgr::SetDirMyDocuments();
    if (auto* file = CFileMgr::OpenFile("gta_sa.set", "rb")) {
        if (!ControlsManager.LoadSettings(file)) {
            ControlsManager.ReinitControls();
        }
        CFileMgr::CloseFile(file);
    }
    CFileMgr::SetDir("");

    SetErrorMode(SEM_FAILCRITICALERRORS);

    // 0x7489FB
    MSG Msg;
    MainLoop(nCmdShow, Msg);

    // if game is loaded, shut it down
    if (gGameState == GAME_STATE_IDLE) {
        CGame::Shutdown();
    }

    // now quit 0x748E75
    AudioEngine.Shutdown();
    FreeVideoModeList();
    RsEventHandler(rsRWTERMINATE, nullptr);
    DestroyWindow(PSGLOBAL(window));
    RsEventHandler(rsTERMINATE, nullptr);
    free(argv);
    ShowCursor(true);

    SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &pvParam, 2u);
    SystemParametersInfo(SPI_SETPOWEROFFACTIVE, 1u, nullptr, 2u); // TODO: GUID_VIDEO_POWERDOWN_TIMEOUT
    SystemParametersInfo(SPI_SETLOWPOWERACTIVE, 1u, nullptr, 2u);
    SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, 1u, nullptr, 2u);
    // nullsub_0x72F3C0()
    SetErrorMode(0);

    return Msg.wParam;
}

void InjectWinMainStuff() {
    RH_ScopedCategory("Win");
    RH_ScopedNamespaceName("Win");

    RH_ScopedGlobalInstall(IsForegroundApp, 0x746060);
    RH_ScopedGlobalInstall(IsAlreadyRunning, 0x7468E0);

    // Unhooking these 2 after the game has started will do nothing
    RH_ScopedGlobalInstall(NOTSA_WinMain, 0x748710, {.locked = true});
    RH_ScopedGlobalInstall(InitInstance, 0x745560, {.locked = true});
}
