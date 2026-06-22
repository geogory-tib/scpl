CC = cc
CCFLAGS = -Wall -Wextra

default:
	$(CC) $(CCFLAGS)  src/*.c -o scpl

debug:
	$(CC) $(CCFLAGS) -g  -fsanitize=address -fno-omit-frame-pointer -static-libasan -DDEBUG src/*.c -o scpl

test: 
	./scpl test.scpl test.S
	as test.S -g -o test.out
	ld test.out -o test

clean:
	rm scpl
