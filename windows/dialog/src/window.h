#pragma once
#include <windows.h>

class window {
public:
  window(HINSTANCE instance);

  void on_initdialog();
  void on_destroy();
  void on_close();
  void on_size(int cx, int cy);
  void on_command(UINT id);

private:
  INT_PTR handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  HINSTANCE instance_;
  HWND hwnd_ = nullptr;
};
