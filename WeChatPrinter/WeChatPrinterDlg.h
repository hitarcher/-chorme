
// WeChatPrinterDlg.h : ͷ�ļ�
#pragma once
#include <mshtml.h>
#include <shellapi.h>
#include "Trade.h"
#include "CommTbl.h"
#include "SqliteFun.h"
#include "CommonFun.h"
#include "Shell.h"
#include "ImageDlg.h"
#include "LOG2.H"
#include <sqlite3.h>
#include "CommTbl.h"
#include "json.hpp"
using json = nlohmann::json;
#include "Admins.h"
#include "Config.h"
#include <stdio.h>

#ifdef DEBUG                            
#pragma comment(lib, "CommUtilsd.lib")  
#else                                   
#pragma comment(lib, "CommUtils.lib")   
#endif         

//����ͷ�ļ�
#include "MakePNG.h"

//��ѹ��ͷ�ļ�
#pragma comment( lib, "coinreceiver.lib" )
#include "tinyzipinterface.h"
#pragma comment( lib, "tinyzip.lib" )

//���ߺ���ͷ�ļ�
#include "ToolsFunction.h"
//�ڴ溯�����ͷ�ļ�
#include <psapi.h>

//CEF3
#include "mycef.h"
#include "simple_app.h"
#include "simple_handler.h"
/************************************************************************/
/*                              ��    ��                                */
/************************************************************************/
enum PROGAMETYPE
{
	SINGLEPROGRAM = 1,			//��Ŀ
	MULITIPLEPROGRAM = 2		//��Ŀ��
};
enum PROGAMMODE
{
	NEW_JSON = 1,				//Ĭ�Ͻ�Ŀ���½�Ŀ,���߽�Ŀ
	TEMPORARY_JSON = 2			//�����岥(ֻ�е���Ŀ)
};
enum SINGLEPRO
{
	PLAY_ALLDAY = 1,			//����ȫ��
	PLAY_DIVIDE = 2				//������ʱ��
};
enum MULITIPEPRO
{
	PLAY_REPEATDAY = 1,			//�࣬���ظ���һ�����ģʽȫ��ֻ��Ϊ1
	PLAY_WEEKDAY = 2,			//�࣬���ظ���һ�����ģʽȫ��ֻ��Ϊ2
	PLAY_CUSTOMIZE = 3			//�࣬�Զ��壬һ�����ģʽȫ��ֻ��Ϊ3
};


// CWeChatPrinterDlg �Ի���
class CWeChatPrinterDlg : public CImageDlg/*,public IDispatch*/
{
// ����
public:
	CWeChatPrinterDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_WECHATPRINTER_DIALOG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	HICON m_hIcon;
	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bExit;													
	CString m_strLastErr;											
	CTrade g_toolTrade;
	int m_nMode;													//����Ŀ���Ƕ��Ŀ

	CString m_strHtmlPath;											//H5ҳ��·��

	//ǩ���߳�
	HANDLE m_hReSign;
	HANDLE m_hReSignEvent;

	//RabbitMQ����
	HANDLE m_hRMQ;
	HANDLE m_hRMQEvent;

	//�ϴ��豸״̬
	HANDLE m_hDviceStatus;
	HANDLE m_hDviceStatusEvent;

	//�жϽ�Ŀѡ��
	HANDLE m_hChooseProgram;
	HANDLE m_hChooseProgramEvent;

	//����
	HANDLE m_hHeartBeat;
	HANDLE m_hHeartBeatEvent;
	
	json jrsp;														//����ʹ���еĽ�Ŀ 
	json jTemplate;													//ƽ�����صĽ�Ŀ	tempalte.json
	json jTemproary;												//�����岥��Ŀ		temproary.json
	json jOldrsp;													//ȡ����Ŀʱ����������ԭ��Ŀ
	json jDefault;													//Ĭ�Ͻ�Ŀ			default.json
	json jOverdue;													//��һ���·��Ľ�Ŀ	templateold.json
	json jForIE;													//��ǰ����h5ʹ�õ����ݣ����ֶ�itemtemplatejson

