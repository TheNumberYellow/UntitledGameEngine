#include "InputModule.h"

#include "..\GameEngine.h"

InputModule* InputModule::s_Instance = nullptr;

InputModule::InputModule()
	: m_Keys{ false }
	, m_MouseState(Engine::GetMousePosition())
	, m_MouseCenter(Vec2i(Engine::GetClientAreaSize().x / 2, Engine::GetClientAreaSize().y / 2))
	, m_MouseLocked(false)
	, m_GamepadState{ GamepadState(), GamepadState(), GamepadState(), GamepadState() }
{
	s_Instance = this;
}

InputModule::~InputModule()
{
}

KeyState& InputModule::GetKeyState(Key key)
{
	return m_Keys[static_cast<size_t>(key)];
}

bool InputModule::IsKeyDown(Key key) const
{
	return m_Keys[static_cast<size_t>(key)].pressed;
}

void InputModule::SetKeyDown(Key key, bool pressed)
{
	m_Keys[static_cast<size_t>(key)].UpdateState(pressed);
}

MouseState& InputModule::GetMouseState()
{
	return m_MouseState;
}

void InputModule::UpdateMousePos(Vec2i newPos)
{
	m_MouseState.UpdateMousePos(newPos, m_MouseLocked, m_MouseCenter);
}

void InputModule::UpdateMouseWheel(int delta)
{
	m_MouseState.UpdateMouseWheel(delta);
}

void InputModule::SetMouseLocked(bool locked)
{
	m_MouseLocked = locked;
}

void InputModule::SetMouseCenter(Vec2i newCenter)
{
	m_MouseCenter = newCenter;
}

GamepadState& InputModule::GetGamepadState(int controllerIndex /*= 0*/)
{
	return m_GamepadState[controllerIndex];
}

void InputModule::InputCharacter(char c)
{
	m_CharQueue.push(c);
}

bool InputModule::ConsumeCharacter(char& c)
{
	if (m_CharQueue.empty())
	{
		return false;
	}

	c = m_CharQueue.front();
	m_CharQueue.pop();

	return true;
}

void InputModule::ClearCharacters()
{
	while (!m_CharQueue.empty())
	{
		m_CharQueue.pop();
	}
}

void InputModule::OnFrameEnd()
{
	m_MouseState.UpdateMouseWheel(0);
	ClearCharacters();
}

void InputModule::Resize(Vec2i newSize)
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

bool MouseState::IsButtonDown(Mouse button) const
{
	return m_Buttons[static_cast<size_t>(button)];
}

void MouseState::SetButtonDown(Mouse button, bool pressed)
{
	m_Buttons[static_cast<size_t>(button)] = pressed;
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
