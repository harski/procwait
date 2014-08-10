.TH PROCWAIT 1 "VERSION"

.SH NAME
procwait \- wait for process to terminate
.SH SYNOPSIS
\fBprocwait\fP [\fIOPTIONS\fP] \fIPID\fP
.SH DESCRIPTION
\fBprocwait\fP allows to wait until a process identified by PID is terminated.
.SH OPTIONS
.TP
\fB-h\fP, \fB--help\fP
Shows brief help and exits.
.TP
\fB-s\fP \fINUM\fP, \fB--sleep\fP \fINUM\fP
Time to sleep (in seconds) between process checks.
.TP
\fB-V\fP, \fB--version\fP
Shows program version and exits.
.TP
\fB-v\fP, \fB--verbose\fP
Turn on extra output.
.SH COPYRIGHT
Copyright (c) 2013-2014 Tuomo Hartikainen. Procwait is free software; see the
sources for copying conditions.
.SH AUTHOR
Tuomo Hartikainen <tth@harski.org>.
.SH SEE ALSO
\fBwait(1p)\fP