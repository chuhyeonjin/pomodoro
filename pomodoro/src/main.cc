#include "pomodoro_app.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);

  PomodoroApp app(hInstance);

  if (!app.Initialize(lpCmdLine, nCmdShow)) {
    return FALSE;
  }

  return app.RunMessageLoop();
}