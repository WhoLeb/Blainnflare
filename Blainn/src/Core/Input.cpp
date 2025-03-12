#include "pch.h"
#include "Input.h"

#include "Application.h"

namespace Blainn
{
	void Input::Init()
	{
		Rid[0].usUsagePage = 0x01; //usage page generic
		Rid[0].usUsage = 0x02; // usage generic mouse
		Rid[0].dwFlags = 0;
		Rid[0].hwndTarget = nullptr;

		if (RegisterRawInputDevices(Rid, _countof(Rid), sizeof(Rid[0])) == FALSE)
			throw std::runtime_error("Failed to register raw input device");
	}

	void Input::Update()
	{
		TransitionPressedKeys();
		TransitionPressedButtons();
		if (s_CursorLocked)
		{
			RECT rect;
			GetClientRect(GetActiveWindow(), &rect);
			POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
			ClientToScreen(GetActiveWindow(), &center);
			SetCursorPos(center.x, center.y);
		}
		//UpdateMouseDelta();
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

	void Input::UpdateMouseDelta(LONG x, LONG y)
	{
		s_MouseDeltaX = x;
		s_MouseDeltaY = y;
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		if (s_CursorMode == mode)
			return;
		s_CursorMode = mode;
		switch (mode)
		{
		case Blainn::CursorMode::Normal:
			ShowCursor(TRUE);
			s_CursorLocked = false;
			break;
		case Blainn::CursorMode::Hidden:
			while(ShowCursor(FALSE) >= 0);
			s_CursorLocked = false;
			break;
		case Blainn::CursorMode::Locked:
			while(ShowCursor(FALSE) >= 0);
			s_CursorLocked = true;
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

	KeyPressedDelegate Input::OnKeyPressedDelegate;
	KeyReleasedDelegate Input::OnKeyReleasedDelegate;
	MouseButtonPressedDelegate Input::OnMouseButtonPressedDelegate;
	MouseButtonReleasedDelegate Input::OnMouseButtonReleasedDelegate;

}
