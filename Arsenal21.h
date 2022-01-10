#pragma once
#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <regex>
#include <iterator>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <codecvt>
#include <random>
#include <stdexcept>
#include <cctype>
#include <queue>

#include <curl/curl.h>

#include <WinCrypt.h>
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_BLUE "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RESET "\x1b[0m"
#define INFO_B "\x1b[0m[\x1b[34m+\x1b[0m] "
#define SUCCESS_B "\x1b[0m[\x1b[32m+\x1b[0m] "
#define ERROR_B "\x1b[0m[\x1b[31m+\x1b[0m] "
#define BUFSIZE 1024
#define MD5LEN  16
#define UTF_2 0b10000000

struct tr {
	size_t open;
	size_t size;
};
class SubStr {
public:
	std::string* base;
	size_t open;
	size_t close;
	SubStr() : base(NULL) {};
	SubStr(std::string& s) : base(&s) {};
	std::string str(int trim = 0) {
		if (trim) { return base->substr(open + trim, (close - trim - open)); }
		else { return base->substr(open, (close - open) + 1); }

	}
};
class Attr : public SubStr {
public:
	SubStr name;
	SubStr value;
	bool is_str;
	Attr(std::string& s) : SubStr{ s }, name{ s }, value{s} {}
};
class Elem : public SubStr {
public:
	int closing;
	SubStr type;
	std::vector<Attr> attributes;
	Elem* closer = NULL;
	SubStr* body = NULL;
	bool is_void;
	Elem() {};
	Elem(std::string& s) : SubStr{ s }, type(s) {};
};
bool getMD5ofFile(const char*, std::string&);
std::wstring getWideString(const char*);
uint32_t randnum();
size_t getFileSize(const char*);
std::string readFileToString(const char* file);
void timestamp();
float USD2Float(std::string& s);
bool is(char c, std::regex& r);
bool isletter(char);
bool is(char, std::vector<char>);

class HTML {
	std::vector<Elem*> elem_v;
	std::string& s;
	void _mainloop();
	struct Flags {
		int doctype = 0;
		int elem = 0;
		int attr = 0;
		int quote = 0;
		int esc = 0;
	} *flags;
public:
	HTML(std::string&);

};