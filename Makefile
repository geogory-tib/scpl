CC = cc
CCFLAGS = -Wall -Wextra

default:
	$(CC) $(CCFLAGS)  src/*.c -o scpl

debug:
	$(CC) $(CCFLAGS) -g  -fsanitize=address -fno-omit-frame-pointer -static-libasan -DDEBUG src/*.c -o scpl

clean:
	rm scpl
