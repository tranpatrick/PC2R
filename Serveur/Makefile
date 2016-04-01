CC=gcc
LFLAGS= -pthread
CFLAGS=-W -Wall -ansi -pedantic

BIN=bin/
OBJ=obj/
INCLUDE=include/
LIB=lib/
SRC=src/

H=$(INCLUDE)clientlist.h $(INCLUDE)funcserver.h

all: $(BIN)main

##test: test.c clientlist.o

#$(BIN)%: $(SRC)%.c $(OBJ)clientlist.o
$(BIN)%: $(OBJ)%.o $(OBJ)funcserver.o $(OBJ)clientlist.o $(OBJ)board.o $(OBJ)server.o
	@if [ -d $(BIN) ]; then : ; else mkdir $(BIN); fi
	$(CC) $(LFLAGS) -o $@ $^

$(OBJ)%.o: $(SRC)%.c
	@if [ -d $(OBJ) ]; then : ; else mkdir $(OBJ); fi
	$(CC) $(CFLAGS) -o $@ -c $^ 

cleantild:
	rm -rf $(SRC)*~ $(INCLUDE)*~

clean: cleantild
	rm -rf $(OBJ)*.o $(BIN)*
