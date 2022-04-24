#include "InputModule.h"

#include "..\GameEngine.h"

InputModule::InputModule()
	: m_Keys{ false }
	, m_MouseState(Engine::GetMousePosition())
	, m_MouseCenter(Vec2i(Engine::GetClientAreaSize().x / 2, Engine::GetClientAreaSize().y / 2))
	, m_MouseLocked(false)
{
}

InputModule::~InputModule()
{
}

bool InputModule::IsKeyDown(Key key) const
{
	return m_Keys[static_cast<size_t>(key)];
}

void InputModule::SetKeyDown(Key key, bool pressed)
{
	m_Keys[static_cast<size_t>(key)] = pressed;
}

MouseState& InputModule::GetMouseState()
{
	return m_MouseState;
}

void InputModule::UpdateMousePos(Vec2i newPos)
{
	m_MouseState.UpdateMousePos(newPos, m_MouseLocked, m_MouseCenter);
}

void InputModule::SetMouseLocked(bool locked)
{
	m_MouseLocked = locked;
}

void InputModule::SetMouseCenter(Vec2i newCenter)
{
	m_MouseCenter = newCenter;
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

Vec2i MouseState::GetMousePos() const
{
	return m_CurrentPos;
}

Vec2i MouseState::GetDeltaMousePos() const
{
	return m_DeltaMouse;
}

bool MouseState::IsButtonDown(Mouse button) const
{
	return m_Buttons[static_cast<size_t>(button)];
}

void MouseState::SetButtonDown(Mouse button, bool pressed)
{
	m_Buttons[static_cast<size_t>(button)] = pressed;
}
