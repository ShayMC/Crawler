
#include <sstream>
#include <iostream>
#include <map>
#include <set>
#include <string.h>
#include <cstdlib>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include "HNode.h"
#include "Crawler.h"
#include "HttpClient.h"
#include "WordsTransfer.h"
#include "Common.h"

using namespace std;

multimap<string, string> unvisitedUrls; // <host, urlStr>	we crawl urls on the same host first
multimap<string, HNode> HostData; // <host, visitedHost and not for Robot Url>

pthread_mutex_t hostDataMutex = PTHREAD_MUTEX_INITIALIZER;  // not allowed robot urls
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
								for(int i=0; i < _numThreads; i++) {
																if(pthread_create(&tids[i], NULL, threadFun, this) != 0) {
																								cerr << "create threads error" << endl;
																								return -1;
																}
								}

								// Wait for the threads.
								cout << "main thread is waiting for threads" << endl;
								for (int i = 0; i < _numThreads; ++i) {
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
								if(url.getHost().size() < 6)
																return;
								if(url.getIp() == "")
																return;

								string protocol = url.getProtocol();
								for(size_t i = 0; i < protocol.size(); ++i)
																protocol[i] = tolower(protocol[i]);
								if(protocol != "http")
																return;

								pthread_mutex_lock(&hostDataMutex);
								multimap<string, HNode>::iterator it = HostData.find(url.getHost());
								if(it != HostData.end())
								{
																string links = (it->second).getNoRobotUrls();
																istringstream is(links);
																string link="";
																while(is >> link) {
																								if(url.getUrlStr().find(link)!=string::npos) {
																																pthread_mutex_unlock(&hostDataMutex);
																																return;
																								}
																}
								}

								//cout << "HOST: " << url.getHost() <<"  URL: " << url.getUrlStr() << endl;
								unvisitedUrls.insert(unvisitedUrls.end(),make_pair(url.getHost(), url.getUrlStr()));

								pthread_mutex_unlock(&hostDataMutex);
}

void Crawler::addNoRobotdUrl(string urlStr, string host)
{
								if(urlStr.empty() || urlStr.size() > 256)
																return;

								pthread_mutex_lock(&hostDataMutex);

								multimap<string, HNode>::iterator it = HostData.find(host);
								if(it != HostData.end()) {

																string links = (it->second).getNoRobotUrls();
																links += urlStr +" ";
																(it->second).setNoRobotUrls(links);
								}
								else
																cerr << "No host data[rboot.txt]: " << urlStr << " Host: " <<  host << endl;

								pthread_mutex_unlock(&hostDataMutex);
}

void Crawler::store(Url &url, HttpHeader &httpHeader, HttpContent &httpContent)
{
								pthread_mutex_lock(&_rawPagesOfsMutex);

								_rawPagesOfs << RAW_URL_BEGIN_TAG << " " << url.getUrlStr() << endl;
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

void Crawler::crawlSend(Url &url)
{
								HttpClient httpClient;

								// invoke httpClient.requestWebPage(Url &unvisitedUrl, WebPage & webPage)
								HttpHeader httpHeader;
								HttpContent httpContent;

								if(httpClient.requestWebPage(url, httpHeader, httpContent) == -1) {
																return;
								}

								// if content type of page is not we wanted, return
								string contentType = httpHeader.getContentType();
								if(contentType != "text/html" && contentType != "text/plain"
											&& contentType != "text/xml" && contentType != "text/rtf") {
																cout <<  url.getUrlStr() << ": not wanted type http content" << endl;
																return;
								}

								string contentEncoding = httpHeader.getContentEncoding();

								if(contentEncoding == "gzip") {
																cout << url.getUrlStr() << ": contentEncoding is gzip" << endl;
																return;
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
																for(unsigned int i = 0; i < links.size(); ++i) {
																								if(links[i].compare(url.getUrlStr())!=0)
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
																pthread_mutex_lock(&hostDataMutex);

																// if unvisitedUrls empty, sleep some seconds
																// and record the number of sleep for timeout
																if(unvisitedUrls.empty()) {
																								pthread_mutex_unlock(&hostDataMutex);
																								usleep(1000);
																								++sleepTimeCnt;
																								continue;
																}

																// get one unvisitedUrl to deal with,
																multimap<string, string>::iterator it = unvisitedUrls.begin();
																// paser url
																Url url(it->second);
																multimap<string, HNode>::iterator itHost = HostData.find(url.getHost());
																if(itHost != HostData.end())
																{
																								if((itHost->second).isHostBusy())
																								{
																																//cerr << "[skip!!]" << endl;
																																pthread_mutex_unlock(&hostDataMutex);
																																continue;
																								}
																								else{
																																(itHost->second).setHostBusy();
																																unvisitedUrls.erase(it);
																																pthread_mutex_unlock(&hostDataMutex);
																								}
																}
																else
																{
																								Url urlRobbot(it->first+"/robots.txt");
																								HNode h("");
																								h.setHostBusy();
																								HostData.insert(make_pair(url.getHost(),h));
																								unvisitedUrls.erase(it);
																								pthread_mutex_unlock(&hostDataMutex);
																								crawlSend(urlRobbot);
																}

																//cerr << "Num Pages Crawled:"<< _nWebPagesCrawled << ", Url arr size: "<< unvisitedUrls.size() << ", Host arr size: "<< HostData.size() << ", Current url: "<< url.getUrlStr() << endl;
																crawlSend(url);

																pthread_mutex_lock(&hostDataMutex);

																multimap<string, HNode>::iterator itD = HostData.find(url.getHost());
																if(itD != HostData.end())
																{
																								(itD->second).setHostNotBusy();

																}
																else cerr << "No host data[error]: " << url.getHost() << endl;

																if(HostData.size() >= 150) {
																								itD = HostData.begin();
																								for(; itD != HostData.end(); itD++)
																								{
																																if(!(itD->second.isHostBusy()))
																																								HostData.erase(itD);
																								}
																}

																pthread_mutex_unlock(&hostDataMutex);
								}
}
