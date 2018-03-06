BIN_FILES  = client server keys

CC = gcc


CCGLAGS =	-Wall  -g

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lpthread


all: CFLAGS=$(CCGLAGS)
all: $(BIN_FILES)
.PHONY : all

test: keys.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

client: client.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

server: server.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(BIN_FILES) *.o

.SUFFIXES:
.PHONY : clean
