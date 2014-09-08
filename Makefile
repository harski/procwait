include config.mk

TARGET=procwait
OBJS=go.o proc.o procwait.o strutil.o
MAN=$(TARGET).1

ifdef VERSION
VFLAG=-DVERSION=\"$(VERSION)\"
endif

all: $(TARGET) $(MAN)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS)

go.o: go.c go.h
	$(CC) -c $(CFLAGS) $< -o $@

proc.o: proc.c proc.h error.h go.h queue.h
	$(CC) -c $(CFLAGS) $< -o $@

procwait.o: procwait.c error.h proc.h queue.h config.mk
	$(CC) -c $(CFLAGS) $(VFLAG) $< -o $@

strutil.o: strutil.c strutil.h error.h
	$(CC) -c $(CFLAGS) $< -o $@

dist: clean
	mkdir -p $(TARGET)-$(VERSION)
	@cp -R LICENSE Makefile README config.mk procwait.1.mk *.c *.h \
		$(TARGET)-$(VERSION)
	@tar -cf $(TARGET)-$(VERSION).tar $(TARGET)-$(VERSION)
	@gzip $(TARGET)-$(VERSION).tar
	@rm -rf $(TARGET)-$(VERSION)

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
