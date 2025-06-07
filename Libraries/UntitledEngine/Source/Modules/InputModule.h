#pragma once

#include "..\Math\Vector.h"
#include "..\Interfaces\Resizeable_i.h"

#include <queue>

enum class MouseButton
{
	LMB,
	RMB,
	MIDDLE,

	Count
};

enum class Key
{
	Escape,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	Zero,
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	
	Space,
	Alt,
	Tab,
	Shift,
	Ctrl,
	Up,
	Down,
	Left,
	Right,
	Delete,

	Plus,
	Minus,

	Count
};

enum class Button
{
	Face_North,
	Face_West,
	Face_East,
	Face_South,
	DPad_Up,
	DPad_Left,
	DPad_Right,
	DPad_Down,
	Start,
	Back,
	Shoulder_Right,
	Shoulder_Left,
	Thumb_Left,
	Thumb_Right,

	Count
};

struct KeyState
{
	explicit operator bool() const
	{
		return pressed;
	}

	void UpdateState(bool pressed)
	{
		if (this->pressed && pressed)
		{
			justReleased = false;
			justPressed = false;
		}
		if (!this->pressed && pressed)
		{
			justReleased = false;
			justPressed = true;
		}
		if (this->pressed && !pressed)
		{
			justReleased = true;
			justPressed = false;
		}
		if (!this->pressed && !pressed)
		{
			justReleased = false;
			justPressed = false;
		}

		this->pressed = pressed;
	}

	bool pressed = false;
	bool justPressed = false;
	bool justReleased = false;
};

class MouseState
{
public:
	MouseState();

	MouseState(Vec2i initPos);
	void UpdateMousePos(Vec2i newPos, bool mouseLocked, Vec2i center);
	void UpdateMouseWheel(int delta);

	Vec2i GetMousePos() const;
	Vec2i GetDeltaMousePos() const;
	int GetDeltaMouseWheel() const;

	KeyState GetMouseButtonState(MouseButton button) const;
	void SetMouseButtonDown(MouseButton button, bool pressed);

	void ResetMouseState();

private:

	int m_DeltaMouseWheel = 0;

	Vec2i m_PrevPos = Vec2i(0, 0);
	Vec2i m_CurrentPos = Vec2i(0, 0);
	Vec2i m_DeltaMouse = Vec2i(0, 0);

	KeyState m_Buttons[static_cast<size_t>(MouseButton::Count)];
};

class GamepadState
{
public:

	void SetEnabled(bool enabled);
	void UpdateLeftStickAxis(Vec2f newAxis);
	void UpdateRightStickAxis(Vec2f newAxis);

	void UpdateLeftTriggerAnalog(float newValue);
	void UpdateRightTriggerAnalog(float newValue);

	void SetButtonDown(Button button, bool pressed);

	bool IsEnabled();
	Vec2f GetLeftStickAxis();
	Vec2f GetRightStickAxis();
	
	float GetLeftTriggerAnalog();
	float GetRightTriggerAnalog();

	KeyState GetButtonState(Button button);

private:
	bool m_Enabled = false;
	
	Vec2f m_LeftStickAxis;
	Vec2f m_RightStickAxis;

	float m_LeftTriggerAnalog;
	float m_RightTriggerAnalog;

	KeyState m_Buttons[static_cast<size_t>(Button::Count)];
};

class SystemInputState
{
public:

	SystemInputState();

	KeyState& GetKeyState(Key key);
	bool IsKeyDown(Key key) const;
	void SetKeyDown(Key key, bool pressed);

    MouseState& GetMouseState();
    void UpdateMousePos(Vec2i newPos);
    void UpdateMouseWheel(int delta);

    void SetMouseLocked(bool locked);
    void SetMouseCenter(Vec2i newCenter);

    GamepadState& GetGamepadState(int controllerIndex = 0);

    void InputCharacter(char c);
    bool ConsumeCharacter(char& c);
    void ClearCharacters();

	void Reset();

private:

	KeyState m_Keys[static_cast<size_t>(Key::Count)];
    MouseState m_MouseState;
	GamepadState m_GamepadState[4];

	std::queue<char> m_CharQueue;

    bool m_MouseLocked;
    Vec2i m_MouseCenter;

};

class InputModule : public IResizeable
{
public:
	InputModule();
	~InputModule();

	KeyState& GetKeyState(Key key);
	bool IsKeyDown(Key key) const;
	void SetKeyDown(Key key, bool pressed);

	MouseState& GetMouseState();
	void UpdateMousePos(Vec2i newPos);
	void UpdateMouseWheel(int delta);

	void SetMouseLocked(bool locked);
	void SetMouseCenter(Vec2i newCenter);

	GamepadState& GetGamepadState(int controllerIndex = 0);

	void InputCharacter(char c);
	bool ConsumeCharacter(char& c);
	void ClearCharacters();

	void ResetAllInputState();

	void OnFrameEnd();

	// Inherited via IResizeable
	virtual void Resize(Vec2i newSize) override;

	static InputModule* Get() { return s_Instance; };
	
	SystemInputState m_LocalSystemInputState;

private:



	static InputModule* s_Instance;
};