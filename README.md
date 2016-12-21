PROCWAIT
========

Procwait is a program that blocks until processes it tracks terminate. It is
analogous to `wait` but is used to wait for any processes, not only its own
children. When all of the tracked processes terminate, procwait terminates too.
Procwait is a Linux program written in C99.


HOW IT WORKS
------------

A process is perceived to be running if there exists a directory called
`/proc/PID`. However, simply relying on polling whether the directory still
exists is racy: the tracked process can terminate and a new process can spawn
with the same PID between polls. It may not be very likely in most use-cases as
there are `/proc/sys/kernel/pid_max` available PIDs, but it is possible.

To reduce the possibility of a race condition procwait records the name and the
start time of the tracked process at startup. The content of `/proc/PID/stat`
is read every time a process is polled, and if the process' start time or the
process' name doesn't match the recorded information the tracked process is
considered terminated.


COMPILING, INSTALLING AND UNINSTALLING
--------------------------------------

The intended workflow expects a `make` utility. Edit the config.mk file to suit
your system, and build the program by running

    $ make

The Makefile also has *install* and *uninstall* targets. After you have
adjusted the variables in config.mk and build the program you can install it by
issuing

    $ make install

and uninstall with

    $ make uninstall


USAGE
-----

For information about the program usage and options see the procwait manpage
procwait.1. Man page is generated when you build the program or you can build
it separately by running

    $ make procwait.1


BUGS
----

If you have found a bug in the program, please report it to the author
directly. See AUTHOR section below for contact information.


AUTHOR
------

Written by Tuomo Hartikainen <tth@harski.org>.


LICENSE
-------

Licensed under the 2-clause BSD license. See the supplied LICENSE file for
details.
