#ifndef __DEVICE_STRUCT_H__
#define __DEVICE_STRUCT_H__

typedef struct DEVICE 
{
	//设备ID
	char szDeviceID[32];			//设备ID号
	//设备编码
	char szDeviceRegCode[64];	//设备编码
	//设备名称
	char szDeviceName[128];		//设备名称
	//是否使用
	BOOL bIsUse;				//设备是否再用
	//并口
	char szLPT[8];				//并口号
	BYTE bLPT;					//并口号
	//串口
	char szCom[8];				//串口号
	BYTE bCom;					//串口号
	UINT uOpnSpeed;				//波特率
	//继电器参数
	BYTE bSlaveAddr;			//下位机地址
	BYTE bCoilcount;			//线圈个数
	BYTE bInputCount;			//光耦个数

	DEVICE()
	{
		ReSet();
	}
	void ReSet()
	{
		memset(this,0x00,sizeof(DEVICE));
	}
	//设备名称|设备编码|设备接口（串口）|是否在用
	BOOL operator << (char* pStr)
	{
		ReSet();
		CString strtemp;
		if(NULL == pStr || 0 == strlen(pStr)) return FALSE;
		std::vector<CString> vct= SplitString(pStr,"|");
		if(vct.size()) 
		{
			strtemp = vct[0];
			strcpy_s(szDeviceName,strtemp.GetBuffer(0));
		}
		if(vct.size() > 1) 
		{
			strtemp = vct[1];
			strcpy_s(szDeviceRegCode,strtemp.GetBuffer(0));
		}
		//COM|LPT|USB|DAM
		if(vct.size() > 2) 
		{
			strtemp = vct[2];
			std::vector<CString> vctsub = SplitString(strtemp,",");
			if(vctsub.size()) 
			{
				if(0 == vctsub[0].Compare("COM"))
				{
					if(vctsub.size() > 1)
					{
						vctsub[1].MakeUpper();
						strcpy_s(szCom,vctsub[1].GetBuffer(0));
						vctsub[1].Replace("COM","");
						bCom = atoi(vctsub[1].GetBuffer(0));
					}
					if(vctsub.size() > 2) uOpnSpeed = atoi(vctsub[2].GetBuffer(0));
				}
				if(0 == vctsub[0].Compare("LPT"))
				{
					if(vctsub.size() > 1)
					{
						vctsub[1].MakeUpper();
						strcpy_s(szLPT,vctsub[1].GetBuffer(0));
						vctsub[1].Replace("LPT","");
						bLPT = atoi(vctsub[1].GetBuffer(0));
					}
					if(vctsub.size() > 2) uOpnSpeed = atoi(vctsub[2].GetBuffer(0));
				}
				if(0 == vctsub[0].Compare("USB"))
				{
					if(vctsub.size() > 1) strcpy_s(szDeviceID,vctsub[1].GetBuffer(0));
				}
				if(0 == vctsub[0].Compare("DAM"))
				{
					if(vctsub.size() > 1)
					{
						vctsub[1].MakeUpper();
						strcpy_s(szCom,vctsub[1].GetBuffer(0));
						vctsub[1].Replace("COM","");
						bCom = atoi(vctsub[1].GetBuffer(0));
					}
					if(vctsub.size() > 2) uOpnSpeed = atoi(vctsub[2].GetBuffer(0));
					if(vctsub.size() > 3) bSlaveAddr = atoi(vctsub[3].GetBuffer(0));
					if(vctsub.size() > 4) bCoilcount = atoi(vctsub[4].GetBuffer(0));
					if(vctsub.size() > 5) bInputCount = atoi(vctsub[5].GetBuffer(0));
				}
			}
		}
		if(vct.size() > 3) 
		{
			strtemp = vct[3];
			bIsUse = atoi(strtemp.GetBuffer(0));
		}
		return TRUE;
	}
}*LPDEVICE;

