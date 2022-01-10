#include "Arsenal21.h"
using namespace std;
int deleteDupFiles(const char* dir = (const char*)std::filesystem::current_path().string().c_str()) {
	std::filesystem::path pat(dir);
	map<string, string> MD5Table;
	for (auto it : std::filesystem::directory_iterator(pat)) {
		std::string md5;
		if (it.is_regular_file()) {
			getMD5ofFile(it.path().string().c_str(), md5);
			pair<string, string> pr(md5, pat.string());
			pair<std::map<string,string>::iterator, bool> response=MD5Table.insert(pair<string, string>{md5, pat.string()});
			if (response.second == false) {
				cout << "Duplicate File Detected - " << it.path().filename().string() << ":" << md5 << endl;
				std::error_code err;
				if (int ret = std::filesystem::remove(it.path(), err) != 0) {cout << err << endl;}
			}
		}
	}
	return 1;
}


namespace CURLspace {
	map<string, string> MD5Table;
	vector<string> filesWrittenTo;
	ofstream f;
	int response_counter = 0;
	int page_counter = 0;
	int USProxies = 0;
	std::ofstream responses_f;

	void cleanup() {
		f.close();
	}
	void delete_dup_responses() {
		for (vector<string>::iterator it = filesWrittenTo.begin(); it != filesWrittenTo.end(); ++it) {
			std::string md5;
			getMD5ofFile(it->c_str(), md5);
			pair<string, string> p(md5, *it);
			MD5Table.insert(p);
			cout << p.second << endl;
			if (p.second.compare("false") == 0) {
				cout << "Duplicate File Detected: " << *it << "\tMD5: " << md5 << endl;
				std::filesystem::remove(std::filesystem::current_path().append(*it));
			}
		}
	}
	int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
		switch (type) {
		case CURLINFO_HEADER_OUT:
			std::cout << "=> Send Header" << std::endl;
			break;
		case CURLINFO_DATA_OUT:
			std::cout << "=> Send data" << std::endl;
			break;
		case CURLINFO_SSL_DATA_OUT:
			std::cout << "=> Send SSL data" << std::endl;
			break;
		case CURLINFO_HEADER_IN:
			std::cout << "=> Recv header" << std::endl;
			break;
		case CURLINFO_DATA_IN:
			std::cout << "=> Recv data" << std::endl;
			break;
		case CURLINFO_SSL_DATA_IN:
			std::cout << "=> Recv SSL data" << std::endl;
			break;
		}
		printf("%.*s", size, data);
		return 0;
	}
	int write_callback(char* data, size_t size, size_t nmemb, std::string* writerData) {
		string data_s(data);
		if (!USProxies) {
			std::smatch m;
			std::regex r("(United States [(])([0-9]+)");
			if (std::regex_search(data_s, m, r)) {
				//std::cout << m[2] << std::endl;
				USProxies = std::stoi(std::string(m[2]), NULL, 10);
			}
		}
		if (!f.is_open()) {
			string filename("response.txt");
			filename.insert(8, to_string(++page_counter));
			f.open(filename, ios::trunc);
			filesWrittenTo.push_back(filename);
		}
		else if (size_t page = data_s.find("<!DOCTYPE") != string::npos) {
			f.close();
			string filename("response.txt");
			filename.insert(8, to_string(++page_counter));
			f.open(filename, ios::trunc);
			filesWrittenTo.push_back(filename);
		}
		f.write(data, size * nmemb);
		//std::cout << data_s << std::endl;
		return size * nmemb;
	}
	void curlopt_https_pop(CURL* curl, bool dbg = 0) {
		CURLcode code;
		struct curl_slist* list = NULL;
		list = curl_slist_append(list, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
		list = curl_slist_append(list, "accept-language: en-US;en;q=0.9");
		code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		if (dbg) {
			code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
			code = curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
		}
		/*curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, [](char* buffer, size_t size, size_t nitems, void* userdata)->size_t{
			std::cout << buffer << std::endl;
			return size * nitems;
		});*/

	}
	void shandler(char& c) {
		static std::string de("<tr><td>");
		static bool flag;
		for (string::iterator sti = de.begin(); sti < de.end(); ++sti) {
			/*if (*sti == c && (sti == (de.end() - 1) && ) {
			}*/
			if (*sti == c) {

			}
		}
	}
	bool trans_handler_c(char& c) {
		static bool enc = 0;
		if (c == '<') {
			enc = 1;
			return 0;
		}
		if (c == '>') { enc = 0; return 0; }
		if (enc) { return 0; }
		else { return 1; }
		return 0;
	}
	string trans_handler(string s) {
		std::string buf;
		copy_if(s.begin(), s.end(), back_inserter(buf), trans_handler_c);
		return buf;
	}
	vector<string> trfields_get(string s) {
		bool insideTR = 0;
		std::vector<std::string> tr_v;
		std::string* tr = new std::string;
		for (string::iterator si = s.begin(); si < s.end(); ++si) {
			size_t dist = std::distance(s.begin(), si);
			if (s.compare(dist, 4, "<tr>", 4) == 0) {
				insideTR = 1;
			}
			else if (s.compare(dist, 5, "</tr>", 5) == 0) {
				tr_v.push_back(*tr);
				tr = new std::string;
				insideTR = 0;
			}
			if (insideTR) { *tr += *si; }
		}
		return tr_v;
	}
	struct proxy {
		std::string country;
		std::string city;
		std::string latency;
		std::string protocol;
		std::string anonimity;
		std::string last_tested;
		std::string address;
		std::string port;
		void print() {
			cout << "Country:\s" << country << "\tCity:\s" << city << "\tLatency:\s" << latency << "\tProto\s" << protocol << "\tAnonymity:\s" << anonimity << "\sLast Checked:\s" << last_tested << "\tAddress:\s" << address << "\tPort:\s" << port << std::endl;
		}
	};
	void print(vector<string> v) {
		for (vector<string>::iterator i = v.begin(); i < v.end(); ++i) {
			cout << *i << endl;
		}
	}
	void parse_attempt(std::string s) {
		vector<string> v = trfields_get(s);
		vector<string> v2;
		for (std::vector<string>::iterator i = v.begin(); i < v.end(); ++i) {
			std::string* buf = new std::string;
			bool enc = 0;
			for (string::iterator is = i->begin(); is < i->end(); ++is) {
				if (*is == '<') {
					if (buf->length() > 0) {
						v2.push_back(*buf);
						buf = new std::string;
					}
					enc = 1;
				}
				if (!enc) {
					*buf += *is;
				}
				else if (*is == '>') { enc = 0; }
			}
		}
		vector<vector<string>::iterator> viv;
		for (vector<string>::iterator i = v2.begin(); i < v2.end();) {
			if (*i == " ") {
				i = v2.erase(i);
			}
			else { ++i; }
		}
		//print(v2);
		vector<struct proxy> v3;
		int level = 0;
		
		for_each(v3.begin(), v3.end(), [](struct proxy p)->void {p.print(); });
	}
	//vector<string> gen_urls() {
	//	static int counter = 0;
	//	std::string url("https://hidemy.name/en/proxy-list/?country=US&start=");
	//	url.append(std::to_string(counter++)).append("#list");
	//	urls.push_back(url);
	//}
#define MAX_PARALLEL 10
	void getAllUsProxies_multicurl() {

		CURL* curl = curl_easy_init();
		CURLcode code;
		std::string response;
		curl_easy_setopt(curl, CURLOPT_URL, "https://hidemy.name/en/proxy-list/?country=US#list");
		//curl_easy_setopt(curl, CURLOPT_URL, "https://github.com/aria2/aria2/releases/tag/release-1.36.0");
		curlopt_https_pop(curl, 1);
		curl_easy_perform(curl);
		if (!USProxies) {
			return;
		}
		CURLM* cm;
		CURLMsg* msg;
		int still_alive = 1;
		int msgs_left = -1;
		cm = curl_multi_init();
		curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);
		std::vector<std::string> urls;
		for (int i = 0; i < USProxies; i += 64) {
			std::string url("https://hidemy.name/en/proxy-list/?country=US&start=");
			url.append(std::to_string(i)).append("#list");
			urls.push_back(url);
		}
		int transfers = 0;
		vector<CURL*> cvec;
		for (std::vector<std::string>::iterator it = urls.begin(); it != urls.end(); ++it) {
			//(!transfers) ? ((urls.size() < 10) ? (transfers = urls.size()) : (transfers = 10)) : (NULL);
			CURL* c = curl_easy_init();
			curl_easy_setopt(c, CURLOPT_URL, it->c_str());
			curlopt_https_pop(c, 1);
			cvec.push_back(c);
			if (!transfers) { (urls.size() < 10) ? (transfers = urls.size()) : (transfers = 10); }
			if (it - urls.begin() < 9) { curl_multi_add_handle(cm, c); }
		}
		if (urls.size() >= 10) {
			do {
				curl_multi_perform(cm, &still_alive);
				while ((msg = curl_multi_info_read(cm, &msgs_left))) {
					if (msg->msg == CURLMSG_DONE) {
						char* url;
						CURL* e = msg->easy_handle;
						curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
						fprintf(stderr, "R: %d - %s <%s>\n",
							msg->data.result, curl_easy_strerror(msg->data.result), url);
						curl_multi_remove_handle(cm, e);
						curl_easy_cleanup(e);
					}
					else {
						fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
					}
					if (transfers < urls.size()) {
						curl_multi_add_handle(cm, cvec[transfers++]);
					}
					/*if (still_alive)
						curl_multi_wait(cm, NULL, 0, 100, NULL);*/
				}
			} while (transfers < urls.size() || still_alive);
		}
		else {
			while ((msg = curl_multi_info_read(cm, &msgs_left))) {
				if (msg->msg == CURLMSG_DONE) {
					char* url;
					CURL* e = msg->easy_handle;
					curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
					fprintf(stderr, "R: %d - %s <%s>\n",
						msg->data.result, curl_easy_strerror(msg->data.result), url);
					curl_multi_remove_handle(cm, e);
					curl_easy_cleanup(e);
				}
				else {
					fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
				}
			}
		}
		cleanup();
		curl_easy_cleanup(curl);
		curl_multi_cleanup(cm);
	}
	void getAllUsProxies() {

		CURL* curl = curl_easy_init();
		CURLcode code;
		std::string response;
		curl_easy_setopt(curl, CURLOPT_URL, "https://hidemy.name/en/proxy-list/?country=US#list");
		//curl_easy_setopt(curl, CURLOPT_URL, "https://github.com/aria2/aria2/releases/tag/release-1.36.0");
		curlopt_https_pop(curl, 1);
		curl_easy_perform(curl);
		if (!USProxies) {
			return;
		}
		std::vector<std::string> urls;
		for (int i = 0; i < USProxies; i += 64) {
			std::string url("https://hidemy.name/en/proxy-list/?country=US&start=");
			url.append(std::to_string(i)).append("#list");
			urls.push_back(url);
		}
		int transfers = 0;
		vector<CURL*> cvec;
		for (int i = 0; i < USProxies; i += 64) {
			//(!transfers) ? ((urls.size() < 10) ? (transfers = urls.size()) : (transfers = 10)) : (NULL);
			std::string url("https://hidemy.name/en/proxy-list/?country=US&start=");
			url.append(std::to_string(i)).append("#list");
			CURL* c = curl_easy_init();
			curl_easy_setopt(c, CURLOPT_URL, url.c_str());
			curlopt_https_pop(c, 1);
			cvec.push_back(c);
		}
		for (std::vector<CURL*>::iterator it = cvec.begin(); it != cvec.end(); ++it) {
			curl_easy_perform(*it);
		}
		for (std::vector<CURL*>::iterator it = cvec.begin(); it != cvec.end(); ++it) {
			curl_easy_cleanup(*it);
		}
	}
};
namespace XMLParsing {
#define ERR_HTML_STRING_LEFT_UNCLOSED -2
#define SUCCESS 1
	string HTMLCommentOpenTag("<!--");
	string HTMLCommentCloseTag("-->");
	string HTMLStringDoubleQ("\"");
	string HTMLStringSingleQ("\'");
	
