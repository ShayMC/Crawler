
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include "Url.h"
#include "HttpHeader.h"
#include "HttpContent.h"

using namespace std;

class HttpClient
{
public:
HttpClient();
~HttpClient();
int requestWebPage(Url &url, HttpHeader &httpHeader, HttpContent &httpContent);

private:
string _preHost;
int _preSockFd;

int tcpConnect(string ip, int port);
int nonbConnect(int sockFd,struct sockaddr* sa,int timeoutSeconds);

ssize_t rio_writen(int fd, const char *usrbuf, size_t n);
ssize_t rio_readn(int fd, char *usrbuf, size_t n);
int receiveHeader(int sockFd, string &headerStr, int timeoutSeconds);
int receiveContent(int sockFd, unsigned int expectLength, string &contentStr, int timeoutSeconds);
};

#endif
