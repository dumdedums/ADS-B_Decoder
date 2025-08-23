CC = gcc
LDFLAGS = -lm

test: test.c adsb.h
	$(CC) test.c $(LDFLAGS) -o test

decode.o: decode.c adsb.h
	$(CC) -c decode.c

clean:
	rm -f *.o
