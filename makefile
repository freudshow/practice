CROSS			=arm-none-linux-gnueabi-
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

$(MKCCTII): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCCTII)

$(MKCJQIII): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII)

$(DEFINCJQIII): $(SRC)
	$(CC) $(MKCJQIIILISTEN) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII)


clean:
	@rm -vf $(BIN) *.o *~
