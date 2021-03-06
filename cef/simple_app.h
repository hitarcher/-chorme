// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp, public CefBrowserProcessHandler {
 public:
	 SimpleApp(CefString default_url, HWND parent_hwnd, RECT parent_rect)
		 : m_default_url(default_url)
		 , m_parent_hwnd(parent_hwnd)
		 , m_parent_rect(parent_rect)
	 {};

  // CefApp methods:
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE {
    return this;
  }

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() OVERRIDE;
  CefRefPtr<CefClient> GetDefaultClient() OVERRIDE;

  // edit by mingl : OnBeforeCommandLineProcessing
  virtual void OnBeforeCommandLineProcessing(
	  const CefString& process_type,
	  CefRefPtr<CefCommandLine> command_line) OVERRIDE;

private:
	CefString m_default_url;
	HWND m_parent_hwnd;
	RECT m_parent_rect;

 private:
  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
