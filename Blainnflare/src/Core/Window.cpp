#include "Window.h"
namespace
{
}
namespace Blainn
{
	Window* Window::Create(const HINSTANCE hInstance, const WindowDesc& description)
	{
		return new Window(hInstance, description);
	}


	Window::Window(const HINSTANCE hInstance, const WindowDesc& description)
		: m_ApplicationInstance(hInstance), m_Description(description)
	{
		Init();
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init()
	{
		m_Data.Title = m_Description.Title;
		m_Data.Width = m_Description.Width;
		m_Data.Height = m_Description.Height;

		if (!m_bIsInitialized)
		{
			//std::wstring wideTitle = std::wstring(m_Data.Title.begin(), m_Data.Title.end()).c_str();
			const wchar_t CLASS_NAME[] = L"WindowName";// wideTitle.c_str();

			WNDCLASS wc = {};
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			wc.hIcon = LoadIcon(0, IDI_WINLOGO);
			wc.lpszMenuName = 0;
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			wc.lpfnWndProc = WindowProc;
			wc.hInstance = m_ApplicationInstance;
			wc.lpszClassName = CLASS_NAME;

			if(!RegisterClass(&wc))
				throw std::exception("Failed to register window class");

			m_Window = CreateWindow(
				CLASS_NAME,
				L"Winodw",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				m_Data.Width, m_Data.Height,
				NULL,
				NULL,
				m_ApplicationInstance,
				NULL
			);

			if (!m_Window)
				throw std::exception("Failed to instanciate window");
			m_bIsInitialized = true;

			ShowWindow(m_Window, SW_SHOW);
			UpdateWindow(m_Window);
		}
	}

	void Window::Shutdown()
	{
	}

	LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg) {
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDc = BeginPaint(hwnd, &ps);

			FillRect(hDc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);

			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}