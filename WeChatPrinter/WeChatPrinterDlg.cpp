#include "stdafx.h"
#include "WeChatPrinter.h"
#include "WeChatPrinterDlg.h"
#include "afxdialogex.h"
#include "myimagecompress.h"
#include "simple_handler.h"
#include "NetworkTips.h"
#include "MediaInfoDemo.h" //Dynamicly-loaded library (.dll or .so)

using namespace MediaInfoDLL;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif            

#define TIMER_RESIGN		1001									//ǩ��
#define TIMER_LOADPAGE		1002									//����H5����
#define TIMER_RECV_MSG		1003									//����RMQ��Ϣ
#define TIMER_LOADTEMPORARY 1004									//���ؽ����岥����
#define TIMER_CHOOSEPROGAME 1005									//ѡ���Ŀ
#define TIMER_CHECKINCOMPELEDFILE 1006								//���δ��ɵ��ز�
#define TIMER_CHECKMEMORY	1007									//����ڴ�
#define TIMER_OFFLINEREBOT  1008									//��������rmq��һ��ʱ��δ�յ�offline��Ϣ��������

#define BTN_ADMIN_LOGOUT	2000									
#define OFFLINETIME 31												//31������
#define templatezip "template.json"
#define defaultJson "template/default.json"
#define AutoUpdateMsg "�Զ�����\\Msg.ini"
#define AutoUpdateEXE "�Զ�����\\AutoUpdate.exe"

CWeChatPrinterDlg*g_CWeChatPrinterDlg = NULL;
static CString g_strRecvMsg = "";									//����Rmq����Ϣ
BOOL g_bIsTemproaryOn = FALSE;										//��ǰ����ʱ��Ŀ�Ĳ���ʱ�䡣
BOOL g_bDownload = TRUE;											//�ж��Ƿ���Ҫ����
CString g_strRecordCurrentPrograme = "";							//��¼ѡ���Ŀ�̵߳Ľ�Ŀ
CString g_strItemid = "";											//�ֶ��л��Ľ�ĿID
BOOL bUnderSwitchMode = FALSE;										//�����л�ģʽ����״̬���ȼ��ߣ����ڲ岥�����������·���
static int nCheckTimes = 0;											//���������λ�û���µ����Ͳ������ˡ�
int g_nZipStatus = 0;												//�жϳ����˶����ز��Ƿ�ѹ������ 0 û��ѹ�� 1 ѹ���� 2 ѹ�����
static int g_nZipnum = 0;											//����ѹ������,���ø��ֶ�
BOOL g_bH5IsReady = FALSE;
vector<CString>g_vecBigVideName;
vector<CString>g_vecBigVideContent;
typedef void(__stdcall *_CallBack_Recv)(char* bMsg);
typedef int(_stdcall *lpRMQ_CALLBACK)(_CallBack_Recv);
typedef int(_stdcall *lpRMQ_SUB)(const char *, int, const char *, const char *, const char *,
	const char *, const char *, int, int);
lpRMQ_SUB _RMQ_SUB;
lpRMQ_CALLBACK _RMQ_CALLBACK;
CString g_strStartTime = "";
CConfig	g_Config;

HHOOK hMouseHook;
LRESULT CALLBACK OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam);

static void CALLBACK _Recv(char* bMsg)
{
	g_strRecvMsg = bMsg;
	g_CWeChatPrinterDlg->SetTimer(TIMER_RECV_MSG, 10, NULL);
}


CWeChatPrinterDlg::CWeChatPrinterDlg(CWnd* pParent /*=NULL*/)
	: CImageDlg(CWeChatPrinterDlg::IDD, pParent)
{
	::CoInitialize(NULL);
	m_strLastErr = "";
	m_bExit = FALSE;
	g_CWeChatPrinterDlg = this;
	m_nMode = 0;
	m_strHtmlPath = "";

	jrsp = "";
	jTemplate = "";
	jTemproary = "";
	jOldrsp = "";
	jDefault = "";
	jOverdue = "";
	jForIE = "";

	m_iNextBitSpace = 1;
	m_strLastSignTime = "";
	m_OnlineTime = "";

	m_rc = CRect(0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	m_strAdminEnter = "";

	m_hReSign = NULL;
	m_hReSignEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hRMQ = NULL;
	m_hRMQEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hChooseProgram = NULL;
	m_hChooseProgramEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hDviceStatus = NULL;
	m_hDviceStatusEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWeChatPrinterDlg::DoDataExchange(CDataExchange* pDX)
{
	CImageDlg::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWeChatPrinterDlg, CImageDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()
// CWeChatPrinterDlg ��Ϣ�������
BOOL CWeChatPrinterDlg::OnInitDialog()
{
	CImageDlg::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	/************************************************************************/
	/*                        �� �� �� ��                                   */
	/************************************************************************/
	SET_LOGTYPE((LOG_TYPE)(LOGTYPE_DEBUG | LOGTYPE_ERROR | LOGTYPE_SPECIAL));
	LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "==============START==============", "");
	//ɾ������30�����־
	LOG_CLEAR(30);
	//cef_log.log ���ܻ��쳣�������ÿ���������鲢ɾ��
	g_strStartTime = GetCurTime(DAY_NORMAL).c_str();
	ULONGLONG size;
	CString strCEFLOGPath = get_fullpath("cef_catch//cef_log.log").c_str();
	CFileStatus fileStatus;
	if (CFile::GetStatus(strCEFLOGPath, fileStatus))
	{
		size = fileStatus.m_size / 1024 / 1024;
		if (size > 100)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "cef_log.log size = %lld MB", size);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "ɾ��cef_log.log");
			DeleteFile(strCEFLOGPath);
		}
	}
	// ����ȫ�ֹ��ӣ��������������棬���ⱻ�����ؼ��ڵ�
#ifdef STARTHOOK
	hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, OnMouseEvent, theApp.m_hInstance, 0);
#endif
	// ���ڸ����Զ����³��򣬱������һЩ�쳣�汾�޷�����
	CString strUpdateFile = GetFullPath("AutoUpdate.exe");
	if (CheckFileExist(strUpdateFile))
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "��⵽���� %s", strUpdateFile);
		Sleep(2000);
		int iProcessID = FindProcess("AutoUpdate.exe");
		while (iProcessID > 0)
		{
			KillProcess(iProcessID);
			iProcessID = FindProcess("AutoUpdate.exe");
			LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "��⵽AutoUpdate.exe���ڣ�ǿ�ƹر� %s", strUpdateFile);
			Sleep(500);
		}
		bool b1, b2, b3, b4;
		b1 = mv(strUpdateFile, GetFullPath(AutoUpdateEXE));
		b2 = mv(GetFullPath("SoftAuthorization.dll"), GetFullPath("�Զ�����\\SoftAuthorization.dll"));
		b3 = mv(GetFullPath("2.jpg"), GetFullPath("�Զ�����\\2.jpg"));
		b4 = mv(GetFullPath("������Ȩ.exe"), GetFullPath("�Զ�����\\������Ȩ.exe"));
		if (FALSE == b1)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "AutoUpdate����ʧ�ܣ�%s,%s,%d,%d,%d", strUpdateFile, GetFullPath(AutoUpdateEXE),b2,b3,b4);
			goto EXIT;
		}
	}
#ifdef CHECKUPDATE
	//�Ƿ������Զ����¼��
	if (FALSE == CheckUpdate())
	{
		goto EXIT;
	}
#endif

	//���ػ�������
	if (FALSE == g_Config.LoadBaseCfg())
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_CEF, "OnInitDialog", "[g_Config.LoadBaseCfg()][%s]", g_Config.GetLastErr());
		goto EXIT;
	}
	//���ü���ҳ��ĵ�ַ
	m_strHtmlPath = "file:///" + GetFullPath(g_Config.m_strRelatePath + "index.html");
	m_strHtmlPath.Replace("\\", "/");
	ConvertGBKToUtf8(m_strHtmlPath);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_CEF, "OnInitDialog", "���ز����ɹ�");
	//cef3��ʼ�����˴������������̣��Ӵ����ϵ���־���ظ����֡�
	if (FALSE == cef_init(m_strHtmlPath))
	{
		goto EXIT;
	}

	/***********************************************************************************************************/
	
	//���ض�̬��
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnInitDialog", "׼����ʼ���ض�̬��");
	if (FALSE == LoadRMQPubAndRMQSUBDLL())
	{
		goto EXIT;
	}

	DWORD dwThreadId = 0;
	m_hReSign = CreateThread(NULL, 0, ReSignThreadProc, this, 0, &dwThreadId);
	m_hRMQ = CreateThread(NULL, 0, RMQThreadProc, this, 0, &dwThreadId);
	m_hDviceStatus = CreateThread(NULL, 0, DeviceStatusThreadProc, this, 0, &dwThreadId);
	m_hChooseProgram = CreateThread(NULL, 0, ChooseProgramThreadProc, this, 0, &dwThreadId);
	if (m_hReSign && m_hRMQ && m_hDviceStatus && m_hChooseProgram)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnInitDialog", "�̴߳����ɹ�");
	}
	else
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "OnInitDialog", "�̴߳���ʧ��");
		goto EXIT;
	}

	SetWindowText(INFO_PUBLISH_SCREEN_NAME);
#ifdef TESTMODE
	ShowCursor(TRUE);
#else
	ShowCursor(FALSE);
#endif 
//���ܻᵼ�³����޷�����
// 	if (g_Config.m_bTopMost)
// 	{
// 		//::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//��ǰ��
// 		SetForegroundWindow();
// 	}
//ProxyStart_http();
	
	NetWorkTipsInit();

	//ѡ����صĽ�Ŀjson����������ѹ������ѹ��ѡ������岥��Ŀ��������ͨ��Ŀ
	if (FALSE == ChooseJson())
	{
		goto EXIT;
	}
	//ѡ���Ŀ
	SetTimer(TIMER_CHOOSEPROGAME,10, NULL);
	//ǩ��
	SetTimer(TIMER_RESIGN, 100, NULL);
	//����Ƿ������ʱ�ļ�
	SetTimer(TIMER_CHECKINCOMPELEDFILE, 3000, NULL);
	//����ڴ�������
	SetTimer(TIMER_CHECKMEMORY, 5000, NULL);
	//��30��ɾ��һ��û�й������ز�
	DelateResource();
	//��ʾ��Ŀ�ﳬ��1k�ֱ��ʵ���Ƶ�ز�-1min
	RemindTooBigVideoName();

	//system("HideTraywnd.cmd");


	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnInitDialog", "�����ʼ���ɹ�");
	return TRUE;
EXIT:
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnInitDialog", "�����쳣�˳�");
	EndDialog(FALSE);
	return FALSE;
}

void CWeChatPrinterDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CImageDlg::OnPaint();
	}
}

HCURSOR CWeChatPrinterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
//����
void CWeChatPrinterDlg::OnDestroy()
{
	m_bExit = TRUE;

	SetEvent(m_hReSignEvent);
	WaitForSingleObject(m_hReSign, 3000);
	CloseHandle(m_hReSign);
	m_hReSign = NULL;

	SetEvent(m_hDviceStatusEvent);
	WaitForSingleObject(m_hDviceStatus, 3000);
	CloseHandle(m_hDviceStatus);
	m_hDviceStatus = NULL;

	SetEvent(m_hRMQEvent);
	WaitForSingleObject(m_hRMQ, 3000);
	CloseHandle(m_hRMQ);
	m_hRMQ = NULL;

	SetEvent(m_hChooseProgramEvent);
	WaitForSingleObject(m_hChooseProgram, 3000);
	CloseHandle(m_hChooseProgram);
	m_hChooseProgram = NULL;

	//cef_close();
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "==============DESTROY==============", "");
	CImageDlg::OnDestroy();
}


