#pragma once

// 源码-github：https://github.com/chromiumembedded/cef
// 源码-bitbucket：https://bitbucket.org/chromiumembedded/cef/src/master/
// bin文件（不含mp4解析）：https://cef-builds.spotifycdn.com/index.html
// 编译方法：https://bitbucket.org/chromiumembedded/cef/wiki/MasterBuildQuickStart#markdown-header-overview
// 使用：https://dev.yunxin.163.com/docs/product/%E9%80%9A%E7%94%A8/Demo%E6%BA%90%E7%A0%81%E5%AF%BC%E8%AF%BB/PC%E9%80%9A%E7%94%A8/Demo%20CEF%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97
// 国内下载：https://blog.csdn.net/epubcn/article/details/82689929
// 标签集合：https://peter.sh/experiments/chromium-command-line-switches/

#pragma comment(lib, "libcef.lib")
#pragma comment(lib, "libcef_dll_wrapper_mt.lib")

#if defined(CEF_USE_SANDBOX)
#pragma comment(lib, "cef_sandbox.lib")

#endif
#include "simple_app.h"
#include "simple_handler.h"


int cef_init(CString m_strHtmlPath);
void cef_close();
void cef_load_url(IN std::string _utf_url, IN int delay = 0);
void cef_exec_js(IN std::string _utf_js, IN int delay = 0);
