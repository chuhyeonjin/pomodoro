#pragma once

#include "../resource/Resource.h"
#include "framework.h"

enum class TimerMode {
  Work,
  ShortBreak,
  LongBreak
};

class PomodoroTimer {
 public:
  PomodoroTimer(HWND hwnd);
  LRESULT CALLBACK HandleTick();
  void NextMode();
  D2D1_COLOR_F GetBackgroundColor();
  D2D1_COLOR_F GetPieChartBackgroundColor();
  const WCHAR* GetModeString();
  void GetRemainingMinuteString(WCHAR*& str);
  void GetRemainingSecondString(WCHAR*& str);
  void GetRoundString(WCHAR*& str);
  FLOAT GetRemainingTimePercent();
  bool IsPaused();
  void TogglePause();
  
 private:
  HWND hwnd_;
  TimerMode mode_;
  INT remaining_seconds_;
  UINT16 round_;
  bool is_paused_;

  void SetRemainingSecondByMode();
};