# WWWSpod Makefile

all:  input.cgi pinger.cgi output.cgi wsctl

cgis: input.cgi pinger.cgi output.cgi

input.cgi: input.c webspod.h config.h
	gcc -o input.cgi input.c

pinger.cgi: pinger.c webspod.h config.h
	gcc -o pinger.cgi pinger.c

output.cgi: output.c webspod.h config.h
	gcc -o output.cgi output.c

wsctl: wsctl.c webspod.h config.h
	gcc -o wsctl wsctl.c

