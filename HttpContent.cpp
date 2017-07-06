
#include <cstring>
#include "HttpContent.h"
#include "Url.h"

using namespace std;

HttpContent::HttpContent(string contentStr) : _contentStr(contentStr), _length(contentStr.size())
{
}

HttpContent::HttpContent()
{
}

HttpContent::~HttpContent()
{
}

void HttpContent::setContentStr(string contentStr)
{
								_contentStr = contentStr;
								_length = contentStr.size();
}

string HttpContent::getContentStr()
{
								return _contentStr;
}

int HttpContent::getLength()
{
								return _length;
}

// We only get the link as the following form:
// <a href="url">Text to be displayed</a>
void HttpContent::getLinks(vector<string> &links)
{
								string::size_type posBegin = 0, posEnd = 0;
								while((posBegin = _contentStr.find("<a ", posBegin)) != string::npos
														&& (posEnd = _contentStr.find("</a>", posBegin)) != string::npos) {
																posBegin += strlen("<a ");
																string anchor = _contentStr.substr(posBegin, posEnd - posBegin);
																string::size_type pos = 0;
																if((pos = anchor.find("href", pos)) == string::npos)
																								continue;

																// skip spaces
																size_t i;
																for(i = pos+strlen("href"); i < anchor.size() && isspace(anchor[i]); ++i)
																								;

																// meet =
																if(i >= anchor.size() || anchor[i] != '=')
																								continue;

																// skip spaces
																for(++i; i < anchor.size() && isspace(anchor[i]); ++i)
																								;

																// meet "
																if(i >= anchor.size() || anchor[i] != '\"')
																								continue;

																// skip spaces
																for(++i; i < anchor.size() && isspace(anchor[i]); ++i)
																								;
																if(i >= anchor.size())
																								continue;

																// meet url
																string::size_type urlBegin = i;

																for(; i < anchor.size() && (anchor[i - 1] == '\\' || anchor[i] != '\"'); ++i)
																								;
																if(i >= anchor.size())
																								continue;
																// meet "
																string link = anchor.substr(urlBegin, i - urlBegin);
																if(!Url::isSpamLink(link))
																								links.push_back(link);
								}
}

void HttpContent::setNotForRobotStr(string links)
{

								_NotForRobotStr = links;
}

string HttpContent::getNotForRobot()
{
								return _NotForRobotStr;
}

void HttpContent::linksRobotHtml()
{
								string links = "";
								string::size_type posBegin = 0, posEnd = 0, posStop = 0;
								bool end = false;

								if((posBegin = _contentStr.find("User-agent:*", posBegin)) == string::npos
											&& (posBegin = _contentStr.find("User-agent: *", posBegin)) == string::npos)
																return;

								posStop = posBegin + strlen("User-agent:");
								if((posStop = _contentStr.find("User-agent: ", posStop)) != string::npos)
																end= true;
								while((posBegin = _contentStr.find("Disallow:", posBegin)) != string::npos
														&& (posEnd = _contentStr.find("\n", posBegin)) != string::npos) {
																if(end && posBegin >= posStop) break;
																posBegin += strlen("Disallow:");
																string link = _contentStr.substr(posBegin, posEnd - posBegin);
																if(!Url::isSpamLink(link)) {
																								links+=link + " ";
																}
								}
								setNotForRobotStr(links);
}