	json GetCurrentJson() { return jrsp; }
	void ChangeCurrentJson(json jTemp) { jrsp = jTemp; }

	static CWeChatPrinterDlg* m_pThis;

	int m_iNextBitSpace ;										    //�´�����ʱ��
	CString m_strLastSignTime;									    //����ǩ��ʱ��
	CString m_OnlineTime;										    //����ʱ��
	CString m_strOrgCode;											//ǩ���������  
	CString m_strDeviceType;									    //ǩ�����豸����
																    
	CefRefPtr<SimpleApp> m_cef_app;								    
	void cef_init();											    
	void cef_close();											    
	void cef_load_url(IN std::string _utf_url, IN int delay=0);
	void cef_exec_js(IN std::string _utf_js, IN int delay = 0); 
	// ����ģ��
	void LoadTemplate();

	//ǩ���߳�
	static DWORD WINAPI	ReSignThreadProc(LPVOID pParam);
	DWORD WINAPI ReSignThreadContent(LPVOID pParam);

	// RMQ�߳�
	static DWORD WINAPI	RMQThreadProc(LPVOID pParam);
	DWORD WINAPI RMQThreadContent(LPVOID pParam);

	//�����豸״̬
	static DWORD WINAPI	DeviceStatusThreadProc(LPVOID pParam);
	DWORD WINAPI DeviceStatusThreadContent(LPVOID pParam);

	//�жϽ�Ŀѡ��
	static DWORD WINAPI	ChooseProgramThreadProc(LPVOID pParam);
	DWORD WINAPI ChooseProgramThreadContent(LPVOID pParam);

	//����
	static DWORD WINAPI HeartBeatThreadProc(LPVOID pParam);
	DWORD WINAPI HeartBeatThreadContent(LPVOID pParam);

	// RMQ�·�����
	BOOL RMQ_DealCustomMsg(CString strMsg);	
	//����Pub��SUB������̬��
	BOOL LoadRMQPubAndRMQSUBDLL();	
	//SUB���ӷ����������ý��ջص�����
	BOOL RMQ_SUBConnect();			

	//ɾ���ɵ��ز�
	void DelateResource();

	//���json���Ƿ�����Դ���json�ظ���2��3��4��5 json��1�Ƚϣ�1json���е��ز���2��3��4��5û�еĻ���ɾ����Щ�ز�
	BOOL CheckOldResource(vector<json>vecJson);

	//���������أ��޸���Դ·�������¼���json���ж��Ƿ��ǽ�Ŀ���ǽ�Ŀ��
	BOOL ParseNewTemplateOfH5(CString strJson, BOOL bIsSingleProgram  , /*BOOL bIsOffline =FALSE,*/ BOOL bParseOnly = FALSE);

	//ѡ����صĽ�Ŀ����������ѹ������ѹ��ѡ������岥��Ŀ��������ͨ��Ŀ
	BOOL ChooseJson();

	//�жϲ����ؽ����岥��Ŀ
	BOOL LoadTemporayJson();

	//�ж��������߽�Ŀѹ����
	BOOL LoadOfflinePacket();

	//����Զ����³����Ƿ�����
	BOOL CheckUpdate();

	// ��Ļ��ͼ,����ͼƬ��·��
	CString ScreenShot(void);
	
	//��Ȿ���Ƿ���û����������ļ�
	BOOL CheckIncompleted();

	//�޸Ľ�Ŀ�Ľ�ֹ���ںͽ�ֹʱ�䣬�޸Ľ�Ŀ���Ľ�ֹ���ڵ�����ʱ�䡣
	//����ʱ���޸ģ����ݵ�ǰ�·��Ľ�Ŀ����һ�����޸ĳ����е�ʱ�䣬�Լ����ص�json,
	//�����ǰ����ͨ��Ŀ���޸�ʱ�䣬�޸ı���ʱ�䣬���¼��أ�����ǽ����岥��ֻ�޸Ľ����岥��
	BOOL ExtendDeadline(CString strProgammeID, CString strNewDeadline, vector<CString> vecTime);

