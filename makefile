

all: CrawlerDriver.cpp
	g++ -Wall -Wvla -Werror -c HtmlPreprocesser.cpp -o HtmlPreprocesser.o
	g++ -Wall -Wvla -Werror -c WordsTransfer.cpp -o WordsTransfer.o
	g++ -c Url.cpp -o Url.o
	g++ -c HttpContent.cpp -o HttpContent.o
	g++ -Wall -Wvla -Werror -c HttpHeader.cpp -o HttpHeader.o
	g++ -c HttpClient.cpp -o HttpClient.o
	g++ -c Crawler.cpp -o Crawler.o
	g++ -Wall -Wvla -Werror CrawlerDriver.cpp HtmlPreprocesser.o WordsTransfer.o HttpHeader.o Url.o HttpContent.o HttpClient.o Crawler.o -lpthread -o Crawler


clean:
	rm HtmlPreprocesser.o
	rm WordsTransfer.o
	rm Url.o
	rm HttpContent.o
	rm HttpHeader.o
	rm HttpClient.o
	rm Crawler.o
	rm Crawler
	    @echo Clean done
