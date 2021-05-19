DESCRIPTION = "HumidityServer"
LICENSE = "GPLv3"
VERSION = 0.5


SRC += database.c
SRC += humserver.c
OBJ = $(SRC:.c=.o)

CC=gcc
CFLAGS += -Wall
CFLAGS += -L/usr/lib/mysql
CFLAGS += -lmysqlclient
CFLAGS += -I.


humserver: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) humserver

# make variables:
#  @ contains target of each rule
#  ^ contains dependencies of each rule
#  < first item of dependency list
# use all variables with $
#
# one peculiarity of the make syntax, as opposed to say the bourne shell syntax, 
# is that only the first character following the dollar sign is considered to be 
# the variable name. If we want to use longer names, we have to parenthesize the 
# name before applying the dollar sign to extract its value
# http://nuclear.mutantstargoat.com/articles/make/
# http://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/