void CWeChatPrinterDlg::NetWorkTipsInit()
{
	// ����״̬
	std::thread th_access_http([&]() {
		network_tips();
	});
	th_access_http.detach();
}

void CWeChatPrinterDlg::network_tips()
{
	int posss = g_Config.m_strHttpUrl.Find("release-web");
	CString strHost = g_Config.m_strHttpDwonloadUrl.Left(posss - 1);

	CNetworkTips tips;
	tips.set_checking_httpurl(strHost.GetBuffer(0));
	tips.set_position(2);
	tips.DoModal();
}
 
BOOL CWeChatPrinterDlg::ZipImg()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ZipImg", "��ʼѹ���ز�");
	LOG2(LOGTYPE_DEBUG, LOG_NAME_ZIP, "ZipImg", "��ʼѹ���ز�");

	if (g_nZipStatus == 2) return TRUE;

	bool b1 = is_64bitsystem();
	float b2 = GetMemory();
	vector<CString>vecTemp;
	vector<CString>vecImg;
	unsigned int nSumImg = 0;
	unsigned int nSumFailed = 0;
	//����64λϵͳ�����ڴ�С��4G�Ļ�����ѹ��ͼƬ
#ifdef ZIPIMGANYWAY
	if (1)
#else
	if (FALSE == b1 || b2 < 4)
#endif	
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ZipImg", "ѹ�������Ժ�");

		FindContent(jTemplate, vecTemp);
		for (unsigned int i = 0; i < vecTemp.size(); i++)
		{
			if ((vecTemp[i].Find("jpg")>=0) || (vecTemp[i].Find("png") >= 0))
			{
				CString strTemp = vecTemp[i];
				vecImg.push_back(strTemp);
			}
		}
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ZipImg", "����%d��ͼƬ��ѹ�������Ժ�", vecImg.size());

		for (unsigned int i = 0; i < vecImg.size(); i++)
		{
			CString strBackName ="www/static/backImg";
			//���ͼƬѹ��ʧ�ܣ���һֱ����ѹ�����������־����ʾ������������Ῠ��nothingshow������
			LOG2(LOGTYPE_DEBUG, LOG_NAME_ZIP, "ZipImg", "%s ��ʼѹ��", vecImg[i]);
			Status  nret = compress_image(get_fullpath((g_Config.m_strRelatePath + vecImg[i]).GetBuffer(0)), get_fullpath((g_Config.m_strRelatePath + strBackName).GetBuffer(0)));
			if (Ok != nret)
			{
				nSumFailed++;
				LOG2(LOGTYPE_DEBUG, LOG_NAME_ZIP, "ZipImg", "%s ѹ������,������ %d\n\n\n", vecImg[i], nret);
			}else 
				LOG2(LOGTYPE_DEBUG, LOG_NAME_ZIP, "ZipImg", "%s ѹ�����\n", vecImg[i], nret);
		}
		nSumImg = vecImg.size();
		g_nZipStatus = 2;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ZipImg", "ѹ����� ,ϵͳΪ%dλ,�ڴ��СΪ%4.2fGB,�ɹ�����%d�ţ�ʧ������%d�š�",b1 ? 64 : 32,b2, nSumImg- nSumFailed, nSumFailed);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_ZIP, "ZipImg", "ѹ�����,�ɹ�����%d�ţ�ʧ������%d�š�\n\n", nSumImg - nSumFailed, nSumFailed);

	return TRUE;
}

void CWeChatPrinterDlg::LoadTemplate()
{
	int i = 0;
// 	while (FALSE == g_bH5IsReady)
// 	{
// 		Sleep(1000);
// 		i++;
// 		if (i == 30)
// 		{
// 			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadTemplate", "�ۼ�%d��δ�յ�HTTP֪ͨ��ǿ�Ƽ���ģ��", i);
// 			break;
// 		}
// 	}
	std::string strTemp = jForIE.dump();
	if (strTemp == "")
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadTemplate", "jForIE[����Ϊ��]");
		return ;
	}
	CString strContent = strTemp.c_str();
	ConvertGBKToUtf8(strContent);
	CString strInitInterface;
	//����һ��ת�壬������� ���ӵ�xml��ʽ������ʧ��
	strInitInterface.Format(_T("loadPage('%s');"), easyEscape(strContent.GetBuffer(0)).c_str());
	cef_exec_js(strInitInterface.GetBuffer(0));

#ifdef DETAIlEDLOG
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadTemplate", "����ģ��ɹ�,����ģ��Ϊ\r\n%s\r\n", strContent);
#else
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadTemplate", "����ģ��ɹ�");
#endif

}

/****************************		��Ŀ��غ���		*****************************************/

//���template/1.json������json����,����ļ������ڣ����ܻ�����һ���յ��ļ�
json LoadjsonFile(CString strJsonName)
{
	CFile file;
	CFileStatus status;
	CString strConfigFile = GetFullPath(g_Config.m_strRelatePath + strJsonName);
	json jTemp = "";
	if (FALSE == CheckFileExist(strConfigFile))
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][%s]�ļ�������", strConfigFile);
		return jTemp;
	}
	//1 ���ж��ļ��Ƿ����
	if (file.Open(strConfigFile, CFile::modeReadWrite) == FALSE &&
		file.Open(strConfigFile, CFile::modeCreate | CFile::modeReadWrite) == FALSE)
	{
		CString strLastErr = "";
		strLastErr.Format("�ļ���ʧ�ܣ�����Ȩ�޲�������%s��", strConfigFile);
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][%s]", strLastErr);
		return jTemp;
	}
	file.GetStatus(status);
	//2 ���ж��ļ��Ƿ�Ϊ��
	if (status.m_size == 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[LoadjsonFile][�ļ�Ϊ��]");
		return jTemp;
	}
	else
	{
		char* pTemplate = (char*)calloc((int)status.m_size + 1, sizeof(char));
		file.Read(pTemplate, (int)status.m_size);
		CString strJson = pTemplate;
		if (strJson == "")
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadjsonFile", "[%s][����Ϊ��]", strJsonName);
			return jTemp;
		}
		ConvertUtf8ToGBK(strJson);
		jTemp = json::parse(strJson.GetBuffer(0));
		strJson.ReleaseBuffer();
		free(pTemplate);
		pTemplate = NULL;
	}
	file.Close();
	return jTemp;
}

//�ж����޲����ؽ����岥��Ŀ
BOOL CWeChatPrinterDlg::LoadTemporayJson()
{
	CString strTargetFile = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	CString strErrMsg = "";
	if (CheckFileExist(strTargetFile))
	{
		jTemproary = LoadjsonFile(g_Config.m_strTemporaryJson);
		if (jTemproary == "")
		{
			strErrMsg = "jTemproary ����Ϊ��";
			goto EXIT;
		}
		if (jTemproary.find("body") == jTemproary.end())
		{
			strErrMsg = "��Ŀ��ʽ����ȷ";
			goto EXIT;
		}
		json jBody = jTemproary["body"];
		if (jBody.find("datalist") != jBody.end())
		{
			strErrMsg = "��Ŀ��ʽ����ȷ���ǵ���Ŀ";
			goto EXIT;
		}

		CString strMode = jTemproary["body"]["data"]["releasemode"].get<std::string>().c_str();
		if (atoi(strMode) != TEMPORARY_JSON)
		{
			strErrMsg = "����ģʽ������";
			goto EXIT;
		}
		CString strEndDate = jTemproary["body"]["data"]["itemenddate"].get<std::string>().c_str();
		if (CheckTimeLimit(strEndDate))
		{
			strErrMsg = "�岥��Ŀ��ʱ��ɾ����ʱ��Ŀ" + strEndDate;
			goto EXIT;
		}
		return TRUE;
	}
	return FALSE;
EXIT:
	DeleteFile(strTargetFile);
	LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadTemporayJson", strErrMsg);
	return FALSE;
}

//�ж��������߽�Ŀѹ����
BOOL CWeChatPrinterDlg::LoadOfflinePacket()
{
	//1 �ҵ�ָ��·���µ�ָ������ZIP��
	CString strZipName1 = g_Config.m_strOrgCode + "_ITEM.zip";
	CString strZipName2 = g_Config.m_strOrgCode + "_ITEMLIST.zip";
	CString strTargetFile = "";
	int nProgameType = 0;
	if (CheckFileExist(GetFullPath(g_Config.m_strRelatePath + strZipName1)))
	{
		strTargetFile = GetFullPath(g_Config.m_strRelatePath + strZipName1);
		nProgameType = SINGLEPROGRAM;
	}
	else if (CheckFileExist(GetFullPath(g_Config.m_strRelatePath + strZipName2)))
	{
		strTargetFile = GetFullPath(g_Config.m_strRelatePath + strZipName2);
		nProgameType = MULITIPLEPROGRAM;
	}
	else
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "����û�����߽�Ŀѹ����");
		return FALSE;
	}

	CString strResourcePath = "OfflineResource\\";
	CString strResourceFullPath = GetFullPath(g_Config.m_strRelatePath + strResourcePath);
	CString strNewPath = GetFullPath(g_Config.m_strRelatePath + "www\\static\\");
	// ��ʼ��ѹ
	RN_RESULT nResult = _unzip(strTargetFile, strResourceFullPath);

	if (nResult)// ��ѹʧ��
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "��ѹ���߽�Ŀ��ʧ�ܣ���������룺%d��", nResult);
		return FALSE;
	}
	else// ��ѹ�ɹ�
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadOfflinePacket", "��ʼ��ѹ���߽�Ŀѹ����");

		json jOffline = LoadjsonFile(strResourcePath + templatezip);
		if (jOffline == "")
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", templatezip, "����Ϊ��");
			return FALSE;
		}
		json jHead = jOffline["head"];
		if (jHead.find("organcode") == jHead.end() || jHead.find("devicecode") == jHead.end())
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "��ȡ[organcode]��[devicecode]�ֶ�ʧ��");
			return FALSE;
		}
		CString strOrgancode = jHead["organcode"].get<std::string>().c_str();
		CString strDevicecode = jHead["devicecode"].get<std::string>().c_str();
		if (strOrgancode.CompareNoCase(g_Config.m_strOrgCode) || strDevicecode.Find(g_Config.m_strDeviceCode) < 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadOfflineJson", "�ֶ�ֵ���� Organcode = [%s],Devicecode = [%s] \n��ǰ Organcode = [%s],Devicecode = [%s]"
				, strOrgancode, strDevicecode, g_Config.m_strOrgCode, g_Config.m_strDeviceCode);
			return FALSE;
		}
		CString strOFFLineJson = jOffline.dump(4).c_str();
		if (strOFFLineJson.Find("upload") >= 0)
		{
			g_bDownload = FALSE;
			ParseNewTemplateOfH5(strOFFLineJson, nProgameType == SINGLEPROGRAM ? TRUE : FALSE);
			g_bDownload = TRUE;

			//DeleteFile(strResourceFullPath + "template.json");
			//�ƶ�ѹ��·�������ļ��� NewPath
			RemoveFileToOtherPath(strResourceFullPath, strNewPath, "*");
			// ɾ�������ļ�
			DeleteFile(strTargetFile);
			//ɾ�ļ��к�����
			RemoveDir(strResourceFullPath);
		}
	}
	return TRUE;
}

