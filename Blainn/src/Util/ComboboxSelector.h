#pragma once

namespace Blainn
{

	class ComboBoxSelector
	{
	public:
		ComboBoxSelector(HINSTANCE hInstance, HWND parent, const std::vector<std::wstring>& items)
			: m_hInstance(hInstance),
			m_parentHwnd(parent),
			m_items(items),
			m_popupHwnd(nullptr),
			m_comboHwnd(nullptr),
			m_okButtonHwnd(nullptr),
			m_cancelButtonHwnd(nullptr),
			m_selectedIndex(-1)
		{
		}

		int ShowModal();

	private:
		// One window class for all ComboBoxSelector instances
		static void EnsureWindowClassRegistered(HINSTANCE hInstance);

		static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HINSTANCE	m_hInstance;
		HWND		m_parentHwnd;
		std::vector<std::wstring> m_items;

		// Popup elements
		HWND m_popupHwnd;
		HWND m_comboHwnd;
		HWND m_okButtonHwnd;
		HWND m_cancelButtonHwnd;

		int  m_selectedIndex;
	};

}
