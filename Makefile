default: all

TARGETS=bin/mcp1 bin/mcp2 bin/mcp3 bin/mcp4 bin/mcp6 bin/mcp7 bin/mcp8 bin/mcp9 bin/mcp10
CFLAGS=-std=c99 -Wall -Werror -Wextra -Wno-unused -pedantic

all: $(TARGETS) test

bin/mcp1: mcp1.o
bin/mcp2: mcp2.o
bin/mcp3: mcp3.o
bin/mcp4: mcp4.o
bin/mcp6: mcp6.o
bin/mcp7: mcp7.o rijndael.o
bin/mcp8: mcp8.o

bin/mcp9: mcp9.o
bin/mcp10: mcp10.o rijndael.o

bin/%:
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	
clean:
	$(RM) $(TARGETS) *.o

test:
	#./run-tests-1.sh
	./run-tests-2.sh