CFLAGS = -Wall -O2

all: apu-ehci

apu-ehco: apu-ehci.o
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	@rm -f *.o apu-ehci
