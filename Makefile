CC = cc
CFLAGS = -g -mno-ms-bitfields	#MS bitfields (default on Win) break packed structs
LDFLAGS = -lm

test: test.c decode.o adsb.h
	$(CC) $(CFLAGS) test.c decode.o $(LDFLAGS) -o test

decode.o: decode.c adsb.h
	$(CC) $(CFLAGS) -c decode.c

clean:
	rm -f ./*.o ./test
