#include "tray_icon.h"

TrayIcon::TrayIcon(HWND hwnd)
    : hwnd_(hwnd), icon_uid_(1) {}

bool TrayIcon::AddTrayIcon(HICON icon, WCHAR* window_title) {
  NOTIFYICONDATA icon_data = {};
  icon_data.cbSize = sizeof(icon_data);
  icon_data.hWnd = hwnd_;
  icon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
  icon_data.uCallbackMessage = APPWM_NOTIFYICON;
  icon_data.uID = icon_uid_;
  icon_data.hIcon = icon;
  StringCchCopy(icon_data.szTip, ARRAYSIZE(icon_data.szTip), window_title);

  int retry = 0;
  const int max_retry = 20;
  while (!Shell_NotifyIcon(NIM_ADD, &icon_data)) {
    if (retry++ < max_retry) return false;
    Sleep(80);
  }

  return true;
}

bool TrayIcon::SetIcon(HICON icon) {
  NOTIFYICONDATA icon_data = {};
  icon_data.cbSize = sizeof(icon_data);
  icon_data.hWnd = hwnd_;
  icon_data.uFlags = NIF_ICON;
  icon_data.uID = icon_uid_;
  icon_data.hIcon = icon;

	return Shell_NotifyIcon(NIM_MODIFY, &icon_data);
} 

bool TrayIcon::RemoveTrayIcon() { 
  NOTIFYICONDATA icon_data = {};
  icon_data.cbSize = sizeof(icon_data);
  icon_data.uID = icon_uid_;
  icon_data.hWnd = hwnd_;
  return Shell_NotifyIcon(NIM_DELETE, &icon_data);
}

void TrayIcon::ShowPopupMenu() {
  HMENU popup_menu = CreatePopupMenu();
  AppendMenu(popup_menu, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");

  SetForegroundWindow(hwnd_);

  POINT cursor_pos;
  GetCursorPos(&cursor_pos);
  TrackPopupMenu(popup_menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, cursor_pos.x,
                 cursor_pos.y, 0, hwnd_, NULL);
}

LRESULT CALLBACK TrayIcon::HandleEvent(WPARAM wparam, LPARAM lparam) {
  switch (lparam) {
    case WM_LBUTTONUP:
      ShowWindow(hwnd_, SW_SHOWNORMAL);
      break;
    case WM_RBUTTONDOWN: {
      ShowPopupMenu();
      break;
    }
  }
  return 0;
}