#include "pch.h"
#include "ComboboxSelector.h"

#include "CommCtrl.h"

static const TCHAR* COMBOBOX_SELECTOR_CLASS_NAME = TEXT("ComboBoxSelectorClass");

namespace Blainn
{
	int ComboBoxSelector::ShowModal()
	{
		EnsureWindowClassRegistered(m_hInstance);

		// 2) Create the popup window (WS_POPUP plus optional WS_CAPTION, etc.)
		m_popupHwnd = CreateWindowEx(
			WS_EX_TOOLWINDOW,                 // no taskbar button
			COMBOBOX_SELECTOR_CLASS_NAME,     // custom class
			TEXT("Select an Item"),            // window title (if you add WS_CAPTION)
			WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
			200, 200,                         // initial x,y (we'll reposition soon)
			320, 140,                         // width, height
			m_parentHwnd,                    // owner window
			NULL,
			m_hInstance,
			this); // pass "this" as lpParam so WndProc can get instance pointer

		if (!m_popupHwnd)
		{
			// Creation failed; return -1
			return -1;
		}

		// Optionally, disable parent for a modal effect
		EnableWindow(m_parentHwnd, FALSE);

		// Position the popup in the center of the parent
		RECT parentRect;
		GetWindowRect(m_parentHwnd, &parentRect);
		int parentWidth = parentRect.right - parentRect.left;
		int parentHeight = parentRect.bottom - parentRect.top;
		int popupWidth = 320;
		int popupHeight = 140;
		int xPos = parentRect.left + (parentWidth - popupWidth) / 2;
		int yPos = parentRect.top + (parentHeight - popupHeight) / 2;

		SetWindowPos(m_popupHwnd, HWND_TOP, xPos, yPos, popupWidth, popupHeight, SWP_SHOWWINDOW);

		// 3) Run a local (modal) message loop until the popup is destroyed
		m_selectedIndex = -1; // default

		// If you want a timeout, set it up here
		//auto startTime = std::chrono::steady_clock::now();
		//const int TIMEOUT_SECONDS = 30; // example

		MSG msg;
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					// Something posted a quit message => break entire loop
					break;
				}
				// If our window is destroyed, break
				if (!IsWindow(m_popupHwnd))
				{
					break;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				// Sleep briefly to avoid busy loop
				Sleep(10);
			}

			// Once the popup is destroyed, we leave
			if (!IsWindow(m_popupHwnd))
				break;
		}

		// Re-enable the parent if it was disabled
		EnableWindow(m_parentHwnd, TRUE);
		SetForegroundWindow(m_parentHwnd);

		// Return the user’s final selection
		return m_selectedIndex;
	}

	void ComboBoxSelector::EnsureWindowClassRegistered(HINSTANCE hInstance)
	{
		static bool s_registered = false;
		if (s_registered)
			return; // already done

		WNDCLASSEX wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &ComboBoxSelector::s_WndProc;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcex.lpszClassName = COMBOBOX_SELECTOR_CLASS_NAME;

		RegisterClassEx(&wcex);
		s_registered = true;
	}

	LRESULT ComboBoxSelector::s_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ComboBoxSelector* pThis = nullptr;

		if (msg == WM_NCCREATE)
		{
			CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
			pThis = (ComboBoxSelector*)cs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
		}
		else
		{
			pThis = (ComboBoxSelector*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		}

		if (pThis)
		{
			return pThis->WndProc(hWnd, msg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	LRESULT ComboBoxSelector::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			// Save the window handle
			m_popupHwnd = hWnd;

			// Create child combo box
			m_comboHwnd = CreateWindowEx(
				0,
				WC_COMBOBOX,
				NULL,
				CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
				10, 10, 280, 200,
				hWnd,
				(HMENU)101,
				m_hInstance,
				NULL);

			// Populate combo
			for (size_t i = 0; i < m_items.size(); ++i)
			{
				SendMessage(m_comboHwnd, CB_ADDSTRING, 0, (LPARAM)m_items[i].c_str());
			}
			if (!m_items.empty())
			{
				// Select the first item by default
				SendMessage(m_comboHwnd, CB_SETCURSEL, 0, 0);
			}

			// Create OK button
			m_okButtonHwnd = CreateWindowEx(
				0,
				TEXT("BUTTON"),
				TEXT("OK"),
				WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
				10, 70, 80, 25,
				hWnd,
				(HMENU)102,
				m_hInstance,
				NULL);

			// Create Cancel button
			m_cancelButtonHwnd = CreateWindowEx(
				0,
				TEXT("BUTTON"),
				TEXT("Cancel"),
				WS_CHILD | WS_VISIBLE,
				110, 70, 80, 25,
				hWnd,
				(HMENU)103,
				m_hInstance,
				NULL);

			return 0;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case 102: // OK button
			{
				// Get selected index from combo
				int sel = (int)SendMessage(m_comboHwnd, CB_GETCURSEL, 0, 0);
				if (sel >= 0)
					m_selectedIndex = sel;

				DestroyWindow(hWnd); // close popup
			}
			break;

			case 103: // Cancel button
			{
				m_selectedIndex = -1;
				DestroyWindow(hWnd);
			}
			break;
			}
		}
		break;

		case WM_CLOSE:
			m_selectedIndex = -1;
			DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			return 0;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

}