	inline bool escaped(string& s, size_t loc) {
		size_t it = loc;
		int esc_counter = 0;
		char c;
		try {
			c = s.at(--it);
			while (c == '\\') {
				++esc_counter;
				c = s.at(--it);
			}
		}
		catch (out_of_range& e) {NULL;}
		if (esc_counter % 2 == 0)
			return false;
		else return true;
	}
	inline bool openclose_element(string& s, size_t c) {
		char it = s.at(c - 1);
		while (it == ' ' || it == '\t') {
			it = s.at(it - 1);
			if (it == '/') {
				return true;
			}
		}

	}
	inline bool enclosed(string& s, size_t loc, vector<SubStr> tags) {
		for (vector<SubStr>::iterator it = tags.begin(); it != tags.end(); ++it) {
			if ((loc <= it->close) && (loc >= it->open)) {
				return true;
			}
		}
		return false;
	}
	void parse() {
		vector<string> voidElements = { "area", "base", "br", "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr","DOCTYPE"};
		//string s("\\\\\"IamA\\\\\\\"TestS\"tringAIAmATe\"stStri\"ng");
		//size_t t = test.find('\\');
		//cout << test[t] << endl;
		regex HTMLAttrNameValidChars("[-a-zA-Z0-9_:.]");
		regex HTMLAttrNameValidFirstChars("[a-zA-Z_:]");
		string s = readFileToString("C:\\Users\\parse\\source\\repos\\Arsenal21\\firstpage_response.txt");
		size_t it = s.find('\"');
		vector<SubStr> strings;
		//cout << "\033[s";
		int esc = 0;
		int quote_f = 0;
		vector<SubStr> dquote_v;
		SubStr* dquote = new SubStr(s);
		SubStr* squote = new SubStr(s);
		Elem* elem = new Elem(s);
		Attr* attr = new Attr(s);
		vector<Elem*> elem_v;
		bool squote_f=0;
		bool script = 0;
		int attr_f = 0;
		int elem_i = 0;
		int counter = 0;
		map<string, queue<Elem*>> openElements;
		vector<int*> flipAtLoopEnd;
		for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
			size_t from_end = distance(it, s.end());
			size_t it_i = distance(s.begin(), it);
			//cout << INFO_B << it_i << " : "  << from_end << endl;
			if (*it == '\\') {
				++esc;
			}else if (!esc || esc % 2 == 0)
			{
				if (*it == '<' && !quote_f)
				{
					elem->open = it_i;
					elem_i = 1;
				} else if (*it == '\"') {
					if (quote_f == 2) {
						flipAtLoopEnd.push_back(&quote_f);
						dquote->close = it_i;
						dquote = new SubStr(s);
					}
					else if (quote_f != 1) {
						dquote->open = it_i;
						quote_f = 2;
					}
				}
				else if (*it == '\'') {
					if (quote_f == 1) {
						flipAtLoopEnd.push_back(&quote_f);
						squote->close = it_i;
						squote = new SubStr(s);
					}
					else if (quote_f != 2) {
						squote->open = it_i;
						quote_f = 1;
					}
				}
				if (elem_i) {
					if (elem_i == 1) {
						if (isalnum(*it)) {
							elem->type.open = it_i;
							while (isalnum(*it)) {
								++it;
								it_i = distance(s.begin(), it);
							}
							--it;
							it_i = distance(s.begin(), it);
							elem->type.close = it_i;
							string type_tmp = elem->type.str();
							//cout << type_tmp << ' ';
							for (vector<string>::iterator it2 = voidElements.begin(); it2 != voidElements.end(); ++it2) {
								if (it2->compare(type_tmp) == 0) { elem->is_void = 1; elem->closing = 2; }
							}
							++elem_i;
						}
					} else if (elem_i % 2 == 0){
						if (isletter(*it) || is(*it, { '_', ':','.','-' }) && !attr->open) {
							attr->name.open = it_i;
							attr->open = attr->name.open;
						} else if (*it == '=') {
								attr->name.close = it_i - 1;
								++elem_i;
						}
					} else {
						if (!attr->value.open) { (quote_f) ? (attr->is_str = 1) : (attr->is_str = 0); attr->value.open = it_i; }
						else if (attr->is_str) {
							if (quote_f == 2) {
								if (dquote->close) {
									attr->value.close = it_i;
									elem->attributes.push_back(*attr);
									attr = new Attr(s);
									++elem_i;
								}
							}
							else if (quote_f) {
								if (squote->close) {
									attr->value.close = it_i;
									elem->attributes.push_back(*attr);
									attr = new Attr(s);
									++elem_i;
								}
							}
						}
						else {
							while (isletter(*it) || is(*it, {'_', ':','.','-'})) {
								++it;
								it_i = distance(s.begin(), it);
							}
							--it;
							it_i = distance(s.begin(), it);
							attr->value.close = it_i;
							elem->attributes.push_back(*attr);
							attr = new Attr(s);
							++elem_i;
						}
					}
					if (*it == '/' && !quote_f) {
						if (!elem->type.open) {
							elem->closing = 1;
						}
						else {
							elem->closing = 2;
						}
					} else if (*it == '>' && !quote_f) {
						elem->close = it_i;
						elem_v.push_back(elem);
						elem = new Elem(s);
						flipAtLoopEnd.push_back(&elem_i);
					}
				}
			}
			if (esc) { esc = 0; }
			for (std::vector<int*>::iterator it2 = flipAtLoopEnd.begin(); it2 != flipAtLoopEnd.end(); ++it2){(**it2) ? (**it2 = 0) : (**it2 = 1);}
			flipAtLoopEnd.clear();
		}
		int ctr = 0;

