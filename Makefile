default: all

TARGETS=bin/mcp1 bin/mcp2 bin/mcp3 bin/mcp4 bin/mcp6 bin/mcp7 bin/mcp8 \
	bin/mcp9 bin/mcp10 bin/mcp11 bin/mcp12 bin/mcp13 bin/mcp14 bin/mcp15 bin/mcp16 \
	bin/mcp17 bin/mcp18 bin/mcp19 bin/mcp20 bin/mcp21 bin/mcp22 bin/mcp23
CFLAGS=-std=c99 -Wall -Werror -Wextra -Wno-unused -pedantic -g

all: clean $(TARGETS) test

bin/mcp1: mcp1.o
bin/mcp2: mcp2.o
bin/mcp3: mcp3.o
bin/mcp4: mcp4.o
bin/mcp6: mcp6.o
bin/mcp7: mcp7.o rijndael.o
bin/mcp8: mcp8.o

bin/mcp9: mcp9.o
bin/mcp10: mcp10.o rijndael.o
bin/mcp11: mcp11.o rijndael.o
bin/mcp12: mcp12.o rijndael.o
bin/mcp13: mcp13.o rijndael.o
bin/mcp14: mcp14.o rijndael.o
bin/mcp15: mcp15.o
bin/mcp16: mcp16.o rijndael.o

bin/mcp17: mcp17.o rijndael.o
bin/mcp18: mcp18.o rijndael.o
bin/mcp19: mcp19.o rijndael.o
bin/mcp20: mcp20.o rijndael.o
bin/mcp21: mcp21.o
bin/mcp22: mcp22.o
bin/mcp23: mcp23.o

bin/%:
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)
	
clean:
	$(RM) $(TARGETS) *.o

test:
	./run-tests-1.sh
	./run-tests-2.sh
	./run-tests-3.sh
