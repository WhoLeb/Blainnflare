#pragma once

#include "KeyCodes.h"

#include <map>
#include <utility>

namespace Blainn
{
	struct KeyData
	{
		KeyCode Key;
		KeyState State = KeyState::None;
		KeyState OldState = KeyState::None;
	};

	struct ButtonData
	{
		MouseButton Button;
		KeyState State = KeyState::None;
		KeyState OldState = KeyState::None;
	};

	class Input
	{
	public:
		static void Update();

		static bool IsKeyPressed(KeyCode keyCode);
		static bool IsKeyHeld(KeyCode keyCode);
		static bool IsKeyDown(KeyCode keyCode);
		static bool IsKeyReleased(KeyCode keyCode);

		static bool IsMouseButtonPressed(MouseButton mouseButton);
		static bool IsMouseButtonHeld(MouseButton mouseButton);
		static bool IsMouseButtonDown(MouseButton mouseButton);
		static bool IsMouseButtonReleased(MouseButton mouseButton);

		static int GetMouseX();
		static int GetMouseY();
		static std::pair<int, int> GetMousePosition();
		static std::pair<int, int> GetMouseDelta();

		static void SetCursorMode(CursorMode mode);
		static CursorMode GetCursorMode();

		// Internal stuff
		static void TransitionPressedKeys();
		static void TransitionPressedButtons();
		static void UpdateKeyState(KeyCode key, KeyState newState);
		static void UpdateButtonState(MouseButton button, KeyState newState);
		static void ClearReleasedKeys();

	private:
		static void UpdateMouseDelta();

	private:
		inline static std::map<KeyCode, KeyData> s_KeyData;
		inline static std::map<MouseButton, ButtonData> s_MouseData;

		inline static int s_MouseDeltaX = 0;
		inline static int s_MouseDeltaY = 0;
		inline static int s_LastMouseX = 0;
		inline static int s_LastMouseY = 0;
		inline static bool s_CursorLocked = false;

		inline static CursorMode s_CursorMode;
	};
}
