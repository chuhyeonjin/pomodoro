#pragma once

#include "../resource/Resource.h"
#include "framework.h"

#define APPWM_NOTIFYICON (WM_APP + 1)

class TrayIcon {
 private:
  HWND hwnd_;
  GUID icon_guid_;
  void ShowPopupMenu();
 public:
  TrayIcon(HWND hwnd);
  // RETURN: sucess to create tray icon -> true 
  bool AddTrayIcon(HICON icon);
  bool RemoveTrayIcon();
  LRESULT CALLBACK HandleEvent(WPARAM wParam, LPARAM lParam);
};