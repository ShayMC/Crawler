
#include <stdlib.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include "HttpClient.h"

static const int DEFAULT_TIMEOUT_SECONDS = 30;
static const int MAX_HTTPCONTENT_SIZE = 5*1024*1024;

HttpClient::HttpClient() : _preSockFd(-1)
{
}

HttpClient::~HttpClient()
{
}

ssize_t HttpClient::rio_writen(int fd, const char *usrbuf, size_t n)
{
								size_t nleft = n;
								const char *bufp = usrbuf;
								ssize_t nwrittten;

								while(nleft > 0) {
																if((nwrittten = write(fd, bufp, nleft)) <= 0) {
																								if(errno == EINTR)
																																nwrittten = 0;
																								else
																																return -1;
																}
																nleft -= nwrittten;
																bufp += nwrittten;
								}
								return n;
}

ssize_t HttpClient::rio_readn(int fd, char *usrbuf, size_t n)
{
								size_t nleft = n;
								ssize_t nread;
								char *bufp = usrbuf;

								while (nleft > 0) {
																if ((nread = read(fd, bufp, nleft)) < 0) {
																								if (errno == EINTR) /* interrupted by sig handler return */
																																nread = 0;  /* and call read() again */
																								else
																																return -1;  /* errno set by read() */
																}
																else if (nread == 0)
																								break;  /* EOF */
																nleft -= nread;
																bufp += nread;
								}
								return (n - nleft);  /* return >= 0 */
}

int HttpClient::receiveHeader(int sockFd, string &headerStr, int timeoutSeconds)
{
								int flags;
								flags=fcntl(sockFd,F_GETFL,0);
								if(flags<0) {
																cerr << "1.fcntl() error in receiveHeader()< 0" << endl;
																return -1;
								}
								flags|=O_NONBLOCK;
								if(fcntl(sockFd,F_SETFL,flags)<0) {
																cerr << "2.fcntl() error in receiveHeader" << endl;
																return -1;
								}

								int newlines = 0;
								int bytesRead = 0;
								while(newlines < 2) {
																fd_set rfds;
																FD_ZERO(&rfds);
																FD_SET(sockFd, &rfds);

																struct timeval tv;
																tv.tv_sec = timeoutSeconds;
																tv.tv_usec = 0;

																int selectRet;
																if(timeoutSeconds >= 0) // wait DEFAULT_TIMEOUT_SECONDS seconds
																								selectRet = select(sockFd+1, &rfds, NULL, NULL, &tv);
																else // wait infintely
																								selectRet = select(sockFd+1, &rfds, NULL, NULL, NULL);

																if(selectRet == 0 && timeoutSeconds < 0) {
																								cerr << "select should wait infintely" << endl;
																								return -1;
																}else if(selectRet == -1) {
																								cerr << "select error" << endl;
																								return -1;
																}
																else if(selectRet == 0) {
																								cerr << "select timeout after " << timeoutSeconds << " seconds" << endl;
																								return -1;
																}

																char c;
																int ret = read(sockFd, &c, 1);
																if(ret <= 0) {
																								cerr << "read error" << endl;
																								return -1;
																}
																headerStr += c;
																++bytesRead;

																if(c == '\r') {  /* Ignore CR */
																								continue;
																}
																else if(c == '\n') /* LF is the separator */
																								newlines++;
																else
																								newlines = 0;
								}
								return bytesRead;
}

