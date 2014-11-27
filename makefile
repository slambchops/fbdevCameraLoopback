CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=main.cpp camera.cpp display.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=loopback

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *o $(EXECUTABLE)
