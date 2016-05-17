#pragma once
#include <windows.h>
#include <string>

class window {
public:
  window(HINSTANCE instance);

  void write(const std::string& str);

  void on_create();
  void on_destroy();
  void on_size(int cx, int cy);
  void on_command(UINT id);

private:
  LRESULT handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  HINSTANCE instance_;
  HWND hwnd_ = nullptr;
  HWND border_ = nullptr;
  HWND console_ = nullptr;
};