//ѡ����صĽ�Ŀ����������ѹ������ѹ��ѡ������岥��Ŀ��������ͨ��Ŀ
BOOL CWeChatPrinterDlg::ChooseJson()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseJson", "��ʼѡ���Ŀ");

	jDefault = LoadjsonFile(defaultJson);
	if ("" == jDefault)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "jDefault ����Ϊ��");
		return FALSE;
	}
	jTemplate = LoadjsonFile(g_Config.m_strTempalteJson);
	jOverdue = LoadjsonFile(g_Config.m_strOldTemplate);
	if ("" == jTemplate)
	{
		if ("" != jOverdue)
		{
			//û��template.json �Ͳ���templateold.json
			ChangeCurrentJson(jOverdue);
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "jTemplate ����Ϊ��,�޸�Ϊ�ɽ�Ŀ");
		}
		else
		{	
			//û��templateold.json �Ͳ���default.json
			ChangeCurrentJson(jDefault);
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "jOverdue ����Ϊ��,�޸�ΪĬ�Ͻ�Ŀ");
		}
	}
	else
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "�޸�ΪjTemplate");
		ChangeCurrentJson(jTemplate);
	}

	//�ж��������߽�Ŀѹ����
	LoadOfflinePacket();
	//�ж����޽����岥��Ŀ���Ƿ�Ϲ棬û�оͼ���ԭ��Ŀ
	if (LoadTemporayJson())
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ChooseJson", "�޸�ΪjTemproary�������岥");
		ChangeCurrentJson(jTemproary);
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseJson", "���ؽ�Ŀ�ɹ�");
	return TRUE;
}

//������Ŀ
BOOL CWeChatPrinterDlg::ParseNewTemplateOfH5(CString strJson, BOOL bIsSingleProgram,/* BOOL bIsOffline,*/ BOOL bParseOnly)
{
	// ����json
	if (strJson == "")
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ParseNewTemplateOfH5", "strJson[����Ϊ��]");
		return FALSE;
	}
	json jALL = json::parse(strJson.GetBuffer(0));
	strJson.ReleaseBuffer();
	json j = jALL["body"]["data"];
	//����ģʽ����Ĭ�Ͻ�Ŀ�ͽ����岥����
	int iReleseMode = NEW_JSON;
	if (bIsSingleProgram)
	{
		iReleseMode = atoi(j["releasemode"].get<std::string>().c_str());
	}
	else
	{
		/*From ƽ̨��������Ķ��Ŀ��û��ʱ��(timeseg)�ֶΣ��Ͳ�Ҫ�������أ���ʡ��Դ
		����Ŀ�������ȫ�죬ûʱ��Ҳ���ԡ����Ŀû��ȫ��ģʽ��ֻ�����ظ����������
		2020.6.15 carouselmode Ҳ��û�е�*/
		j = jALL["body"]["datalist"];
		json jTemp;
		for (json::iterator it = j.begin(); it != j.end(); ++it)
		{
			json jTemp2 = *it;
			if (jTemp2.find("carouselmode") != jTemp2.end())
			{
				jTemp.push_back(jTemp2);
			}
		}
		j = jTemp;
	}

	for (unsigned int i = 0; bIsSingleProgram || i < j.size(); i++)
	{
		json jPage;
		//�ж��Ƿ��ǵ�ģ��
		if (bIsSingleProgram)
			jPage = j["itemtemplatejson"]["page"];
		else
			jPage = j[i]["itemtemplatejson"]["page"];
		if (jPage.is_array() == FALSE)
		{
			m_strLastErr = "����template json[page]��ʽ����ȷ";
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ParseNewTemplateOfH5", "[err][%s]", m_strLastErr);
			return FALSE;
		}
		json jArray_edit = json::array();//����һ��������
		json jo = json::object();		 //����һ���ն���
		for (json::iterator it = jPage.begin(); it != jPage.end(); jArray_edit.push_back(jo), ++it)
		{
			jo = *it;
			json jArray_edit2 = json::array();//����һ��������
			json jo2 = json::object();		  //����һ���ն���
			json jElements = jo["elements"];
			for (json::iterator it2 = jElements.begin(); it2 != jElements.end(); jArray_edit2.push_back(jo2), ++it2)
			{
				// arrayתobject
				jo2 = *it2;
				if (jo2.is_object() == false) continue;
				std::string strFilePath = "www/static/";

				//-----------------//
				//����group�������//
				//-----------------//
				json jArray_edit3 = json::array();//����һ��������
				json jo3 = json::object();		  //����һ���ն���
				json jGroup = jo2["group"];
				for (json::iterator it3 = jGroup.begin(); it3 != jGroup.end(); jArray_edit3.push_back(jo3), ++it3)
				{
					jo3 = *it3;
					//-------------------//
					//����bgImg�������//
					//-------------------//
					std::string strUrl = jo3["bgImg"].get<std::string>();
					if ("" != strUrl)
					{
						std::string strFilePath = "template/";
						// http���ز�����·��
						CString strLocalPath = "";
						strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
						if (bParseOnly == TRUE) continue;
						DownLoadFile(strUrl.c_str(), strLocalPath);
						jo3["bgImg"] = strLocalPath;
					}
					//�ų����� img��vidoe��carousel ���content��Դ���أ�����btn��content������һ������
					std::string strType = jo3["type"].get<std::string>();
					if (strType == "img" || strType == "video" || strType == "carousel")
					{
						json::iterator posGroupContent = jo3.find("content");
						json jGroupUrls(posGroupContent.value());
						std::string strUrl = *jGroupUrls.begin();
						if ("" == strUrl)  continue;
						// http���ز�����·��
						CString strLocalPath = "";
						strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
						jGroupUrls = strLocalPath.GetBuffer(0);
						strLocalPath.ReleaseBuffer();
						if (bParseOnly == TRUE) continue;
						DownLoadFile(strUrl.c_str(), strLocalPath);
						jo3["content"] = jGroupUrls;
					}
				}
				jo2["group"] = jArray_edit3;

				//--------------------//
				//����resource�������//
				//--------------------//
				json jArray_edit4 = json::array();//����һ��������
				json jo4 = json::object();		  //����һ���ն���
				json jResource = jo2["resource"];
				for (json::iterator it3 = jResource.begin(); it3 != jResource.end(); jArray_edit4.push_back(jo4), ++it3)
				{
					jo4 = *it3;
					json::iterator posGroupContent = jo4.find("content");
					json jResource(posGroupContent.value());
					std::string strUrl = *jResource.begin();
					if ("" == strUrl)  continue;
					// http���ز�����·��
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
					jResource = strLocalPath.GetBuffer(0);
					strLocalPath.ReleaseBuffer();
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo4["content"] = jResource;
				}
				jo2["resource"] = jArray_edit4;

				//-------------------//
				//����content�������//
				//-------------------//
				//�ų����� img��vidoe��carousel ���content��Դ���أ�����btn��content������һ������
				std::string strType = jo2["type"].get<std::string>();
				if (strType == "img" || strType == "video" || strType == "carousel")
				{
					json::iterator posContent = jo2.find("content");
					if (posContent == jo2.end() || posContent.value().is_string() == false) continue;
					json jUrls(posContent.value());
					std::string strUrl = *jUrls.begin();
					if ("" == strUrl)  continue;
					// http���ز�����·��
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));//ȡ ���һ��б�ܺ��������
					jUrls = strLocalPath.GetBuffer(0);
					strLocalPath.ReleaseBuffer();
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo2["content"] = jUrls;
				}

				//-------------------//
				//����bgImg�������//
				//-------------------//
				std::string strUrl = jo2["bgImg"].get<std::string>();
				if ("" != strUrl)
				{
					std::string strFilePath = "template/";
					// http���ز�����·��
					CString strLocalPath = "";
					strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
					if (bParseOnly == TRUE) continue;
					DownLoadFile(strUrl.c_str(), strLocalPath);
					jo2["bgImg"] = strLocalPath;
				}
			}
			//-----------------//
			//����bgImg�������//
			//-----------------//
			std::string strUrl = jo["bgImg"].get<std::string>();
			if ("" != strUrl)
			{
				std::string strFilePath = "template/";
				// http���ز�����·��
				CString strLocalPath = "";
				strLocalPath.Format("%s%s", strFilePath.c_str(), GetFileName(strUrl.c_str()));
				if (bParseOnly == TRUE) continue;
				DownLoadFile(strUrl.c_str(), strLocalPath);
				jo["bgImg"] = strLocalPath;
			}
			jo["elements"] = jArray_edit2;
		}

		jPage = jArray_edit;
		if (bIsSingleProgram)
		{
			j["itemtemplatejson"]["page"] = jPage;
			break;
		}
		else
		{
			j[i]["itemtemplatejson"]["page"] = jPage;
		}
		if (bParseOnly == TRUE) return TRUE;
	}
	//�滻jALL�������
	if (bIsSingleProgram)
		jALL["body"]["data"] = j;
	else
		jALL["body"]["datalist"] = j;

	//���浽��json���ƣ�template����temporary��Ĭ�ϻ��߽����岥
	std::string strPath = g_Config.m_strTempalteJson;

	//�����岥��������ļ���Ҳ��һ������ԭ�ļ�����ͻ�����Ǳ�ͨ�� CheckOldResource ������ԭ�ļ�����
	if (TEMPORARY_JSON == iReleseMode)
	{
		CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
		if (jTemproary != "")
		{
			vector<json>vecJson;
			//����ӾɵĽ����岥��Ŀ
			vecJson.push_back(jTemproary);
			vecJson.push_back(jTemplate);
			vecJson.push_back(jOverdue);
			vecJson.push_back(jDefault);
			vecJson.push_back(jALL);
			if (CheckFileExist(strOldTemplatePath))
			{
				CheckOldResource(vecJson);
			}
		}
		strPath = g_Config.m_strTemporaryJson;
		g_bIsTemproaryOn = TRUE;//���·���������������ȼ�������ж��߳�������
		jTemproary = jALL;
	}
	else
	{
		//�����·�
		CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strOldTemplate);
		if (jOverdue != "")
		{
			vector<json>vecJson;
			//�������json
			vecJson.push_back(jOverdue);
			vecJson.push_back(jTemplate);
			vecJson.push_back(jTemproary);
			vecJson.push_back(jDefault);
			vecJson.push_back(jALL);
			if (CheckFileExist(strOldTemplatePath))
			{
				CheckOldResource(vecJson);
			}
		}
		//�ж�template.json �Ƿ���ڣ����ڵĻ��Ϳ���ɾ��old.json ,��������ԭtemplate.json Ϊold
		CString strTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTempalteJson);
		if (CheckFileExist(strTemplatePath))
		{
			DeleteFile(strOldTemplatePath);
			rename(strTemplatePath, strOldTemplatePath);
		}
		jOverdue = jTemplate;
		jTemplate = jALL;
	}

	ChangeCurrentJson(jALL);
	std::string strNewJson = jALL.dump(4);
	CFile filenew(GetFullPath(g_Config.m_strRelatePath + strPath.c_str()), CFile::modeCreate | CFile::modeWrite);
	CString strUtf = strNewJson.c_str();
	ConvertGBKToUtf8(strUtf);
	filenew.Write(strUtf, strUtf.GetLength());
	filenew.Close();
	return TRUE;
}

