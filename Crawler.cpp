
#include <sstream>      // std::istringstream
#include <iostream>
#include <map>
#include <set>
#include <string.h>
#include <cstdlib>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include "Crawler.h"
#include "HttpClient.h"
#include "WordsTransfer.h"
#include "Common.h"

using namespace std;

multimap<string, string> unvisitedUrls; // <host, urlStr>	we crawl urls on the same host first
multimap<string, string> noRobotUrl; // <host, not allowed robot urls>


pthread_mutex_t noRobotUrlMutex = PTHREAD_MUTEX_INITIALIZER;  // not allowed robot urls
pthread_mutex_t unvisitedUrlsMutex = PTHREAD_MUTEX_INITIALIZER;  // unvisited urls
pthread_mutex_t _rawPagesOfsMutex = PTHREAD_MUTEX_INITIALIZER;

Crawler::Crawler(ifstream &urlSeedIfs, ofstream &rawPagesOfs, int numThreads) :
								_urlSeedIfs(urlSeedIfs), _rawPagesOfs(rawPagesOfs), _numThreads(numThreads)
{
								_nWebPagesCrawled = 0;
}

static void sigTerm(int x)
{
								cerr << "Terminated!" << endl;
								exit(0);
}

void Crawler::addUnvisitedUrl(string urlStr)
{
								// check the url
								if(urlStr.empty() || urlStr.size() > 256)
																return;
								if(Url::isSpamLink(urlStr))
																return;
								if(Url::isImageUrl(urlStr))
																return;

								Url url(urlStr);
								if(url.getIp() == "")
																return;
								if(url.getHost().size() < 6)
																return;
								string protocol = url.getProtocol();
								for(size_t i = 0; i < protocol.size(); ++i)
																protocol[i] = tolower(protocol[i]);
								if(protocol != "http")
																return;

								pthread_mutex_lock(&noRobotUrlMutex);
								multimap<string, string>::iterator it = noRobotUrl.find(url.getHost());
								if(it != noRobotUrl.end())
								{
																string links=it->second;

																istringstream is(links);
																string link="";
																while(is >> link) {

																								if(url.getUrlStr().find(link)!=string::npos)
																																return;
																}
								}
								pthread_mutex_unlock(&noRobotUrlMutex);


								pthread_mutex_lock(&unvisitedUrlsMutex);
								//cout << "HOST: " << url.getHost() <<"  URL: " << url.getUrlStr() << endl;
								unvisitedUrls.insert(unvisitedUrls.end(),make_pair(url.getHost(), url.getUrlStr()));
								pthread_mutex_unlock(&unvisitedUrlsMutex);
}

void Crawler::addNoRobotdUrl(string urlStr, string host)
{
								if(urlStr.empty() || urlStr.size() > 256)
																return;

								pthread_mutex_lock(&noRobotUrlMutex);
								multimap<string, string>::iterator it = noRobotUrl.find(host);
								if(it != noRobotUrl.end()) {
																string links =it->second;

																links += urlStr +" ";
																it->second = links;
								}
								pthread_mutex_unlock(&noRobotUrlMutex);
}

void Crawler::initUnvisitedUrlsSet()
{
								string line;
								while(getline(_urlSeedIfs, line)) {
																addUnvisitedUrl(line);
								}
								return;
}

static void *threadFun(void *arg)
{
								((Crawler *)arg)->crawl();
								return ((void *)0);
}

