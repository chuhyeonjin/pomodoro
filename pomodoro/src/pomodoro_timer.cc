#include "pomodoro_timer.h"

const UINT WORK_TIME = 60 * 30;
const UINT SHORT_BREAK_TIME = 60 * 5;
const UINT LONG_BREAK_TIME = 60 * 20;
const UINT LONG_BREAK_INTERVAL = 4;

PomodoroTimer::PomodoroTimer(HWND hwnd)
    : hwnd_(hwnd),
      mode_(TimerMode::Work),
      remaining_seconds_(WORK_TIME),
      round_(1),
      is_paused_(true) {}

LRESULT CALLBACK PomodoroTimer::HandleTick() {
  if (--remaining_seconds_ < 0) {
    NextMode();
  }
  RedrawWindow(hwnd_, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
  return 0;
}

void PomodoroTimer::NextMode() {
  switch (mode_) {
    case TimerMode::Work:
      mode_ = (round_ % LONG_BREAK_INTERVAL == 0 ? TimerMode::LongBreak : TimerMode::ShortBreak);
      break;
    case TimerMode::ShortBreak:
      mode_ = TimerMode::Work;
      round_++;
      break;
    case TimerMode::LongBreak:
      mode_ = TimerMode::Work;
      round_++;
      break;
  }
  SetRemainingSecondByMode();
}

void PomodoroTimer::SetRemainingSecondByMode() {
  switch (mode_) {
    case TimerMode::Work:
      remaining_seconds_ = WORK_TIME;
      break;
    case TimerMode::ShortBreak:
      remaining_seconds_ = SHORT_BREAK_TIME;
      break;
    case TimerMode::LongBreak:
      remaining_seconds_ = LONG_BREAK_TIME;
      break;
  }
}

D2D1_COLOR_F PomodoroTimer::GetBackgroundColor() {
  switch (mode_) {
    case TimerMode::Work:
      return D2D1::ColorF(0xE24B4B);
    case TimerMode::ShortBreak:
      return D2D1::ColorF(0x75D78A);
    case TimerMode::LongBreak:
      return D2D1::ColorF(0x2E9E47);
  }
}

D2D1_COLOR_F PomodoroTimer::GetPieChartBackgroundColor() {
  switch (mode_) {
    case TimerMode::Work:
      return D2D1::ColorF(0xD92323);
    case TimerMode::ShortBreak:
      return D2D1::ColorF(0x4DCB69);
    case TimerMode::LongBreak:
      return D2D1::ColorF(0x227735);
  }
}

const WCHAR* PomodoroTimer::GetModeString() {
  switch (mode_) {
    case TimerMode::Work:
      return L"Work";
    case TimerMode::ShortBreak:
      return L"Short Break";
    case TimerMode::LongBreak:
      return L"Long Break";
  }
}

void PomodoroTimer::GetRemainingMinuteString(WCHAR*& str) {
  UINT remaining_minute = remaining_seconds_ / 60;
  WCHAR result[3] = {L'0' + (remaining_minute / 10 > 9 ? 9 : remaining_minute / 10),
                L'0' + (remaining_minute / 10 > 9 ? 9 : remaining_minute % 10),
                L'\0'};
  memcpy(str, result, 3 * sizeof(WCHAR));
}

void PomodoroTimer::GetRemainingSecondString(WCHAR*& str) {
  WCHAR result[3] = {L'0' + (remaining_seconds_ % 60 / 10),
                L'0' + (remaining_seconds_ % 60 % 10),
                L'\0'};
  memcpy(str, result, 3 * sizeof(WCHAR));
}

void PomodoroTimer::GetRoundString(WCHAR*& str) {
  std::wstring result(L"Round ");
  result += std::to_wstring(round_);
  memcpy(str, result.c_str(), (result.size() + 1) * sizeof(WCHAR));
}

FLOAT PomodoroTimer::GetRemainingTimePercent() {
  UINT set_time;
  switch (mode_) {
    case TimerMode::Work:
      set_time = WORK_TIME;
      break;
    case TimerMode::ShortBreak:
      set_time = SHORT_BREAK_TIME;
      break;
    case TimerMode::LongBreak:
      set_time = LONG_BREAK_TIME;
      break;
  }
  return (FLOAT)remaining_seconds_ / (FLOAT)set_time;
}

bool PomodoroTimer::IsPaused() { return is_paused_; }

void PomodoroTimer::TogglePause() {
  if (is_paused_) {
    SetTimer(hwnd_, IDT_POMODORO, 1000, nullptr);
    is_paused_ = false;
    HandleTick();
  } else {
    KillTimer(hwnd_, IDT_POMODORO);
    is_paused_ = true;
    RedrawWindow(hwnd_, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
  }
}
