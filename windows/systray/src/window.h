#pragma once
#include <windows.h>
#include <shellapi.h>

class window {
public:
  window(HINSTANCE instance);

  void on_create();
  void on_destroy();
  void on_command(UINT id);

  void on_tray(UINT id);
  void on_tray(UINT id, int x, int y);

private:
  LRESULT handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  HINSTANCE instance_;
  HWND hwnd_ = nullptr;

  NOTIFYICONDATA tray_ = {};
};
