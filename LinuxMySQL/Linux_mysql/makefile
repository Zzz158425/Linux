SRCS:=$(wildcard	*.c)
EXES:=$(patsubst	%.c,%,$(SRCS))
CC:=gcc
all:$(EXES)
%:%.c
	$(CC)	$^	-o	$@	-g	-lpthread	-lmysqlclient
clean:
	$(RM)	$(EXES)
rebuild:clean	all
