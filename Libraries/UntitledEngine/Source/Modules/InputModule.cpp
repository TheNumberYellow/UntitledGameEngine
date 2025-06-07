#include "InputModule.h"

#include "..\GameEngine.h"

InputModule* InputModule::s_Instance = nullptr;

SystemInputState::SystemInputState()
    : m_Keys{ false }
    , m_MouseState(Engine::GetMousePosition())
    , m_GamepadState{ GamepadState(), GamepadState(), GamepadState(), GamepadState() }
{
}

KeyState& SystemInputState::GetKeyState(Key key)
{
	return m_Keys[static_cast<size_t>(key)];
}

bool SystemInputState::IsKeyDown(Key key) const
{
	return m_Keys[static_cast<size_t>(key)].pressed;
}

void SystemInputState::SetKeyDown(Key key, bool pressed)
{
	m_Keys[static_cast<size_t>(key)].UpdateState(pressed);
}

MouseState& SystemInputState::GetMouseState()
{
	return m_MouseState;
}

void SystemInputState::UpdateMousePos(Vec2i newPos)
{
	m_MouseState.UpdateMousePos(newPos, m_MouseLocked, m_MouseCenter);
}

void SystemInputState::UpdateMouseWheel(int delta)
{
	m_MouseState.UpdateMouseWheel(delta);
}

void SystemInputState::SetMouseLocked(bool locked)
{
	m_MouseLocked = locked;
}

void SystemInputState::SetMouseCenter(Vec2i newCenter)
{
    Engine::SetCursorCenter(newCenter);
    m_MouseCenter = newCenter;
}

GamepadState& SystemInputState::GetGamepadState(int controllerIndex)
{
	return m_GamepadState[static_cast<size_t>(controllerIndex)];
}

void SystemInputState::InputCharacter(char c)
{
	m_CharQueue.push(c);
}

bool SystemInputState::ConsumeCharacter(char& c)
{
    if (m_CharQueue.empty())
    {
        return false;
    }

    c = m_CharQueue.front();
    m_CharQueue.pop();

    return true;

}

void SystemInputState::ClearCharacters()
{
}

void SystemInputState::Reset()
{
    for (KeyState& Key : m_Keys)
    {
        Key.UpdateState(false);
    }

    m_MouseState.ResetMouseState();
}

InputModule::InputModule()
	: m_LocalSystemInputState(SystemInputState())
{
	s_Instance = this;
}

InputModule::~InputModule()
{
}

KeyState& InputModule::GetKeyState(Key key)
{
	return m_LocalSystemInputState.GetKeyState(key);
}

bool InputModule::IsKeyDown(Key key) const
{
	return m_LocalSystemInputState.IsKeyDown(key);
}

void InputModule::SetKeyDown(Key key, bool pressed)
{
	m_LocalSystemInputState.SetKeyDown(key, pressed);
}

MouseState& InputModule::GetMouseState()
{
	return m_LocalSystemInputState.GetMouseState();
}

void InputModule::UpdateMousePos(Vec2i newPos)
{
	m_LocalSystemInputState.UpdateMousePos(newPos);
}

void InputModule::UpdateMouseWheel(int delta)
{
	m_LocalSystemInputState.UpdateMouseWheel(delta);
}

void InputModule::SetMouseLocked(bool locked)
{
    m_LocalSystemInputState.SetMouseLocked(locked);
    
}

void InputModule::SetMouseCenter(Vec2i newCenter)
{
	m_LocalSystemInputState.SetMouseCenter(newCenter);
}

GamepadState& InputModule::GetGamepadState(int controllerIndex /*= 0*/)
{
	return m_LocalSystemInputState.GetGamepadState(controllerIndex);
}

void InputModule::InputCharacter(char c)
{
	m_LocalSystemInputState.InputCharacter(c);
}

bool InputModule::ConsumeCharacter(char& c)
{
	return m_LocalSystemInputState.ConsumeCharacter(c);
}

void InputModule::ClearCharacters()
{
	m_LocalSystemInputState.ClearCharacters();
}

void InputModule::ResetAllInputState()
{
	m_LocalSystemInputState.Reset();
}

void InputModule::OnFrameEnd()
{
	UpdateMouseWheel(0);
	ClearCharacters();
}

void InputModule::Resize(Vec2i newSize)
{
}

MouseState::MouseState()
{
}

MouseState::MouseState(Vec2i initPos)
	: m_Buttons{ false }
{
	m_PrevPos = initPos;
	m_CurrentPos = initPos;
}

void MouseState::UpdateMousePos(Vec2i newPos, bool mouseLocked, Vec2i center)
{
	if (mouseLocked)
	{
		m_PrevPos = m_CurrentPos;
		m_CurrentPos = newPos;
		m_DeltaMouse = m_CurrentPos - center;
	}
	else
	{
		m_PrevPos = m_CurrentPos;
		m_CurrentPos = newPos;
		m_DeltaMouse = m_CurrentPos - m_PrevPos;
	}
}

void MouseState::UpdateMouseWheel(int delta)
{
	m_DeltaMouseWheel = delta;
}

Vec2i MouseState::GetMousePos() const
{
	return m_CurrentPos;
}

Vec2i MouseState::GetDeltaMousePos() const
{
	return m_DeltaMouse;
}

int MouseState::GetDeltaMouseWheel() const
{
	return m_DeltaMouseWheel;
}

KeyState MouseState::GetMouseButtonState(MouseButton button) const
{
	return m_Buttons[static_cast<size_t>(button)];
}

void MouseState::SetMouseButtonDown(MouseButton button, bool pressed)
{
	m_Buttons[static_cast<size_t>(button)].UpdateState(pressed);
}

void MouseState::ResetMouseState()
{
	for (KeyState& Button : m_Buttons)
	{
		Button.UpdateState(false);
	}
}

void GamepadState::SetEnabled(bool enabled)
{
	m_Enabled = enabled;
}

void GamepadState::UpdateLeftStickAxis(Vec2f newAxis)
{
	m_LeftStickAxis = newAxis;
}

void GamepadState::UpdateRightStickAxis(Vec2f newAxis)
{
	m_RightStickAxis = newAxis;
}

void GamepadState::UpdateLeftTriggerAnalog(float newValue)
{
	m_LeftTriggerAnalog = newValue;
}

void GamepadState::UpdateRightTriggerAnalog(float newValue)
{
	m_RightTriggerAnalog = newValue;
}

void GamepadState::SetButtonDown(Button button, bool pressed)
{
	m_Buttons[static_cast<size_t>(button)].UpdateState(pressed);
}

bool GamepadState::IsEnabled()
{
	return m_Enabled;
}

Vec2f GamepadState::GetLeftStickAxis()
{
	return m_LeftStickAxis;
}

Vec2f GamepadState::GetRightStickAxis()
{
	return m_RightStickAxis;
}

float GamepadState::GetLeftTriggerAnalog()
{
	return m_LeftTriggerAnalog;
}

float GamepadState::GetRightTriggerAnalog()
{
	return m_RightTriggerAnalog;
}

KeyState GamepadState::GetButtonState(Button button)
{
	return m_Buttons[static_cast<size_t>(button)];
}

