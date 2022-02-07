#pragma once
#ifndef PROXYTOH5_H_
#define PROXYTOH5_H_
#include<iostream>
using namespace std;

// Æô¶¯´úÀí
void ProxyStart_http();
void ProxyConsume_http(IN std::string path, IN std::string request, OUT std::string & replay);
#endif