		for (std::vector<Elem*>::iterator it = elem_v.begin(); it != elem_v.end(); ++it) {
			//printf("\nElement %i:\n\tType=%s\t", ++ctr, it->type.str());
			auto elemType = (*it)->type.str();
			cout << (*it)->str() << endl << "Is void:\t" << (*it)->is_void << endl << "Is Closing:\t" << (*it)->closing << endl << endl;
			if (!(*it)->closing) {
				openElements[(*it)->type.str()].push(*it);
			}
			else if ((*it)->closing == 1) {
				if ((openElements.find(elemType) != openElements.end()) && openElements[elemType].size()) {
					//(*it)->closer
					Elem* tmp;
					try {
						tmp = openElements[elemType].front();
					}
					catch (exception& e) { cout << e.what() << endl; return; }
					if (tmp != NULL) {
						tmp->closer = *it;
						tmp->body = new SubStr(s);
						tmp->body->open = tmp->close + 1;
						tmp->body->close = (*it)->open - 1;
						cout << tmp->body->str() << endl;
						openElements[elemType].pop();
						cout << (*it)->str() << endl << "Is void:\t" << (*it)->is_void << endl << "Is Closing:\t" << (*it)->closing << endl << "Body:\t" << tmp->body->str() << endl << endl;
					}
				}
			}
			//cout << it->str() << endl;
		}
		for (std::vector<Elem*>::iterator it = elem_v.begin(); it != elem_v.end(); ++it) {

		}
	}
	vector<tr> get_enclosed(string& s, const char* o, const char* c, bool verbose = 0) {
		vector<tr> v;
		for (size_t open = s.find(o); open != string::npos; open = s.find(o, open)) {
			size_t close = s.find(c, open);
			v.push_back({ open + strlen(o), close - open - strlen(o) });
			if (verbose) {
				cout << s.substr(open + strlen(o), close - open - strlen(o)) << endl;
			}
			open = close;

		}
		return v;
	}
	void parse_attempt_3() {
		string s = readFileToString("responses.txt");
		vector<tr> trv = get_enclosed(s, "<tr>", "</tr>");
		vector<vector<string>> tdv2;
		int counter = 1;
		for (vector<tr>::iterator it = trv.begin(); it < trv.end(); ++it) {
			string trf = s.substr(it->open, it->size);
			//cout << trf << endl;
			vector<tr> tdv = get_enclosed(trf, "<td>", "</td>");
			for (vector<tr>::iterator itt = tdv.begin(); itt < tdv.end(); ++itt) {
				cout << trf.substr(itt->open, itt->size) << endl;
			}
			cout << endl << endl;
		}
	}

};

