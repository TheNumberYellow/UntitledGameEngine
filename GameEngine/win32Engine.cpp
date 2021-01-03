#include <Windows.h>

#include "GameEngine.hpp"

static bool running = true;
static HWND WindowHandle;
static bool cursorLocked = false;

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

Vec2i Engine::GetScreenSize()
{
	RECT screenRect;
	GetWindowRect(WindowHandle, &screenRect);

	return Vec2i(screenRect.right - screenRect.left, screenRect.bottom - screenRect.top);
}

Vec2i Engine::GetMousePosition()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(WindowHandle, &p);

	return Vec2i(p.x, p.y);
}

void Engine::DEBUGPrint(std::string string)
{
	OutputDebugStringA((LPCSTR)string.c_str());
}

void Engine::LockCursor()
{
	cursorLocked = true;
}

void Engine::UnlockCursor()
{
	cursorLocked = false;
}


void WarpMouseToWindowCenter()
{
	Vec2i screenSize = Engine::GetScreenSize();
	POINT cursorPos;
	cursorPos.x = screenSize.x / 2;
	cursorPos.y = screenSize.y / 2;

	ClientToScreen(WindowHandle, &cursorPos);

	SetCursorPos(cursorPos.x, cursorPos.y);
}

void Engine::StopGame()
{
	running = false;
}

// Window callback function
LRESULT CALLBACK WindowProc(_In_ HWND WindowHandle, _In_ UINT Message, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Message)
	{
	case WM_CLOSE:
		DestroyWindow(WindowHandle);
		break;
	case WM_DESTROY:
		running = false;
		break;
	default:
		return DefWindowProc(WindowHandle, Message, wParam, lParam);
	}
	return 0;
}


// Program entry point
int WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ LPSTR CommandLine, _In_ int ShowCommand)
{
	// Initialize window class
	WNDCLASSEX wndClass = {};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_OWNDC;
	wndClass.lpfnWndProc = WindowProc;
	wndClass.hInstance = Instance;
	wndClass.lpszClassName = L"GameEngineWindowClass";

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
		nullptr
	);

	if (!WindowHandle)
	{
		return 1;
	}
	
	ShowWindow(WindowHandle, SW_SHOW);

	QueryPerformanceFrequency((LARGE_INTEGER*)&TICKS_PER_SECOND);

	Renderer Renderer;
	
	ControlInputs Inputs;
	
	Inputs.LatestMouse = Engine::GetMousePosition();
	Inputs.DeltaMouse = Vec2i(0, 0);
	
	Vec2i screenSize = Engine::GetScreenSize();

	Initialize(Renderer);

	SetCursor(0);

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

			if (GetFocus() == WindowHandle && cursorLocked)
			{
				SetCursor(0);
				WarpMouseToWindowCenter();
			}

			Vec2i screenSize = Engine::GetScreenSize();
			Inputs.LatestMouse = Vec2i(screenSize.x / 2, screenSize.y / 2);
			
			Inputs.keysDown.w = GetAsyncKeyState('W');
			Inputs.keysDown.a = GetAsyncKeyState('A');
			Inputs.keysDown.s = GetAsyncKeyState('S');
			Inputs.keysDown.d = GetAsyncKeyState('D');
			Inputs.keysDown.space = GetAsyncKeyState(VK_SPACE);

			Inputs.keysDown.esc = GetAsyncKeyState(VK_ESCAPE);

			Inputs.mouse.leftMouseButton = GetAsyncKeyState(VK_LBUTTON);
			Inputs.mouse.rightMouseButton = GetAsyncKeyState(VK_RBUTTON);
			
		}
		
		Renderer.SetTime(Engine::GetElapsedTime());
		Update(Renderer, Inputs);
	}
	return 0;
}