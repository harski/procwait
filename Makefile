
cc=cc
cflags=-O2 -std=c99 -Wall -Wextra -pedantic
target=pidwait
objs=pidwait.o stat.o

all: $(target)

$(target): $(objs)
	cc -o $@ $(cflags) $(objs)

pidwait.o: pidwait.c pidwait.h
	$(cc) -c $(cflags) $< -o $@

stat.o: stat.c stat.h
	$(cc) -c $(cflags) $< -o $@

clean:
	rm -f $(target) $(objs)
