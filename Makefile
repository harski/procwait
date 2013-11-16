
target=pidwait
srcfile=pidwait.c

all: $(target)

debug: $(target)-debug

$(target): $(srcfile)
	cc -o $@ -O2 -std=c99 $<

$(target)-debug: $(srcfile)
	cc -o $@ -DDEBUG -Wall -Wextra -pedantic -std=c99 $<

clean:
	rm -f $(target)
	rm -f $(target)-debug
