#ifndef __MY_IMAGE_COMPRESS__H__
#define __MY_IMAGE_COMPRESS__H__

// https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-setting-jpeg-compression-level-use
// https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-retrieving-the-class-identifier-for-an-encoder-use
// https://blog.csdn.net/kaizi318/article/details/72458350

#pragma comment( lib, "gdiplus.lib" )
#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
using namespace Gdiplus;

#include "myos.h"
#include "mystring.h"

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

void Transform_to_png(std::string orig_fiepath)
{
	Image* imageoriginal = new Image(string2wstring(orig_fiepath, "zh-CN").c_str());

	// Get the CLSID of the png encoder.
	//image/bmp 
	//image/jpeg
	//image/gif 
	//image/tiff
	//image/png
	CLSID encoderClsid;
	GetEncoderClsid(L"image/png", &encoderClsid);

	imageoriginal->Save(string2wstring(orig_fiepath + ".tmp", "zh-CN").c_str(), &encoderClsid);
	delete imageoriginal;

	mv((orig_fiepath + ".tmp").c_str(), orig_fiepath.c_str());
}

Status Compress_image_to_tmp(std::string orig_fiepath, std::string backup_folder, std::string dst_tmp = "compress.jpg.tmp", unsigned int maxWidth = 4096, unsigned int maxHeight = 4096, unsigned long quality = 95)
{
	CLSID             encoderClsid;
	EncoderParameters encoderParameters;
	Status            stat;

	// Get an image from the disk.
	Image* imageoriginal = new Image(string2wstring(orig_fiepath, "zh-CN").c_str());

	// �Ƿ���Ҫת��
	if (imageoriginal->GetWidth() <= maxWidth && imageoriginal->GetHeight() <= maxHeight) {
		delete imageoriginal;
		return Ok;
	}

	// ����
	mkdir(backup_folder.c_str());
	copyto(orig_fiepath.c_str(), (backup_folder + "/" + get_filename(orig_fiepath)).c_str());
	
	// ���óߴ�
	int dstWidth = imageoriginal->GetWidth();
	int dstHeight = imageoriginal->GetHeight();
	if (imageoriginal->GetWidth() > imageoriginal->GetHeight())
	{
		dstWidth = maxWidth;
		dstHeight = int(float(imageoriginal->GetHeight() * maxWidth) / float(imageoriginal->GetWidth()));
	}
	else
	{
		dstHeight = maxHeight;
		dstWidth = int(float(imageoriginal->GetWidth() * maxHeight) / float(imageoriginal->GetHeight()));
	}

	// Graphics imgGraphics �Ƿ�ɹ�
	Graphics *imgGraphics = new Graphics(imageoriginal);
	if (Ok != imgGraphics->GetLastStatus())
	{
		delete imageoriginal;
		delete imgGraphics;

		Transform_to_png(orig_fiepath);

		return Compress_image_to_tmp(orig_fiepath, backup_folder, dst_tmp, maxWidth, maxHeight, quality);
	}

	Bitmap dstbitmap(dstWidth, dstHeight, imgGraphics);
	Graphics dstbmpGraphics(&dstbitmap);
	dstbmpGraphics.DrawImage(imageoriginal, 0, 0, dstWidth, dstHeight);

	// Get the CLSID of the JPEG encoder.
	GetEncoderClsid(L"image/jpeg", &encoderClsid);

	// Before we call Image::Save, we must initialize an
	// EncoderParameters object. The EncoderParameters object
	// has an array of EncoderParameter objects. In this
	// case, there is only one EncoderParameter object in the array.
	// The one EncoderParameter object has an array of values.
	// In this case, there is only one value (of type ULONG)
	// in the array. We will let this value vary from 0 to 100.

	encoderParameters.Count = 1;
	encoderParameters.Parameter[0].Guid = EncoderQuality;
	encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParameters.Parameter[0].NumberOfValues = 1;

	// Save the image as a JPEG with quality level xx.
	encoderParameters.Parameter[0].Value = &quality;

	// ���浽��ʱ·��
	stat = dstbitmap.Save(string2wstring(dst_tmp, "zh-CN").c_str(), &encoderClsid, &encoderParameters);

	delete imageoriginal;
	delete imgGraphics;
	return stat;
}

// ѹ��ͼƬ�ߴ磨�ȱȣ���������ѹ���ɹ�֮���滻Դ�ļ���Դ�ļ����ݵ�backup_folder
//  @orig_fiepath: ͼƬԴ·��
//  @backup_folder: ����·��
//  @maxWidth:  ����
//  @maxHeight: ����
//  @quality:   ѹ�������� 0-100
// ����ֵ��Status Ok��ѹ���ɹ�
// ����compress_image("../big.jpg", "../backup");
//     compress_image("../4X4������1.png", "../backup");
//     compress_image("../1.png", "../backup");
Status compress_image(std::string orig_fiepath, std::string backup_folder,unsigned int maxWidth = 1920, unsigned int maxHeight = 1920, unsigned long quality = 55)
{
	std::string dst_tmp = now(t_time2)+".compress.jpg.tmp";
	Status stat = Compress_image_to_tmp(orig_fiepath, backup_folder, dst_tmp, maxWidth, maxHeight, quality);
	if (stat == Ok && exist(dst_tmp.c_str()))
		mv(dst_tmp.c_str(), orig_fiepath.c_str());
	return stat;
}


#endif