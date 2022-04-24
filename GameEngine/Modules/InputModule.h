#pragma once

#include "..\Math\Vector.h"
#include "..\Interfaces\Resizeable_i.h"

enum class Mouse
{
	LMB,
	RMB,

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

	Count
};

class MouseState
{
public:
	MouseState(Vec2i initPos);
	void UpdateMousePos(Vec2i newPos, bool mouseLocked, Vec2i center);

	Vec2i GetMousePos() const;
	Vec2i GetDeltaMousePos() const;

	bool IsButtonDown(Mouse button) const;
	void SetButtonDown(Mouse button, bool pressed);

private:
	bool m_LeftMouseButtonDown = false;
	bool m_RightMouseButtonDown = false;

	Vec2i m_PrevPos = Vec2i(0, 0);
	Vec2i m_CurrentPos = Vec2i(0, 0);
	Vec2i m_DeltaMouse = Vec2i(0, 0);

	bool m_Buttons[static_cast<size_t>(Mouse::Count)];
};

class InputModule : public IResizeable
{
public:
	InputModule();
	~InputModule();

	bool IsKeyDown(Key key) const;
	void SetKeyDown(Key key, bool pressed);

	MouseState& GetMouseState();
	void UpdateMousePos(Vec2i newPos);

	void SetMouseLocked(bool locked);
	void SetMouseCenter(Vec2i newCenter);

	// Inherited via IResizeable
	virtual void Resize(Vec2i newSize) override;
private:

	bool m_Keys[static_cast<size_t>(Key::Count)];
	MouseState m_MouseState;

	bool m_MouseLocked;
	Vec2i m_MouseCenter;

};