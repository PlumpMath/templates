#include "window.h"
#include <resource.h>
#include <stdexcept>
#include <string>

window::window(HINSTANCE instance) : instance_(instance)
{
  // Load the window icon.
  auto icon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN));

  // Register the main application window class.
  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
    auto self = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (msg == WM_CREATE) {
      self = reinterpret_cast<window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else if (msg == WM_DESTROY) {
      SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    }
    return self ? self->handle(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
  };
  wc.hInstance = instance;
  wc.hIcon = icon;
  wc.hIconSm = icon;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW);
  wc.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN);
  wc.lpszClassName = PRODUCT;

  if (!RegisterClassEx(&wc)) {
    MessageBox(nullptr, L"Could not register the main application window class.", PROJECT, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    PostQuitMessage(1);
    return;
  }

  // Create the main application window.
  auto es = 0x0L;
  auto ws = WS_OVERLAPPEDWINDOW;
  
  if (!CreateWindowEx(es, PRODUCT, PROJECT, ws, 0, 0, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, this)) {
    MessageBox(nullptr, L"Could not create the main application window.", PROJECT, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    PostQuitMessage(1);
    return;
  }
}

void window::on_create()
{
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

LRESULT window::handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  // Handle windows messages.
  try {
    switch (msg) {
    case WM_CREATE:
      hwnd_ = hwnd;
      on_create();
      return 0;
    case WM_DESTROY:
      on_destroy();
      hwnd_ = nullptr;
      return 0;
    case WM_SIZE:
      on_size(LOWORD(lparam), HIWORD(lparam));
      return 0;
    case WM_COMMAND:
      on_command(LOWORD(wparam));
      return 0;
    }
  }
  catch (const std::exception& e) {
    std::wstring msg;
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, nullptr, 0) + 1);
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, e.what(), -1, &msg[0], static_cast<int>(msg.size())));
    MessageBox(hwnd, msg.c_str(), PROJECT, MB_OK | MB_ICONERROR);
    DestroyWindow(hwnd);
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}