CROSS			=arm-none-linux-gnueabi-
CC				=$(CROSS)gcc
TARGET			=serial
MKCCTII			=listen
MKCJQIII		=serialiii
DEFINCCTII		=-DCCTII
DEFINCJQIII		=-DCJQIII
DEFINLISTEN		=-DLISTEN
DEFINNOR			=-DNORMAL
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
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCCTII) $(DEFINLISTEN)

$(MKCJQIII): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII) $(DEFINNOR)


clean:
	@rm -vf $(BIN) *.o *~