//void get_strings(string& s) {
//	bool flag = 0;
//	vector<size_t, size_t> v;
//	size_t* o = new size_t, * c = new size_t;
//	for (string::iterator it = s.begin(); it < s.end(); ++it) {
//		if (*it == '"') {
//			if (flag) {
//				flag = 0;
//				*c = distance(s.begin(), it);
//				v.push_back((*o, *c));
//				delete o, c;
//				o = new size_t;
//				c = new size_t;
//			}
//			else {
//				flag = 1;
//				*o = distance(s.begin(), it);
//			}
//		}
//	}
//	for_each(v.begin(), v.end(), [](size_t open, size_t close) {
//		});
//}

void parse_primes() {
	
	string s = readFileToString("primes.txt");
	
	for (string::iterator it = s.begin(); it < s.end();) {
		if (*it == ' ') { s.erase(it); }
		else if (*it == ',') {
			s.erase(it);
		}
		else {
			++it;
		}
	}
	cout << s << endl;
}
ofstream f;
int	curlhandle_getfirstpage(char* data, size_t size, size_t nmemb, std::string* writerData) {
	f.write(data, size * nmemb);
	return size * nmemb;
}
void getFirstPage() {

	f.open("firstpage_response.txt", ios::out);
	CURL* curl = curl_easy_init();
	CURLcode code;
	struct curl_slist* list = NULL;
	list = curl_slist_append(list, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
	list = curl_slist_append(list, "accept-language: en-US;en;q=0.9");
	code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	code = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlhandle_getfirstpage);
	code = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	//code = curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CURLspace::debug_callback);
	curl_easy_setopt(curl, CURLOPT_URL, "https://hidemy.name/en/proxy-list/?country=US#list");
	//curl_easy_setopt(curl, CURLOPT_URL, "https://github.com/aria2/aria2/releases/tag/release-1.36.0");
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	f.close();
}
namespace CSV {
	void parse(string& s) {
		vector<SubStr> cols;
		vector<SubStr>* rows = new vector<SubStr>;
		multimap<string, SubStr> rows_m;
		map<string, vector<SubStr>> rows_map;
		
		SubStr* col = new SubStr(s);
		bool firstline = 1;
		bool colFlag = 0;
		int colCounter = 0;
		for (string::iterator it = s.begin(); it != s.end(); ++it) {
			if (firstline) {
				if (isalnum(*it)) {
					if (!colFlag) {
						col->open = distance(s.begin(), it);
						colFlag = 1;
					}
				}
				else if ((*it == ',' && colFlag) || *it == '\n') {
					col->close = distance(s.begin(), it - 1);
					cols.push_back(*col);
					col = new SubStr(s);
					colFlag = 0;
				}

			}
			else {
				if (*it != ',') {
					if (*it == '\n') {
						col->close = distance(s.begin(), it - 1);
						rows_m.insert(pair<string, SubStr>(cols[colCounter - 1].str(), *col));
						col = new SubStr(s);
						colFlag = 0;
					}
					else if (!colFlag) {
						
						col->open = distance(s.begin(), it);
						++colCounter;
						colFlag = 1;
					}
				}
				else if (colFlag) {
					col->close = distance(s.begin(), it - 1);
					rows_m.insert(pair<string, SubStr>(cols[colCounter - 1].str(), *col));
					string t = col->str();
					col = new SubStr(s);
					colFlag = 0;
					
				}
			}
			if (firstline && *it == '\n') {
				firstline = 0;
				col = new SubStr(s);
			}
			else if (*it == '\n') {
				colCounter = 0;
			}
		}
		//std::pair<std::multimap<std::string, SubStr>::iterator, std::multimap<std::string, SubStr>::iterator>
		cout << cols[0].str() << endl;
		for (vector<SubStr>::iterator it = cols.begin(); it != cols.end(); ++it) {
			cout << it->str() << endl;
		}
		auto col1 = rows_m.find(string("Price in Cents"));
		auto col2 = rows_m.find(string("Item Name"));
		cout << setprecision(6) << endl;
		map<float, int> prices;
		while (col1->first.compare("Price in Cents") == 0) {
			if (col2->second.str().find("Operation Riptide") != string::npos){
				string price = col1->second.str(1);
				cout << col2->second.str() << endl;
				//remove(price.begin(), price.end(), '"');
				//remove(price.begin(), price.end(), '\"');
				uint32_t pricei = stoi(price);
				++prices[pricei];
			}
			//cout << col1->second.str() << endl;
			++col1;
			++col2;
		}
		for (map<float, int>::iterator it = prices.begin(); it != prices.end(); ++it) {
			cout << it->first << " : " << it->second << endl;
		}
		//cout << s << endl;
	}
};
int main()
{
	cout << 1 % 3 << endl;
	int k = 2 % 3 + 5 % 3;
	cout << k << endl;
	int x = 111'111'111;
	cout << x << endl;
	cout << "Hello World!" << endl;
	return 0;
	//
	//curl_global_init(CURL_GLOBAL_ALL);
	//CURLspace::getAllUsProxies();
	//CURLspace::delete_dup_responses();
	//deleteDupFiles();
	//XMLParsing::parse();
	//auto s = readFileToString("market_history.csv");
	//CSV::parse(s);
	//XMLParsing::parse();
	//parse_primes();
	auto s =readFileToString("C:\\Users\\parse\\source\\repos\\Arsenal21\\firstpage_response.txt");
	HTML h(s);
	return 0;
}
/*for (vector<tr>::iterator it = trv.begin(); it < trv.end(); ++it) {
		string trf = s.substr(it->open, it->size);
		vector<tr> tdv = get_enclosed(trf, "<td>", "</td>");
		cout << "Proxy " << counter << ":\n";
			cout << tdv.size() << endl;
		for (vector<tr>::iterator it2 = tdv.begin(); it2 < tdv.end(); ++it2) {
			size_t dist = std::distance(tdv.begin(), it2);
			string ss = trf.substr(it2->open, it2->size);
			if (dist == 0) {
				std::cout << "\t" << trf.substr(it2->open, it2->size) << endl;
			}
		}
		counter++;
	}*/
	//string field = s.substr(open, (close + 4) - open);
