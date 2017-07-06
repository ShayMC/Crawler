#ifndef COMMON_H
#define COMMON_H

#include <string>

using std::string;

const std::string RAW_URL_BEGIN_TAG = "OPENSE.RECORD.URL:";
const std::string WORDS_URL_BEGIN_TAG = "WORDS.RECORD.URL:";
const std::string RAW_CONTENT_LENGTH_BEGIN_TAG = "OPENSE.RECORD.CONTENT_LENGTH:";
const std::string RAW_HTTP_CONTENT_BEGIN_TAG = "OPENSE.RECORD.HTTP_CONTENT:";

const std::string RAW_PAGES_PATH = "data/Html.data";
const std::string URL_PATH = "data/Urls.txt";
const std::string DICT_PATH = "data/Words.data";

#endif
