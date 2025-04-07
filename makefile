CROSS			=arm-linux-gnueabihf-
CC				=$(CROSS)gcc
RKCC			=aarch64-none-linux-gnu-gcc
TARGET			=serial
MKCCTI			=jzqi
MKCCTII			=jzqii
MKZBIII			=zbiii
MKCJQIII		=cjqiii
MKTTU 			=ttu
MKE9361C0		=e9361c0
MKE9361RK		=e9361rk
DEFINCCTI		=-DCCTI
DEFINCCTII		=-DCCTII
DEFINCCTIII		=-DCCTIII
DEFINCJQIII		=-DCJQIII
DEFINETTU		=-DTTU
DEFINEE9361C0	=-DE9361_C0
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

$(MKTTU): $(SRC)
	$(CC) $(OBJ) $(CFLAS) $(SRC) $(INC) $(DEFINETTU)
	scp -P 8888  ${BIN} sysadm@192.168.31.101:/home/sysadm/bin/

$(MKE9361C0): $(SRC)
	$(CC) $(OBJ) $(CFLAS) $(SRC) $(INC) $(DEFINEE9361C0)

$(DEFINCJQIII): $(SRC)
	$(CC) $(MKCJQIIILISTEN) $(CFLAGS) $(SRC) $(INC) $(DEFINCJQIII)

$(MKE9361RK): $(SRC)
	$(RKCC) $(OBJ) $(CFLAGS) $(SRC) $(INC) $(DEFINEE9361C0)

stack: DSAAC2eCode/stackli.c DSAAC2eCode/teststkl.c
	$(CC) -o ./bin/stack $(CFLAGS) DSAAC2eCode/stackli.c DSAAC2eCode/teststkl.c  $(INC)

clean:
	@rm -vf $(BIN) *.o *~
