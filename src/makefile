testshm:
	gcc -o /home/floyd/dfebin/lib/libshm.so -fPIC -shared shm.c -lrt -lpthread
	gcc -o /home/floyd/dfebin/bin/createregion -lshm createregion.c -lrt -lpthread -L"/home/floyd/dfebin/lib"
	gcc -o /home/floyd/dfebin/bin/shmtest -lshm shmtest.c -lrt -lpthread -L"/home/floyd/dfebin/lib"
killshm:
	ps -ef | grep shmtest | awk '{print $2}' | xargs sudo kill -9
sql:sqltest
	arm-none-linux-gnueabi-gcc -o ../bin/sqltest -I ../inc -L ../lib -lsqlite3 sqltest.c
	arm-none-linux-gnueabi-strip ../bin/sqltest

all:testshm
