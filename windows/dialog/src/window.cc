#include "window.h"
#include <resource.h>
#include <stdexcept>
#include <string>

window::window(HINSTANCE instance) : instance_(instance)
{
  // Create the main application window.
  auto hwnd = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_MAIN), nullptr, [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> INT_PTR {
    auto self = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (msg == WM_INITDIALOG) {
      self = reinterpret_cast<window*>(lparam);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else if (msg == WM_DESTROY) {
      SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    }
    return self ? self->handle(hwnd, msg, wparam, lparam) : FALSE;
  }, reinterpret_cast<LPARAM>(this));

  if (!hwnd) {
    MessageBox(nullptr, L"Could not create the main application window.", PROJECT, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    PostQuitMessage(1);
    return;
  }
}

void window::on_initdialog()
{
  // Set the window icon.
  auto icon = LoadIcon(instance_, MAKEINTRESOURCE(IDI_MAIN));
  SendMessage(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
  SendMessage(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));

  // Determine the window border sizes.
  RECT wrc = {};
  GetWindowRect(hwnd_, &wrc);

  RECT crc = {};
  GetClientRect(hwnd_, &crc);

  auto bx = (wrc.right - wrc.left) - (crc.right - crc.left);
  auto by = (wrc.bottom - wrc.top) - (crc.bottom - crc.top);

  // Resize the window.
  auto cx = 800 + bx;
  auto cy = 600 + by;

  SetWindowPos(hwnd_, nullptr, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOMOVE);

  // Move the window.
  if (auto monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONULL)) {
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(monitor, &mi)) {
      auto x = ((mi.rcWork.right - mi.rcWork.left) - cx) / 2;
      auto y = ((mi.rcWork.bottom - mi.rcWork.top) - cy) / 2;
      SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
    }
  }

  // Show the window.
  ShowWindow(hwnd_, SW_SHOW);
}

void window::on_destroy()
{
  // Stop the main message loop.
  PostQuitMessage(0);
}

void window::on_close()
{
  // Destroy the window.
  DestroyWindow(hwnd_);
}

void window::on_size(int cx, int cy)
{
  // Resize the controls.
}

void window::on_command(UINT id)
{
  // Handle windows commands.
  switch (id) {
  case IDM_EXIT:
    PostMessage(hwnd_, WM_CLOSE, 0, 0);
    break;
  }
}

INT_PTR window::handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  // Handle windows messages.
  try {
    switch (msg) {
    case WM_INITDIALOG:
      hwnd_ = hwnd;
      on_initdialog();
      return TRUE;
    case WM_DESTROY:
      on_destroy();
      hwnd_ = nullptr;
      return TRUE;
    case WM_CLOSE:
      on_close();
      return TRUE;
    case WM_SIZE:
      on_size(LOWORD(lparam), HIWORD(lparam));
      return 0;
    case WM_COMMAND:
      on_command(LOWORD(wparam));
      return TRUE;
    }
  }
  catch (const std::exception& e) {
    std::wstring msg;
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0) + 1);
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, &msg[0], static_cast<int>(msg.size())));
    MessageBox(hwnd, msg.c_str(), PROJECT, MB_OK | MB_ICONERROR);
    DestroyWindow(hwnd);
  }
  return FALSE;
}
