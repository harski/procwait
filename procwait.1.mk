.TH PROCWAIT 1 "VERSION"

.SH NAME
procwait \- wait for process to terminate
.SH SYNOPSIS
\fBprocwait\fP [\fIOPTIONS\fP] \fIPID\fP...
.SH DESCRIPTION
\fBprocwait\fP allows to wait until a process or (processes) identified by PID
is terminated.
.SH OPTIONS
.TP
\fB-h\fP, \fB--help\fP
Shows brief help and exits.
.TP
\fB-n \fINAME\fP, \fB--name \fINAME\fP
Parse PID from process NAME.
.TP
\fB-q\fP, \fB--quiet\fP
Only print essential output and errors.
.TP
\fB-s\fP \fINUM\fP[ms], \fB--sleep\fP \fINUM\fP[ms]
Seconds (milliseconds) to sleep between process checks.
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