//���json���Ƿ�����Դ���json�ظ���2��3��4��5 json��1�Ƚϣ�1json���е��ز���2��3��4��5û�еĻ���ɾ����Щ�ز�
BOOL  CWeChatPrinterDlg::CheckOldResource(vector<json>vecJson)
{
	//������json����ز�ȫ��ͳ�Ƴ�����Ȼ��;ɵĽ��бȽϣ�
	//�Ƚ���ģ���û���ظ����زĽ���ɾ������ɾ����json,����ǰjson�ĳɾ�json���
	vector<vector<CString>>vecResource;
	vector<CString>vecLeft;
	vector<CString>vecOld;
	if (vecJson[0] == "")
	{
		return FALSE;
	}
	for (unsigned int i = 0; i < vecJson.size(); i++)
	{
		json jTemp = vecJson[i];
		if (jTemp == "") continue;
		vector<CString>vecTemp;
		FindContent(jTemp, vecTemp);
		vecResource.push_back(vecTemp);
	}
	vecOld = vecResource[0];
	for (unsigned int i = 0; i < vecOld.size(); i++)
	{
		BOOL bFind = FALSE;
		for (unsigned int m = 1; m < vecResource.size(); m++)
		{
			vector<CString>vecTemp2 = vecResource[m];
			for (unsigned int n = 0; n < vecTemp2.size(); n++)
			{
				if (vecOld[i].CompareNoCase(vecTemp2[n]) == 0)
				{
					bFind = TRUE;
					break;
				}
			}
			if (bFind)break;
		}
		if (bFind)continue;
		vecLeft.push_back(vecOld[i]);
	}
	for (unsigned int i = 0; i < vecLeft.size(); i++)
	{
		CString strTemp = g_Config.m_strRelatePath + vecLeft[i];
		DeleteFile(GetFullPath(strTemp));
	}
	return TRUE;
}

//��Ȿ���Ƿ���û����������ļ�
BOOL CWeChatPrinterDlg::CheckIncompleted()
{
	CString strPath = GetFullPath(g_Config.m_strRelatePath + "www\\static\\*.download");
	//���ұ�����û��download�ļ����о�ƴ�ֶΣ�ȥ����rmqde���������ӿڡ�
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strPath);
	BOOL bFind = FALSE;
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		CString strname = finder.GetFileName();
		bFind = TRUE;
		break;
	}	if (bFind)
	{
		CString strOrder = "PUBLISH|" + g_Config.m_strDeviceCode + "|";
		json jTemp = GetCurrentJson();
		CString strItemID = jTemp["body"]["data"]["itemid"].get<std::string>().c_str();
		CString strType = "ITEM";
		json jBody = jTemp["body"];
		//if (jBody.find("datalist") != jBody.end())
		if (jBody["datalist"].is_null() == FALSE)
		{
			strType = "ITEMLIST";
		}
		RMQ_DealCustomMsg(strOrder + strItemID + "|" + strType);
		return FALSE;
	}
	nCheckTimes = 0;
	return TRUE;
}

BOOL CWeChatPrinterDlg::RemindTooBigVideoName()
{
	vector<CString>vecTemp;
	vector<CString>vecTemp2;
	vector<CString>vecNeedRemind;

	FindContent(jTemplate, vecTemp);
	// ��MP4�ļ�
	for (int i = 0; i < vecTemp.size(); i++)
	{
		if (vecTemp[i].Find("mp4") >=0 )
		{
			vecTemp2.push_back(vecTemp[i]);
		}
	}
	// ��MP4�ļ�������û�в����ϳߴ���ļ����еĻ�����Ƶ��������������
	for (int i = 0; i < vecTemp2.size(); i++)
	{
		MediaInfo MI;
		std::string strTemp = g_Config.m_strRelatePath + vecTemp2[i];
		MI.Open(GetFullPath(strTemp.c_str()).GetBuffer(0));

		CString width, height;
		width = MI.Get(stream_t::Stream_Video, 0, "Width").c_str();
		height = MI.Get(stream_t::Stream_Video, 0, "Height").c_str();

		int no_menu_bar_width = GetSystemMetrics(SM_CXSCREEN);
		int no_menu_bar__height = GetSystemMetrics(SM_CYSCREEN);

		if (atoi(width) >no_menu_bar_width || atoi(height) >no_menu_bar__height )
		{
			for (int j = 0; j < g_vecBigVideContent.size(); j++)
			{
				if (vecTemp2[i] == g_vecBigVideContent[j])
				{
					vecNeedRemind.push_back(g_vecBigVideName[j]);
					break;
				}
			}
		}
		//CString all = MI.Inform().c_str();//���������Ϣ
		MI.Close();
	}
	if (vecNeedRemind.size() == 0) return FALSE;

	CString strRemindCotent = "��Ŀ�д��ڳߴ�������Ƶ������ϵ���пƼ���ȥ�زĸ���,��Ƶ�ڽ�Ŀ�����ƣ�\r\n";
	for (int i = 0; i < vecNeedRemind.size(); i++)
	{
		strRemindCotent += vecNeedRemind[i] + "\r\n";
	}

	MessageBoxTimeout(m_hWnd, strRemindCotent,"������Ƶ�ز���ʾ",  MB_OKCANCEL, 0, 60*1000);
}
/****************************		���ܺ���		*****************************************/

//�ų����������ظ���Ԫ�أ���Ϊ""��ֵ
template<typename T>
void deduplication(T& c)
{
	//�����Ƚ�vector����
	sort(c.begin(), c.end());
	//��Ȼ��ʹ��unique�㷨,unique����ֵ���ظ�Ԫ�صĿ�ʼλ�á�
	T::iterator new_end = unique(c.begin(), c.end());//"ɾ��"���ڵ��ظ�Ԫ��
	//�����ɾ��������Ƕ��ظ�����
	c.erase(new_end, c.end());//ɾ��(������ɾ��)�ظ���Ԫ��

	for (unsigned int i = 0; i < c.size(); i++)
	{
		if (c[i] == "")
		{
			c.erase(c.begin() + i);
			break;
		}
	}
}

BOOL  FindContent(json jALL, vector<CString> &vecTemp)
{
	if (jALL == "")
	{
		return FALSE;
	}
	json j = jALL["body"]["data"];
	json jBody = jALL["body"];
	BOOL bIsSingleProgram = FALSE;
	if (j.find("itemtemplatejson") != j.end() && jBody.find("datalist") == jBody.end())
	{
		bIsSingleProgram = TRUE;
	}
	else
		j = jALL["body"]["datalist"];

	for (unsigned int i = 0; bIsSingleProgram || i < j.size(); i++)
	{
		json jPage;
		//�ж��Ƿ��ǵ�ģ��
		if (bIsSingleProgram)
			jPage = j["itemtemplatejson"]["page"];
		else
			jPage = j[i]["itemtemplatejson"]["page"];
		if (jPage.is_array() == FALSE)
		{
			CString strLastErr = "����template json[page]��ʽ����ȷ";
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "FindContent", "[err][%s]", strLastErr);
			return FALSE;
		}

		json jArray_edit = json::array();//����һ��������
		json jo = json::object();		 //����һ���ն���
		for (json::iterator it = jPage.begin(); it != jPage.end(); jArray_edit.push_back(jo), ++it)
		{
			jo = *it;
			json jArray_edit2 = json::array();//����һ��������
			json jo2 = json::object();		 //����һ���ն���
			json jElements = jo["elements"];
			for (json::iterator it2 = jElements.begin(); it2 != jElements.end(); jArray_edit2.push_back(jo2), ++it2)
			{
				// arrayתobject
				jo2 = *it2;
				if (jo2.is_object() == false) continue;
				std::string strFilePath = "www/static/";

				//-----------------//
				//group�������//
				//-----------------//
				json jArray_edit3 = json::array();//����һ��������
				json jo3 = json::object();		  //����һ���ն���
				json jGroup = jo2["group"];
				for (json::iterator it3 = jGroup.begin(); it3 != jGroup.end(); jArray_edit3.push_back(jo3), ++it3)
				{
					jo3 = *it3;
					//-----------------//
					//bgImg�������//
					//-----------------//
					std::string strUrl = jo3["bgImg"].get<std::string>();
					vecTemp.push_back(strUrl.c_str());

					//�ų����� img��vidoe��carousel ���content��Դ���أ�����btn��content������һ������
					std::string strType = jo3["type"].get<std::string>();
					if (strType == "img" || strType == "video" || strType == "carousel")
					{
						json::iterator posGroupContent = jo3.find("content");
						json jGroupUrls(posGroupContent.value());
						std::string strUrl = *jGroupUrls.begin();
						if ("" == strUrl)  continue;
						vecTemp.push_back(strUrl.c_str());
						if (strType == "video")
						{
							json::iterator posName = jo3.find("name");
							json jUrls(posName.value());
							std::string strName = *jUrls.begin();
							g_vecBigVideName.push_back(strName.c_str());
							g_vecBigVideContent.push_back(strUrl.c_str());
						}
					}
				}
				//--------------------//
				//resource�������//
				//--------------------//
				json jArray_edit4 = json::array();//����һ��������
				json jo4 = json::object();		  //����һ���ն���
				json jResource = jo2["resource"];
				for (json::iterator it3 = jResource.begin(); it3 != jResource.end(); jArray_edit4.push_back(jo4), ++it3)
				{
					jo4 = *it3;
					json::iterator posGroupContent = jo4.find("content");
					json jResource(posGroupContent.value());
					std::string strUrl = *jResource.begin();
					if ("" == strUrl)  continue;
					vecTemp.push_back(strUrl.c_str());

				}
				//-------------------//
				//content�������//
				//-------------------//
				//�ų����� img��vidoe��carousel ���content��Դ���أ�����btn��content������һ������
				std::string strType = jo2["type"].get<std::string>();
				if (strType == "img" || strType == "video" || strType == "carousel")
				{
					json::iterator posContent = jo2.find("content");
					if (posContent == jo2.end() || posContent.value().is_string() == false) continue;
					json jUrls(posContent.value());
					std::string strUrl = *jUrls.begin();
					if ("" == strUrl)  continue;
					vecTemp.push_back(strUrl.c_str());
					if (strType == "video")
					{
						json::iterator posName = jo2.find("name");
						json jUrls(posName.value());
						std::string strName = *jUrls.begin();
						g_vecBigVideName.push_back(strName.c_str());
						g_vecBigVideContent.push_back(strUrl.c_str());
					}
				}
				//-----------------//
				//bgImg�������//
				//-----------------//
				std::string strUrl = jo2["bgImg"].get<std::string>();
				vecTemp.push_back(strUrl.c_str());

			}
			//-----------------//
			//bgImg�������//
			//-----------------//
			std::string strUrl = jo["bgImg"].get<std::string>();
			vecTemp.push_back(strUrl.c_str());
		}
		if (bIsSingleProgram)
		{
			break;
		}
	}

	/*�ų���Ϊ�գ��ظ���ѡ��
		int ary[] = { 1, 1, 2, 3, 2, 4, 3 };
		vector<int> vec(ary, ary + sizeof(ary) / sizeof(int));
		deduplication(vec);
	*/
	deduplication(vecTemp);
	return TRUE;
}

void DownLoadFile(CString strUrl, CString strLocalPath)
{
#ifndef DOWNLOAD_NO
	if (FALSE == g_bDownload)
	{
		return;
	}

	CString strTargetFile = GetFullPath(g_Config.m_strRelatePath + strLocalPath);
	if (FALSE == CheckFileExist(strTargetFile))
	{
		//int nDownload = HTTP_Download(g_Config.m_strHttpDwonloadUrl + strUrl, strTargetFile);
		int nDownload = HTTP_Download2(g_Config.m_strHttpDwonloadUrl + strUrl, strTargetFile,"admin","123456","");
		if (nDownload != TRUE)
		{
			CString strLastErr = "";
			strLastErr.Format("����ʧ��[%s]", g_Config.m_strHttpDwonloadUrl + strUrl);
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "DownLoadFile", "[HTTP_Download()][err][%d][%s]",
				nDownload, strLastErr);
		}
		else
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "DownLoadFile", "[HTTP_Download()][ok][%s]",
				g_Config.m_strHttpDwonloadUrl + strUrl);
		}
	}
#endif
}