//void functionality_demo() {
//	std::string* s;
//	s = new std::string("Example string");
//	*s += '\x3f';
//	std::cout << *s;
//}
//void parse_attempt_1() {
// 	std::string s = readFileToString("responses.txt");
// 	string::iterator si = s.begin(), si_end = s.end();
// 	bool insideQuotes = 0;
// 	bool insideTR = 0;
// 	std::vector<std::string> strings;
// 	std::vector<std::string> tr_fields;
// 	std::string* sbuf = new std::string;
// 	std::string* tr_buf = new std::string;
// 	clockOps();
// 	int counter = 0;
// 	while (si != si_end) {
// 		size_t dist = std::distance(s.begin(), si);
// 		if (*si == '\x22' && (*(si - 1) != '\x5c')) {
// 			if (insideQuotes) {
// 				insideQuotes = 0;
// 				*sbuf += *si;
// 				strings.push_back(*sbuf);
// 				sbuf = new std::string;
// 			}
// 			else {
// 				insideQuotes = 1;
// 			}
// 		}
// 		if (insideQuotes) {
// 			*sbuf += *si;
// 		}
// 		if (s.compare(dist, 4, "<tr>", 4) == 0) {
// 			insideTR = 1;
// 		}
// 		else if (s.compare(dist, 5, "</tr>", 5) == 0) {
// 			tr_fields.push_back(*tr_buf);
// 			tr_buf = new std::string;
// 			insideTR = 0;
// 		}
// 		if (insideTR) {
// 			*tr_buf += *si;
// 		}
// 		++counter;
// 		++si;

