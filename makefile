CROSS			=arm-none-linux-gnueabi-
CC				=$(CROSS)gcc
TARGET			=serial
BIN				=./bin/$(TARGET)
SRC				=./src/serial.c ./src/lib.c
INC				=-I ./inc
OBJ				=-o $(BIN)
CFLAGS			=-g -Wall
CP				=cp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(OBJ) $(CFLAGS) $(SRC) $(INC)

clean:
	@rm -vf $(BIN) *.o *~