BOOL CheckTimeLimit(CString strEndDate)
{
	CString strResult = "";
	CString strCurDate = GetCurTime(DAY_LONG).c_str();
	if (strCurDate > strEndDate)
	{
		CString strYear = strEndDate.Mid(0, 4);
		CString strMonth = strEndDate.Mid(5, 2);
		CString strDay = strEndDate.Mid(8, 2);
		strResult.Format("��Ŀ�ѹ��ڣ�����ʱ����%s��%s��%s��", strYear, strMonth, strDay);
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "CheckTimeLimit", "%s", strResult);
		return TRUE;
	}
	return FALSE;
}

vector<int> getHours(CString strHours)
{
	//08:00-09:00|11:00-12:00
	vector<int>vecTime;
	vector<CString>vecHours = SplitString(strHours, "|");
	CString strTemp = "";
	for (unsigned int i = 0; i < vecHours.size(); i++)
	{
		vector<CString>vecHM = SplitString(vecHours[i], "-");
		if (vecHM.size() % 2 != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "getHours", "���ڸ�ʽ����[%s]", strHours);
			return vecTime;
		}
		int iBeginTime = atoi(vecHM[0].Mid(0, 2) + vecHM[0].Mid(3, 2) + "00");
		int iLastTime = atoi(vecHM[1].Mid(0, 2) + vecHM[1].Mid(3, 2) + "00");
		vecTime.push_back(iBeginTime);
		vecTime.push_back(iLastTime);
	}
	return vecTime;
}

void getDays(CString strDays, vector<int> &vecDay, vector<int> &vecHM)
{
	vector<CString>vecDate = SplitString(strDays, "|");

	CString strTemp = "";
	CString strHM = "";
	for (unsigned int i = 0; i < vecDate.size(); i++)
	{
		if (vecDate[i].Find(":") < 0)
		{
			CString strTemp2 = vecDate[i];
			strTemp2.Replace("-", "");
			vecDay.push_back(atoi(strTemp2));
		}
		else
			strTemp += vecDate[i] + "|";
	}
	strHM = strTemp.Left(strTemp.GetLength() - 1);
	vecHM = getHours(strHM);

}

int GetWaitTime(vector<int>vecHM, int iCurrentTime, int iPlayMode)
{
	int isize = vecHM.size();
	int iRemainingTime = 0;
	int * pTime = new int[isize + 1];
	for (unsigned int i = 0; i < vecHM.size(); i++)
		pTime[i] = vecHM[i];
	pTime[isize] = iCurrentTime;
	qsort(pTime, isize + 1, sizeof(int), cmpfunc);//900��1020��1000��1100��1200����

	int iNum = 0;
	for (iNum = 0; iNum < isize; iNum++)
	{
		if (pTime[iNum] == iCurrentTime)//�ڷ�Χ�ڵ��Ѿ����ų��ˡ�����Ŀ�����ҵ�1100
		{
			break;
		}
	}

	if (iNum == isize)//����ĩλ����1300����Ҫתһ���ˡ�
	{
		//�������Ϊ���ظ����Զ���ģʽ���ں������ж�Ϊĩβ��Ҫתһ���������Ͳ�ִ�е���ʱ
		if (iPlayMode > PLAY_REPEATDAY)
		{
			iRemainingTime = -1;
		}
		else
			iRemainingTime = 24 * 60 * 60 - (iCurrentTime / 10000 - pTime[0] / 10000) * 60 * 60
			- (iCurrentTime / 100 % 100 - pTime[0] / 100 % 100) * 60
			- (iCurrentTime % 100 % 100 - pTime[0] % 100 % 100);
	}
	else
	{
		// 		int a = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60;
		// 		int b = (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60;
		// 		int c= (pTime[iNum + 1] % 100%100 );
		// 		int d = iCurrentTime % 100 % 100;//154000 ֱ����10000�Ļ����õ�����4000�������1w��ʣ��ǧλ
		iRemainingTime = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60
			+ (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60
			+ (pTime[iNum + 1] % 100 % 100 - iCurrentTime % 100 % 100);
	}

	delete[]pTime;
	pTime = NULL;

	return iRemainingTime;
}

int GetCloseTime(vector<int>vecHM, int iCurrentTime)
{
	int isize = vecHM.size();
	int * pTime = new int[isize + 1];
	for (unsigned int i = 0; i < vecHM.size(); i++)
		pTime[i] = vecHM[i];
	pTime[isize] = iCurrentTime;
	qsort(pTime, isize + 1, sizeof(int), cmpfunc);//900��1020��1000��1100��1200����

	int iNum = 0;
	for (iNum = 0; iNum < isize; iNum++)
	{
		if (pTime[iNum] == iCurrentTime)//�ڷ�Χ�ڵ��Ѿ����ų��ˡ�����Ŀ�����ҵ�1100
		{
			break;
		}
	}

	int	iCloseRemainingTime = (pTime[iNum + 1] / 10000 - iCurrentTime / 10000) * 60 * 60
		+ (pTime[iNum + 1] / 100 % 100 - iCurrentTime / 100 % 100) * 60
		+ (pTime[iNum + 1] % 100 % 100 - iCurrentTime % 100 % 100);

	delete[]pTime;
	pTime = NULL;

	return iCloseRemainingTime;
}

BOOL CWeChatPrinterDlg::ExtendDeadline(CString strProgammeID, CString strNewDeadline, vector<CString> vecTime)
{
	json jALL = GetCurrentJson();
	if (jALL == "")
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "��ǰ��ĿΪ�գ��쳣");
		return FALSE;
	}
	if (jALL == jDefault)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ExtendDeadline", "��ǰ��Ĭ�Ͻ�Ŀ���޷��ӳ�ʱ��");
		return FALSE;
	}
	json jData = jALL["body"]["data"];
	if (jData.find("itemenddate") != jData.end())
	{
		CString strItemID = jData["itemid"].get<std::string>().c_str();
		if (strItemID.CompareNoCase(strProgammeID) != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "ָ��Ľ�ĿID���ĿID����");
			return FALSE;
		}
		jData["itemenddate"] = strNewDeadline;
		int  nPlaymode = atoi(jData["playmode"].get<std::string>().c_str());
		if (nPlaymode != PLAY_ALLDAY)
		{
			CString strTimeseg = "";
			for (unsigned int i = 0; i < vecTime.size(); i++)
			{
				strTimeseg += vecTime[i];
				if (vecTime.begin() + i + 1 == vecTime.end())
				{
					break;
				}
				strTimeseg += "|";
			}
			if (strTimeseg != "")
				jData["timeseg"] = strTimeseg;
		}
	}
	else if (jData.find("itemlistendtime") != jData.end())
	{
		CString strItemListID = jData["itemlistid"].get<std::string>().c_str();
		if (strItemListID.CompareNoCase(strProgammeID) != 0)
		{
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ExtendDeadline", "ָ��Ľ�Ŀ��ID���Ŀ��ID����");
			return FALSE;
		}
		jData["itemlistendtime"] = strNewDeadline;
	}
	jALL["body"]["data"] = jData;

	//���ݵ�ǰ�·��Ľ�Ŀ����һ�����޸ĳ����е�ʱ�䣬�Լ����ص�json
	CString strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTempalteJson);

	int iReleseMode = 0;
	if (jData.find("releasemode") != jData.end())
	{
		iReleseMode = atoi(jData["releasemode"].get<std::string>().c_str());
	}

	if (iReleseMode == TEMPORARY_JSON)
	{
		jTemproary = jALL;
		strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	}
	else
	{
		jTemplate = jALL;
	}
	//�޸ĵ�ǰ���Ž�Ŀ
	ChangeCurrentJson(jALL);
	std::string strNewJson = jALL.dump(4);
	CFile filenew(strJsonPath, CFile::modeCreate | CFile::modeWrite);
	CString strUtf = strNewJson.c_str();
	ConvertGBKToUtf8(strUtf);
	filenew.Write(strUtf, strUtf.GetLength());
	filenew.Close();
	return TRUE;
}

// ��Ļ��ͼ,����ͼƬ��·��
CString CWeChatPrinterDlg::ScreenShot(void)
{
	CWnd *pDesktop = GetDesktopWindow();
	CDC *pDC = pDesktop->GetDC();
	CRect rect;

	//��ȡ���ڵĴ�С  
	pDesktop->GetClientRect(&rect);

	//���浽���ļ���
	CString strFileName(GetAppPathW().c_str());
	strFileName += _T("ScreenShot\\");
	CreateDirectory((LPCTSTR)strFileName, NULL);
	CTime t = CTime::GetCurrentTime();
	CString tt = t.Format("%Y%m%d_%H%M%S");
	strFileName += tt;
	strFileName += _T(".png");
	//����ΪPNG
	CMakePNG MakePNG;
	BOOL bret = MakePNG.MakePNG(pDC->m_hDC, rect, strFileName);
	if (FALSE == bret)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ScreenShot", " \n��ͼʧ��-[%s]\n", MakePNG.getErrorMsg());
	}
	ReleaseDC(pDC);
	return strFileName;
}

//����Զ����³����Ƿ�����
BOOL CWeChatPrinterDlg::CheckUpdate()
{
	//�����Զ����³����Ƿ�������û�еĻ��ͽ��Լ��ر��������Լ���
	CString strMsgPath = GetFullPath(AutoUpdateMsg);
	if (FALSE == CheckFileExist(strMsgPath))
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_CEF, "CheckUpdate", " [%s]δ�ҵ�", strMsgPath);
		CString strPath = GetFullPath(AutoUpdateEXE);
		//StartProcess2(strPath, FALSE);//2020.12.29 ʧЧ�ˣ�������,ԭ��·�����пո�
		StartProcess3(strPath);
		return FALSE;
	}
	BOOL bPassed = GetPrivateProfileInt("Time", "Passed", 0, strMsgPath);//����Ƿ���Ȩ
	BOOL bUpdated = GetPrivateProfileInt("Time", "Updated", 0, strMsgPath);//����Ƿ���¹���
	int iProcessID = FindProcess("AutoUpdate.exe");
	if (FALSE == bPassed || FALSE == bUpdated /*|| iProcessID <= 0*/)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_CEF, "CheckUpdate", "%d %d %d", bPassed, bUpdated, iProcessID);

		//����3.0.1.0 �޸�Ϊ����̣�������������룬�ᵼ�³��򷴸��ظ���������Ϊ���������������¡� 2022 
// 		if (iProcessID <= 0)//һ�������Զ������������һ�δ�رյ������������EXE��
// 		{
// 			CString strPath = GetFullPath(AutoUpdateEXE);
// 			StartProcess3(strPath);
// 		}
// 		return FALSE;
	}
	return TRUE;
}

//ɾ���ɵ��ز�
void CWeChatPrinterDlg::DelateResource()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "DelateResource", "��ʼɾ�����ز�");

	CString strOldTemplatePath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
	if (jTemproary != "")
	{
		vector<json>vecJson;
		//����ӾɵĽ����岥��Ŀ
		vecJson.push_back(jTemproary);
		vecJson.push_back(jTemplate);
		vecJson.push_back(jOverdue);
		vecJson.push_back(jDefault);
		if (CheckFileExist(strOldTemplatePath))
		{
			CheckOldResource(vecJson);
		}
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "DelateResource", "ɾ�����ز����");
}

//˯�ߺ���1������ʱ��������
void CWeChatPrinterDlg::sleepFunction1(int Time)//α��̬��sleep�����ڳ�ʱ��˯��ȴ�޷������˳����߳�
{
	int iTime = Time / 1000;
	//�˳�������ʱ�䵽��˳���������
	while (--iTime && FALSE == m_bExit)
	{
		Sleep(1000);
	}
}

