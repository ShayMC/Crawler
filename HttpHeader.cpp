
#include <sstream>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include "HttpHeader.h"

using namespace std;

HttpHeader::HttpHeader()
{
								parserHeader();
}

HttpHeader::HttpHeader(string headerStr) : _headerStr(headerStr)
{
								parserHeader();
}

void HttpHeader::setHeaderStr(string headStr)
{
								_headerStr = headStr;
								parserHeader();
}

// Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
void HttpHeader::parserHeader()
{
								_statusCode = -1;
								_contentLength = -1;
								_location = "";
								_contentEncoding = "";
								_contentType = "";

								if(_headerStr.empty())
																return;

								string headerStr = _headerStr;

								// extract StatusCode:
								string::size_type lineEnd = headerStr.find("\r\n");
								string statusLine = headerStr.substr(0, lineEnd);
								istringstream iss(statusLine);
								string httpVersion;
								iss >> httpVersion;
								iss >> _statusCode;

								string::size_type lineBegin = lineEnd + strlen("\r\n");
								while(lineBegin < headerStr.size()
														&& ((lineEnd = headerStr.find("\r\n", lineBegin)) != string::npos)) {

																string line = headerStr.substr(lineBegin, lineEnd - lineBegin);
																lineBegin = lineEnd + strlen("\r\n");
																if(line.empty())
																								continue;
																//cout << "line:" << line << endl;
																string::size_type pos;
																string::size_type pos2;

																// extract ContentLength:
																if((pos = line.find("Content-Length: ")) != string::npos) {
																								pos += strlen("Content-Length: ");
																								_contentLength = atoi(line.substr(pos).c_str());
																}
																// extract Location:
																else if((pos = line.find("Location: ")) != string::npos) {
																								pos += strlen("Location: ");
																								_location = line.substr(pos);
																}
																// extract Content-Encoding:
																else if((pos = line.find("Content-Encoding: ")) != string::npos) {
																								pos += strlen("Content-Encoding: ");
																								_contentEncoding = line.substr(pos);
																}
																// extract Content-Type:
																else if((pos = line.find("Content-Type: ")) != string::npos) {
																								pos += strlen("Content-Type: ");
																								if((pos2 = line.find(';')) != string::npos) {
																																pos2-=pos;
																																_contentType = line.substr(pos,pos2);
																								}
																								else _contentType = line.substr(pos);
																}
																// extract Connection:
																else if((pos = line.find("Connection: ")) != string::npos) {
																								pos += strlen("Connection: ");
																								_connection = line.substr(pos);
																}
																else{
																}
								}
}

HttpHeader::~HttpHeader()
{
}

string HttpHeader::getHeaderStr()
{
								return _headerStr;
}

int HttpHeader::getStatusCode()
{
								return _statusCode;
}
string HttpHeader::getLocation()
{
								return _location;
}

string HttpHeader::getContentType()
{
								return _contentType;
}

int HttpHeader::getContentLength()
{
								return _contentLength;
}

string HttpHeader::getContentEncoding()
{
								return _contentEncoding;
}

string HttpHeader::getConnection()
{
								return _connection;
}
