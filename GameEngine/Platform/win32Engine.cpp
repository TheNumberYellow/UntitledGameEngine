#include <Windows.h>

#include "..\GameEngine.hpp"

static bool running = true;
static HWND WindowHandle;

static Vec2i cursorCenter;
static bool cursorLocked = false;
static bool cursorHidden = false;

static int64_t TICKS_PER_SECOND;

float Engine::GetElapsedTime()
{
    int64_t tickCount;
    QueryPerformanceCounter((LARGE_INTEGER*)&tickCount);

    float seconds = ((float)tickCount / (float)TICKS_PER_SECOND);

    std::string message = std::to_string(seconds) + "\n";

    //DEBUGPrint(message.c_str());

    return seconds;
}

Vec2i Engine::GetWindowSize()
{
    RECT screenRect;
    GetWindowRect(WindowHandle, &screenRect);

    return Vec2i(screenRect.right - screenRect.left, screenRect.bottom - screenRect.top);
}

Vec2i Engine::GetClientAreaSize()
{
    RECT clientRect;
    GetClientRect(WindowHandle, &clientRect);

    return Vec2i(clientRect.right, clientRect.bottom);
}

Vec2i Engine::GetMousePosition()
{
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(WindowHandle, &p);

    return Vec2i(p.x, p.y);
}

bool Engine::GetMouseDown(int button)
{
    switch (button)
    {
    case 0:
        return GetAsyncKeyState(VK_LBUTTON);
    case 1:
        return GetAsyncKeyState(VK_RBUTTON);
    default:
        return false;
    }
}

void Engine::DEBUGPrint(std::string string)
{
    OutputDebugStringA((LPCSTR)string.c_str());
}

void Engine::FatalError(std::string errorMessage)
{
    Engine::UnlockCursor();
    Engine::ShowCursor();

    MessageBoxA(NULL, errorMessage.c_str(), "Error", MB_OK | MB_ICONEXCLAMATION);
    exit(EXIT_FAILURE);
}

void Engine::SetCursorCenter(Vec2i center)
{
    cursorCenter = center;
}

void Engine::LockCursor()
{
    cursorLocked = true;
}

void Engine::UnlockCursor()
{
    cursorLocked = false;
}

void Engine::HideCursor()
{
    if (!cursorHidden)
    {
        ::ShowCursor(false);
        cursorHidden = true;
    }
}

void Engine::ShowCursor()
{
    if (cursorHidden)
    {
        ::ShowCursor(true);
        cursorHidden = false;
    }
}

void Engine::StopGame()
{
    running = false;
}

void WarpMouseToWindowCenter()
{
    POINT cursorPos;
    cursorPos.x = cursorCenter.x;
    cursorPos.y = cursorCenter.y;

    ClientToScreen(WindowHandle, &cursorPos);

    SetCursorPos(cursorPos.x, cursorPos.y);
}

void GetKeyboardState(ControlInputs& inputs)
{
    inputs.keysDown.w = GetAsyncKeyState('W');
    inputs.keysDown.a = GetAsyncKeyState('A');
    inputs.keysDown.s = GetAsyncKeyState('S');
    inputs.keysDown.d = GetAsyncKeyState('D');
    inputs.keysDown.q = GetAsyncKeyState('Q');
    inputs.keysDown.e = GetAsyncKeyState('E');

    inputs.keysDown.zero = GetAsyncKeyState(0x30);
    inputs.keysDown.one = GetAsyncKeyState(0x31);
    inputs.keysDown.two = GetAsyncKeyState(0x32);
    inputs.keysDown.three = GetAsyncKeyState(0x33);
    inputs.keysDown.four = GetAsyncKeyState(0x34);
    inputs.keysDown.five = GetAsyncKeyState(0x35);
    inputs.keysDown.six = GetAsyncKeyState(0x36);
    inputs.keysDown.seven = GetAsyncKeyState(0x37);
    inputs.keysDown.eight = GetAsyncKeyState(0x38);
    inputs.keysDown.nine = GetAsyncKeyState(0x39);

    inputs.keysDown.space = GetAsyncKeyState(VK_SPACE);
    inputs.keysDown.alt = GetAsyncKeyState(VK_MENU);
    inputs.keysDown.tab = GetAsyncKeyState(VK_TAB);
    inputs.keysDown.shift = GetAsyncKeyState(VK_SHIFT);
    inputs.keysDown.ctrl = GetAsyncKeyState(VK_CONTROL);

    inputs.keysDown.up = GetAsyncKeyState(VK_UP);
    inputs.keysDown.down = GetAsyncKeyState(VK_DOWN);
    inputs.keysDown.left = GetAsyncKeyState(VK_LEFT);
    inputs.keysDown.right = GetAsyncKeyState(VK_RIGHT);

    inputs.keysDown.esc = GetAsyncKeyState(VK_ESCAPE);

    inputs.mouse.leftMouseButton = GetAsyncKeyState(VK_LBUTTON);
    inputs.mouse.rightMouseButton = GetAsyncKeyState(VK_RBUTTON);
}

