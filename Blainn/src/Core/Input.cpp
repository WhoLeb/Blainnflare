#include "pch.h"
#include "Input.h"

#include "Application.h"

namespace Blainn
{
	inline void Input::Update()
	{
		TransitionPressedKeys();
		TransitionPressedButtons();
		UpdateMouseDelta();
	}

	bool Input::IsKeyPressed(KeyCode key)
	{
		return s_KeyData.find(key) != s_KeyData.end() && s_KeyData[key].State == KeyState::Pressed;
	}

	bool Input::IsKeyHeld(KeyCode key)
	{
		return s_KeyData.find(key) != s_KeyData.end() && s_KeyData[key].State == KeyState::Held;
	}

	bool Input::IsKeyDown(KeyCode key)
	{
		return false;
	}

	bool Input::IsKeyReleased(KeyCode key)
	{
		return s_KeyData.find(key) != s_KeyData.end() && s_KeyData[key].State == KeyState::Released;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return s_MouseData.find(button) != s_MouseData.end() && s_MouseData[button].State == KeyState::Pressed;
	}

	bool Input::IsMouseButtonHeld(MouseButton button)
	{
		return s_MouseData.find(button) != s_MouseData.end() && s_MouseData[button].State == KeyState::Held;
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return false;
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return s_MouseData.find(button) != s_MouseData.end() && s_MouseData[button].State == KeyState::Released;
	}

	int Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return x;
	}

	int Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return y;
	}

	::std::pair<int, int> Input::GetMousePosition()
	{
		auto& instance = ::Blainn::Application::Get().GetWindow();
		return instance.GetCursorPosition();
	}

	std::pair<int, int> Input::GetMouseDelta()
	{
		return { s_MouseDeltaX, s_MouseDeltaY };
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		s_CursorMode = mode;
		switch (mode)
		{
		case Blainn::CursorMode::Normal:
			ShowCursor(TRUE);
			ClipCursor(nullptr);
			s_CursorLocked = false;
			break;
		case Blainn::CursorMode::Hidden:
			ShowCursor(FALSE);
			ClipCursor(nullptr);
			s_CursorLocked = false;
			break;
		case Blainn::CursorMode::Locked:
		{
			ShowCursor(FALSE);
			HWND hwnd = GetActiveWindow();
			if (!hwnd) return;

			RECT rect;
			GetClientRect(hwnd, &rect);
			POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
			ClientToScreen(hwnd, &center);
			ClipCursor(&rect);
			SetCursorPos(center.x, center.y);
			s_CursorLocked = true;
		}
			break;
		default:
			break;
		}
	}

	CursorMode Input::GetCursorMode()
	{
		return s_CursorMode;
	}

	void Input::TransitionPressedKeys()
	{
		for (const auto& [key, keyData] : s_KeyData)
		{
			if (keyData.State == KeyState::Pressed)
				UpdateKeyState(key, KeyState::Held);
		}
	}

	void Input::TransitionPressedButtons()
	{
		for (const auto& [button, buttonData] : s_MouseData)
		{
			if (buttonData.State == KeyState::Pressed)
				UpdateButtonState(button, KeyState::Held);
		}
	}

	void Input::UpdateKeyState(KeyCode key, KeyState newState)
	{
		auto& keyData = s_KeyData[key];
		keyData.Key = key;
		keyData.OldState = keyData.State;
		keyData.State = newState;
	}

	void Input::UpdateButtonState(MouseButton button, KeyState newState)
	{
		auto& buttonData = s_MouseData[button];
		buttonData.Button = button;
		buttonData.OldState = buttonData.State;
		buttonData.State = newState;
	}

	void Input::ClearReleasedKeys()
	{
		for (const auto& [key, keyData] : s_KeyData)
		{
			if (keyData.State == KeyState::Released)
				UpdateKeyState(key, KeyState::None);
		}
		for (const auto& [button, buttonData] : s_MouseData)
		{
			if (buttonData.State == KeyState::Released)
				UpdateButtonState(button, KeyState::None);
		}
	}

	void Input::UpdateMouseDelta()
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(GetActiveWindow(), &p);

		if (s_CursorLocked)
		{
			// Calculate delta movement
			s_MouseDeltaX = p.x - s_LastMouseX;
			s_MouseDeltaY = p.y - s_LastMouseY;

			// Reset cursor to center to prevent large deltas
			RECT rect;
			GetClientRect(GetActiveWindow(), &rect);
			POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
			ClientToScreen(GetActiveWindow(), &center);
			SetCursorPos(center.x, center.y);

			s_LastMouseX = center.x;
			s_LastMouseY = center.y;
		}
		else
		{
			s_MouseDeltaX = 0;
			s_MouseDeltaY = 0;
			s_LastMouseX = p.x;
			s_LastMouseY = p.y;
		}
	}

}