/****************************	RMQ		*****************************************/
BOOL CWeChatPrinterDlg::LoadRMQPubAndRMQSUBDLL()
{
	HINSTANCE hRMQDll = LoadLibrary("RMQ.dll");
	_RMQ_SUB = (lpRMQ_SUB)GetProcAddress(hRMQDll, "_RMQ_SUB");
	_RMQ_CALLBACK = (lpRMQ_CALLBACK)GetProcAddress(hRMQDll, "_RMQ_CALLBACK");
	if (NULL == hRMQDll)
	{
		m_strLastErr = "RMQ_SUB��̬�����ʧ��...";
		goto EXIT;
	}
	if (NULL == _RMQ_CALLBACK)
	{
		m_strLastErr = "_RMQ_CALLBACK����ʧ��...";
		goto EXIT;
	}
	if (NULL == _RMQ_SUB)
	{
		m_strLastErr = "_RMQ_SUB����ʧ��...";
		goto EXIT;
	}
	_RMQ_CALLBACK(_Recv);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "LoadRMQPubAndRMQSUBDLL", "��̬����سɹ�");
	return TRUE;
EXIT:
	LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "LoadRMQPubAndRMQSUBDLL", "��̬�����ʧ��:[%s]", m_strLastErr);
	return FALSE;
}
////////////////////////////////////////////////////////////////////
BOOL CWeChatPrinterDlg::RMQ_SUBConnect()
{
	if (g_Config.m_strDeviceCode.GetLength() <= 0 || g_Config.m_strIPAddress.GetLength() <= 0 || g_Config.m_nPort < 0
		|| g_Config.m_strAccount.GetLength() <= 0 || g_Config.m_strPassword.GetLength() <= 0 || g_Config.m_nModeName < 0
		|| g_Config.m_strExchangeName.GetLength() <= 0 || g_Config.m_vecRouteKey.size() <= 0 || g_Config.m_nChannel < 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQSUBConnect", "�����ļ�������д�쳣...");
		return FALSE;
	}
	CString strRouteKey = "";
	for (unsigned int i = 0; i < g_Config.m_vecRouteKey.size(); i++)
	{
		if ("" != g_Config.m_vecRouteKey[i])
			strRouteKey += ("" == strRouteKey ? "" : "|") + g_Config.m_vecRouteKey[i];
	}

	int nState = _RMQ_SUB(g_Config.m_strIPAddress, g_Config.m_nPort, g_Config.m_strAccount,
		g_Config.m_strPassword, g_Config.m_strExchangeName, g_Config.m_strDeviceCode,
		strRouteKey, g_Config.m_nModeName, g_Config.m_nChannel);

	if (nState != 0)
	{
		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQSUBConnect", "����RabbitMQƽ̨ʧ��...");
		return FALSE;
	}
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQSUBConnect", "����RabbitMQƽ̨�ɹ�...");

	return TRUE;
}
////////////////////////////////////////////////////////////////////
BOOL CWeChatPrinterDlg::RMQ_DealCustomMsg(CString strMsg)
{
	if (strMsg == "") return FALSE;
	std::vector<CString> vct = SplitString(strMsg.GetBuffer(), "|");
	strMsg.ReleaseBuffer();
	if (vct[0].CompareNoCase("OFFLINE") != 0)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_RMQ, "RMQ_DealCustomMsg", "[%s]\n", strMsg);
	}
	else
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "RMQ_DealCustomMsg", "[%s]\n����������������������������������������������������������������������������", strMsg);
	}
	if (vct.size() >= 1)
	{
		if (vct[0].CompareNoCase("PUBLISH") == 0 && vct.size() == 4)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�յ����½�Ŀָ��");
			g_nZipStatus = 0;
			CString strJson = "";
			m_nMode = 0;
			if (vct[3] == "ITEMLIST") m_nMode = MULITIPLEPROGRAM;
			else if (vct[3] == "ITEM") m_nMode = SINGLEPROGRAM;
			else return FALSE;
			BOOL bRet = g_toolTrade.GetTemplate(g_Config.m_strHttpUrl, strJson, m_nMode);
			if (bRet)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "GetTemplate ��ȡ��ĿJSON�ɹ�");
				bRet = g_toolTrade.UpLoadProGrameStatus(g_Config.m_strHttpUrl, m_nMode, 2);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus �ϴ��������ؽ�Ŀ״̬�ɹ�");
				}
				else
				{
					LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus �ϴ��������ؽ�Ŀ״̬ʧ��[%s]", g_toolTrade.GetLastErr());
				}
				bRet = ParseNewTemplateOfH5(strJson, m_nMode == SINGLEPROGRAM ? TRUE : FALSE);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "ParseNewTemplateOfH5 �������½�Ŀ�ɹ�");
					g_strItemid = "";
					SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
				}
				else
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "ParseNewTemplateOfH5 �������½�Ŀʧ��");
				}
				//�ϴ��Ƿ�����ģ��ɹ���Ϣ
				bRet = g_toolTrade.UpLoadProGrameStatus(g_Config.m_strHttpUrl, m_nMode, bRet == TRUE ? 3 : 4);
				if (bRet)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus �ϴ����½�Ŀ��Ϣ�ɹ�");
				}
				else
				{
					LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadProGrameStatus �ϴ����½�Ŀ��Ϣʧ��[%s]", g_toolTrade.GetLastErr());
				}
				SetTimer(TIMER_CHECKINCOMPELEDFILE, 1000, NULL);
			}
			else
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "GetTemplate ��ȡ���½�Ŀʧ�� [%s]", g_toolTrade.GetLastErr());
			}
		}
		else if (vct[0].CompareNoCase("DEFAULT") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "ȡ������������Ĭ�Ͻ�Ŀ");
			//��������岥״̬�Ժ�������������Ӱ��
			g_bIsTemproaryOn = FALSE;
			CString strJsonPath = GetFullPath(g_Config.m_strRelatePath + g_Config.m_strTemporaryJson);
			//��������岥�����ȼ��Ժ�����ͨ��Ŀ���Ӱ�죬ֱ��ɾ��
			DeleteFile(strJsonPath);
			ChangeCurrentJson(jDefault);
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
		}
		else if (vct[0].CompareNoCase("OFFLINE") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "RMQ_DealCustomMsg", "�յ������豸״̬��Ϣ");
			SetEvent(m_hDviceStatusEvent);
			// 			if (FALSE == g_bIsOnLinePro)
			// 			{
			// 				SwitchJsonToOnline();
			// 			}
			// 			SetTimer(TIMER_LOADOFFLINE, OFFLINETIMELIMTE * 1000, NULL);
		}
		else if (vct[0].CompareNoCase("DEVICE") == 0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�յ����³���֪ͨ");
			CString strPath = GetFullPath(AutoUpdateEXE);
			StartProcess3(strPath);
			//StartProcess(strPath, "", 0, 0);
			EndDialog(FALSE);
		}
		else if (vct[0].CompareNoCase("EXCHANGETIME") == 0 && vct.size() >= 3)
		{
			vector<CString> vecTime;
			vecTime.assign(vct.begin() + 3, vct.end());
			if (ExtendDeadline(vct[1], vct[2], vecTime))
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�����µĽ�ֹʱ�䵽%s", vct[2]);
				SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			}
		}
		else if (vct[0].CompareNoCase("DEVICECONTROLL") == 0)//�豸����
		{
			if (vct[1].CompareNoCase(g_Config.m_strDeviceCode) == 0)
			{
				if (vct[2].CompareNoCase("TURNOFF") == 0)//�ػ�
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�豸�����ػ�");
					ShutDown();
				}
				else if (vct[2].CompareNoCase("TURNBACK") == 0)//����
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�豸��������");
					Reboot();
				}
				else if (vct[2].CompareNoCase("VOLUMECONTRO") == 0)//��������
				{
					CString strVolumn = vct[3];
					strVolumn.Replace("%", "");
					int nVolumn = atoi(strVolumn);
					if (nVolumn >= 0 && nVolumn <= 100)
					{
						SetVolumeLevel(nVolumn);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "��ǰ��������Ϊ %d", nVolumn);
					}
				}
				else if (vct[2].CompareNoCase("SCREENCUT") == 0)//�����ϴ�
				{
					CString strPicName = vct[3];
					CString strFile = ScreenShot();
					CString strBase64Pic = Base64EncodePic(strFile);
					BOOL bRet = g_toolTrade.UpLoadPic(g_Config.m_strHttpUrl, strBase64Pic, strPicName);
					if (bRet)
					{
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadPic �ϴ���ͼ�ɹ�");
					}
					else
					{
						LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "UpLoadPic �ϴ���ͼʧ�� [%s]", g_toolTrade.GetLastErr());
					}
					RemindTooBigVideoName();
				}
				else if (vct[2].CompareNoCase("TURNSTATUS") == 0)
				{
					LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "����ˢ��֪ͨ�ɹ�");
					SetEvent(m_hDviceStatusEvent);
				}
			}
		}
		else if (vct[0].CompareNoCase("CONTROL") == 0)
		{
			if (g_strItemid == vct[1])
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "��Ŀ��itemidΪ%s,�뵱ǰ��ͬ�����л�", g_strItemid);
				return TRUE;
			}
			g_strItemid = vct[1];
			bUnderSwitchMode = TRUE;
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�л���Ŀ��itemidΪ%s", g_strItemid);
		}
		else if (vct[0].CompareNoCase("CONTROLRECOVERY") == 0)
		{
			bUnderSwitchMode = FALSE;
			SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "�ָ���������");
		}
		else
		{
			// δ֪��Ϣ
			LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "RMQ_DealCustomMsg", "[δ֪��Ϣ][%s]", vct[0]);
		}
	}
	return TRUE;
}

/****************************	�߳�	*****************************************/
DWORD CWeChatPrinterDlg::RMQThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->RMQThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::RMQThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		Sleep(100);

		DWORD dwResult = WaitForSingleObject(m_hRMQEvent, INFINITE);

		if (TRUE == m_bExit) return TRUE;

		if (dwResult == WAIT_OBJECT_0)
		{
			RMQ_DealCustomMsg(g_strRecvMsg);
		}
		ResetEvent(m_hRMQEvent);
	}

	return TRUE;
}