	BOOL StartProcess3(CString strpath);

	//�������߽�Ŀ������ǰ��template �ĳ�templateonline��templaiteOffline �ĳ� template
// 	void SwitchJsonToOffline();
// 	
// 	//�������߽�Ŀ������ǰ��template �ĳ�templaiteOffline��templateonline �ĳ� template
// 	void SwitchJsonToOnline();
	void sleepFunction1(int Time);//α��̬��sleep�����ڳ�ʱ��˯��ȴ�޷������˳����߳�

public:
	// ���ڴ�С
	CRect	m_rc;
	//�˳�ʱ������
	CString m_strAdminEnter;

	void OnAdminEnter(CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()

};
/************************************************************************\
* ���json�����е���Դ·�����ų����ظ��ģ����浽������					*
* ��Σ�jALL ��ƽ̨��Ŀjson ��vecTemp ���������������Դ·��			*
* ����ֵ��true �ɹ���False ʧ��											*
\************************************************************************/
BOOL FindContent(json jALL, vector<CString> &vecTemp);		

//���template/1.json������json����
json LoadjsonFile(CString strJsonName);

/************************************************************************\
* �����ļ�																*
* ��Σ�strUrl���ļ�url ��strLocalPath �ļ��ľֲ���ַ���硰www/static/1.jpg��
* ����ֵ����															*
\************************************************************************/
void DownLoadFile(CString strUrl, CString strLocalPath);

/************************************************************************\
* ��json�ﴫ���ʱ����䵽������										*
* ��Σ�08:00-09:00|11:00-12:00
* ����ֵ��80000��90000��110000�ȵ�int��������							*
\************************************************************************/
vector<int> getHours(CString strHours);	

/************************************************************************\
* ��json�ﴫ���ʱ����䵽������										*
* ��Σ�strDays ��2019-02-25|2019-02-31|08:00-09:00|11:00-12:00��
		vecDay ��������ַ�����vecHM ���ʱ���ַ���						*
* ����ֵ�� 20190225,80000,120000										*
\************************************************************************/
void getDays(CString strDays, vector<int> &vecDay, vector<int> &vecHM);	


/************************************************************************\
* �жϽ�Ŀ�����Ƿ����													*
* ��Σ��������ڣ���ʽ����2021-01-11									*
* ����ֵ�� falseΪû���ڣ�trueΪ����									*
\************************************************************************/
BOOL  CheckTimeLimit(CString strEndDate);			

/************************************************************************\
* ��ȡ��Ŀʣ�ಥ��ʱ��													*
* ��Σ�vecHM �ý�Ŀ���е�ʱ��Σ���getHours��ȡ��						*
*		iCurrentTime ��ǰʱ�䣬HHMMSS�ĸ�ʽ��							*
*		iPlayMode ���Ÿ�ʽ���ο�MULITIPEPRO								*
* ����ֵ�� ʣ�ಥ��ʱ��,��λ��											*
\************************************************************************/
int GetWaitTime(vector<int>vecHM, int iCurrentTime, int iPlayMode = 1);

/************************************************************************\
* ��ȡ��Ŀʣ��ر�ʱ��													*
* ��Σ�vecHM �ý�Ŀ���е�ʱ��Σ���getHours��ȡ��						*
*		iCurrentTime ��ǰʱ�䣬HHMMSS�ĸ�ʽ��							*
* ����ֵ�� ʣ��ر�ʱ��,��λ��											*
\************************************************************************/
int GetCloseTime(vector<int>vecHM, int iCurrentTime);				

BOOL HTTP_Download2(CString strURL, CString strFilePath, CString strUsername, CString strPassword, CString strProxyList);

void GetSystemMemoryInfo();

//��������
void RobotProgamme();
/************************************************************************/
/*                            ȫ�ֱ�������                              */
/************************************************************************/
extern CConfig	g_Config;