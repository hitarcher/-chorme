#pragma once
#ifndef BASE_64_H
#define BASE_64_H
#include <string>
std::string _base64_encode(unsigned char const*, unsigned int len);
unsigned char* _base64_decode(std::string const& encoded_string);
#endif

