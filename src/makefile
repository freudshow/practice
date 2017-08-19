all:sqltest
	arm-none-linux-gnueabi-gcc -o ../bin/sqltest -I ../inc -L ../lib -lsqlite3 sqltest.c
	arm-none-linux-gnueabi-strip ../bin/sqltest
