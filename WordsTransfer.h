
#ifndef WORDSTRANSFER_H
#define WORDSTRANSFER_H

#include <fstream>
#include <string>
#include "Url.h"

using namespace std;

class WordsTransfer
{
public:
WordsTransfer();
void openFiles(const string rawPagesPath, const string dictPath);
void creatWordsTransfer(const string rawPagesPath, const string dictPath,  Url &url);
void closeFiles();

private:
ifstream _rawPagesFile;
ofstream _dictFile;
};

#endif
