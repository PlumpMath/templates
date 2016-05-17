#include "window.h"
#include <resource.h>
#include <commctrl.h>
#include <richedit.h>
#include <algorithm>
#include <stdexcept>
#include <string>

#define MARGIN    5L  // border margin
#define PADDING   3L  // text padding

window::window(HINSTANCE instance) : instance_(instance)
{
  // Load the window icon.
  auto icon = LoadIcon(instance, MAKEINTRESOURCE(IDI_MAIN));

  // Register the main application window class.
  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
#ifdef _WIN64
    auto self = reinterpret_cast<window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (msg == WM_CREATE) {
      self = reinterpret_cast<window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else if (msg == WM_DESTROY) {
      SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
    }
#else
    auto self = reinterpret_cast<window*>(GetWindowLong(hwnd, GWL_USERDATA));
    if (msg == WM_CREATE) {
      self = reinterpret_cast<window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
      SetWindowLong(hwnd, GWL_USERDATA, reinterpret_cast<LONG>(self));
    } else if (msg == WM_DESTROY) {
      SetWindowLong(hwnd, GWL_USERDATA, 0);
    }
#endif
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
  auto cx = CW_USEDEFAULT;
  auto cy = CW_USEDEFAULT;

  if (!CreateWindowEx(es, PRODUCT, PROJECT, ws, CW_USEDEFAULT, CW_USEDEFAULT, cx, cy, nullptr, nullptr, instance, this)) {
    MessageBox(nullptr, L"Could not create the main application window.", PROJECT, MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    PostQuitMessage(1);
    return;
  }
}

void window::write(const std::string& str)
{
  // Write the string to the console control.
  if (console_) {
    std::wstring msg;
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0) + 1);
    msg.resize(MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &msg[0], static_cast<int>(msg.size())));
    CHARRANGE cr = { -1, -1 };
    SendMessage(console_, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&cr));
    SendMessage(console_, EM_REPLACESEL, 0, reinterpret_cast<LPARAM>(msg.data()));
    SendMessage(console_, WM_VSCROLL, SB_BOTTOM, 0);
  }
}

void window::on_create()
{
  // Center the window.
  if (auto monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST)) {
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(monitor, &mi)) {
      auto cx = std::min(800L, mi.rcWork.right - mi.rcWork.left);
      auto cy = std::min(600L, mi.rcWork.bottom - mi.rcWork.top);
      auto x = ((mi.rcWork.right - mi.rcWork.left) - cx) / 2;
      auto y = ((mi.rcWork.bottom - mi.rcWork.top) - cy) / 2;
      SetWindowPos(hwnd_, nullptr, x, y, cx, cy, SWP_NOACTIVATE);
    }
  }

  // Create the controls.
  auto ws = WS_CHILD | WS_VISIBLE;

  border_ = CreateWindow(WC_STATIC, nullptr, ws | SS_BLACKFRAME, 0, 0, 100, 100, hwnd_, nullptr, instance_, nullptr);
  if (!border_) {
    throw std::runtime_error("Could not create the border control.");
  }

  console_ = CreateWindow(L"RichEdit20W", nullptr, ws | WS_VSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 100, 100, hwnd_, nullptr, instance_, nullptr);
  if (!console_) {
    throw std::runtime_error("Could not create the richedit control.");
  }

  RECT rc = { PADDING, PADDING, 100 - PADDING * 2, 100 - PADDING * 2 };
  SendMessage(console_, EM_SETRECT, 1, reinterpret_cast<LPARAM>(&rc));

  auto hdc = GetDC(console_);
  auto size = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
  ReleaseDC(console_, hdc);
  auto font = CreateFontW(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, L"Lucida Console");

  SendMessage(console_, WM_SETFONT, reinterpret_cast<WPARAM>(font), 0);

  // Resize the controls.
  GetClientRect(hwnd_, &rc);
  on_size(rc.right - rc.left, rc.bottom - rc.top);

  // Show the window.
  ShowWindow(hwnd_, SW_SHOW);

  // Test the console.
  write("Hello World!");
}

void window::on_destroy()
{
  // Destroy the controls.
  DestroyWindow(console_);
  console_ = nullptr;

  DestroyWindow(border_);
  border_ = nullptr;

  // Stop the main message loop.
  PostQuitMessage(0);
}

void window::on_size(int cx, int cy)
{
  // Resize the controls.
  MoveWindow(border_, MARGIN, MARGIN, cx - MARGIN * 2, cy - MARGIN * 2, TRUE);
  MoveWindow(console_, MARGIN + 1, MARGIN + 1, cx - MARGIN * 2 - 2, cy - MARGIN * 2 - 2, TRUE);
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
