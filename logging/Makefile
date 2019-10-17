SRC := $(wildcard *.c)
HEADERS := $(wildcard *.h)


test: $(SRC) $(HEADERS)
	gcc -g -o test $(SRC) -Wall -pthread -I ..
	
clean:
	rm *.log
	rm test