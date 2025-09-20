CC = g++
CFLAGS = -std=c++17 -O2 -I.. -I../lv2 -I../jack -I./include
LDFLAGS = 
SOURCES = http_server.cpp
TARGET = audiosystembridge-http

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
