ifndef platform
	ifeq ($(OS),Windows_NT)
		platform=windows
	else
		UNAME := $(shell uname -s)
		ifeq ($(UNAME),Linux)
			platform=linux
		endif
		ifeq ($(UNAME),Darwin)
			platform=macos
		endif
	endif
endif

ifndef SDK_PATH
    ifeq ($(platform),linux)
        SDK_PATH=BlackmagicDeckLinkSDK/Linux/include
    else ifeq ($(platform),macos)
        SDK_PATH=BlackmagicDeckLinkSDK/Mac/include
    else ifeq ($(platform),windows)
        SDK_PATH=BlackmagicDeckLinkSDK/Win/include
    endif
endif

CXXFLAGS=-c -std=c++11 -Wall -I external/cppsocket -I $(SDK_PATH)
LDFLAGS=-lpthread -ldl
ifeq ($(platform),macos)
LDFLAGS+=-framework CoreFoundation
endif

SOURCES=external/cppsocket/Acceptor.cpp \
	external/cppsocket/Connector.cpp \
	external/cppsocket/Network.cpp \
	external/cppsocket/Socket.cpp \
	$(SDK_PATH)/DeckLinkAPIDispatch.cpp \
	src/main.cpp \
	src/BMDSplit.cpp
OBJECTS=$(SOURCES:.cpp=.o)

BINDIR=./bin
EXECUTABLE=bmdsplit

all: directories $(SOURCES) $(EXECUTABLE)

debug: CXXFLAGS+=-DDEBUG -g
debug: directories $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(BINDIR)/$@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -rf src/*.o external/cppsocket/*.o $(BINDIR)/$(EXECUTABLE) $(BINDIR)

directories: ${BINDIR}

${BINDIR}: 
	mkdir -p ${BINDIR}
