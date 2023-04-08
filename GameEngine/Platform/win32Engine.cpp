#include <Windows.h>

#include "..\GameEngine.h"

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

void Engine::SetMousePosition(Vec2i pos)
{
    POINT cursorPos;
    cursorPos.x = pos.x;
    cursorPos.y = pos.y;

    ClientToScreen(WindowHandle, &cursorPos);

    SetCursorPos(cursorPos.x, cursorPos.y);
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
    OutputDebugStringA((LPCSTR)((string + "\n").c_str()));
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

bool Engine::IsWindowFocused()
{
    return GetFocus() == WindowHandle;
}

void WarpMouseToWindowCenter()
{
    Engine::SetMousePosition(cursorCenter);
}

void GetKeyboardState(InputModule& inputs)
{
    inputs.SetKeyDown(Key::A, GetAsyncKeyState('A'));
    inputs.SetKeyDown(Key::B, GetAsyncKeyState('B'));
    inputs.SetKeyDown(Key::C, GetAsyncKeyState('C'));
    inputs.SetKeyDown(Key::D, GetAsyncKeyState('D'));
    inputs.SetKeyDown(Key::E, GetAsyncKeyState('E'));
    inputs.SetKeyDown(Key::F, GetAsyncKeyState('F'));
    inputs.SetKeyDown(Key::G, GetAsyncKeyState('G'));
    inputs.SetKeyDown(Key::H, GetAsyncKeyState('H'));
    inputs.SetKeyDown(Key::I, GetAsyncKeyState('I'));
    inputs.SetKeyDown(Key::J, GetAsyncKeyState('J'));
    inputs.SetKeyDown(Key::K, GetAsyncKeyState('K'));
    inputs.SetKeyDown(Key::L, GetAsyncKeyState('L'));
    inputs.SetKeyDown(Key::M, GetAsyncKeyState('M'));
    inputs.SetKeyDown(Key::N, GetAsyncKeyState('N'));
    inputs.SetKeyDown(Key::O, GetAsyncKeyState('O'));
    inputs.SetKeyDown(Key::P, GetAsyncKeyState('P'));
    inputs.SetKeyDown(Key::Q, GetAsyncKeyState('Q'));
    inputs.SetKeyDown(Key::R, GetAsyncKeyState('R'));
    inputs.SetKeyDown(Key::S, GetAsyncKeyState('S'));
    inputs.SetKeyDown(Key::T, GetAsyncKeyState('T'));
    inputs.SetKeyDown(Key::U, GetAsyncKeyState('U'));
    inputs.SetKeyDown(Key::V, GetAsyncKeyState('V'));
    inputs.SetKeyDown(Key::W, GetAsyncKeyState('W'));
    inputs.SetKeyDown(Key::X, GetAsyncKeyState('X'));
    inputs.SetKeyDown(Key::Y, GetAsyncKeyState('Y'));
    inputs.SetKeyDown(Key::Z, GetAsyncKeyState('Z'));

    inputs.SetKeyDown(Key::Zero, GetAsyncKeyState(0x30));
    inputs.SetKeyDown(Key::One, GetAsyncKeyState(0x31));
    inputs.SetKeyDown(Key::Two, GetAsyncKeyState(0x32));
    inputs.SetKeyDown(Key::Three, GetAsyncKeyState(0x33));
    inputs.SetKeyDown(Key::Four, GetAsyncKeyState(0x34));
    inputs.SetKeyDown(Key::Five, GetAsyncKeyState(0x35));
    inputs.SetKeyDown(Key::Six, GetAsyncKeyState(0x36));
    inputs.SetKeyDown(Key::Seven, GetAsyncKeyState(0x37));
    inputs.SetKeyDown(Key::Eight, GetAsyncKeyState(0x38));
    inputs.SetKeyDown(Key::Nine, GetAsyncKeyState(0x39));

    inputs.SetKeyDown(Key::Space, GetAsyncKeyState(VK_SPACE));
    inputs.SetKeyDown(Key::Alt, GetAsyncKeyState(VK_MENU));
    inputs.SetKeyDown(Key::Tab, GetAsyncKeyState(VK_TAB));
    inputs.SetKeyDown(Key::Shift, GetAsyncKeyState(VK_SHIFT));
    inputs.SetKeyDown(Key::Ctrl, GetAsyncKeyState(VK_CONTROL));

    inputs.SetKeyDown(Key::Up, GetAsyncKeyState(VK_UP));
    inputs.SetKeyDown(Key::Down, GetAsyncKeyState(VK_DOWN));
    inputs.SetKeyDown(Key::Left, GetAsyncKeyState(VK_LEFT));
    inputs.SetKeyDown(Key::Right, GetAsyncKeyState(VK_RIGHT));

    inputs.SetKeyDown(Key::Escape, GetAsyncKeyState(VK_ESCAPE));

    inputs.GetMouseState().SetButtonDown(Mouse::LMB, GetAsyncKeyState(VK_LBUTTON));
    inputs.GetMouseState().SetButtonDown(Mouse::RMB, GetAsyncKeyState(VK_RBUTTON));
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
        Vec2i newSize = { LOWORD(lParam), HIWORD(lParam) };

        if (modules->AreAllModulesInitialized())
        {
            UIModule* ui = modules->GetUI();
            ui->Resize(newSize);

            GraphicsModule* graphics = modules->GetGraphics();
            graphics->Resize(newSize);

            InputModule* input = modules->GetInput();
            input->Resize(newSize);

            modules->GetText()->Resize(newSize);

            Resize(*modules, newSize);
        
            graphics->OnFrameStart();
            ui->OnFrameStart();
            Update(*modules);
            graphics->OnFrameEnd();
            ui->OnFrameEnd();
        }

        break;
    }
    case WM_CHAR:
        if (modules->AreAllModulesInitialized())
        {
            InputModule* input = modules->GetInput();

            input->InputCharacter((char)wParam);
        }
        break;

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
    InputModule Input;
    UIModule UI(renderer, Text, Input);

    Modules.SetGraphics(&Graphics);
    Modules.SetCollision(&Collisions);
    Modules.SetText(&Text);
    Modules.SetInput(&Input);
    Modules.SetUI(&UI);

    Vec2i screenSize = Engine::GetClientAreaSize();
    cursorCenter.x = screenSize.x / 2;
    cursorCenter.y = screenSize.y / 2;

    Initialize(Modules);
    Resize(Modules, screenSize);

    while (running)
    {

        MSG Message = {};
        while (PeekMessage(&Message, WindowHandle, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        Input.UpdateMousePos(Engine::GetMousePosition());
        Input.SetMouseLocked(cursorLocked);

        if (Engine::IsWindowFocused())
        {
            if (cursorLocked)
            {
                WarpMouseToWindowCenter();
            }
        }

        GetKeyboardState(Input);

        Graphics.OnFrameStart();
        UI.OnFrameStart();
        Update(Modules);
        Graphics.OnFrameEnd();
        UI.OnFrameEnd();
        Input.OnFrameEnd();
    }
    return 0;
}