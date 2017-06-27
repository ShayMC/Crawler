
#ifndef HTTTHEADER_H
#define HTTTHEADER_H

#include <string>

using namespace std;

class HttpHeader
{
public:
HttpHeader();
HttpHeader(string headerStr);
void setHeaderStr(string headStr);
~HttpHeader();
string getHeaderStr();

int getStatusCode();
string getLocation();
string getContentType();
int getContentLength();
string getContentEncoding();
string getConnection();

private:
void parserHeader();

string _headerStr;
int _statusCode;
string _location;
string _contentType;
int _contentLength;
string _contentEncoding;
string _connection;
};

#endif
