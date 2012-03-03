CFLAGS = -I../../minifs/build-df3120/staging/usr/include
LDFLAGS = -L../../minifs/build-df3120/staging/usr/lib -lz -lpng14

CC = arm-v4t-linux-uclibcgnueabi-gcc


.PHONY: all clean


all: disppic


disppic: main.c 
	$(CC) -o $@ main.c $(CFLAGS) $(LDFLAGS)

run:
	./disppic TEST.png

clean:
	rm -f disppic *~

