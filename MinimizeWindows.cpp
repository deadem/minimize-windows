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
	// Check if the window is on the primary monitor
	if ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) == 0) {
		return TRUE;
	}

	// Minimize the window
	ShowWindow(hwnd, SW_SHOWMINNOACTIVE);

	// Continue enumeration
	return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE,
	_In_opt_ HINSTANCE,
	_In_ LPWSTR,
	_In_ int)
{
	EnumWindows(EnumWindowsProc, 0);

	return 0;
}
