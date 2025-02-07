#include "Window.h"

#include "Application.h"
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

			WNDCLASS wc = {};
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = WindowProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = m_ApplicationInstance;
			wc.hIcon = LoadIcon(0, IDI_WINLOGO);
			wc.hCursor = LoadCursor(0, IDC_ARROW);
			wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
			wc.lpszMenuName = 0;
			wc.lpszClassName = CLASS_NAME;

			if (!RegisterClass(&wc))
			{
				MessageBox(0, L"Failed to register window class", 0, 0);
				return false;
			}

			RECT R = { 0, 0, m_Data.Width, m_Data.Height };
			AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
			int width = R.right - R.left;
			int height = R.bottom - R.top;

			m_Window = CreateWindow(
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT,
				width, height,
				NULL,
				NULL,
				m_ApplicationInstance,
				NULL
			);

			if (!m_Window)
			{
				MessageBox(0, L"Failed to Create main window", 0, 0);
				return false;
			}
			m_bIsInitialized = true;

			ShowWindow(m_Window, SW_SHOW);
			UpdateWindow(m_Window);
		}
		return true;
	}

	void Window::Shutdown()
	{
	}

	LRESULT Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return Application::Get().MsgProc(hwnd, uMsg, wParam, lParam);
	}
}