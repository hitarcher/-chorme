#pragma once
#include "stdafx.h"
#include "Heads.h"
#include "ProxyToH5.h"
#include "WeChatPrinter.h"	// 用到 theApp ,不加这个就会报IDD等错误
#include "WeChatPrinterDlg.h"// // 用到 maindlg
#include "mycef.h"
#include "myhttp.h"

 int g_ZipStatus = 0;												//判断程序运动后素材是否被压缩过了 0 没有压缩 1 压缩中 2 压缩完成
 int g_nZipnum = 0;													//单次压缩计数,弃用该字段

httplib::Server server_httpproxy;

void ProxyStart_http()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "创建HTTP服务");

	// 创建http代理
	server_httpproxy.Post("/cpp", [&](const Request &req, Response &res, const ContentReader &content_reader) {
		std::string body;
		content_reader([&](const char *data, size_t data_length) {
			body.append(data, data_length);
			return true;
		});
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "接收到 HTTP通知 body=%s", body.c_str());
		std::string replay;
		ProxyConsume_http(req.path, body, replay);//处理H5的信息
												  //res.set_content(replay.c_str(), "json");//返回给H5
	});
	//H5可以通过，http://127.0.0.1:8866/public/*.* 的格式访问我static里面的任意素材。
	//如果不加绝对路径的话，在调试模式下，可能会访问不到素材，报404
	server_httpproxy.set_mount_point("/public", get_fullpath("/data/GUI/www/static").c_str());
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "ProxyStart_http 1 ");
	std::thread thread_proxy_http([&]() {
		bool bRet = server_httpproxy.listen("0.0.0.0", 8866);//监听H5的消息
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "开启HTTP监听");
	});
	thread_proxy_http.detach();
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "ProxyStart_http end ");

}

void ProxyConsume_http(IN std::string path, IN std::string request, OUT std::string & replay)
{
	if (request == "")
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ProxyConsume_http", "request[内容为空]");
			return;
		}
		json jrequeset = json::parse(easytoGBK(request).c_str());
	
		if (jrequeset["code"] == "zip")//交易码
		{
			return;
	
			g_ZipStatus = 1;
			//保存压缩后的文件
			std::string strBase64 = jrequeset["image_transformate_base64"].get<std::string>();
			CString strTemp = jrequeset["image_path"].get<std::string>().c_str();
			int pos = strTemp.ReverseFind('/');
			CString strName = "www//static//";
			strName += strTemp.Right(strTemp.GetLength() - pos - 1);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "%s 待压缩", strName);
			//Base64ToPicture(strBase64.c_str(), GetFullPath(g_Config.m_strRelatePath + strName));
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "%s 压缩完成", strName);
	
			//g_nZipnum++;
	
			replay = "ziping";
		}
		else if (jrequeset["code"] == "zipok")
		{
			return;
			vector <std::string> vecImages = jrequeset["images"];
			//判断处理的图片数量和本地已经转换好的图片数量计数是否一致，没有的话就循环
			while (vecImages.size() != g_nZipnum)
			{
				if (g_ZipStatus != 1) break;
				Sleep(500);
			}
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "压缩完成，启动程序");
	
			//maindlg->SetTimer(TIMER_LOADPAGE, DELAY_TIME * 1000, NULL);
			g_nZipnum = 0;
			g_ZipStatus = 2;
	
			replay = "zipok";
		}
		else if (jrequeset["code"] == "H5OK")
		{
			//replay = "success";
	
			CString strInitInterface;
			strInitInterface.Format(_T("GetH5OK();"));
			//maindlg->cef_exec_js(strInitInterface.GetBuffer(0));
	
			//g_bH5IsReady = TRUE;
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "H5加载完毕");
		}
}
