cxx=g++
TARGET = ftmalloc
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o, $(SOURCES))
INC_DIR = $(shell pwd)
LIBRARYS	= -pthread
MACROS = -DC_STRING_FUNC

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(cxx) $(MACROS) -g -o $@ $(OBJECTS) -I$(INC_DIR) $(LIBRARYS)

.PHONY : clean
clean:
	-rm -f $(TARGET)
	-rm -f *.o

%.o : %.cpp
	$(cxx) $(MACROS) -c -o $@ -I. $<

