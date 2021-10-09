#pragma once

#ifndef __MY_HTTP_H__
#define __MY_HTTP_H__

/************************************************************************\
* 参考：https://github.com/yhirose/cpp-httplib
* openssl下载路径：https://slproweb.com/products/Win32OpenSSL.html
* 说明：http/https  server/client
\************************************************************************/

//#define CPPHTTPLIB_OPENSSL_SUPPORT

#pragma comment(lib, "libcrypto_static.lib")
#pragma comment(lib, "libssl_static.lib")

#include "httplib.h"
using namespace httplib;

#include "myos.h"

// 例：
//auto res = easyDownload(
//	"http://smartbank.jsruiyin.com:8081"
//	, "/release-web"
//	, "/upload/image/admin/20200817/744961164265390080.png"
//	, "111111111111111111.png"
//  , true
//  , [=](uint64_t len, uint64_t total) {
//    CString tip;
//    tip.Format("%lld / %lld bytes => %d%% complete\n",
//    	len, total,
//    	(int)(len * 100 / total));
//    return true;
//	}
//);

inline
auto easyDownload(std::string host, std::string postpath, std::string url, std::string localfilepath, httplib::Progress progress)
{
	std::string path = postpath.append(url);
	std::string downloading = localfilepath + ".download";
	fstream fi;
	fi.open(downloading.c_str(), fstream::binary | fstream::out | fstream::app);

	httplib::Client cli(host.c_str());
	cli.set_read_timeout(30);
	auto res = cli.Get(easytoUTF(path).c_str()
		, [&](const char *data, size_t data_length) {
		// 实时下载数据
		fi.write(data, data_length);
		return true;
	}
		, progress
		);
	fi.close();
	if (res.error() == Success) std::rename(downloading.c_str(), localfilepath.c_str());
	return res;
}

inline
auto easyDownload(std::string host, std::string postpath, std::string url, std::string localfilepath)
{
	auto res = easyDownload(host, postpath, url, localfilepath,
		[&](uint64_t len, uint64_t total) {
		// 实时显示下载进度
		printf("%lld / %lld bytes => %d%% complete\n",
			len, total,
			(int)(len * 100 / total));
		return true;
	});
	return res;
}

// 例：
//std::string strlocal;
//auto res = easyDownload(
//	"http://smartbank.jsruiyin.com:8081"
//	, "/release-web"
//	, "/upload/image/admin/20200817/744961164265390080.png"
//	, get_fullpath("tmp").GetBuffer(0)
//	, strlocal
//);
inline
auto easyDownload(std::string host, std::string postpath, std::string url, std::string localfolder, std::string& localfilepath, bool boverwrite = false)
{
	mkdir(localfolder.c_str());
	localfilepath.append(localfolder);
	localfilepath.append("/");
	localfilepath.append(get_filename(url));
	if (boverwrite == false && exist(localfilepath.c_str()) == true)
	{
		auto res = std::make_shared<httplib::Response>();
		res->status = 200;
		return httplib::Result{ res , httplib::Error::Success };
	}
	return easyDownload(host, postpath, url, localfilepath);
}

inline
auto easyDownload(std::string host, std::string postpath, std::string url, std::string localfolder, std::string& localfilepath, bool boverwrite, httplib::Progress progress)
{
	mkdir(localfolder.c_str());
	localfilepath.append(localfolder);
	localfilepath.append("/");
	localfilepath.append(get_filename(url));
	if (boverwrite == false && exist(localfilepath.c_str()) == true)
	{
		auto res = std::make_shared<httplib::Response>();
		res->status = 200;
		return httplib::Result{ res , httplib::Error::Success };
	}
	return easyDownload(host, postpath, url, localfilepath, progress);
}

#endif