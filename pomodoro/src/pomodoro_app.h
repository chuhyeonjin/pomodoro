#pragma once

#include "../resource/Resource.h"
#include "framework.h"
#include "pomodoro_timer.h"
#include "tray_icon.h"

#define MAX_LOADSTRING 100

class PomodoroApp {
 private:
  HWND hwnd_;
  HINSTANCE hinstance_;
  WCHAR window_title_[MAX_LOADSTRING];
  WCHAR window_class_[MAX_LOADSTRING];
  TrayIcon tray_icon_;
  PomodoroTimer pomodoro_timer_;

  ID2D1Factory* direct2d_factory_;
  IDWriteFactory* directwrite_factory_;
  ID2D1HwndRenderTarget* render_target_;
  ID2D1SolidColorBrush* pie_chart_background_brush_;
  ID2D1SolidColorBrush* white_brush_;
  IDWriteTextFormat* time_text_format_;
  IDWriteTextFormat* mode_text_format_;
  IDWriteTextFormat* round_text_format_;
  const WCHAR* text_;
  UINT32 text_length_;
  HRESULT CreateDeviceIndependentResources();
  HRESULT CreateDeviceResources();
  void DiscardDeviceResources();

  bool InitializeWindow(LPWSTR cmd_line, int cmd_show);

  HRESULT DrawStartButton(ID2D1PathGeometry* path_geometry);
  HRESULT DrawPauseButton(ID2D1PathGeometry* path_geometry);
  HRESULT DrawPie(ID2D1PathGeometry* path_geometry, D2D_POINT_2F center,
                  FLOAT sweep_angle, FLOAT outer_radius);

 public:
  PomodoroApp(HINSTANCE hinstance);
  ~PomodoroApp();
  bool Initialize(LPWSTR cmd_line, int cmd_show);
  int RunMessageLoop();
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                                  LPARAM lParam);
  HRESULT Render();
};