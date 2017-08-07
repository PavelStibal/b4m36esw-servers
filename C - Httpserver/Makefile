CXX = g++
CXXFLAGS = -Wall -O2 -std=c++0x
INCLUDES = -I/path/to/libmicrohttpd/include/ -I/path/to/zlib/include/
LFLAGS = -L/path/to/libmicrohttpd/lib/ -L/path/to/zlib/lib/
LIBS = -lmicrohttpd -lz -lpthread
EXECUTABLE_NAME = Httpserver

all:
	$(CXX) *.cpp $(INCLUDES) $(CXXFLAGS) -o $(EXECUTABLE_NAME) $(LFLAGS) $(LIBS)

clean:
	rm $(EXECUTABLE_NAME)