// 		//auto vhandler = [](std::string s) {for_each(s.begin(), s.end(), shandler); };
// 		//std::for_each(tr_fields.begin(), tr_fields.end(), handler);
// 		//static std::string de("<tr><td>");

// 		clockOps();
// 		std::cout << counter << std::endl;
// 	}
// }
//void regex_attempt() {
// 	std::string s = readFileToString("responses.txt");
// 	regex r("(?<!\\)\"(\\\"|^[\"])+\"");
// 	//regex r2("(^[\"])+");
// 	sregex_iterator it(s.begin(), s.end(), r), end;
// 	while (it != end) {
// 		cout << (*it)[2] << endl;
// 		++it;
// 	}
//     cout << getFileSize("responses.txt") << endl;
// }
// void handler(std::string s) {

// }
/*for (vector<string>::iterator i = v2.begin(); i < v2.end(); ++i) {
			if (i->contains("United States")) {
				proxy p;
				p.country = "United States";
				regex r("[0-9]+\sms");
				p.print();
				if (regex_match(*(i + 1), r)) {
					p.latency = *(i + 1);
					i += 2;
				}
				else {
					p.city = *(i + 1);
					p.latency = *(i + 2);
					i += 3;
				}
				if (i->contains("HTTP") || i->contains("HTTPS") || i->contains("SOCKS4") || i->contains("SOCKS5")) {
					p.protocol = *i;
					++i;
					p.anonimity = *i;
					++i;
					p.last_tested = *i;
					++i;
					p.address = *i;
					++i;
					p.port = *i;
					v3.push_back(p);
					p.print();
				}
			}
		}*/
		//	while (it != string::npos) {
				//		
				//		if (!escaped(s, it)) {
				//			size_t holder = it;
				//			it = s.find('\"', ++it);
				//			if (it == string::npos) { return ERR_HTML_STRING_LEFT_UNCLOSED; }
				//			while (it != string::npos) {
				//				if (!escaped(s, it)) {
				//					//cout << s.substr(holder, (it - holder) + 1) << endl;
				//					strings.push_back({holder, it});
				//					break;
				//				}
				//				else { it = s.find('\"', ++it); }

				//			}
				//		}
				//		it = s.find('\"', ++it);
				//	}
				//	it = s.find('<');
				//	vector<Tag> elements;
				//	while (it != string::npos) {
				//		if (!escaped(s, it) && !enclosed(s, it, strings)) {
				//			size_t holder = it;
				//			it = s.find('>', ++it);
				//			while (it != string::npos) {
				//				if (!escaped(s, it)) {
				//					if (it == holder) { cout << holder << endl; }
				//					else { cout << "\033[u" << "\033[K" << holder; }
				//					//elements.push_back({ holder, it });
				//					break;
				//				}
				//				it = s.find('>', ++it);
				//			}
				//		}
				//		it = s.find('<', ++it);
				//	}
				//}
