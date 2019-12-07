SRC := $(wildcard log*.c)
HEADERS := $(wildcard *.h)
FALGS := -Wall -pthread -I .. -g -rdynamic -O0
OBJS := test test_backtrace test_async

all: $(OBJS)


test: test.c $(SRC) $(HEADERS)
	gcc -o $@ $^ $(FALGS)

test_backtrace: test_backtrace.c $(SRC) $(HEADERS)
	gcc -o $@ $^ $(FALGS)

test_async: test_async.c $(SRC) $(HEADERS)
	gcc -o $@ $^ $(FALGS)

clean:
	rm -rf *.log
	rm -rf $(OBJS)