
target=pidwait
srcfile=pidwait.c

all: $(target)

debug: $(target)-debug

$(target): $(srcfile)
	cc -o $@ -O2 $<

$(target)-debug: $(srcfile)
	cc -o $@ -DDEBUG -Wall -Wextra -pedantic $<

clean:
	rm -f $(target)
	rm -f $(target)-debug
