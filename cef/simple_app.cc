// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
#include "simple_app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_handler.h"

namespace {

// When using the Views framework this object provides the delegate
// implementation for the CefWindow that hosts the Views-based browser.
class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
    // Add the browser view and show the window.
    window->AddChildView(browser_view_);
    window->Show();

    // Give keyboard focus to the browser view.
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
    // Allow the window to close if the browser says it's OK.
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser)
      return browser->GetHost()->TryCloseBrowser();
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) OVERRIDE {
    return CefSize(800, 600);
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  SimpleBrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) OVERRIDE {
    // Create a new top-level Window for the popup. It will show itself after
    // creation.
    CefWindow::CreateTopLevelWindow(
        new SimpleWindowDelegate(popup_browser_view));

    // We created the Window.
    return true;
  }

 private:
  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(SimpleBrowserViewDelegate);
};

}  // namespace

void SimpleApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Create the browser using the Views framework if "--use-views" is specified
  // via the command-line. Otherwise, create the browser using the native
  // platform framework.
  const bool use_views = command_line->HasSwitch("use-views");

  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_views));

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  if (use_views) {
    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, m_default_url, browser_settings, nullptr, nullptr,
        new SimpleBrowserViewDelegate());

    // Create the Window. It will show itself after creation.
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
  } else {
    // Information used when creating the native window.
    CefWindowInfo window_info;
	window_info.SetAsChild(m_parent_hwnd, m_parent_rect);

	/* edit by mingl : 如果设置了SetAsPopup 那么就会弹出新窗口 */
	//window_info.SetAsPopup(m_parent_hwnd, "cefsimple");
	/*end by mingl*/
	
    // Create the first browser window.
    CefBrowserHost::CreateBrowser(window_info, handler, m_default_url, browser_settings,
                                  nullptr, nullptr);
  }
}

CefRefPtr<CefClient> SimpleApp::GetDefaultClient() {
  // Called when a new browser window is created via the Chrome runtime UI.
  return SimpleHandler::GetInstance();
}

void SimpleApp::OnBeforeCommandLineProcessing(
	const CefString& process_type,
	CefRefPtr<CefCommandLine> command_line)
{
	// eidt by mingl : 设置 在无交互模式下 开启视频自动播放
	command_line->AppendSwitchWithValue("--autoplay-policy", "no-user-gesture-required");
	// 设置单进程执行
	command_line->AppendSwitchWithValue("--single-process", "1");
	// 字体不以系统的大小而改变
	command_line->AppendSwitchWithValue("--force-device-scale-factor", "1");
	// 跨域
	command_line->AppendSwitchWithValue("--disable-web-security", "1");
	// 增加最大分辨率
	//command_line->AppendSwitchWithValue("max-untiled-layer-width", "20000");
	//command_line->AppendSwitchWithValue("max-untiled-layer-height", "20000");
	// 设置可用内存
	// 1024*100 kb = 100m
	command_line->AppendSwitchWithValue("--gpu-program-cache-size-kb", "51200");
	command_line->AppendSwitchWithValue("--mem-pressure-system-reserved-kb", "51200");
	command_line->AppendSwitchWithValue("--memory-pressure-thresholds", "1");
	command_line->AppendSwitchWithValue("--memory-pressure-thresholds-mb", "50");
	// crash报告
	command_line->AppendSwitchWithValue("--full-memory-crash-report", "1");
	//禁用硬件解码
	command_line->AppendSwitchWithValue("--disable-accelerated-video-decode", "1");
	//禁用GPU加速
	command_line->AppendSwitchWithValue("--disable-gpu", "1");

}


