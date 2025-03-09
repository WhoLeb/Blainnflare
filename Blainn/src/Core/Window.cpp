#include "pch.h"

#include "Window.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Input.h"

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

namespace Blainn
{
	Window* Window::Create(const WindowDesc& description)
	{
		return new Window(description);
	}


	Window::Window(const WindowDesc& description)
		: m_Description(description)
	{
	}

	Window::~Window()
	{
		Shutdown();
	}

	bool Window::Init()
	{
		m_Data.Title = m_Description.Title;
		m_Data.Width = m_Description.Width;
		m_Data.Height = m_Description.Height;

		if (!m_bIsInitialized)
		{
			std::wstring wideTitle = std::wstring(m_Data.Title.begin(), m_Data.Title.end()).c_str();
			const wchar_t* CLASS_NAME = wideTitle.c_str();

			WNDCLASSEX wc = {};
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = StaticWindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = HINST_THISCOMPONENT;
			wc.hIcon = LoadIcon(0, IDI_WINLOGO);
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			wc.lpszMenuName = 0;
			wc.lpszClassName = CLASS_NAME;

			if (!RegisterClassEx(&wc))
			{
				MessageBox(0, L"Failed to register window class", 0, 0);
				return false;
			}

			RECT R = { 0, 0, m_Data.Width, m_Data.Height };
			AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
			int width = R.right - R.left;
			int height = R.bottom - R.top;

			m_Window = CreateWindowEx(
				0, 
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				width, height,
				nullptr,
				nullptr,
				HINST_THISCOMPONENT,
				this
			);

			if (!m_Window)
			{
				DWORD error = GetLastError();
				wchar_t buffer[256];
				swprintf(buffer, 256, L"CreateWindowEx failed! Error code: %lu", error);
				MessageBox(nullptr, buffer, L"Error", MB_ICONERROR);
				return false;
			}
			m_bIsInitialized = true;

			ShowWindow(m_Window, SW_SHOW);
			UpdateWindow(m_Window);
		}
		return true;
	}

	void Window::Show()
	{
	}

	std::pair<int, int> Window::GetCursorPosition()
	{
		POINT pos;
		if (GetCursorPos(&pos))
		{
			ScreenToClient(m_Window, &pos);
			return { pos.x, pos.y };
		}
		return { INT_MAX, INT_MAX };
	}

	void Window::Shutdown()
	{
		DestroyWindow(m_Window);
	}

	LRESULT Window::StaticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));


		if (!pThis && uMsg != WM_NCCREATE)
			return DefWindowProc(hwnd, uMsg, wParam, lParam);

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = static_cast<Window*>(pCreate->lpCreateParams);

			if (pThis) {
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
				OutputDebugString(L"WM_NCCREATE: pThis set successfully.\n");
			}
			else {
				OutputDebugString(L"WM_NCCREATE: pThis is NULL!\n");
			}
		}

		if (pThis)
			return pThis->WindowProc(hwnd, uMsg, wParam, lParam);

		OutputDebugString(L"pThis is NULL in StaticWindowProc!\n");
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	LRESULT Window::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		// when the window is activated or deactivated
		case WM_ACTIVATE:
		{
			WindowMinimizeEvent event(LOWORD(wParam) == WA_INACTIVE);
			m_Data.EventCallback(event);
			return 0;
		}
		// when the user resizes the window
		case WM_SIZE:
		{
			int width = LOWORD(lParam), height = HIWORD(lParam);
			m_Data.Width = width;
			m_Data.Height = height;

			WindowResizeEvent event(wParam, width, height);
			m_Data.EventCallback(event);
			return 0;
		}
		// when the user grabs the resize bars
		case WM_ENTERSIZEMOVE:
		{
			WindowMovedEvent event(true);
			m_Data.EventCallback(event);
			return 0;
		}
		// when the user releases the resize bars. Here everything is reset based on
		// the new dimensions
		case WM_EXITSIZEMOVE:
		{
			WindowMovedEvent event(false);
			m_Data.EventCallback(event);
			return 0;
		}
		// when the window is destroyed
		case WM_DESTROY:
		{
			WindowCloseEvent event;
			m_Data.EventCallback(event);
			return 0;
		}
		// When a user presses a key not corresponding to a mnemonic or accelerator key.
		case WM_MENUCHAR:
			// Don't beep on Alt+Enter
			return MAKELRESULT(0, MNC_CLOSE);

		case WM_GETMINMAXINFO:
		{
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			Input::UpdateButtonState(Button::Left, KeyState::Pressed);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonDownEvent event(Button::Left, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			Input::UpdateButtonState(Button::Middle, KeyState::Pressed);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonDownEvent event(Button::Middle, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			Input::UpdateButtonState(Button::Right, KeyState::Pressed);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonDownEvent event(Button::Right, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_LBUTTONUP:
		{
			Input::UpdateButtonState(Button::Left, KeyState::Released);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonReleasedEvent event(Button::Left, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			Input::UpdateButtonState(Button::Middle, KeyState::Released);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonReleasedEvent event(Button::Middle, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			Input::UpdateButtonState(Button::Right, KeyState::Released);
			int xOffset = (short)LOWORD(lParam), yOffset = (short)HIWORD(lParam);
			MouseButtonReleasedEvent event(Button::Right, xOffset, yOffset);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int offsetX = (int)(short)LOWORD(lParam);
			int offsetY = (int)(short)HIWORD(lParam);
			MouseMovedEvent event(offsetX, offsetY);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_KEYUP:
		{
			Input::UpdateKeyState(static_cast<KeyCode>(wParam), KeyState::Released);
			KeyReleasedEvent event((int)wParam);
			m_Data.EventCallback(event);
			return 0;
		}
		case WM_KEYDOWN:
		{
			Input::UpdateKeyState(static_cast<KeyCode>(wParam), KeyState::Pressed);
			// 30th bit is set to 1 if the button was already pressed when the
			// message is sent. So it is set to 1 if the button is held.
			KeyPressedEvent event((int)wParam, (lParam & BIT(30)) != 0);
			m_Data.EventCallback(event);
			return 0;
		}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}