int Crawler::multiPthreadCrawl()
{
								// set the signal function
								signal(SIGTERM, sigTerm);
								signal(SIGKILL, sigTerm);
								signal(SIGINT, sigTerm);
								signal(SIGPIPE, SIG_IGN);
								signal(SIGCHLD,SIG_IGN);

								// time start:
								char strTime[128];
								time_t tDate;
								memset(strTime,0,128);
								time(&tDate);
								strftime(strTime, 128,"%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
								cout << "\n\nBegin at: " << strTime << "\n\n";

								// initialize unvisitedUrls set with urls int the seed-file
								initUnvisitedUrlsSet();

								// creat threads:
								cout << "creating " << _numThreads << " threads" << endl;
								pthread_t *tids = (pthread_t*)malloc(_numThreads*sizeof(pthread_t));
								if( tids == NULL) {
																cerr << "malloc error" << endl;
																return -1;
								}
								for(unsigned int i=0; i < _numThreads; i++) {
																if(pthread_create(&tids[i], NULL, threadFun, this) != 0) {
																								cerr << "create threads error" << endl;
																								return -1;
																}
								}

								// Wait for the threads.
								cout << "main thread is waiting for threads" << endl;
								for (unsigned int i = 0; i < _numThreads; ++i) {
																if(pthread_join(tids[i], NULL) != 0) {
																								cerr << "pthread_join error" << endl;
																								return -1;
																}
								}
								cout << _numThreads << " threads" << " closed." << endl;

								// time finish:
								memset(strTime,0,128);
								time(&tDate);
								strftime(strTime, 128,"%a, %d %b %Y %H:%M:%S GMT", gmtime(&tDate));
								cout << "\n\nEnd at: " << strTime << "\n\n";
								return 0;
}

void Crawler::store(Url &url, HttpHeader &httpHeader, HttpContent &httpContent)
{
								pthread_mutex_lock(&_rawPagesOfsMutex);

								if(url.getIfRobotsUrl()) {
																_rawPagesOfs << ROBOTS_BEGIN_TAG << " " << url.getUrlStr() << endl;
								}
								else _rawPagesOfs << RAW_URL_BEGIN_TAG << " " << url.getUrlStr() << endl;

								_rawPagesOfs << "OPENSE.RECORD.ID: " << _nWebPagesCrawled++ << endl;
								_rawPagesOfs << "OPENSE.RECORD.IP: " << url.getIp() << endl;
								_rawPagesOfs << RAW_CONTENT_LENGTH_BEGIN_TAG << " " << httpContent.getLength() << endl;
								_rawPagesOfs << "OPENSE.RECORD.HTTP_HEADER:\n" << httpHeader.getHeaderStr() << endl;
								_rawPagesOfs << RAW_HTTP_CONTENT_BEGIN_TAG << "\n" << httpContent.getContentStr() << endl;
								cout << endl;

								WordsTransfer WordsTransfer;
								WordsTransfer.creatWordsTransfer(RAW_PAGES_PATH,DICT_PATH,url);

								pthread_mutex_unlock(&_rawPagesOfsMutex);
								return;
}

bool Crawler::crawlSend(Url &url)
{

								HttpClient httpClient;

								// invoke httpClient.requestWebPage(Url &unvisitedUrl, WebPage & webPage)
								HttpHeader httpHeader;
								HttpContent httpContent;

								if(httpClient.requestWebPage(url, httpHeader, httpContent) == -1) {
																return false;
								}
								// if content type of page is not we wanted, return
								string contentType = httpHeader.getContentType();
								if(contentType != "text/html" && contentType != "text/plain"
											&& contentType != "text/xml" && contentType != "text/rtf") {
																cout <<  url.getUrlStr() << ": not wanted type http content" << endl;
																return false;
								}

								string contentEncoding = httpHeader.getContentEncoding();

								if(contentEncoding == "gzip") {
																cout << url.getUrlStr() << ": contentEncoding is gzip" << endl;
																return false;
								}

								//extract links from page
								if(url.getIfRobotsUrl()) {
																httpContent.linksRobotHtml();
																istringstream is(httpContent.getNotForRobot());
																string link = "";
																while(is >> link) {
																								addNoRobotdUrl(link, url.getHost());
																}
								}
								else
								{
																vector<string> links;
																//save web page to file
																store(url, httpHeader, httpContent);
																httpContent.getLinks(links);
																for(int i = 0; i < links.size(); ++i) {
																								addUnvisitedUrl(links[i]);
																}
								}


}

void Crawler::crawl()
{
								// create a http client to request pages according to conresponding urls
								HttpClient httpClient;

								// Crawl Loop begin:
								unsigned sleepTimeCnt = 0;
								while(sleepTimeCnt < 1000*200) {
																pthread_mutex_lock(&unvisitedUrlsMutex);

																// if unvisitedUrls empty, sleep some seconds
																// and record the number of sleep for timeout
																if(unvisitedUrls.empty()) {
																								pthread_mutex_unlock(&unvisitedUrlsMutex);
																								usleep(1000);
																								++sleepTimeCnt;
																								continue;
																}

																// get one unvisitedUrl to deal with,
																multimap<string, string>::iterator it = unvisitedUrls.begin();
																// paser url
																Url url(it->second);
																Url urlRobbot(it->first+"/robots.txt");

																unvisitedUrls.erase(it);
																pthread_mutex_unlock(&unvisitedUrlsMutex);

																multimap<string, string >::iterator temp = noRobotUrl.find(url.getHost());
																if(temp == noRobotUrl.end())
																{
																								string urls="";
																								pthread_mutex_lock(&noRobotUrlMutex);
																								noRobotUrl.insert(make_pair(url.getHost(),urls));
																								pthread_mutex_unlock(&noRobotUrlMutex);
																								crawlSend(urlRobbot);
																}

																crawlSend(url);
								}
}
