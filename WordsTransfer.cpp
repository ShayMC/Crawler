
#include <vector>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include "WordsTransfer.h"
#include "HtmlPreprocesser.h"
#include "Url.h"
#include "Common.h"

using namespace std;

WordsTransfer::WordsTransfer()
{
}

void WordsTransfer::openFiles(const string rawPagesPath,const string dictFilePath)
{
								_rawPagesFile.open(rawPagesPath.c_str(), ifstream::in | ifstream::binary |ios::app);
								if(!_rawPagesFile) {
																cerr << "cann't open file " << rawPagesPath << endl;
																exit(-1);
								}
								_dictFile.open(dictFilePath.c_str(), ofstream::out | ofstream::binary | ios::app);
								if(!_dictFile) {
																cerr << "cann't open file " << dictFilePath << endl;
																exit(-1);
								}
}

void WordsTransfer::closeFiles()
{
								_rawPagesFile.close();
								_dictFile.close();
}

void WordsTransfer::creatWordsTransfer(const string rawPagesPath, const string dictPath, Url &url)
{
								if(url.getIfRobotsUrl()) {
																return;
								}

								openFiles(rawPagesPath,dictPath);
								string line;

								istringstream issLine(line);

								// get the length of the content
								int contentLength = 0;
								const string contentLengthBeg = "OPENSE.RECORD.CONTENT_LENGTH:";
								while(getline(_rawPagesFile, line)) {
																if(line.size() >= contentLengthBeg.size()
																			&& line.compare(0, contentLengthBeg.size(), contentLengthBeg) == 0) {
																								issLine.str(line);
																								issLine.clear();
																								string tmpStr;
																								issLine >> tmpStr >> contentLength;
																								break;
																}
								}

								// get the content
								string content;
								const string contentBeg = "OPENSE.RECORD.HTTP_CONTENT:";
								while(getline(_rawPagesFile, line)) {
																if(line.size() >= contentBeg.size()
																			&& line.compare(0, contentBeg.size(), contentBeg) == 0) {
																								content.reserve(contentLength);
																								char ch;
																								while(contentLength-- > 0 && _rawPagesFile.get(ch))
																																content += ch;
																								break;
																}
								}
								// do some html preprocessing
								string ret[8];
								HtmlPreprocesser::delBetweenHtmlTags(content, "script", ret[0]);
								HtmlPreprocesser::delBetweenHtmlTags(ret[0], "style", ret[1]);
								HtmlPreprocesser::delComments(ret[1], ret[2]);
								HtmlPreprocesser::delTags(ret[2], ret[3]);
								HtmlPreprocesser::replaceStr(ret[3], "&nbsp", " ", ret[4]);
								HtmlPreprocesser::delOddchar(ret[4]," ", ret[5]);
								HtmlPreprocesser::mergeSpaces(ret[5], ret[6]);

								vector<string> words;

								issLine.str(ret[6]);
								issLine.clear();

								for(string word; issLine >> word; )
																words.push_back(word);

								_dictFile << WORDS_URL_BEGIN_TAG << " " << url.getUrlStr() << endl;

								for(vector<string>::iterator it = words.begin(); it != words.end(); ++it) {
																//cout << (*it) << endl;
																_dictFile << (*it)  << endl;
								}
								_dictFile << flush;
								closeFiles();
}
