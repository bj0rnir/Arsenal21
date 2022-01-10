#include "Arsenal21.h"
std::wstring getWideString(const char* s) {
	std::wstring ws;
	ws.resize(strlen(s));
	MultiByteToWideChar(CP_UTF8, 0, s, strlen(s), &ws[0], ws.size());
	return ws;
}
float USD2Float(std::string& s) {
	std::remove(s.begin(), s.end(), '$');
	std::remove(s.begin(), s.end(), '\"');
	std::remove(s.begin(), s.end(), '\'');
	std::remove(s.begin(), s.end(), ' ');
	return stof(s);
}