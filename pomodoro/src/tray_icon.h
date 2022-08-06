#pragma once

#include "../resource/Resource.h"
#include "framework.h"

#define APPWM_NOTIFYICON (WM_APP + 1)

class TrayIcon {
 private:
  HWND hwnd_;
  UINT icon_uid_;
  void ShowPopupMenu();
 public:
  TrayIcon(HWND hwnd);
  // RETURN: sucess to create tray icon -> true 
  bool AddTrayIcon(HICON icon, WCHAR* window_title);
  bool SetIcon(HICON icon);
  bool RemoveTrayIcon();
  LRESULT CALLBACK HandleEvent(WPARAM wParam, LPARAM lParam);
};