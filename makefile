

all: CrawlerDriver.cpp
	g++ -Wall -Wvla -Werror -c HNode.cpp -o HNode.o
	g++ -Wall -Wvla -Werror -c HtmlPreprocesser.cpp -o HtmlPreprocesser.o
	g++ -Wall -Wvla -Werror -c WordsTransfer.cpp -o WordsTransfer.o
	g++ -Wall -Wvla -Werror -c Url.cpp -o Url.o
	g++ -Wall -Wvla -Werror -c HttpContent.cpp -o HttpContent.o
	g++ -Wall -Wvla -Werror -c HttpHeader.cpp -o HttpHeader.o
	g++ -Wall -Wvla -Werror -c HttpClient.cpp -o HttpClient.o
	g++ -Wall -Wvla -Werror -c Crawler.cpp -o Crawler.o
	g++ -Wall -Wvla -Werror CrawlerDriver.cpp HNode.o HtmlPreprocesser.o WordsTransfer.o HttpHeader.o Url.o HttpContent.o HttpClient.o Crawler.o -lpthread -o Crawler


clean:
	rm HtmlPreprocesser.o
	rm HNode.o
	rm WordsTransfer.o
	rm Url.o
	rm HttpContent.o
	rm HttpHeader.o
	rm HttpClient.o
	rm Crawler.o
	rm Crawler
	    @echo Clean done
