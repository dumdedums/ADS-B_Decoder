CC = gcc
CFLAGS = -g
LDFLAGS = -lm

test: test.c decode.o adsb.h
	$(CC) $(CFLAGS) test.c decode.o $(LDFLAGS) -o test

decode.o: decode.c adsb.h
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f ./*.o ./test
