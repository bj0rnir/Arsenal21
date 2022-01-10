#include "Arsenal21.h"
bool is(char c, std::regex& r) {
	char cs[2] = { c, '\0' };
	std::cmatch cm;
	bool b = std::regex_match(cs, cm, r);
	return std::regex_match(cs, cm, r);
}


bool isletter(char c) {if(64<c<91){return true;}else if(96<c<123){return true;}return false;}
bool is(char c,std::vector<char> v){for(std::vector<char>::iterator it=v.begin();it!=v.end();++it){if(c==*it)return true;}return false;}
