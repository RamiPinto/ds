BIN_FILES  = keys client server

CC = gcc
CCGLAGS =	-Wall  -g

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread


all: CFLAGS=$(CCGLAGS)
all: $(BIN_FILES)
.PHONY : all

client: client.o libkeys.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -L. -lkeys

server: server.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

libkeys.a:
	ar -rv libkeys.a keys.o

keys:
	$(CC) -Wall -c keys.c

clean:
	rm -f $(BIN_FILES) *.o *.a

.SUFFIXES:
.PHONY : clean
