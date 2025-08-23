CC = gcc
CFLAGS = -g
LDFLAGS = -lm

test: test.c adsb.h decode.o
	$(CC) $(CFLAGS) decode.o test.c $(LDFLAGS) -o test

decode.o: decode.c adsb.h
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f ./*.o ./test
