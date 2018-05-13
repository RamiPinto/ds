BIN_FILES  = services server test

CC = gcc
CCGLAGS =	-Wall  -g

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread


all: CFLAGS=$(CCGLAGS)
all: $(BIN_FILES)
.PHONY : all

server: server.o libservices.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -L. -lservices -lrt

test: test.o libservices.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ -L. -lservices -lrt

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

libservices.a:
	ar -rv libservices.a services.o

services:
	$(CC) -Wall -c services.c

clean:
	rm -f $(BIN_FILES) *.o *.a

.SUFFIXES:
.PHONY : clean