DWORD CWeChatPrinterDlg::ReSignThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->ReSignThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::ReSignThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hReSignEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			BOOL bRet = g_toolTrade.Login(g_Config.m_strHttpUrl, m_iNextBitSpace, m_strLastSignTime, m_OnlineTime,m_strOrgCode,m_strDeviceType);
			if (FALSE == bRet)
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ReSignThreadContent", "[g_toolTrade.Login][err][%s]", g_toolTrade.GetLastErr());
				//ǩ��ʧ�ܣ�20s����ǩ��һ��
				SetTimer(TIMER_RESIGN, 20 * 1000, NULL);
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "ReSignThreadContent", "ǩ��ʧ�ܣ�20s������ǩ��...");
			}
			else
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ReSignThreadContent", "ǩ���ɹ�");
				//����ԭ�ȵĻ����ź��豸���ͣ��ټ���RMQ���ã�������ƽ̨
				g_Config.WriteStringToCfgDev("RMQ","OrgCode", m_strOrgCode);
				g_Config.WriteStringToCfgDev("RMQ", "DeviceType", m_strDeviceType);
				g_Config.LoadRMQCfg();
				RMQ_SUBConnect();

				// ���һ��ʱ�䣬���û��OFFLINE��Ϣ �������ʱ������ʱ�䣬���������
				//���޸���Ե��ǣ�������ǩ���ɹ�������ȴ��;�Ͽ�RMQ,����һ��ʼ���޷�����RMQ���豸������������ָܻ����豸��
				SetTimer(TIMER_OFFLINEREBOT, OFFLINETIME * 60 * 1000, NULL);


				//���Թ��ܣ���������δ����
				//SetEvent(m_hHeartBeatEvent);
				break;
			}
		}
		ResetEvent(m_hReSignEvent);
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::DeviceStatusThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->DeviceStatusThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::DeviceStatusThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hDviceStatusEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			//�ϴ��豸״̬
			BOOL bRet = g_toolTrade.UpLoadDeviceStatus(g_Config.m_strHttpUrl);

			if (bRet)
			{
				CTime tmBegin = CTime::GetCurrentTime();
				CString str_Begin = tmBegin.Format("%Y%m%d%H%M%S");
				LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "DeviceStatusThreadContent", "[�豸״̬�ϴ��ɹ���][%s]", str_Begin);
			}
			else
			{
				CTime tmBegin = CTime::GetCurrentTime();
				CString str_Begin = tmBegin.Format("%Y%m%d%H%M%S");
				LOG2(LOGTYPE_DEBUG, LOG_NAME_OFFLINE, "DeviceStatusThreadContent", "[�豸״̬�ϴ�ʧ�ܣ�][%s][%s]", str_Begin, g_toolTrade.GetLastErr());
			}
			SetTimer(TIMER_OFFLINEREBOT, OFFLINETIME * 60 * 1000, NULL);//�����еĵط����ˣ��豸һֱ���ߣ�������Ҫһֱ����
		}
		ResetEvent(m_hDviceStatusEvent);
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::ChooseProgramThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg *pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->ChooseProgramThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::ChooseProgramThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		Sleep(10);
		DWORD dwResult = WaitForSingleObject(m_hChooseProgramEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼѡ���Ŀ");
			BOOL bInTimeArea = FALSE;													//�Ƿ��ڲ���ʱ��������
			CString strCurDate = GetCurTime(DATE_NORMAL).c_str();						//YYYYMMDDHHMMSS, atoi()��Ч����Ϊ10
			int iCloseRemainingTime = 0;												//����ر���ʣ��ʱ��
			int iRemainingTime = 0;														//���벥����ʣ��ʱ��
			int iPlayMode = 0;															//�ֲ�ģʽ
			int iReleaseMode = 1;														//�岥ģʽ	
			int iCurDate = atoi(strCurDate.Left(8));									//��õ�ǰ���ڣ�YYYYMMDD��20200407
			int iCurTime = atoi(strCurDate.Mid(8, 6));									//��õ�ǰʱ�䣬HHMMSS�ĸ�ʽ����101830
			vector<int> vecHM;															//���浱�ս�Ŀ������ʱ��
			CString strTempItemID = "";													//ѭ������ʱ��Ž�Ŀid
			json jTemp = GetCurrentJson();

			//���template��ɾ������ǰ��Ŀ����Ϊ�գ���ʱʹ��Ĭ��||���߸������Ѿ�����
			if (jTemp == "" || CheckTimeLimit(jTemp["body"]["data"]["itemenddate"].get<std::string>().c_str()))
			{
				ChangeCurrentJson(jDefault);
				jTemp = jDefault;
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ǰ��������Ŀ���Ѿ����ڣ���ʼ����Ĭ�Ͻ�Ŀ��ȫ��");
			}
			//����Ŀ		
			if (jTemp["body"].find("datalist") == jTemp["body"].end())
			{
				json jData = jTemp["body"]["data"];
				iPlayMode = atoi(jData["playmode"].get<std::string>().c_str());
				iReleaseMode = atoi(jTemp["body"]["data"]["releasemode"].get<std::string>().c_str());
				if (iReleaseMode == NEW_JSON && FALSE == g_bIsTemproaryOn)
				{
					//1 ȫ�첥��ģʽ
					if (PLAY_ALLDAY == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���ŵ�ǰ��Ŀ������Ŀ��ȫ��");
					}
					//2 ��ʱ�β���ģʽ
					else if (PLAY_DIVIDE == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						CString strPlaytime = jData["timeseg"].get<std::string>().c_str();
						vecHM = getHours(strPlaytime);
						//Mode 1 �ڷ�Χ�ڣ��������š�����ʱ�ر�ʱ��
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								bInTimeArea = TRUE;
								SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
								iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
								SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
								LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���ŵ�ǰ��Ŀ������Ŀ������%d���Ӻ��л���������Ŀ", (int)(iCloseRemainingTime/60));
								break;
							}
						}
						//Mode 2 ���ڷ�Χ�ڣ�����ʱ����
						if (FALSE == bInTimeArea)
						{
							jForIE = jDefault["body"]["data"]["itemtemplatejson"];//�޸ĵ�ǰjson,����Ĭ�ϵ�
							SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
							iRemainingTime = GetWaitTime(vecHM, iCurTime);
							SetTimer(TIMER_CHOOSEPROGAME, iRemainingTime * 1000, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ����Ĭ�Ͻ�Ŀ������Ŀ������%d���Ӻ��л���ǰ��Ŀ", (int)(iRemainingTime / 60));

						}
					}
				}
				else if (iReleaseMode == TEMPORARY_JSON)
				{
					//Ҫ������岥��Ŀ���� ���� ���ڵȴ�ʱ���

					json jData = jTemp["body"]["data"];
					iPlayMode = atoi(jData["playmode"].get<std::string>().c_str());
					//1 ȫ�첥��ģʽ
					if (PLAY_ALLDAY == iPlayMode)
					{
						g_bIsTemproaryOn = TRUE;
						jForIE = jData["itemtemplatejson"];
						SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���Ž����岥��Ŀ��ȫ��");
					}
					//2 ��ʱ�β���ģʽ
					else if (PLAY_DIVIDE == iPlayMode)
					{
						jForIE = jData["itemtemplatejson"];
						CString strPlaytime = jData["timeseg"].get<std::string>().c_str();
						vecHM = getHours(strPlaytime);
						//Mode 1 �ڷ�Χ�ڣ��������š�����ʱ�ر�ʱ��
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								g_bIsTemproaryOn = TRUE;
								bInTimeArea = TRUE;
								SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
								iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
								SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
								LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���Ž����岥��Ŀ������%d���Ӻ��л���������Ŀ", (int)(iCloseRemainingTime / 60));
								break;
							}
						}
						//Mode 2 ���ڷ�Χ�ڣ�����ʱ����
						if (FALSE == bInTimeArea)
						{
							int g_iTemproaryRemainingTime = GetWaitTime(vecHM, iCurTime);		  //������Ŀ�ȴ�����ʱ��
							SetTimer(TIMER_LOADTEMPORARY, g_iTemproaryRemainingTime * 1000, NULL);//����ʱ��killertime����ɱ������¼�������ؽ�Ŀ��time�¼�����ͻ��
							ChangeCurrentJson(jTemplate);											//����ԭ�ȵĽ�Ŀ
							g_bIsTemproaryOn = FALSE;
							SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���Ž����岥��Ŀ������%d���Ӻ��л���ǰ��Ŀ", (int)(iRemainingTime / 60));
						}
					}
				}
			}
			//���Ŀ
			else
			{
				json jDataList = jTemp["body"]["datalist"];
				json jTodayData;														//���յ�datalist�����Ա
				int iCurWeekDay = GetWeekDay();											//���������
				vector<int> vecALLHM;													//���浱�ս�Ŀ���������ʱ�䣬���ڵ���ʱ������Ŀ

				for (json::iterator it = jDataList.begin(); it != jDataList.end(); ++it)
				{
					jTodayData = *it;
					if (jTodayData.find("carouselmode") != jTodayData.end())// ��ƽ̨�·��е����ظ�û��ʱ�䣬�������û�У�ʱ����������Ҳ�� nulll
					{
						iPlayMode = atoi(jTodayData["carouselmode"].get<std::string>().c_str());
					}
					else
					{
						goto NO_TIME;
					}
					jForIE = jTodayData["itemtemplatejson"];

					strTempItemID = jTodayData["itemid"].get< std::string>().c_str();
					if (bUnderSwitchMode)
					{
						if (g_strItemid.CompareNoCase(strTempItemID) == 0 )
						{
							SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
							g_strItemid = strTempItemID;
							bUnderSwitchMode = FALSE;
							break;
						}
						continue;
					}

					for (json::iterator it = jTodayData["timesegitem"].begin(); it != jTodayData["timesegitem"].end(); ++it)
					{
						BOOL bFindDay = FALSE;											//�Ƿ��ҵ����յ�datalist�����Ա
						json jTemp = *it;
						CString strPlaytime = jTemp["timeseg"].get<std::string>().c_str();
						if (PLAY_WEEKDAY == iPlayMode || PLAY_REPEATDAY == iPlayMode)
							vecHM = getHours(strPlaytime);
						// һ���ж������Ƿ��ڷ�Χ��
						//���ظ�
						if (PLAY_WEEKDAY == iPlayMode)
						{
							CString strWeekDay = "";									//��Ž�Ŀ�������
							if (jTemp.find("weekday") != jTemp.end())
							{
								strWeekDay = jTemp["weekday"].get<std::string>().c_str();
							}
							if (atoi(strWeekDay) == iCurWeekDay)
							{
								bFindDay = TRUE;
							}
							if (FALSE == bFindDay) continue;//ok
						}
						//�Զ���
						else if (PLAY_CUSTOMIZE == iPlayMode)
						{
							vector<int> vecDay;											//����Զ����Ŀ�е���������
							getDays(strPlaytime, vecDay, vecHM);
							//��Ϊ���ڣ���������������
							if (1 == vecDay.size())
							{
								if (iCurDate == vecDay[0]) bFindDay = TRUE;
							}
							else if (2 == vecDay.size())
							{
								if (iCurDate >= vecDay[0] && iCurDate <= vecDay[1]) bFindDay = TRUE;
							}
							if (FALSE == bFindDay) continue;
						}
						bFindDay = TRUE;//���ظ����̶�Ϊ�ҵ���

						//���� �ж�ʱ���Ƿ��ڷ�Χ��
						for (unsigned int i = 0; i < vecHM.size(); i += 2)
						{
							if (iCurTime >= vecHM[i] && iCurTime <= vecHM[i + 1])
							{
								bInTimeArea = TRUE;
								break;
							}
							vecALLHM.push_back(vecHM[i]);
							vecALLHM.push_back(vecHM[i + 1]);
						}
						if (bInTimeArea) break;
					}
				NO_TIME:
					//����ý�Ŀ��������ȷ�����ڣ������һ����Ա ����Ч��ʱ��Σ�ѭ��
					if (FALSE == bInTimeArea && (it + 1) != jDataList.end()) continue;

					//�з���ʱ��ľ��˳�ѭ��
					if (bInTimeArea)
					{
						SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
						iCloseRemainingTime = GetCloseTime(vecHM, iCurTime);
						SetTimer(TIMER_CHOOSEPROGAME, iCloseRemainingTime * 1000, NULL);
						LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ���Ŷ��Ŀ������%d���Ӻ��л���������Ŀ", (int)(iCloseRemainingTime / 60));
						break;
					}
					// �������еĽ�Ŀ�ﶼ�Ҳ�������ʱ��ʱ������ʱ����Ĳ���
					else
					{
						jForIE = jDefault["body"]["data"]["itemtemplatejson"];//�޸ĵ�ǰjson,����Ĭ�ϵ�
						SetTimer(TIMER_LOADPAGE, DELAY_TIME*1000, NULL);
						//�������Ϊ���ظ����Զ���ģʽ���ں������ж�Ϊĩβ��Ҫתһ���������Ͳ�ִ�е���ʱ
						iRemainingTime = GetWaitTime(vecALLHM, iCurTime, iPlayMode);
						if (iRemainingTime >= 0)
						{
							SetTimer(TIMER_CHOOSEPROGAME, iRemainingTime * 1000, NULL);
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "��ʼ����Ĭ�Ͻ�Ŀ������%d���Ӻ��л����Ŀ", (int)(iRemainingTime / 60));
						}
						else
						{
							LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "ChooseProgram", "�����޷����Ǽ򵥵Ķ��Ŀ����ʼ����Ĭ�Ͻ�Ŀ");
						}
						break;
					}

				}
			}
			ResetEvent(m_hChooseProgramEvent);
		}
	}
	return TRUE;
}