typedef struct DEVICE_ICCARD_MSG 
{
	char szCardID[32];				//卡号
	char szFilePathHtmlName[128];	//卡对应的显示信息(html文件；文件名)
	char szFilePathHtmlPath[512];	//卡对应的显示信息(html文件；路径)
	DEVICE_ICCARD_MSG()
	{
		memset(this,0x00,sizeof(DEVICE_ICCARD_MSG));
	}
	BOOL CompareCardID(char* pCardID)
	{
		if(pCardID == NULL || strlen(pCardID) == 0) return FALSE;
		return strcmp(szCardID,pCardID) == 0 ? TRUE:FALSE;
	}
	BOOL operator << (char* pStr)
	{
		if(pStr == NULL || strlen(pStr) == 0) return FALSE;
		std::vector<CString> vct= SplitString(pStr,",");
		if(vct.size()) 
		{
			strcpy_s(szCardID,vct[0].GetBuffer(0));
		}
		if(vct.size() > 1) 
		{
			strcpy_s(szFilePathHtmlName,vct[1].GetBuffer(0));
		}
		return TRUE;
	}
}*LPDEVICE_ICCARD_MSG;

typedef struct DEVICE_USBCAM_MSG 
{
	char szBaseCam[128];			//基准
	char szBaseCamPath[512];		//基准路径

	float HistogramRange1[2];
	float *HistogramRange[1];

	char szFilePathHtmlName[128];	//卡对应的显示信息(html文件；文件名)
	char szFilePathHtmlPath[512];	//卡对应的显示信息(html文件；路径)
	DEVICE_USBCAM_MSG()
	{
		memset(this,0x00,sizeof(DEVICE_USBCAM_MSG));
		HistogramRange1[0] = 0;
		HistogramRange1[1] = 255;
		HistogramRange[0] = &HistogramRange1[0];
	}
	BOOL CompareCapture(char* pCapture)
	{
		if(pCapture == NULL || strlen(pCapture) == 0) return FALSE;
		return strcmp(szBaseCam,pCapture) == 0 ? TRUE:FALSE;
	}
	BOOL operator << (char* pStr)
	{
		if(pStr == NULL || strlen(pStr) == 0) return FALSE;
		std::vector<CString> vct= SplitString(pStr,",");
		if(vct.size()) 
		{
			strcpy_s(szBaseCam,vct[0].GetBuffer(0));
		}
		if(vct.size() > 1) 
		{
			strcpy_s(szFilePathHtmlName,vct[1].GetBuffer(0));
		}
		return TRUE;
	}
}*LPDEVICE_USBCAM_MSG;

typedef struct DEVICE_DAMIO_MSG 
{
	char szType[32];
	BYTE bCoilNo;
	BYTE bTimeOut;
	BOOL bIsCtrl;

	DEVICE_DAMIO_MSG()
	{
		memset(this,0x00,sizeof(DEVICE_DAMIO_MSG));
	}
	BOOL operator << (char* pStr)
	{
		if(pStr == NULL || strlen(pStr) == 0) return FALSE;
		std::vector<CString> vct= SplitString(pStr,",");
		if(vct.size()) 
		{
			strcpy_s(szType,vct[0].GetBuffer(0));
		}
		bCoilNo = (vct.size() > 1) ? atoi(vct[1].GetBuffer(0)):0;
		bTimeOut = (vct.size() > 2) ? atoi(vct[2].GetBuffer(0)):5;
		bIsCtrl = (vct.size() > 3) ? atoi(vct[3].GetBuffer(0)):FALSE;
		return TRUE;
	}
}*LPDEVICE_DAMIO_MSG;

typedef struct IMG_BTN_NET_MSG
{
	int nX;
	int nY;
	int nID;
	char szBtnUP[128];
	char szBtnDOWN[128];
	char szBtnNET[256];

	IMG_BTN_NET_MSG()
	{
		memset(this,0x00,sizeof(IMG_BTN_NET_MSG));
	}
	BOOL operator << (char* pStr)
	{
		if(pStr == NULL || strlen(pStr) == 0) return FALSE;
		std::vector<CString> vct= SplitString(pStr,",");
		nX = (vct.size() > 0) ? atoi(vct[0].GetBuffer(0)):0;
		nY = (vct.size() > 1) ? atoi(vct[1].GetBuffer(0)):0;
		if(vct.size() > 2) 
		{
			strcpy_s(szBtnUP,vct[2].GetBuffer(0));
		}
		if(vct.size() > 3) 
		{
			strcpy_s(szBtnDOWN,vct[3].GetBuffer(0));
		}
		if(vct.size() > 4) 
		{
			strcpy_s(szBtnNET,vct[4].GetBuffer(0));
		}
		return TRUE;
	}
}*LPIMG_BTN_NET_MSG;
































#endif