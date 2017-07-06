
#ifndef CRAWLER_H
#define CRAWLER_H

#include <fstream>
#include "HttpHeader.h"
#include "HttpContent.h"
#include "Url.h"

using namespace std;
class Crawler
{
public:
Crawler(ifstream &urlSeedIfs, ofstream &rawPagesOfs, int numThreads);
int multiPthreadCrawl();
void crawlSend(Url &url);
void crawl();


private:
void store(Url &url, HttpHeader &httpHeader, HttpContent &httpContent);
void addUnvisitedUrl(string url);
void addNoRobotdUrl(string urlStr, string host);
void initUnvisitedUrlsSet();

ifstream &_urlSeedIfs;
ofstream &_rawPagesOfs;
int _nWebPagesCrawled;
int _numThreads;
};

#endif
