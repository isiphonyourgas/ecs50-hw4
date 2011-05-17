all: webprobe

webprobe: webprobe.cpp
	g++ -Wall -g -lpthread -lm webprobe.cpp -o webprobe

clean:
	rm -f webprobe
