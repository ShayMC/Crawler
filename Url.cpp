
#include <iostream>
#include <cstdlib>
#include <map>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "Url.h"

using namespace std;

map<string, string> hostIpPairs;
pthread_mutex_t hostIpPairsMutex = PTHREAD_MUTEX_INITIALIZER;

Url::Url()
{
}

Url::Url(string urlStr) : _urlStr(urlStr)
{
								parserUrlStr();
}

void Url::setUrlStr(string urlStr)
{
								_urlStr = urlStr;
								parserUrlStr();
}

bool Url::isValidHostChar(char ch)
{
								return (isalnum(ch) || ch=='-' || ch=='.' || ch=='_');
}

//url=protocol://hostname[:port]/path/[;parameters][?query]#fragmen
void Url::parserUrlStr()
{
								_protocol = "http";
								_host = "";
								_port = 80;
								_path = "";
								_ip = "";
								_robotsUrl = false;

								if(_urlStr.empty())
																return;
								// extract protocol
								string::size_type pos = _urlStr.find("://");
								if(pos == 0 || pos == string::npos) {
																_protocol = "http";
																pos = 0;
								}
								else{
																_protocol = _urlStr.substr(0, pos);
																pos += strlen("://");
								}

								// extract hostname
								size_t i;
								for(i = pos; i < _urlStr.size() && isValidHostChar(_urlStr[i]); ++i)
																_host += _urlStr[i];

								// extract port number
								if(i < _urlStr.size() && _urlStr[i] == ':') {
																string portStr;
																for(++i; i < _urlStr.size() && isdigit(_urlStr[i]); ++i)
																								portStr += _urlStr[i];
																_port = atoi(portStr.c_str());
								}
								// extract path
								if(i < _urlStr.size())
																_path = _urlStr.substr(i);

								if(isRobotsUrl(_path)) _robotsUrl = true;
}

Url::~Url()
{
}

string Url::getUrlStr()
{
								return _urlStr;
}

string Url::getProtocol()
{
								return _protocol;
}

string Url::getHost()
{
								return _host;
}

string Url::getPath()
{
								return _path;
}

int Url::getPort()
{
								return _port;
}

bool Url::getIfRobotsUrl()
{
								return _robotsUrl;
}

string Url::getIp()
{
								if(!_ip.empty())
																return _ip;
								if(_host.empty())
																return "";

								// _host is acturally  a ip?
								struct sockaddr_in sa;
								int ret = inet_pton(AF_INET, _host.c_str(), &sa.sin_addr);
								if(ret == 1)
																return _host;
								if(ret == -1) {
																cerr << "inet_pton error for host=" << _host << endl;
																return "";
								}

								// try to find in cache
								map<string, string>::iterator it = hostIpPairs.find(_host);
								if(it != hostIpPairs.end())
																return it->second;

								// try to find via DNS server
								struct hostent *hptr = gethostbyname(_host.c_str());
								if(hptr == NULL) {
																cerr << "gethostbyname error for host=" << _host << endl;
																return "";
								}
								if(hptr->h_addrtype != AF_INET) {
																cerr << "unknown address type for host=" << _host << endl;
																return "";
								}

								char ipStr[INET_ADDRSTRLEN];
								char **pptr = NULL;
								for(pptr = hptr->h_addr_list; *pptr != NULL; ++pptr) {
																if(inet_ntop(hptr->h_addrtype, *pptr, ipStr, sizeof(ipStr)) == NULL)
																								continue;
																pthread_mutex_lock(&hostIpPairsMutex);
																if(hostIpPairs.find(_host) == hostIpPairs.end())
																								hostIpPairs[_host] = ipStr;  // add ip in cache
																pthread_mutex_unlock(&hostIpPairsMutex);
																break;
								}
								if(*pptr == NULL) {
																cerr << "not find ip address for host=" << _host << endl;
																return "";
								}

								_ip = ipStr;
								return ipStr;
}

bool Url::isImageUrl(string urlStr)
{
								if(urlStr.empty())
																return false;
								if(urlStr.size() > 256)
																return false;

								string::size_type idx = urlStr.rfind('.');
								string tmp;
								if(idx != string::npos && idx + 1 < urlStr.size())
																tmp = urlStr.substr(idx+1);
								else
																return false;

								for(size_t i = 0; i < urlStr.size(); ++i)
																urlStr[i] = tolower(urlStr[i]);

								const char *image_type[] ={
																"gif","jpg","jpeg","png","bmp",
																"tif","psd"
								};

								for (size_t i = 0; i < sizeof(image_type)/sizeof(image_type[0]); i++)
																if(tmp == image_type[i])
																								return true;
								return false;
}

bool Url::findTwice(const string &str, const string patten)
{
								string::size_type idx = str.find(patten);
								if(idx != string::npos)
																if(idx+1 < str.size() && (idx = str.find(patten, idx + 1)) != string::npos)
																								return true;
								return false;
}

bool Url::isRobotsUrl(string urlStr)
{
								if(urlStr.find("/robots.txt") != string::npos) {
																return true;
								}
								return false;
}

bool Url::isSpamLink(string link)
{
								if(link.empty())
																return true;
								if(link.size() > 256)
																return true;

								for(size_t i = 0; i < link.size(); ++i)
																link[i] = tolower(link[i]);  //Convert uppercase letter to lowercase

								// find two times following symbols, return false
								if(findTwice(link, "?"))
																return true;

								if(findTwice(link, "+"))
																return true;

								if(findTwice(link, "&"))
																return true;

								if(findTwice(link, "//"))
																return true;

								if(findTwice(link, "http"))
																return true;

								if(findTwice(link, "misc"))
																return true;

								if(findTwice(link, "ipb"))
																return true;

								const char *filter_str[] = {
																"cgi-bin", "htbin", "linder", "srs5",  "uin-cgi", // robots.txt of http://www.expasy.org/
																"uhtbin", "snapshot", "=+",  "=-",  "script",
																"gate",  "search", "clickfile", "data/scop", "names",
																"staff/", "enter", "user",  "mail", "pst?",
																"find?", "ccc?",  "fwd?",  "tcon?", "&amp",
																"counter?", "forum", "cgisirsi", "{",  "}",
																"proxy", "login", "00.pl?", "sciserv.pl", "sign.asp",
																"<",  ">",  "review.asp?", "result.asp?", "keyword",
																"\"",  "'",  "php?s=", "error", "showdate",
																"niceprot.pl?", "volue.asp?id", ".css",  ".asp?month", "prot.pl?",
																"msg.asp", "register.asp", "database", "reg.asp", "qry?u",
																"p?msg", "tj_all.asp?page", ".plot.", "comment.php", "nicezyme.pl?",
																"entr",  "compute-map?", "view-pdb?", "list.cgi?", "lists.cgi?",
																"details.pl?", "aligner?", "raw.pl?", "interface.pl?","memcp.php?",
																"member.php?", "post.php?", "thread.php", "bbs/",  "/bbs", "pdf"
								};
								for(size_t i = 0; i < sizeof(filter_str)/sizeof(filter_str[0]); i++) {
																if(link.find(filter_str[i]) != string::npos)
																								return true;
								}
								return false;
}
