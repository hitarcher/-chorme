#pragma once
#include "stdafx.h"
#include "Heads.h"
#include "ProxyToH5.h"
#include "WeChatPrinter.h"	// �õ� theApp ,��������ͻᱨIDD�ȴ���
#include "WeChatPrinterDlg.h"// // �õ� maindlg
#include "mycef.h"
#include "myhttp.h"

 int g_ZipStatus = 0;												//�жϳ����˶����ز��Ƿ�ѹ������ 0 û��ѹ�� 1 ѹ���� 2 ѹ�����
 int g_nZipnum = 0;													//����ѹ������,���ø��ֶ�

httplib::Server server_httpproxy;

void ProxyStart_http()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "����HTTP����");

	// ����http����
	server_httpproxy.Post("/cpp", [&](const Request &req, Response &res, const ContentReader &content_reader) {
		std::string body;
		content_reader([&](const char *data, size_t data_length) {
			body.append(data, data_length);
			return true;
		});
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "���յ� HTTP֪ͨ body=%s", body.c_str());
		std::string replay;
		ProxyConsume_http(req.path, body, replay);//����H5����Ϣ
												  //res.set_content(replay.c_str(), "json");//���ظ�H5
	});
	//H5����ͨ����http://127.0.0.1:8866/public/*.* �ĸ�ʽ������static����������زġ�
	//������Ӿ���·���Ļ����ڵ���ģʽ�£����ܻ���ʲ����زģ���404
	server_httpproxy.set_mount_point("/public", get_fullpath("/data/GUI/www/static").c_str());
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "ProxyStart_http 1 ");
	std::thread thread_proxy_http([&]() {
		bool bRet = server_httpproxy.listen("0.0.0.0", 8866);//����H5����Ϣ
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "����HTTP����");
	});
	thread_proxy_http.detach();
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyStart_http", "ProxyStart_http end ");

}

void ProxyConsume_http(IN std::string path, IN std::string request, OUT std::string & replay)
{
	if (request == "")
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ProxyConsume_http", "request[����Ϊ��]");
			return;
		}
		json jrequeset = json::parse(easytoGBK(request).c_str());
	
		if (jrequeset["code"] == "zip")//������
		{
			return;
	
			g_ZipStatus = 1;
			//����ѹ������ļ�
			std::string strBase64 = jrequeset["image_transformate_base64"].get<std::string>();
			CString strTemp = jrequeset["image_path"].get<std::string>().c_str();
			int pos = strTemp.ReverseFind('/');
			CString strName = "www//static//";
			strName += strTemp.Right(strTemp.GetLength() - pos - 1);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "%s ��ѹ��", strName);
			//Base64ToPicture(strBase64.c_str(), GetFullPath(g_Config.m_strRelatePath + strName));
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "%s ѹ�����", strName);
	
			//g_nZipnum++;
	
			replay = "ziping";
		}
		else if (jrequeset["code"] == "zipok")
		{
			return;
			vector <std::string> vecImages = jrequeset["images"];
			//�жϴ����ͼƬ�����ͱ����Ѿ�ת���õ�ͼƬ���������Ƿ�һ�£�û�еĻ���ѭ��
			while (vecImages.size() != g_nZipnum)
			{
				if (g_ZipStatus != 1) break;
				Sleep(500);
			}
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "ѹ����ɣ���������");
	
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
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ProxyConsume_http", "H5�������");
		}
}
