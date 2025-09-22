CC = cc
CFLAGS = -g
LDFLAGS = -lm

#MS bitfields (default on Win) break packed structs
#UCRT definition for things that work differently between glibc and UCRT
ifdef WIN
CFLAGS += -mno-ms-bitfields -DUCRT
endif

ifdef MAP
MAPFLAGS != gmt-config --cflags
MAPLIB != gmt-config --libs
CFLAGS += -DMAPPING $(MAPFLAGS)
LDFLAGS += $(MAPLIB)
endif

.PHONY: all clean

main: main.c decode.o logger.o adsb.h
	$(CC) $(CFLAGS) main.c decode.o logger.o $(LDFLAGS) -o main

all: main test

test: test.c decode.o adsb.h
	$(CC) $(CFLAGS) test.c decode.o $(LDFLAGS) -o test

decode.o: decode.c decode.h adsb.h
	$(CC) $(CFLAGS) -c decode.c

logger.o: logger.c logger.h adsb.h
	$(CC) $(CFLAGS) -c logger.c

clean:
	rm -f ./*.o ./test ./main

