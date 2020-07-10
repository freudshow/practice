CROSS			=arm-linux-gnueabihf-
CC				=$(CROSS)gcc
TARGET			=serial
MKCCTI			=jzqi
MKCCTII			=jzqii
MKZBIII			=zbiii
MKCJQIII		=cjqiii
DEFINCCTI		=-DCCTI
DEFINCCTII		=-DCCTII
DEFINCCTIII		=-DCCTIII
DEFINCJQIII		=-DCJQIII
BIN				=./bin/$(TARGET)
SRC				=./src/serial.c ./src/lib.c
INC				=-I ./inc
OBJ				=-o $(BIN)
CFLAGS			=-g -Wall
CP				=cp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC)
	
$(MKCCTI): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCCTI)

$(MKCCTII): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCCTII)

$(MKCJQIII): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII)

$(DEFINCJQIII): $(SRC)
	$(CC) $(MKCJQIIILISTEN) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII)

stack: DSAAC2eCode/stackli.c DSAAC2eCode/teststkl.c
	$(CC) -o ./bin/stack $(CFLAGS) DSAAC2eCode/stackli.c DSAAC2eCode/teststkl.c  $(INC)

clean:
	@rm -vf $(BIN) *.o *~