/*for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
	size_t from_end = distance(it, s.end());
	size_t it_i = distance(s.begin(), it);
	//cout << INFO_B << it_i << " : "  << from_end << endl;
	if (*it == '\\') {
		++esc;
	}
	else if (!esc || esc % 2 == 0)
	{
		if (*it == '<' && !dquote)
		{
			elem->open = it_i;
			elem_i = 1;
		}
		else if (*it == '\"') {
			if (dquote) {
				dquote = 0;
				dquote_t->close = it_i;
				dquote_v.push_back(*dquote_t);
				if (attr_f && attr_f % 2 == 0 && attr->is_str) {
					attr->value = *dquote_t;
				}
				dquote_t = new SubStr(s);
			}
			else {
				dquote_t->open = it_i;
				dquote = 1;
			}
		}
		if (elem_i && !script) {
			if (elem_i == 1) {
				if (isalnum(*it)) {
					if (!elem->type.open) {
						elem->type.open = it_i;
					}
				}
				else if (elem->type.open) {
					elem->type.close = it_i - 1;
					elem_i = 2;
				}
			}
			else if (elem_i % 2 == 0) {
				if (is(*it, HTMLAttrNameValidFirstChars) && !attr->open) {
					attr->name.open = it_i;
					attr->open = attr->name.open;
				}
				else if (*it == '=') {
					attr->name.close = it_i - 1;
					++elem_i;
				}
			}
			else {
				static bool still = 0;
				if (dquote && !still) {
					attr->open - it_i;
					still = 1;
				}
				else if (!dquote && still) {
					attr->close = it_i;
					attr->value = *dquote_t;
					//cout << attr->str() << endl;
					elem->attributes.push_back(*attr);
					still = 0;
					attr = new Attr(s);
					++elem_i;
				}
				else if (!still) {
					if (!attr->value.open) {
						attr->value.open = it_i;
					}
					else {
						if (!is(*it, HTMLAttrNameValidChars)) {
							attr->value.close = it_i - 1;
							elem->attributes.push_back(*attr);
							attr = new Attr(s);
							++elem_i;
						}
					}
				}
			}
			if (*it == '/' && !dquote) {
				if (!elem->type.open) {
					elem->closing = 1;
				}
				else {
					elem->closing = 2;
				}
			}
			else if (*it == '>' && !dquote && !squote) {
				//if (s.compare(elem->type.open, 6, "script") == 0 && !elem->closing) {
				//	//cout << SUCCESS_B << elem->str() << endl;
				//	//cout << ERROR_B << endl;
				//	//cout << "Caught a script" << std::endl;
				//	script = 1;
				//}
				elem->close = it_i;
				//cout << s.substr(elem->open, (elem->close - elem->open) + 1) << endl;
				elem_v.push_back(*elem);
				elem = new Elem(s);
				elem_i = 0;
			}
		}
		else if (script && !dquote) {
			if (*it == '<') {
				size_t openbracket = distance(s.begin(), it);
				std::cout << "Caught a script" << std::endl;
				++it;
				it_i = distance(s.begin(), it);
				while (isspace(*it)) {
					++it;
					it_i = distance(s.begin(), it);
				}
				if (*it == '/') {
					if (s.compare(it_i, 7, "/script") == 0) {
						++it;
						it_i = distance(s.begin(), it);
						elem->type.open = it_i;
						elem->type.close = distance(s.begin(), it + 5);
						elem->open = openbracket;
						while (*it != '>') {
							++it;
							it_i = distance(s.begin(), it);
						}
						elem->close = distance(s.begin(), it);
						elem_v.push_back(*elem);
						elem = new Elem(s);
						script = 0;
					}
				}
			}
		}
	}
	if (esc) { esc = 0; }
}*/
//static bool holder = 0;
//if (dquote && !still) {
//	attr->open - it_i;
//	still = 1;
//}
//else if (!dquote && still) {
//	attr->close = it_i;
//	attr->value = *dquote;
//	//cout << attr->str() << endl;
//	elem->attributes.push_back(*attr);
//	still = 0;
//	attr = new Attr(s);
//	++elem_i;
//}
//else if (!still) {
//	if (!attr->value.open) {
//		attr->value.open = it_i;
//	}
//	else {
//		if (!is(*it, HTMLAttrNameValidChars)) {
//			attr->value.close = it_i - 1;
//			elem->attributes.push_back(*attr);
//			attr = new Attr(s);
//			++elem_i;
//		}
//	}
//}