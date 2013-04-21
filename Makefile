default: all

TARGETS=bin/mcp1 bin/mcp2 bin/mcp3 bin/mcp4
CFLAGS=-std=c99 -Wall -Werror -Wextra -Wno-unused -pedantic

all: clean $(TARGETS) test

bin/mcp1: mcp1.o
bin/mcp2: mcp2.o
bin/mcp3: mcp3.o
bin/mcp4: mcp4.o

bin/%:
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	
clean:
	$(RM) $(TARGETS) *.o

test:
	./run-tests.sh