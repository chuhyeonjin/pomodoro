#include "tray_icon.h"

TrayIcon::TrayIcon(HWND hwnd)
    : hwnd_(hwnd),
      icon_guid_{0xd86b1327,
                 0xfff2,
                 0x4f76,
                 {0xa3, 0x34, 0xcb, 0x1f, 0x81, 0xc1, 0xcd, 0x25}} {}

bool TrayIcon::AddTrayIcon(HICON icon) {
  NOTIFYICONDATA nid = {};
  nid.cbSize = sizeof(nid);
  nid.hWnd = hwnd_;
  nid.uFlags = NIF_ICON | NIF_GUID | NIF_MESSAGE;
  nid.uCallbackMessage = APPWM_NOTIFYICON;

  nid.guidItem = icon_guid_;
  nid.hIcon = icon;

  int retry = 0;
  const int max_retry = 20;
  while (!Shell_NotifyIcon(NIM_ADD, &nid)) {
    if (retry++ < max_retry) return false;
    Sleep(80);
  }

  return true;
}

bool TrayIcon::RemoveTrayIcon() { 
  NOTIFYICONDATA icon_data;
  icon_data.cbSize = sizeof(icon_data);
  icon_data.uFlags = NIF_GUID;
  icon_data.guidItem = icon_guid_;
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