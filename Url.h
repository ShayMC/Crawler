
#ifndef URL_H
#define URL_H

#include <string>

using namespace std;

class Url
{
public:
Url();
Url(string urlStr);
~Url();

void setUrlStr(string urlStr);
string getUrlStr();

string getProtocol();
string getHost();
string getPath();
int getPort();
string getIp();
bool getIfRobotsUrl();

static bool isRobotsUrl(string urlStr);
static bool isImageUrl(string urlStr);
static bool isSpamLink(string link);

private:
static bool isValidHostChar(char ch);
void parserUrlStr();
static bool findTwice(const string &str, const string patten);

string _urlStr;
string _protocol;
string _host;   // host name
string _path;   // request
int _port;   // port number
string _ip;
bool _robotsUrl;
};

#endif
