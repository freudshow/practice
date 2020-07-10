ARMPREFIX=arm-linux-gnueabihf-
CC=gcc

testshm:
	$(ARMPREFIX)$(CC) -o /home/floyd/dfebin/lib/libshm.so -fPIC -shared shm.c -lrt -lpthread
	$(ARMPREFIX)$(CC) -o /home/floyd/dfebin/bin/createregion -lshm createregion.c -lrt -lpthread -L"/home/floyd/dfebin/lib"
	$(ARMPREFIX)$(CC) -o /home/floyd/dfebin/bin/shmtest -lshm shmtest.c -lrt -lpthread -L"/home/floyd/dfebin/lib"
	scp -P 8888 /home/floyd/dfebin/lib/libshm.so  sysadm@193.168.1.101:/home/sysadm/dfebin/lib/libshm.so
	scp -P 8888 /home/floyd/dfebin/bin/createregion  sysadm@193.168.1.101:/home/sysadm/dfebin/bin/
	scp -P 8888 /home/floyd/dfebin/bin/shmtest  sysadm@193.168.1.101:/home/sysadm/dfebin/bin/
	
killshm:
	ps -ef | grep shmtest | awk '{print $2}' | xargs sudo kill -9
sql:sqltest
	arm-none-linux-gnueabi-gcc -o ../bin/sqltest -I ../inc -L ../lib -lsqlite3 sqltest.c
	arm-none-linux-gnueabi-strip ../bin/sqltest

all:testshm