// Window callback function
LRESULT CALLBACK WindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    ModuleManager* modules;
    if (Message == WM_NCCREATE)
    {
        LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        modules = static_cast<ModuleManager*>(lpcs->lpCreateParams);
        // Store the module manager pointer for later
        SetWindowLongPtr(WindowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(modules));
    }
    else
    {
        modules = reinterpret_cast<ModuleManager*>(GetWindowLongPtr(WindowHandle, GWLP_USERDATA));
    }

    switch (Message)
    {
    case WM_CLOSE:
        DestroyWindow(WindowHandle);
        break;
    case WM_DESTROY:
        running = false;
        break;
    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        UIModule* ui = modules->GetUI();
        if (ui)
        {
            ui->Resize(Vec2i(width, height));
        }
        GraphicsModule* graphics = modules->GetGraphics();
        if (graphics)
        {
            graphics->Resize(Vec2i(width, height));
        }

        Resize(*modules, Vec2i(width, height));

        break;
    }
    case WM_SYSCOMMAND:
        // Catch the behaviour of pressing the ALT button and discard it
        if (wParam == SC_KEYMENU && (lParam >> 16) <= 0) return 0;
        return DefWindowProc(WindowHandle, Message, wParam, lParam);
    default:
        return DefWindowProc(WindowHandle, Message, wParam, lParam);
    }
    return 0;
}


// Program entry point
int WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ LPSTR CommandLine, _In_ int ShowCommand)
{
    ModuleManager Modules;
    
    // Initialize window class
    WNDCLASSEX wndClass = {};
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_OWNDC;
    wndClass.lpfnWndProc = WindowProc;
    wndClass.hInstance = Instance;
    wndClass.lpszClassName = L"GameEngineWindowClass";
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

    LARGE_INTEGER frequency;

    QueryPerformanceFrequency(&frequency);

    if (!RegisterClassEx(&wndClass))
    {
        return 1;
    }

    WindowHandle = CreateWindowEx
    (
        0,
        wndClass.lpszClassName,
        L"Game Engine",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        Instance,
        &Modules
    );

    if (!WindowHandle)
    {
        return 1;
    }

    ShowWindow(WindowHandle, SW_SHOW);

    QueryPerformanceFrequency((LARGE_INTEGER*)&TICKS_PER_SECOND);

    // Set up modules
    Renderer renderer;

    GraphicsModule Graphics(renderer);
    CollisionModule Collisions(renderer);
    TextModule Text(renderer);
    UIModule UI(renderer);

    Modules.SetGraphics(&Graphics);
    Modules.SetCollision(&Collisions);
    Modules.SetText(&Text);
    Modules.SetUI(&UI);

    ControlInputs Inputs;

    Vec2i screenSize = Engine::GetClientAreaSize();
    cursorCenter.x = screenSize.x / 2;
    cursorCenter.y = screenSize.y / 2;

    Inputs.LatestMouse = Engine::GetMousePosition();
    Inputs.DeltaMouse = Vec2i(0, 0);

    Initialize(Modules);

    while (running)
    {

        MSG Message = {};
        while (PeekMessage(&Message, WindowHandle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        {
            Vec2i prevMousePosition = Inputs.LatestMouse;

            Inputs.LatestMouse = Engine::GetMousePosition();
            Inputs.DeltaMouse.x = Inputs.LatestMouse.x - prevMousePosition.x;
            Inputs.DeltaMouse.y = Inputs.LatestMouse.y - prevMousePosition.y;

            if (GetFocus() == WindowHandle)
            {
                if (cursorLocked)
                {
                    WarpMouseToWindowCenter();
                }
            }

            Vec2i screenSize = Engine::GetClientAreaSize();
            Inputs.LatestMouse = cursorCenter;

            GetKeyboardState(Inputs);

        }
        Graphics.OnFrameStart();
        UI.OnFrameStart();
        Update(Modules, Inputs);
        Graphics.OnFrameEnd();
        UI.OnFrameEnd();
    }
    return 0;
}