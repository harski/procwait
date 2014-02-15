
cc=cc
cflags=-O2 -std=c99 -Wall -Wextra -pedantic
target=procwait
objs=procwait.o stat.o

all: $(target)

$(target): $(objs)
	cc -o $@ $(cflags) $(objs)

procwait.o: procwait.c procwait.h
	$(cc) -c $(cflags) $< -o $@

stat.o: stat.c stat.h
	$(cc) -c $(cflags) $< -o $@

clean:
	rm -f $(target) $(objs)
