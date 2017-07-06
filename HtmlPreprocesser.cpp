
#include <cctype>
#include "HtmlPreprocesser.h"

using namespace std;

// get title between <title ...> ... </title ...> in the html contentStr.
void HtmlPreprocesser::getTitle(const string &contentStr, string &title)
{
								const string titleBegTag = "<title";
								const string titleEndTag = "</title";
								title.clear();
								for(size_t i = 0; i < contentStr.size(); ++i) {
																if(imatchHere(contentStr, i, titleBegTag)) {
																								for(i += titleBegTag.size(); i < contentStr.size() && contentStr[i] != '>'; ++i)
																																;
																								for(i += 1; i < contentStr.size() && !imatchHere(contentStr, i, titleEndTag); ++i)
																																title += contentStr[i];
																								break;
																}
								}
}

// case insensitive match here
bool HtmlPreprocesser::imatchHere(const string &str, int i, const string &pat)
{
								size_t j;
								for(j = 0; j + i < str.size() && j < pat.size()
												&& tolower(str[j+i]) == tolower(pat[j]); ++j)
																;
								return j + i <= str.size() && j == pat.size();
}

// delete the content between <tag ...> and </tag ...> including the tags in str
bool HtmlPreprocesser::delBetweenHtmlTags(const string & str, const string tag, string &ret)
{
								string tagBeg = "<" + tag;
								string tagEnd = "</" + tag;

								ret.clear();
								ret.reserve(str.size());

								int nTagBeg = 0;
								for(size_t i = 0; i < str.size(); ++i) {
																if(imatchHere(str, i, tagBeg) && nTagBeg == 0) {
																								++nTagBeg;
																								for(i += tagBeg.size(); i < str.size() && str[i] != '>'; ++i)
																																;
																}
																if(nTagBeg > 0) {
																								if(imatchHere(str, i, tagEnd)) {
																																--nTagBeg;
																																for(i += tagEnd.size(); i < str.size() && str[i] != '>'; ++i)
																																								;
																								}
																}
																else
																								ret += str[i];
								}
								return nTagBeg == 0;
}

// delete all the comments between <!-- and --> in str
bool HtmlPreprocesser::delComments(const string &str, string &ret)
{
								string tagBeg = "<!--";
								string tagEnd = "-->";

								ret.clear();
								ret.reserve(str.size());
								bool inComment = false;
								for(size_t i = 0; i < str.size(); ++i) {
																if(!inComment && str.compare(i, tagBeg.size(), tagBeg) == 0)
																								inComment  = true;
																if(inComment) {
																								if(str.compare(i, tagEnd.size(), tagEnd) == 0) {
																																i += tagEnd.size() - 1;
																																inComment = false;
																								}
																}
																else
																								ret += str[i];
								}
								return !inComment;
}

// delete all the content in the < > including < and >
// < and > must appear in pairs and cann't be nested
bool HtmlPreprocesser::delTags(const string &str, string &ret)
{
								ret.clear();
								ret.reserve(str.size());

								int nTagBeg = 0;
								for(size_t i = 0; i < str.size(); ++i) {
																if(str[i] == '<' && i + 1 < str.size() && !isspace(str[i+1]) && nTagBeg == 0) {
																								++nTagBeg;
																}
																if(nTagBeg > 0) {
																								if(str[i] == '>') {
																																--nTagBeg;
																								}
																}
																else
																								ret += str[i];
								}
								return nTagBeg == 0;
}

// replace every src for dest in str
void HtmlPreprocesser::replaceStr(const string &str, const string src, const string dest, string &ret)
{
								ret.clear();
								ret.reserve(str.size());
								if(src.empty())
																return;
								for(size_t i = 0; i < str.size(); ) {
																if(str.compare(i, src.size(), src) == 0) {
																								ret += dest;
																								i += src.size();
																}
																else
																								ret += str[i++];
								}
}

void HtmlPreprocesser::delOddchar(const string &str, const string dest, string &ret)
{
								ret.clear();
								ret.reserve(str.size());
								for(size_t i = 0; i < str.size(); i++) {
																if((str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z')) {
																								ret += str[i];
																}
																else
																								ret += dest;
								}
}

// merge two or more spaces into one
void HtmlPreprocesser::mergeSpaces(const string &str, string &ret)
{
								ret.clear();
								ret.reserve(str.size());
								bool onSpace = false;
								for(size_t i = 0; i < str.size(); ++i) {
																if(isspace(str[i])) {
																								if(onSpace)
																																continue;
																								else
																																onSpace = true;
																}
																else{
																								onSpace = false;
																}
																ret += str[i];
								}
								return;
}