int HttpClient::receiveContent(int sockFd, int expectLength, string &contentStr, int timeoutSeconds)
{
								int flags;;
								if((flags = fcntl(sockFd,F_GETFL,0)) < 0) {
																cerr << "fcntl error" << endl;
																return -1;
								}
								flags |= O_NONBLOCK;
								if(fcntl(sockFd,F_SETFL,flags)<0) {
																cerr << "fcntl error" << endl;
																return -1;
								}

								int usrBufLen = expectLength;
								char *usrBuf = (char*)malloc(usrBufLen);
								fd_set rfds;
								struct timeval tv;
								int selectRet;
								while(true) {
																FD_ZERO(&rfds);
																FD_SET(sockFd, &rfds);
																if(contentStr.size() >= expectLength)
																								tv.tv_sec = 1;
																else
																								tv.tv_sec = timeoutSeconds;
																tv.tv_usec = 0;

																if(timeoutSeconds >= 0)
																								// set the block timeoutSeconds
																								selectRet = select(sockFd+1, &rfds, NULL, NULL, &tv);
																else
																								// No timeoutSeconds, can block infinitely
																								selectRet = select(sockFd+1, &rfds, NULL, NULL, NULL);
																if(selectRet == -1) {
																								cerr << "select error" << endl;
																								return -1;
																}
																else if(selectRet == 0) {
																								if(contentStr.size()!= expectLength) {
																																cerr <<"select timeoutSeconds" << endl;
																																return -1;
																								}
																								else
																																break;
																}

																int nRead;
																if((nRead = read(sockFd, usrBuf, sizeof(usrBuf))) > 0) {
																								contentStr += string(usrBuf, 0, nRead);
																}
																else if(nRead < 0) {
																								if(errno == EINTR) {
																																//cerr << "read EINTR" << endl;
																								}
																								else if(errno == EWOULDBLOCK) {
																																//perror("EWOULDBLOCK:");
																								}
																								else{
																																//cerr << "read error" << endl;
																																perror("read error");
																																return -1;
																								}
																}
																else if(nRead == 0) { // meet EOF
																								break;
																}
																else
																								;
								}
								free(usrBuf);
								return contentStr.size();
}

int HttpClient::requestWebPage(Url &url, HttpHeader &httpHeader, HttpContent &httpContent)
{
								// constuct a request
								string requestStr;
								string path = url.getPath();
								if(path.empty())
																path = "/";
								requestStr = "GET " + path + " HTTP/1.0\r\nHost: " + url.getHost()
																					+ "\r\nUser-Agent: openSE/1.0 (Ubuntu8.04)\r\nAccept-Language: zh,en-us\r\nAccept-Charset: gb2312,utf-8\r\nConnection: Keep-Alive\r\n\r\n";

								//cout << "requestStr:\n" << requestStr << endl;

								// send request:
								if(url.getHost() != _preHost) {
																if(_preSockFd != -1) {
																								close(_preSockFd);
																								_preSockFd = -1;
																}
								}
								int sockFd;
								bool sendSuccess = false;

								// try to use previous connection
								if(_preSockFd != -1) {
																sockFd = _preSockFd;
																if(rio_writen(sockFd, requestStr.c_str(), requestStr.size()) == -1) {
																								cerr << "use previous connection:rio_writen error"<< endl;
																								close(_preSockFd);
																								_preSockFd = -1;
																}
																else
																								sendSuccess = true;
								}

								if(!sendSuccess) {
																// try to creat a new connection
																sockFd = tcpConnect(url.getIp(), url.getPort());
																if(sockFd == -1) {
																								cerr << "tcpConnect error" << endl;
																								return -1;
																}
																// try to use new connection
																if(rio_writen(sockFd, requestStr.c_str(), requestStr.size()) == -1) {
																								cerr << "rio_writen error for requestStr:" << requestStr << endl;
																								close(sockFd);
																								return -1;
																}
								}

								// receive http header
								string headerStr;
								headerStr.reserve(1024);
								if(receiveHeader(sockFd, headerStr, DEFAULT_TIMEOUT_SECONDS) <= 0) {
																cerr << "receiveHeader error" << endl;
																close(sockFd);
																_preSockFd = -1;
																return -1;
								}

								//cout << "headerStr:\n" << headerStr << endl;

								// parser http header
								httpHeader.setHeaderStr(headerStr);

								// check StatusCode
								int stausCode = httpHeader.getStatusCode();
								if(stausCode == -1) {
																cerr << "not find status code in httpHeader: " << httpHeader.getHeaderStr() << endl;
								}

								if(stausCode == 301 || stausCode == 302) {
																close(sockFd);
																_preSockFd = -1;
																string locationUrlStr = httpHeader.getLocation();
																if(locationUrlStr.empty()) {
																								cerr << "error location in httpHeader: " << httpHeader.getHeaderStr() << endl;
																}
																//locationStr = location;
																Url locationUrl(locationUrlStr);
																return requestWebPage(locationUrl, httpHeader, httpContent);
								}

								if(stausCode < 200 || stausCode > 299) {
																close(sockFd);
																_preSockFd = -1;
																cerr << "staus code beyond [200-300) in httpHeader: " << httpHeader.getHeaderStr() << endl;
																return -1;
								}

								// check content type
								string contentType = httpHeader.getContentType();
								if(contentType.find("image") != string::npos) {
																close(sockFd);
																_preSockFd = -1;
																cerr << "contentType is image in httpHeader: " << httpHeader.getHeaderStr() << endl;
																return -1;
								}

								// check ContentLength
								int contentLength = httpHeader.getContentLength();

								if(contentLength == -1) {
																//cerr << "contentLength is not finded in httpHeader: " << httpHeader.getHeaderStr() << endl;
																contentLength = MAX_HTTPCONTENT_SIZE/10;
								}

								if(contentLength == 0) {
																close(sockFd);
																_preSockFd = -1;
																cerr << "contentLength is 0 in httpHeader: " << httpHeader.getHeaderStr() << endl;
																return -1;
								}

								if(contentLength  > MAX_HTTPCONTENT_SIZE) {
																close(sockFd);
																_preSockFd = -1;
																cerr << "contentLength > MAX_HTTPCONTENT_SIZE in httpHeader: "
																					<< httpHeader.getHeaderStr() << endl;
																return -1;
								}

								// receive content
								string contentStr;
								if(receiveContent(sockFd, contentLength, contentStr, DEFAULT_TIMEOUT_SECONDS)
											== -1) {
																// cerr << contentLength << endl;
																// cerr << sockFd << endl;
																// cerr << contentStr << endl;

																close(sockFd);
																_preSockFd = -1;
																cerr << "receiveContent error for url: " << url.getUrlStr() << endl;
																return -1;
								}
								else
																// cerr << contentLength << endl;
																// cerr << sockFd << endl;
																// cerr << contentStr << endl;
																_preSockFd = sockFd;

								//cout << "contentStr:\n" << contentStr << endl;
								// set http content
								httpContent.setContentStr(contentStr);
}

