CFLAGS = `pkg-config --libs --cflags gtk+-2.0`
LDFLAGS = `pkg-config --libs --cflags gtk+-2.0`
manager: manager.o
	gcc -o $@ $? $(LDFLAGS)
.phony = clean
clean:
	rm *.o manager
