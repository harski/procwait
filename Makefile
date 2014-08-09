include config.mk

TARGET=procwait
OBJS=go.o procwait.o stat.o
MAN=$(TARGET).1
VFLAG=-DVERSION=\"$(VERSION)\"

all: $(TARGET) $(MAN)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

go.o: go.c go.h
	$(CC) -c $(CFLAGS) $< -o $@

procwait.o: procwait.c error.h
	$(CC) -c $(CFLAGS) $(VFLAG) $< -o $@

stat.o: stat.c stat.h error.h
	$(CC) -c $(CFLAGS) $< -o $@

man: $(MAN)

$(MAN): $(MAN).mk
	sed s/VERSION/$(VERSION)/ < $< > $@

install: $(TARGET) $(MAN)
	@mkdir -p $(PREFIX)/bin
	install -m 0755 $(TARGET) $(PREFIX)/bin
	@mkdir -p $(MANPREFIX)/man1
	install -m 0644 $(MAN) $(MANPREFIX)/man1

uninstall:
	rm $(PREFIX)/bin/$(TARGET)
	rm $(MANPREFIX)/man1/$(MAN)

clean:
	rm -f $(TARGET) $(OBJS) $(MAN)

.PHONY: all clean install man uninstall
