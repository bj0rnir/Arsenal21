#include "Arsenal21.h"
using namespace std;

HTML::HTML(string& str) : s(str), flags(new Flags) {
	_mainloop();
}
void HTML::_mainloop(){
	vector<string> voidElements = { "area", "base", "br", "col", "command", "embed", "hr", "img", "input", "keygen", "link", "meta", "param", "source", "track", "wbr","DOCTYPE" };
	SubStr* dquote = new SubStr(s);
	SubStr* squote = new SubStr(s);
	Elem* elem = new Elem(s);
	Attr* attr = new Attr(s);
	map<string, queue<Elem*>> openElements;
	vector<int*> flipAtLoopEnd;
	for (std::string::iterator it = s.begin(); it != s.end(); ++it) {
		size_t from_end = distance(it, s.end());
		size_t it_i = distance(s.begin(), it);
		//cout << INFO_B << it_i << " : "  << from_end << endl;
		if (*it == '\\') {
			++flags->esc;
		}
		else if (!flags->esc || flags->esc % 2 == 0)
		{
			if (*it == '<')
			{
				elem->open = it_i;
				flags->elem = 1;
			}
			else if (*it == '\"') {
				if (flags->quote == 2) {
					flipAtLoopEnd.push_back(&flags->quote);
					dquote->close = it_i;
					dquote = new SubStr(s);
				}
				else if (flags->quote != 1) {
					dquote->open = it_i;
					flags->quote = 2;
				}
			}
			else if (*it == '\'') {
				if (flags->quote == 1) {
					flipAtLoopEnd.push_back(&flags->quote);
					squote->close = it_i;
					squote = new SubStr(s);
				}
				else if (flags->quote != 2) {
					squote->open = it_i;
					flags->quote = 1;
				}
			}
			if (flags->elem) {
				if (flags->elem == 1) {
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
						++flags->elem;
					}
				}
				else if (flags->elem % 2 == 0) {
					if (isletter(*it) || is(*it, { '_', ':','.','-' }) && !attr->open) {
						attr->name.open = it_i;
						attr->open = attr->name.open;
					}
					else if (*it == '=') {
						attr->name.close = it_i - 1;
						++flags->elem;
					}
				}
				else {
					if (!attr->value.open) { (flags->quote) ? (attr->is_str = 1) : (attr->is_str = 0); attr->value.open = it_i; }
					else if (attr->is_str) {
						if (flags->quote == 2) {
							if (dquote->close) {
								attr->value.close = it_i;
								elem->attributes.push_back(*attr);
								attr = new Attr(s);
								++flags->elem;
							}
						}
						else if (flags->quote) {
							if (squote->close) {
								attr->value.close = it_i;
								elem->attributes.push_back(*attr);
								attr = new Attr(s);
								++flags->elem;
							}
						}
					}
					else {
						while (isletter(*it) || is(*it, { '_', ':','.','-' })) {
							++it;
							it_i = distance(s.begin(), it);
						}
						--it;
						it_i = distance(s.begin(), it);
						attr->value.close = it_i;
						elem->attributes.push_back(*attr);
						attr = new Attr(s);
						++flags->elem;
					}
				}
				if (*it == '/') {
					if (!elem->type.open) {
						elem->closing = 1;
					}
					else {
						elem->closing = 2;
					}
				}
				else if (*it == '>') {
					elem->close = it_i;
					this->elem_v.push_back(elem);
					elem = new Elem(s);
					flipAtLoopEnd.push_back(&flags->elem);
				}
			}
		}
		if (flags->esc) {flags->esc = 0; }
		for (std::vector<int*>::iterator it2 = flipAtLoopEnd.begin(); it2 != flipAtLoopEnd.end(); ++it2) { (**it2) ? (**it2 = 0) : (**it2 = 1); }
		flipAtLoopEnd.clear();
	}
	int ctr = 0;

	for (std::vector<Elem*>::iterator it = elem_v.begin(); it != elem_v.end(); ++it) {
		cout << ++ctr << ": " <<  (*it)->type.str() << endl;
		//printf("\nElement %i:\n\tType=%s\t", ++ctr, it->type.str());
		auto elemType = (*it)->type.str();
		//cout << (*it)->str() << endl << "Is void:\t" << (*it)->is_void << endl << "Is Closing:\t" << (*it)->closing << endl << endl;
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
					openElements[elemType].pop();
					//cout << tmp->type.str() << endl;
					if (tmp->type.str().compare("tr") == 0) {
						//cout << tmp->body->str() << endl;
						//cout << (*it)->str() << endl << "Is void:\t" << (*it)->is_void << endl << "Is Closing:\t" << (*it)->closing << endl << "Body:\t" << tmp->body->str() << endl << endl;
					}
				}
			}
		}
	}
}