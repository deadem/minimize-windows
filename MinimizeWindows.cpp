#include "framework.h"
#include "MinimizeWindows.h"
#include <vector>

struct State {
	HWND m_window;
};


std::vector<State> lastWindows;
std::vector<State> allWindows;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM) {
	if (!IsWindowVisible(hwnd)) {
		return TRUE;
	}

	LONG style = GetWindowLong(hwnd, GWL_STYLE);
	if (style & WS_MINIMIZE) {
		return TRUE;
	}

	LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if (exStyle & WS_EX_TOOLWINDOW) {
		return TRUE;
	}

	RECT windowRect;
	if (!GetWindowRect(hwnd, &windowRect)) {
		return TRUE;
	}

	HMONITOR hMonitor = MonitorFromRect(&windowRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	if (!GetMonitorInfo(hMonitor, &monitorInfo)) {
		return TRUE;
	}

	if ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) == 0) {
		return TRUE;
	}

	allWindows.emplace_back(State{ hwnd });
	return TRUE;
}

HHOOK g_hKeyboardHook = NULL;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			// Check if 'D' is pressed along with the Windows key
			if (pKeyboardStruct->vkCode == 'D' && (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000)) {

				allWindows.clear();
				EnumWindows(EnumWindowsProc, 0);

				if (allWindows.empty()) {
					for (auto i = lastWindows.rbegin(); i != lastWindows.rend(); ++i) {
						SetWindowPos(i->m_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
					}
				}
				else {
					for (const auto& state : allWindows) {
						SetWindowPos(state.m_window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_HIDEWINDOW);
					}
				}

				lastWindows = allWindows;

				return 1; // Prevent the event from being passed on to other applications
			}
		}
	}

	return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);	// Call the next hook in the chain
}

int APIENTRY wWinMain(_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR,
	_In_ int)
{
	g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	if (g_hKeyboardHook == NULL) {
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(g_hKeyboardHook);

	return 0;
}
