all: webprobe

webprobe: webprobe.cpp
	g++ -Wall -g -lpthread webprobe.cpp -o webprobe

clean:
	rm -f webprobe
