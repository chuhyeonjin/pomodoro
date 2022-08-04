#include "pomodoro_app.h"
#define _USE_MATH_DEFINES
#include <math.h>

template <class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease) {
  if (*ppInterfaceToRelease != nullptr) {
    (*ppInterfaceToRelease)->Release();
    (*ppInterfaceToRelease) = nullptr;
  }
}

PomodoroApp::PomodoroApp(HINSTANCE hinstance)
    : hinstance_(hinstance), tray_icon_(nullptr), pomodoro_timer_(nullptr) {}

PomodoroApp::~PomodoroApp() {
  SafeRelease(&direct2d_factory_);
  SafeRelease(&directwrite_factory_);
  SafeRelease(&render_target_);
  SafeRelease(&pie_chart_background_brush_);
  SafeRelease(&white_brush_);
  SafeRelease(&time_text_format_);
  SafeRelease(&mode_text_format_);
  SafeRelease(&round_text_format_);
}

void PomodoroApp::DiscardDeviceResources() {
  SafeRelease(&render_target_);
  SafeRelease(&pie_chart_background_brush_);
  SafeRelease(&white_brush_);
}

HRESULT PomodoroApp::CreateDeviceIndependentResources() {
  HRESULT result = S_OK;
  result =
      D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &direct2d_factory_);
  if (!SUCCEEDED(result)) return result;

  result =
      DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                          reinterpret_cast<IUnknown**>(&directwrite_factory_));
  if (!SUCCEEDED(result)) return result;

  if (SUCCEEDED(result))
    result = directwrite_factory_->CreateTextFormat(
        L"Roboto", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL, 70.0f, L"en-us", &time_text_format_);
  if (SUCCEEDED(result))
    result = time_text_format_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  if (SUCCEEDED(result))
    result = time_text_format_->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  if (SUCCEEDED(result))
    result = directwrite_factory_->CreateTextFormat(
        L"Roboto", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"en-us",
        &mode_text_format_);
  if (SUCCEEDED(result))
    result = mode_text_format_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  if (SUCCEEDED(result))
    result = mode_text_format_->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  if (SUCCEEDED(result))
    result = directwrite_factory_->CreateTextFormat(
        L"Roboto", nullptr, DWRITE_FONT_WEIGHT_SEMI_BOLD,
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"en-us",
        &round_text_format_);
  if (SUCCEEDED(result))
    result = round_text_format_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  if (SUCCEEDED(result))
    result = round_text_format_->SetParagraphAlignment(
        DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  return result;
}

HRESULT PomodoroApp::CreateDeviceResources() {
  HRESULT result = S_OK;

  if (render_target_) return result;

  RECT rc;
  GetClientRect(hwnd_, &rc);
  D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

  result = direct2d_factory_->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties(hwnd_, size), &render_target_);

  if (SUCCEEDED(result)) {
    result = render_target_->CreateSolidColorBrush(
        pomodoro_timer_.GetPieChartBackgroundColor(),
        &pie_chart_background_brush_);
  }

  if (SUCCEEDED(result)) {
    result = render_target_->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White), &white_brush_);
  }

  return result;
}

bool PomodoroApp::InitializeWindow(LPWSTR cmd_line, int cmd_show) {
  LoadStringW(hinstance_, IDS_APP_TITLE, window_title_, MAX_LOADSTRING);
  LoadStringW(hinstance_, IDC_POMODORO, window_class_, MAX_LOADSTRING);

  // Register window class
  WNDCLASSEXW wcex = {};

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = PomodoroApp::WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hinstance_;
  wcex.hIcon = LoadIcon(hinstance_, MAKEINTRESOURCE(IDI_POMODORO));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszClassName = window_class_;

  RegisterClassExW(&wcex);

  // Create window
  const LONG width = 310;
  const LONG height = 310;

  const DWORD window_style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = width;
  rect.bottom = height;
  AdjustWindowRectEx(&rect, window_style, FALSE, 0);

  hwnd_ = CreateWindowExW(WS_EX_TOPMOST, window_class_, window_title_,
                          window_style, CW_USEDEFAULT, 0,
                          rect.right - rect.left, rect.bottom - rect.top,
                          nullptr, nullptr, hinstance_, this);

  if (!hwnd_) {
    return FALSE;
  }

  return TRUE;
}

