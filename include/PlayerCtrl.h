// PlayerCtrl.h: interface for the CPlayerCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERCTRL_H__880C8FAD_AB64_493C_B6D7_24DF1AA5F4D6__INCLUDED_)
#define AFX_PLAYERCTRL_H__880C8FAD_AB64_493C_B6D7_24DF1AA5F4D6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wmpplayer4.h"

class CPlayerCtrl : public CWMPPlayer4  
{
public:
	CPlayerCtrl();
	virtual ~CPlayerCtrl();

	void Play(bool bLoop);
	void Pause();
	void Stop();
	void AddList(char* strFile);
	void ClearList();
	void SetVolume(long lVolume);
	long GetVolume();

	DECLARE_MESSAGE_MAP()
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#endif // !defined(AFX_PLAYERCTRL_H__880C8FAD_AB64_493C_B6D7_24DF1AA5F4D6__INCLUDED_)
