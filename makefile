appname := oddscalculator

CXX := g++
CXXFLAGS := -g -O3 -std=c++17 -pthread -lstdc++fs -I/usr/include/eigen3 -I include -L/usr/lib/x86_64-linux-gnu -L/usr/boost/ -no-pie
LDFLAGS := 

LDLIBS := -lsqlite3 -lboost_iostreams -lstdc++fs

srcfiles := $(shell find . -maxdepth 10 -name "*.cpp")
objects  := $(patsubst %.cpp, %.o, $(srcfiles))
$(info $$srcfiles is [${srcfiles}])
all: $(appname)

$(appname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(appname) $(objects) $(LDLIBS)
depend: .depend
.depend: $(srcfiles)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^>>./.depend;
clean:
	rm -f $(objects)
dist-clean: clean
	rm -f *~ .depend
include .depend
