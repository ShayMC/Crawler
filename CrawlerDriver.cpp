
#include <iostream>
#include <cstdlib>
#include "Crawler.h"
#include "Common.h"

int main(int argc, char **argv)
{
								if(argc != 2) {
																cerr << "Usage: " << argv[0] << "numThreads" << endl;
																exit (1);
								}

								int numThreads = atoi(argv[1]);
								if(numThreads <= 0) {
																cerr << "numThreads <= 0 error" << endl;
																exit(1);
								}

								ifstream urlSeedIfs(URL_PATH.c_str());
								if(!urlSeedIfs) {
																cerr << "open file " << URL_PATH << " error" << endl;
																exit(1);
								}

								ofstream rawPagesOfs(RAW_PAGES_PATH.c_str());
								if(!rawPagesOfs) {
																cerr << "open file " << RAW_PAGES_PATH << " error" << endl;
																exit(1);
								}

								Crawler crawler(urlSeedIfs, rawPagesOfs, numThreads);
								crawler.multiPthreadCrawl();

								return 0;
}