bool PomodoroApp::Initialize(LPWSTR cmd_line, int cmd_show) {
  if (!SUCCEEDED(CreateDeviceIndependentResources())) return FALSE;
  if (!InitializeWindow(cmd_line, cmd_show)) return FALSE;

  tray_icon_ = TrayIcon(hwnd_);
  tray_icon_.AddTrayIcon(
      LoadIcon(hinstance_, MAKEINTRESOURCE(IDI_SHORT_BREAK)));

  pomodoro_timer_ = PomodoroTimer(hwnd_);

  ShowWindow(hwnd_, cmd_show);
  UpdateWindow(hwnd_);

  return TRUE;
}

int PomodoroApp::RunMessageLoop() {
  MSG msg;

  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return (int)msg.wParam;
}

D2D_POINT_2F CalcCartesianCoordinate(FLOAT angle, FLOAT radius) {
  double angle_radian = (M_PI / 180.f) * (angle - 90.f);
  FLOAT x = (FLOAT)(radius * cos(angle_radian));
  FLOAT y = (FLOAT)(radius * sin(angle_radian));
  return D2D1::Point2F(x, y);
}

HRESULT PomodoroApp::DrawPie(ID2D1PathGeometry* path_geometry,
                             D2D_POINT_2F center, FLOAT sweep_angle, FLOAT outer_radius) {
  HRESULT result = S_FALSE;

  D2D_POINT_2F start_point =
      CalcCartesianCoordinate(0.f, outer_radius);
  start_point.x += center.x;
  start_point.y += center.y;

  D2D_POINT_2F end_point =
      CalcCartesianCoordinate(0.f - sweep_angle, outer_radius);
  end_point.x += center.x;
  end_point.y += center.y;
  
  ID2D1GeometrySink* geometry_sink = nullptr;
  result = path_geometry->Open(&geometry_sink);
  if (!SUCCEEDED(result)) return result;

  geometry_sink->SetFillMode(D2D1_FILL_MODE_WINDING);
  geometry_sink->BeginFigure(start_point, D2D1_FIGURE_BEGIN_HOLLOW);
  geometry_sink->AddArc(D2D1::ArcSegment(
      end_point, D2D1::SizeF(outer_radius, outer_radius),
      0.0f,
      D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE,
      (sweep_angle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

  geometry_sink->EndFigure(D2D1_FIGURE_END_OPEN);

  result = geometry_sink->Close();
  if (SUCCEEDED(result)) SafeRelease(&geometry_sink);

  return result;
}

HRESULT PomodoroApp::DrawStartButton(ID2D1PathGeometry* path_geometry) {
  HRESULT result = S_FALSE;
  ID2D1GeometrySink* geometry_sink = nullptr;
  result = path_geometry->Open(&geometry_sink);
  if (!SUCCEEDED(result)) return result;

  D2D1_POINT_2F start_point = {151, 247};
  D2D1_POINT_2F middle_point = {151 + 11, 247 + 7};
  D2D1_POINT_2F end_point = {151, 247 + 14};

  geometry_sink->SetFillMode(D2D1_FILL_MODE_WINDING);
  geometry_sink->BeginFigure(start_point, D2D1_FIGURE_BEGIN_FILLED);
  
  geometry_sink->AddLine(middle_point);
  geometry_sink->AddLine(end_point);
  geometry_sink->AddLine(start_point);

  geometry_sink->EndFigure(D2D1_FIGURE_END_CLOSED);

  result = geometry_sink->Close();
  if (SUCCEEDED(result)) SafeRelease(&geometry_sink);

  return result;
}

HRESULT PomodoroApp::DrawPauseButton(ID2D1PathGeometry* path_geometry) {
  HRESULT result = S_FALSE;
  ID2D1GeometrySink* geometry_sink = nullptr;
  result = path_geometry->Open(&geometry_sink);
  if (!SUCCEEDED(result)) return result;

  D2D1_POINT_2F line1_start = {149 + 4, 247};
  D2D1_POINT_2F line1_second = {149, 247};
  D2D1_POINT_2F line1_third = {149, 247 + 14};
  D2D1_POINT_2F line1_end = {149 + 4, 247 + 14};

  D2D1_POINT_2F line2_start = {157, 247};
  D2D1_POINT_2F line2_second = {157, 247 + 14};
  D2D1_POINT_2F line2_third = {157 + 4, 247 + 14};
  D2D1_POINT_2F line2_end = {157 + 4, 247};

  geometry_sink->SetFillMode(D2D1_FILL_MODE_WINDING);
  geometry_sink->BeginFigure(line1_start, D2D1_FIGURE_BEGIN_FILLED);

  geometry_sink->AddLine(line1_start);
  geometry_sink->AddLine(line1_second);
  geometry_sink->AddLine(line1_third);
  geometry_sink->AddLine(line1_end);
  geometry_sink->AddLine(line1_start);

  geometry_sink->AddLine(line2_start);
  geometry_sink->AddLine(line2_second);
  geometry_sink->AddLine(line2_third);
  geometry_sink->AddLine(line2_end);
  geometry_sink->AddLine(line2_start);

  geometry_sink->EndFigure(D2D1_FIGURE_END_OPEN);

  result = geometry_sink->Close();
  if (SUCCEEDED(result)) SafeRelease(&geometry_sink);

  return result;
}

HRESULT PomodoroApp::Render() {
  HRESULT result = S_OK;

  result = CreateDeviceResources();
  if (!SUCCEEDED(result)) return result;

  render_target_->BeginDraw();
  render_target_->SetTransform(D2D1::Matrix3x2F::Identity());
  render_target_->Clear(pomodoro_timer_.GetBackgroundColor());


  D2D1_SIZE_F render_target_size = render_target_->GetSize();
  float width = render_target_size.width;
  float height = render_target_size.height;
  D2D1_POINT_2F center = {width / 2, height / 2};

  pie_chart_background_brush_->SetColor(
      pomodoro_timer_.GetPieChartBackgroundColor());

  D2D1_ELLIPSE pie_chart_background =
      D2D1::Ellipse(center, 140, 140);
  render_target_->DrawEllipse(pie_chart_background, pie_chart_background_brush_,
                              7.f);

  ID2D1PathGeometry* pie_chart_path = nullptr;
  direct2d_factory_->CreatePathGeometry(&pie_chart_path);
  DrawPie(pie_chart_path, center,
          360.f * (pomodoro_timer_.GetRemainingTimePercent() == 1 ? 0.999999f : pomodoro_timer_.GetRemainingTimePercent()), 140.0f);
  render_target_->DrawGeometry(pie_chart_path, white_brush_, 7.0f);

  if (pomodoro_timer_.IsPaused()) {
    ID2D1PathGeometry* start_button_path = nullptr;
    direct2d_factory_->CreatePathGeometry(&start_button_path);
    DrawStartButton(start_button_path);
    render_target_->FillGeometry(start_button_path, white_brush_);
  } else {
    ID2D1PathGeometry* pause_button_path = nullptr;
    direct2d_factory_->CreatePathGeometry(&pause_button_path);
    DrawPauseButton(pause_button_path);
    render_target_->FillGeometry(pause_button_path, white_brush_);
  }

  D2D1_RECT_F time_separator1 = D2D1::Rect(151, 138, 151 + 8, 138 + 8);
  render_target_->FillRectangle(time_separator1, white_brush_);

  D2D1_RECT_F time_separator2 = D2D1::Rect(151, 164, 151 + 8, 164 + 8);
  render_target_->FillRectangle(time_separator2, white_brush_);

  D2D1_RECT_F remaining_minute = D2D1::Rect(46, 124, 46 + 81, 124 + 54);
  WCHAR* remaining_minute_string = new WCHAR[100]();
  pomodoro_timer_.GetRemainingMinuteString(remaining_minute_string);
  text_length_ = (UINT32)wcslen(remaining_minute_string);
  render_target_->DrawText(remaining_minute_string, text_length_,
                           time_text_format_,
                           remaining_minute,
                          white_brush_);
  
  D2D1_RECT_F remaining_second = D2D1::Rect(183, 124, 183 + 81, 124 + 54);
  pomodoro_timer_.GetRemainingSecondString(remaining_minute_string);
  text_length_ = (UINT32)wcslen(remaining_minute_string);
  render_target_->DrawText(remaining_minute_string, text_length_,
                           time_text_format_,
                           remaining_second, white_brush_);

  D2D1_RECT_F mode = D2D1::Rect(0, 65, (int)width, 65 + 30);
  text_ = pomodoro_timer_.GetModeString();
  text_length_ = (UINT32)wcslen(text_);
  render_target_->DrawText(text_, text_length_, mode_text_format_, mode,
                           white_brush_);

  D2D1_RECT_F round_count = D2D1::Rect(0, 195, (int)width, 195 + 25);
  pomodoro_timer_.GetRoundString(remaining_minute_string);
  text_length_ = (UINT32)wcslen(remaining_minute_string);
  render_target_->DrawText(remaining_minute_string, text_length_,
                           round_text_format_, round_count,
                           white_brush_);

  delete[] remaining_minute_string;
  result = render_target_->EndDraw();

  if (result == D2DERR_RECREATE_TARGET) {
    result = S_OK;
    DiscardDeviceResources();
  }

  return result;
}

LRESULT CALLBACK PomodoroApp::WndProc(HWND hwnd, UINT message, WPARAM wParam,
                                      LPARAM lParam) {
  static UINT taskbar_restart_message;

  if (message == WM_CREATE) {
    LPCREATESTRUCT create_struct = (LPCREATESTRUCT)lParam;
    PomodoroApp* pomodoro_app = (PomodoroApp*)create_struct->lpCreateParams;

    ::SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(pomodoro_app));

    taskbar_restart_message = RegisterWindowMessage(TEXT("TaskbarCreated"));

    return 1;
  }

  PomodoroApp* pomodoro_app = reinterpret_cast<PomodoroApp*>(
      static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

  if (message == taskbar_restart_message)
    pomodoro_app->tray_icon_.AddTrayIcon(
        LoadIcon(pomodoro_app->hinstance_, MAKEINTRESOURCE(IDI_SHORT_BREAK)));

  switch (message) {
    case WM_PAINT: {
      pomodoro_app->Render();
      ValidateRect(hwnd, nullptr);
    } break;
    case WM_LBUTTONUP: {
      POINTS click_point = MAKEPOINTS(lParam);
      if (click_point.x >= 139 && click_point.x <= 139 + 32 &&
          click_point.y >= 238 && click_point.y <= 238 + 32) {
        pomodoro_app->pomodoro_timer_.TogglePause();
      }
    } break;
    case WM_DISPLAYCHANGE: {
      InvalidateRect(hwnd, nullptr, FALSE);
    }
    case WM_COMMAND: {
      switch (wParam) {
        case IDM_EXIT:
          pomodoro_app->tray_icon_.RemoveTrayIcon();
          DestroyWindow(hwnd);
          break;
      }
    } break;
    case WM_TIMER:
      if (wParam == IDT_POMODORO)
        return pomodoro_app->pomodoro_timer_.HandleTick();
      else
        DefWindowProc(hwnd, message, wParam, lParam);
      break;
    case WM_CLOSE:
      ShowWindow(hwnd, SW_HIDE);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case APPWM_NOTIFYICON:
      return pomodoro_app->tray_icon_.HandleEvent(wParam, lParam);
    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}