DWORD CWeChatPrinterDlg::HeartBeatThreadProc(LPVOID pParam)
{
	CWeChatPrinterDlg* pDlg = (CWeChatPrinterDlg*)pParam;
	return pDlg->HeartBeatThreadContent(pParam);
}

DWORD CWeChatPrinterDlg::HeartBeatThreadContent(LPVOID pParam)
{
	while (FALSE == m_bExit)
	{
		DWORD dwResult = WaitForSingleObject(m_hHeartBeatEvent, INFINITE);
		if (TRUE == m_bExit) return TRUE;
		if (dwResult == WAIT_OBJECT_0)
		{
			sleepFunction1(m_iNextBitSpace * 1000);
			if (TRUE == m_bExit) return TRUE;
			BOOL bRet = g_toolTrade.HeartBeat(g_Config.m_strHttpUrl, m_iNextBitSpace, m_strLastSignTime, m_OnlineTime);
			if (FALSE == bRet)
			{
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "HeartBeat", "[g_toolTrade.HeartBeat][err][%s]", g_toolTrade.GetLastErr());
				LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "HeartBeatThreadContent", "���������ӿ�ʧ��...");
			}
		}
	}
	ResetEvent(m_hHeartBeatEvent);
	return TRUE;
}

/************************************************************************/
/*                               ��ʱ��                                  */
/************************************************************************/
void CWeChatPrinterDlg::OnTimer(UINT_PTR nIDEvent)
{
	CString strTemporaryJsonPath = "";

	KillTimer(nIDEvent);
	switch (nIDEvent)
	{
	case TIMER_RESIGN:
		SetEvent(m_hReSignEvent);
		break;
	case TIMER_LOADPAGE:
		if (ZipImg())
		{
			LoadTemplate();
		}
		break;
	case TIMER_RECV_MSG:				//��ȡ����
		SetEvent(m_hRMQEvent);
		break;
	case TIMER_LOADTEMPORARY:			//���ؽ����岥�Ľ�Ŀ
		ChangeCurrentJson(jTemproary);
		g_bIsTemproaryOn = TRUE;
		SetTimer(TIMER_CHOOSEPROGAME, 10, NULL);
		break;
	case TIMER_CHOOSEPROGAME:
		SetEvent(m_hChooseProgramEvent);
		break;
	case TIMER_CHECKINCOMPELEDFILE:		//����ļ��Ƿ�����
		if (nCheckTimes++ < 3)
		{
			CheckIncompleted();
		}
		else
			nCheckTimes = 0;
		break;
	case TIMER_CHECKMEMORY:
		GetSystemMemoryInfo();
		// edit by mingli 
		SetTimer(TIMER_CHECKMEMORY, 1000 * 120, NULL);
		//SetTimer(TIMER_CHECKMEMORY, 1000, NULL);
		break;
	case  TIMER_OFFLINEREBOT:
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnTimer", "\n�޷�����RabbitMQ��Ϣ�����򼴽�����\n");
		RobotProgamme();
		break;
	default:
		break;
	}
	CImageDlg::OnTimer(nIDEvent);
}

BEGIN_EVENTSINK_MAP(CWeChatPrinterDlg, CImageDlg)
//	ON_EVENT(CWeChatPrinterDlg, IDC_EXPLORER_MAIN, 102, CWeChatPrinterDlg::OnStatustextchangeExplorerMain, VTS_BSTR)
END_EVENTSINK_MAP()
/************************************************************************/
/*                            HOOK                                      */
/************************************************************************/
LRESULT CALLBACK OnMouseEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (wParam == WM_LBUTTONDOWN)
	{
		static CPoint point;
		GetCursorPos(&point);
		g_CWeChatPrinterDlg->OnAdminEnter(point);
	}
	return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
}

void CWeChatPrinterDlg::OnAdminEnter(CPoint point)
{
	const int nFalg = 150;
	CRect rect1(0, 0, nFalg, nFalg);
	CRect rect2(m_rc.Width() - nFalg, 0, m_rc.Width(), nFalg);
	CRect rect3(0, m_rc.Height() - nFalg, nFalg, m_rc.Height());
	CRect rect4(m_rc.Width() - nFalg, m_rc.Height() - nFalg, m_rc.Width(), m_rc.Height());
	if (rect1.PtInRect(point))
	{
		m_strAdminEnter += "1";
	}
	else if (rect2.PtInRect(point))
	{
		m_strAdminEnter += "2";
	}
	else if (rect3.PtInRect(point))
	{
		m_strAdminEnter += "3";
	}
	else if (rect4.PtInRect(point))
	{
		m_strAdminEnter += "4";
	}
	else
	{
		m_strAdminEnter = "";
	}

	TRACE("��Ļ�����ǣ�%s\n", m_strAdminEnter);
#ifdef EASY_PASSWD
	if (m_strAdminEnter.Find("1") >= 0) ::PostMessage(m_hWnd, WM_COMMAND, BTN_ADMIN_LOGOUT, NULL);
#else
	if (m_strAdminEnter.Find("33") >= 0) ::PostMessage(m_hWnd, WM_COMMAND, BTN_ADMIN_LOGOUT, NULL);
#endif // EASY_PASSWD
}

BOOL CWeChatPrinterDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: �ڴ����ר�ô����/����û���
	switch (wParam)
	{
	case BTN_ADMIN_LOGOUT:
	{
		{
		//	::SetWindowPos(GetSafeHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//��ǰ��
			m_strAdminEnter = "";
			CAdmins m_admin;
			int nResult = m_admin.DoModal();
			if (nResult == 1)
			{
				LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnCommand", "�û��������룺33 ������˳�");
				EndDialog(TRUE);
			}
		//	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//��ǰ��
		}
	}
	break;
	default:
		break;
	}
	return CImageDlg::OnCommand(wParam, lParam);
}

BOOL CWeChatPrinterDlg:: StartProcess3(CString strpath)
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "StartProcess3", "[%s]", "\"" + strpath + "\"");
 	BOOL b1 = StartProcess2("\"" + strpath + "\"", TRUE);//�ɹ���ʧ�ܶ��Ƿ���1
//	BOOL b2 = StartProcess(strpath, "", 0, 0);
//  	if (b1 == FALSE)
//  	{
// 		BOOL b2 = StartProcess(strpath, "", 0, 0);
// 		LOG2(LOGTYPE_ERROR, LOG_NAME_DEBUG, "StartProcess3", "StartProcess ʧЧ[%d]������StartProcess2 [%d]", b1, b2);
//  	}
	return TRUE;
}

//MFC�»�ȡϵͳ�ڴ�͵�ǰ���̵��ڴ�ʹ�����
void GetSystemMemoryInfo()
{
	CString strInfo;
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	GlobalMemoryStatusEx(&statex);

	DWORDLONG physical_memory = statex.ullTotalPhys / (1024 * 1024);
	DWORDLONG avalid_memory = statex.ullAvailPhys / (1024 * 1024);
	DWORDLONG virtual_totalmemory = statex.ullTotalVirtual / (1024 * 1024);
	DWORDLONG virtual_memory = statex.ullAvailVirtual / (1024 * 1024);
	DWORDLONG usePhys = physical_memory - avalid_memory;
	DWORDLONG useVirtual = virtual_totalmemory - virtual_memory;

	float percent_memory = ((float)usePhys / (float)physical_memory) * 100;
	float percent_memory_virtual = ((float)useVirtual / (float)virtual_totalmemory) * 100;

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	DWORD pid = GetCurrentProcessId();
	HANDLE handle;
	handle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));

	int usedMemory = 0;

	PSAPI_WORKING_SET_INFORMATION workSet;
	memset(&workSet, 0, sizeof(workSet));
	BOOL bOk = QueryWorkingSet(handle, &workSet, sizeof(workSet));
	if (bOk || (!bOk && GetLastError() == ERROR_BAD_LENGTH))
	{
		int nSize = sizeof(workSet.NumberOfEntries) + workSet.NumberOfEntries * sizeof(workSet.WorkingSetInfo);
		char* pBuf = new char[nSize];
		if (pBuf)
		{
			QueryWorkingSet(handle, pBuf, nSize);
			PSAPI_WORKING_SET_BLOCK* pFirst = (PSAPI_WORKING_SET_BLOCK*)(pBuf + sizeof(workSet.NumberOfEntries));
			DWORD dwMem = 0;
			for (ULONG_PTR nMemEntryCnt = 0; nMemEntryCnt < workSet.NumberOfEntries; nMemEntryCnt++, pFirst++)
			{
				if (pFirst->Shared == 0)
				{
					dwMem += si.dwPageSize;
				}
			}
			delete pBuf;
			pBuf = NULL;
			if (workSet.NumberOfEntries > 0)
			{
				usedMemory = dwMem / (1024 * 1024);
			}
		}
	}
	strInfo.Format(
		"�����ڴ�ʹ����:%.2f%% �����ڴ�:%lld MB ���������ڴ棺%lld MB "
		"�����ڴ�ʹ����:%.2f%% �����ڴ�:%lld MB ���������ڴ棺%lld MB "
		"����id:%d ��ʹ���ڴ� %d MB"
		, percent_memory, physical_memory, avalid_memory
		, percent_memory_virtual, virtual_totalmemory, virtual_memory
		, pid, usedMemory
	);
	LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "%s", strInfo);
	CloseHandle(handle);

	CString strNowTime = GetCurTime(DAY_NORMAL).c_str();
	if (strNowTime != g_strStartTime)
	{
		LOG2(LOGTYPE_DEBUG, LOG_NAME_DEBUG, "OnTimer", "\n ����������%s,���ڼ�����������\n",g_strStartTime);
		RobotProgamme();
	}
	//�����ڴ�ʹ���� >85 ����  ��ʹ���ڴ� >1100 MB ����������
// 	if (/*percent_memory_virtual >85 ||*/ usedMemory >1100)
// 	{
// 		LOG2(LOGTYPE_DEBUG, LOG_NAME_MEMORY, "GetSystemMemoryInfo", "\n���򼴽�����\n");
// 		RobotProgamme();
//	}
}

void RobotProgamme()
{
	LOG2(LOGTYPE_DEBUG, LOG_NAME_REBOT, "RobotProgamme", "����%d����û���յ�OFFLINE��Ϣ�����򼴽�����",OFFLINETIME);

	// TODO: �ڴ���ӿؼ�֪ͨ����������
	::PostMessage(AfxGetMainWnd()->m_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
	//��ȡexe����ǰ·��
	TCHAR szAppName[MAX_PATH];
	::GetModuleFileName(theApp.m_hInstance, szAppName, MAX_PATH);
	CString strAppFullName;
	strAppFullName.Format(_T("%s"), szAppName);
	//��������
	STARTUPINFO StartInfo;
	PROCESS_INFORMATION procStruct;
	memset(&StartInfo, 0, sizeof(STARTUPINFO));
	StartInfo.cb = sizeof(STARTUPINFO);
	::CreateProcess(
		(LPCTSTR)strAppFullName,
		NULL,
		NULL,
		NULL,
		FALSE,
		NORMAL_PRIORITY_CLASS,
		NULL,
		NULL,
		&StartInfo,
		&procStruct);

}
