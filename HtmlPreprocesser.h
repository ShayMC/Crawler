
#ifndef HTMLPREPROCESSER_H
#define HTMLPREPROCESSER_H

#include <string>

using namespace std;

class HtmlPreprocesser
{
public:
static void getTitle(const string &contentStr, string &title);
static bool delBetweenHtmlTags(const string & str, const string tag, string &ret);
static bool delComments(const string &str, string &ret);
static bool delTags(const string &str, string &ret);
static void replaceStr(const string &str, const string src, const string dest, string &ret);
static void mergeSpaces(const string &str, string &ret);
static void delOddchar(const string &str, const string dest, string &ret);

private:
static bool imatchHere(const string &str, int i, const string &pat);
};

#endif
