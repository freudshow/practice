# 1 "select.c"
# 1 "<command-line>"
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/stdc-predef.h" 1 3 4
# 30 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/stdc-predef.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/predefs.h" 1 3 4
# 31 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/stdc-predef.h" 2 3 4
# 1 "<command-line>" 2
# 1 "select.c"
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 1 3 4
# 24 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/features.h" 1 3 4
# 364 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/features.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/cdefs.h" 1 3 4
# 385 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/cdefs.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/wordsize.h" 1 3 4
# 386 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/cdefs.h" 2 3 4
# 365 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/features.h" 2 3 4
# 388 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/features.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/gnu/stubs.h" 1 3 4






# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/gnu/stubs-soft.h" 1 3 4
# 8 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/gnu/stubs.h" 2 3 4
# 389 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/features.h" 2 3 4
# 25 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4


# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/types.h" 1 3 4
# 27 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/types.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/wordsize.h" 1 3 4
# 28 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
# 121 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/types.h" 3 4
# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/typesizes.h" 1 3 4
# 122 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/types.h" 2 3 4


__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef void * __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;


__extension__ typedef int __fsword_t;

__extension__ typedef int __ssize_t;


__extension__ typedef long int __syscall_slong_t;

__extension__ typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;
# 28 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4


# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/select.h" 1 3 4
# 31 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4


# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/sigset.h" 1 3 4
# 22 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/sigset.h" 3 4
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 34 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4



typedef __sigset_t sigset_t;





# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/time.h" 1 3 4
# 73 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/time.h" 3 4


typedef __time_t time_t;



# 120 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/time.h" 3 4
struct timespec
  {
    __time_t tv_sec;
    __syscall_slong_t tv_nsec;
  };
# 44 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4

# 1 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/time.h" 1 3 4
# 30 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/bits/time.h" 3 4
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 46 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 2 3 4


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 64 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 96 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4

# 106 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 118 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 131 "/home/lhl/soft/arm-2014.05/arm-none-linux-gnueabi/libc/usr/include/sys/select.h" 3 4

# 2 "select.c" 2

fd_set rfds, wfds;

int main(void)
{
    int fd = -1;

    if(open(fd, "/home/lhl/test.txt"))
        perror("open error");
    do { unsigned int __i; fd_set *__arr = (rfds); for (__i = 0; __i < sizeof (fd_set) / sizeof (__fd_mask); ++__i) ((__arr)->__fds_bits)[__i] = 0; } while (0);
    ((void) (((rfds)->__fds_bits)[((fd) / (8 * (int) sizeof (__fd_mask)))] |= ((__fd_mask) 1 << ((fd) % (8 * (int) sizeof (__fd_mask))))));
    select(10, rfds, NULL, NULL);
    if(((((rfds)->__fds_bits)[((fd) / (8 * (int) sizeof (__fd_mask)))] & ((__fd_mask) 1 << ((fd) % (8 * (int) sizeof (__fd_mask))))) != 0))
        printf("file is ready\n");
    exit(0);
}
