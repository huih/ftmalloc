cxx=g++
TARGET = ftmalloc
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o, $(SOURCES))
INC_DIR = $(shell pwd)

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(cxx) -g -o $@ $(OBJECTS) -I$(INC_DIR)

.PHONY : clean
clean:
	-rm -f $(TARGET)
	-rm -f *.o

base_ctl.o : base_ctl.cpp base_ctl.h
main.o : main.cpp mdb_ctl.h
mdb_blank_ctl.o : mdb_blank_ctl.cpp mdb_blank_ctl.h base_ctl.h mdb_ctl_record.h
mdb_ctl_record.o : mdb_ctl_record.cpp mdb_ctl_record.h base_ctl.h mdb_ctl_record.h
mdb_table_ctl.o : mdb_table_ctl.cpp mdb_table_ctl.h base_ctl.h mdb_ctl_record.h
mdb_head_ctl.o : mdb_head_ctl.cpp mdb_head_ctl.h base_ctl.h mdb_ctl_record.h
mdb_ctl.o : mdb_ctl.cpp mdb_ctl.h mdb_head_ctl.h mdb_table_ctl.h mdb_blank_ctl.h
