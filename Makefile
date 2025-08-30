CC = cc
CFLAGS = -g -mno-ms-bitfields	#MS bitfields (default on Win) break packed structs
LDFLAGS = -lm

main: main.c decode.o logger.o adsb.h
	$(CC) $(CFLAGS) main.c decode.o logger.o $(LDFLAGS) -o main

test: test.c decode.o adsb.h
	$(CC) $(CFLAGS) test.c decode.o $(LDFLAGS) -o test

decode.o: decode.c decode.h adsb.h
	$(CC) $(CFLAGS) -c decode.c

logger.o: logger.c logger.h adsb.h
	$(CC) $(CFLAGS) -c logger.c

clean:
	rm -f ./*.o ./test ./main
