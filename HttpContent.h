
#ifndef HTTPCONTENT_H
#define HTTPCONTENT_H

#include <string>
#include <vector>

using namespace std;

class HttpContent
{
public:
HttpContent(string contentStr);
HttpContent();
~HttpContent();

void setContentStr(string contentStr);
void setNotForRobotStr(string links);
string getContentStr();
void getLinks(vector<string> &links);
string getNotForRobot();
void linksRobotHtml();
int getLength();

private:
string _contentStr;
string _NotForRobotStr;
int _length;
};

#endif
