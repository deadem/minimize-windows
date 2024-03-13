#include "framework.h"
#include "MinimizeWindows.h"

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {

	if (!IsWindowVisible(hwnd)) {
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

	ShowWindow(hwnd, SW_SHOWMINNOACTIVE);

	// Continue enumeration
	return TRUE;
}

HHOOK g_hKeyboardHook = NULL;
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		KBDLLHOOKSTRUCT* pKeyboardStruct = (KBDLLHOOKSTRUCT*)lParam;
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			// Check if 'D' is pressed along with the Windows key
			if (pKeyboardStruct->vkCode == 'D' && (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000)) {

				// Minimize all on main Desktop
				EnumWindows(EnumWindowsProc, 0);

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
