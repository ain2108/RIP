CC=gcc

INCLUDES= -Iincludes
CCFLAGS= -g -Wall $(INCLUDES)
LDFLAGS= -lpthread
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
TEMP=$(OBJECTS:src/%.o=%.o)
TARGET=router

all: $(TARGET)
	mkdir -p object_files
	mv *.o object_files
	cat commands.txt

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(TEMP) $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

clean:
	rm -f object_files/*.o $(TARGET)
