# Customize below to suit your system

# procwait version
VERSION = 1.1

# Paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

CC = cc
CFLAGS = -std=c99 -g -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=199309L