int HttpClient::tcpConnect(string ip, int port)
{
								if(ip.empty()) {
																cerr << "ip is empty" << endl;
																return -1;
								}

								struct sockaddr_in sa;
								int ret;
								if((ret = inet_pton(AF_INET, ip.c_str(), &sa.sin_addr)) == 0) {
																cerr <<  "the presentation of ip:" << ip << "is not valid" << endl;
																return -1;
								}
								else if(ret == -1) {
																cerr << "inet_pton error for ip:" << ip << endl;
																return -1;
								}

								sa.sin_family = AF_INET;
								if(port < 0) {
																cerr << "port: " << port << " < 0" << endl;
																return -1;
								}
								sa.sin_port = htons(port);

								int sockFd;
								if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
																cerr << "socket error for ip:" << ip << endl;
																return -1;
								}

								// open option to reuse local address
								int optval = 1;
								if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof (optval)) < 0) {
																cerr << "setsockopt error for ip:" << ip << endl;
																close(sockFd);
																return -1;
								}

								if(nonbConnect(sockFd, (struct sockaddr *)&sa, DEFAULT_TIMEOUT_SECONDS) == -1) {
																cerr << "nonb_connect error for ip:" << ip << endl;
																close(sockFd);
																return -1;
								}
								return sockFd;
}

int HttpClient::nonbConnect(int sockFd,struct sockaddr* sa,int timoutSeconds)
{
								//set the socket as nonblocking
								int flags = fcntl(sockFd,F_GETFL,0);
								if(flags<0)
																return -1;
								flags |= O_NONBLOCK;
								if(fcntl(sockFd,F_SETFL,flags) < 0) {
																cerr << "fcntl error" << endl;
																return -1;
								}
								if(connect(sockFd, sa, sizeof(struct sockaddr)) == 0) {
																//connected immediately
																flags &= ~O_NONBLOCK;
																fcntl(sockFd,F_SETFL,flags);
																return sockFd;
								}

								fd_set mask;
								FD_ZERO(&mask);
								FD_SET(sockFd,&mask);

								struct timeval timeout;
								timeout.tv_sec=timoutSeconds;
								timeout.tv_usec=0;

								int status;
								status=select(sockFd+1,NULL,&mask,NULL,&timeout);

								switch(status) {
								case -1: // Select error, set the socket as default blocking
																flags&=~O_NONBLOCK;
																fcntl(sockFd,F_SETFL,flags);
																cerr << "select error" << endl;
																return -1;
								case 0: //Connection timed out.
																flags&=~O_NONBLOCK;
																fcntl(sockFd,F_SETFL,flags);
																cerr << "select timeout" << endl;
																return -1;
								default: // Connected successfully.
																FD_CLR(sockFd,&mask);
																flags&=~O_NONBLOCK;
																fcntl(sockFd,F_SETFL,flags);
																return 0;
								}
}
