#include <stdio.h>
#include <tchar.h>

#define  WIN_XP    2
#define  WIN_2003  3
#define  WIN_VISTA 4
#define  WIN_7     5
#define  WIN_2008  6
#define  WIN_8     8

int GetOSVer();

//void InitMediaDevice();

void GetSystemVolume(float &dVolumeValue);

void SetSystemVolume(DWORD dVolumeValue);

bool SetSpeakerMute(bool bMute);

void GetMicrophoneValue(DWORD &dMicorphoneValue);

void SetMicrophoneValue(DWORD dMicorphoneValue);

void SetMicrophoneMute(bool bIsMut);

bool GetMicroBoostData(float &fMin, float &fMax, float &fStep);

bool GetMicroBoostLevel(float &dMicroBoostLevel);

void SetMicroBoostLevel(float dMicroBoostLevel);
