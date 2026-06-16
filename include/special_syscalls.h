#ifndef _SPECIAL_SYSCALLS_
#define _SPECIAL_SYSCALLS_


#include <sys/ioctl.h>


#include "sandbox.h"
#include "syscalls_table.h"

// ioctl:
#define I_POP       21251
#define I_FLUSH     21253
#define I_SETSIG    21257
#define I_SRDOPT    21254
#define I_SWROPT    21264
#define I_SENDFD    21261
#define I_ATMARK    21268
#define I_CKBAND    21270
#define I_CANPUT    21271
#define I_LINK      21272
#define I_UNLINK    21273
#define I_PLINK     21274
#define I_PUNLINK   21275
#define I_GETCLTIME 21276
#define I_SETCLTIME 21277
#define I_GETBAND   21278
#define I_GWROPT    21265
#define I_NREAD     21249
#define I_GRDOPT    21255
#define I_GETSIG    21258
#define I_PUSH      21250
#define I_LOOK      21252
#define I_FIND      21259
#define I_PEEK      21260
#define I_STR       21256
#define I_FLUSHBAND 21263
#define I_FDINSERT  21267
#define I_RECVFD    21262
#define I_LIST      21279

// prctl:
#define PR_SET_DUMPABLE	        4
#define PR_SET_KEEPCAPS	        8
#define PR_SET_PDEATHSIG        1
#define PR_SET_NO_NEW_PRIVS	    38
#define PR_SET_CHILD_SUBREAPER  36
#define PR_SET_NAME	            15
#define PR_GET_NAME	            16
#define PR_GET_PDEATHSIG        2
#define PR_GET_CHILD_SUBREAPER	37
#define PR_CAPBSET_READ         23
#define PR_CAPBSET_DROP         24
#define PR_SET_SPECULATION_CTRL 53
#define PR_SET_SECCOMP          22

// futex:
#define FUTEX_WAIT        0
#define FUTEX_WAKE        1
#define FUTEX_FD          2
#define FUTEX_REQUEUE     3
#define FUTEX_CMP_REQUEUE 4
#define FUTEX_WAKE_OP     5
#define FUTEX_WAIT_BITSET 9
#define FUTEX_WAKE_BITSET 10
#define FUTEX_PRIVATE_FLAG   128
#define FUTEX_CLOCK_REALTIME 256

// mremap:
#define MREMAP_FIXED 2

// semctl:
#define IPC_RMID 0
#define IPC_SET  1
#define IPC_STAT 2
#define IPC_INFO 3
#define GETALL   13
#define SETVAL   16
#define SETALL   17
#define SEM_INFO 19

// fcntl:
#define F_DUPFD              0
#define F_GETFD              1
#define F_SETFD              2
#define F_GETFL              3
#define F_SETFL              4
#define F_SETOWN             8
#define F_GETOWN             9
#define F_SETSIG             10
#define F_GETSIG             11
#define F_SETLEASE           1024
#define F_GETLEASE           1025
#define F_NOTIFY             1026
#define F_SETPIPE_SZ         1031
#define F_GETPIPE_SZ         1032
#define F_ADD_SEALS          1033
#define F_GET_SEALS          1034

#define F_GETLK              5
#define F_SETLK              6
#define F_SETLKW             7
#define F_OFD_GETLK          36
#define F_OFD_SETLK          37
#define F_OFD_SETLKW         38
#define F_SETOWN_EX          15
#define F_GETOWN_EX          16

#define F_SETDELEG           1035
#define F_GETDELEG           1036
#define F_SET_RW_HINT        1041
#define F_GET_RW_HINT        1042
#define F_SET_FILE_RW_HINT   1043
#define F_GET_FILE_RW_HINT   1044


#define AS_INT(__n) ({ \
    received_args[__n].type = INT_TYPE; \
    received_args[__n].int_ = (int)raw_arguments[__n]; \
    received_args[__n].int_; \
})
#define DEFINE_SPECIAL_SYSCALL(__sys_name, __pid, __raw_arguments, __received_args, __body) \
u_int8_t process_syscall_##__sys_name(pid_t __pid, const reg_t __raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument __received_args[MAX_SYSCALL_ARGS_NUM]) { __body }


DEFINE_SPECIAL_SYSCALL(open, pid, raw_arguments, received_args, {
    u_int8_t argument_num = 2;
    received_args[0].type = STRING_TYPE;
    received_args[0].str = read_str_from_tracee(pid, raw_arguments[0]);

    received_args[1].type = INT_TYPE;
    const int flags = received_args[1].int_ = (int)raw_arguments[1];

    if (flags & O_CREAT || (flags & O_TMPFILE) == O_TMPFILE) {
        // scanning mode argument
        argument_num = 3;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (unsigned int)raw_arguments[2];
    }

    return argument_num;
})

DEFINE_SPECIAL_SYSCALL(openat, pid, raw_arguments, received_args, {
    u_int8_t argument_num = 3;
    received_args[0].type = INT_TYPE;
    received_args[0].int_ = (int)raw_arguments[0];

    received_args[1].type = STRING_TYPE;
    received_args[1].str = read_str_from_tracee(pid, raw_arguments[1]);

    received_args[2].type = INT_TYPE;
    const int oflag = received_args[2].int_ = (int)raw_arguments[2];

    if (oflag & O_CREAT || (oflag & O_TMPFILE) == O_TMPFILE) {
        // scanning mode argument
        argument_num = 4;
        received_args[3].type = UINT_TYPE;
        received_args[3].uint = (unsigned int)raw_arguments[3];
    }

    return argument_num;
})

u_int8_t process_syscall_ioctl(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    received_args[0].type = INT_TYPE;
    received_args[0].int_ = (int)raw_arguments[0];

    received_args[1].type = UINT_TYPE;
    const u_int32_t op = received_args[1].uint = (u_int32_t)raw_arguments[1];
    switch (op) {
    case I_POP:
    case I_FLUSH:
    case I_SETSIG:
    case I_SRDOPT:
    case I_SWROPT:
    case I_SENDFD:
    case I_ATMARK:
    case I_CKBAND:
    case I_CANPUT:
    case I_LINK:
    case I_UNLINK:
    case I_PLINK:
    case I_PUNLINK:
    // int
        received_args[2].type = INT_TYPE;
        received_args[2].int_ = (int)raw_arguments[2];
        break;
    case I_GETCLTIME:
    case I_SETCLTIME:
    case I_GETBAND:
    case I_GWROPT:
    case I_NREAD:
    case I_GRDOPT:
    case I_GETSIG: 
    {   // int*
        byte_t* temp_arr;
        size_t temp_len = 0;
        temp_arr = read_data_from_tracee(pid, raw_arguments[2], sizeof(int)); // read a single int value by the specified address
        received_args[2].type = INT_TYPE;
        memcpy(&received_args[2].int_, temp_arr, sizeof(int));
        free(temp_arr);
        break;
    }
    case I_PUSH:
    case I_LOOK:
    case I_FIND:
        // str    
        received_args[2].type = STRING_TYPE;
        received_args[2].str = read_str_from_tracee(pid, raw_arguments[2]);
        break;
    
    case I_PEEK:
        // arr
        received_args[2].type = ARRAY_TYPE;
        received_args[2].arr = read_data_from_tracee(pid, raw_arguments[2], received_args[2].arr_len);
        break;
    case I_STR:
    case I_FLUSHBAND:
    case I_FDINSERT:
    case I_RECVFD:
    case I_LIST:
        // addr
        received_args[2].type = ADDRESS_TYPE;
        received_args[2].addr[0] = received_args[2].addr[1] = (uintptr_t)raw_arguments[2];
        break;
    }
    return 2;
}


u_int8_t process_syscall_prctl(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    u_int32_t argument_num = 1;
    received_args[0].type = INT_TYPE;
    int op = received_args[0].int_ = (int)raw_arguments[0];

    switch (op) {
    case PR_SET_DUMPABLE:
    case PR_SET_KEEPCAPS:
    case PR_SET_PDEATHSIG:
    case PR_SET_NO_NEW_PRIVS:
    case PR_SET_CHILD_SUBREAPER:
    case PR_CAPBSET_READ:
    case PR_CAPBSET_DROP:    
        // int
        argument_num = 2;
        received_args[1].type = INT_TYPE;
        received_args[1].int_ = (int)raw_arguments[1];
        break;
    case PR_SET_NAME:
    case PR_GET_NAME:
        // str
        argument_num = 2;
        received_args[1].type = STRING_TYPE;
        received_args[1].str = read_str_from_tracee(pid, raw_arguments[1]);
        break;
    case PR_GET_PDEATHSIG:
    case PR_GET_CHILD_SUBREAPER: {
        // int*
        argument_num = 2;
        byte_t* temp_arr;
        size_t temp_len = 0;
        temp_arr = read_data_from_tracee(pid, raw_arguments[1], sizeof(int)); // read a single int value by the specified address
        received_args[1].type = INT_TYPE;
        memcpy(&received_args[1].int_, temp_arr, sizeof(int));
        free(temp_arr);
        break;
    }
    case PR_SET_SPECULATION_CTRL:
        argument_num = 3;
        received_args[1].type = received_args[2].type = INT_TYPE;
        received_args[1].int_ = (int)raw_arguments[1];
        received_args[2].int_ = (int)raw_arguments[2];
        break;
    case PR_SET_SECCOMP:
        argument_num = 3;    
        received_args[1].type = INT_TYPE;
        received_args[1].int_ = (int)raw_arguments[1];
        received_args[2].type = ADDRESS_TYPE;
        received_args[2].addr[0] = received_args[2].addr[1] = (uintptr_t)raw_arguments[2];
        break;
    }

    return argument_num;
}

// TODO: test tracing futex later (adjust main program to be able to trace futexes)
u_int8_t process_syscall_futex(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    uint8_t argument_num = 2;
    byte_t* temp_arr;
    size_t temp_len = 0;
    temp_arr = read_data_from_tracee(pid, raw_arguments[0], sizeof(u_int32_t)); // read a single int value by the specified address
    received_args[0].type = UINT_TYPE;
    memcpy(&received_args[0].uint, temp_arr, sizeof(u_int32_t));
    free(temp_arr);

    received_args[1].type = INT_TYPE;
    int op = received_args[1].int_ = (int)raw_arguments[1];
    op = (op & ~FUTEX_PRIVATE_FLAG) & ~FUTEX_CLOCK_REALTIME; // turning off thses flags

    switch (op) {
    case FUTEX_WAIT:
        argument_num = 4;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (u_int32_t)raw_arguments[2];

        received_args[3].type = ADDRESS_TYPE;
        received_args[3].addr[0] = received_args[3].addr[1] = (uintptr_t)raw_arguments[3];
        break;
    case FUTEX_FD:
    case FUTEX_WAKE:
        argument_num = 3;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (u_int32_t)raw_arguments[2];
        break;
    case FUTEX_REQUEUE:
        argument_num = 5;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (u_int32_t)raw_arguments[2];

        received_args[3].type = UINT_TYPE;
        received_args[3].uint = (u_int32_t)raw_arguments[3];

        temp_arr = read_data_from_tracee(pid, raw_arguments[4], sizeof(u_int32_t)); // read a single int value by the specified address
        received_args[4].type = UINT_TYPE;
        memcpy(&received_args[4].uint, temp_arr, sizeof(u_int32_t));
        free(temp_arr);
        break;
    case FUTEX_CMP_REQUEUE:
    case FUTEX_WAKE_OP:
        argument_num = 6;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (u_int32_t)raw_arguments[2];

        received_args[3].type = UINT_TYPE;
        received_args[3].uint = (u_int32_t)raw_arguments[3];

        temp_arr = read_data_from_tracee(pid, raw_arguments[4], sizeof(u_int32_t)); // read a single int value by the specified address
        received_args[4].type = UINT_TYPE;
        memcpy(&received_args[4].uint, temp_arr, sizeof(u_int32_t));
        free(temp_arr);

        received_args[5].type = UINT_TYPE;
        received_args[5].uint = (u_int32_t)raw_arguments[5];
        break;
    case FUTEX_WAIT_BITSET:
    case FUTEX_WAKE_BITSET:
        argument_num = 6;
        received_args[2].type = UINT_TYPE;
        received_args[2].uint = (u_int32_t)raw_arguments[2];

        received_args[3].type = ADDRESS_TYPE;
        received_args[3].addr[0] = received_args[3].addr[1] = (uintptr_t)raw_arguments[3];

        received_args[4].type = ADDRESS_TYPE;
        received_args[4].addr[0] = received_args[4].addr[1] = (uintptr_t)NULL; // is NULL anyway according to man7.org

        received_args[5].type = UINT_TYPE;
        received_args[5].uint = (u_int32_t)raw_arguments[5];
        break;
    }

    return argument_num;
}

u_int8_t process_syscall_mremap(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    u_int8_t argument_num = 4;
    received_args[0].type = ADDRESS_TYPE;
    received_args[0].addr[0] = received_args[0].addr[1] = (uintptr_t)raw_arguments[0];

    received_args[1].type = ULONG_TYPE;
    received_args[1].ulong = (unsigned long)raw_arguments[1];

    received_args[2].type = ULONG_TYPE;
    received_args[2].ulong = (unsigned long)raw_arguments[2];

    received_args[3].type = INT_TYPE;
    int flags = received_args[3].int_ = (int)raw_arguments[3];

    if ((flags & MREMAP_FIXED) == MREMAP_FIXED) {
        argument_num = 5;
        received_args[4].type = ADDRESS_TYPE;
        received_args[4].addr[0] = received_args[4].addr[1] = (uintptr_t)raw_arguments[4];
    }

    return argument_num;
}

u_int8_t process_syscall_getsockopt(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    u_int8_t argument_num = 5;
    byte_t *temp_arr = NULL;
    received_args[0].type = INT_TYPE;
    received_args[0].int_ = (int)raw_arguments[0];

    received_args[1].type = INT_TYPE;
    received_args[1].int_ = (int)raw_arguments[1];

    received_args[2].type = INT_TYPE;
    received_args[2].int_ = (int)raw_arguments[2];

    // first getting the optlen by pointer
    socklen_t optlen = 0;
    temp_arr = read_data_from_tracee(pid, raw_arguments[4], sizeof(socklen_t)); // read a single int value by the specified address
    memcpy(&optlen, temp_arr, sizeof(socklen_t));
    free(temp_arr);

    // then reading the array of optlen items in total
    received_args[3].type = ARRAY_TYPE;
    received_args[3].arr_len = (size_t)optlen;
    received_args[3].arr = read_data_from_tracee(pid, raw_arguments[3], optlen);

    received_args[4].type = ADDRESS_TYPE;
    received_args[4].addr[0] = received_args[4].addr[1] = (uintptr_t)raw_arguments[4];

    return argument_num;
}

u_int8_t process_syscall_semctl(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    u_int8_t argument_num = 3;
    received_args[0].type = INT_TYPE;
    received_args[0].int_ = (int)raw_arguments[0];

    received_args[1].type = INT_TYPE;
    received_args[1].int_ = (int)raw_arguments[1];

    received_args[2].type = INT_TYPE;
    int op = received_args[2].int_ = (int)raw_arguments[2];

    switch (op) {
    case SETVAL:
        // int
        argument_num = 4;
        received_args[3].type = INT_TYPE;
        received_args[3].int_ = (int)raw_arguments[3];
        break;
    case SEM_INFO:
    case IPC_INFO:
    case GETALL:
    case SETALL:
    case IPC_STAT:
    case IPC_SET:
        // addr
        argument_num = 4;
        received_args[3].type = ADDRESS_TYPE;
        received_args[3].addr[0] = received_args[3].addr[1] = (uintptr_t)raw_arguments[3];
        break;
    }

    return argument_num;
}

u_int8_t process_syscall_fcntl(pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]) {
    u_int8_t argument_num = 2;
    received_args[0].type = INT_TYPE;
    received_args[0].int_ = (int)raw_arguments[0];

    received_args[1].type = INT_TYPE;
    int op = received_args[1].int_ = (int)raw_arguments[1];

    switch (op) {
    case F_ADD_SEALS:
    case F_SETPIPE_SZ:
    case F_NOTIFY:
    case F_SETLEASE:
    case F_SETSIG:
    case F_SETOWN:
    case F_SETFL:
    case F_SETFD:
    case F_DUPFD:
    case F_DUPFD_CLOEXEC:
        argument_num = 3;
        received_args[2].type = INT_TYPE;
        received_args[2].int_ = (int)raw_arguments[2];
        break;
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
    case F_OFD_GETLK:
    case F_OFD_SETLK:
    case F_OFD_SETLKW:
    case F_GETOWN_EX:
    case F_SETOWN_EX:
    case F_SETDELEG:
    case F_GETDELEG:
    case F_SET_RW_HINT:
    case F_GET_RW_HINT:
    case F_GET_FILE_RW_HINT:
    case F_SET_FILE_RW_HINT:
        argument_num = 3;
        received_args[2].type = ADDRESS_TYPE;
        received_args[2].addr[0] = received_args[2].addr[1] = (uintptr_t)raw_arguments[2];
        break;
    }

    return argument_num;
}


u_int8_t process_special_syscall(reg_t syscall_num, pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]){
    // dispatcher
    switch (syscall_num) {
    case SYSN_OPEN:
        return process_syscall_open(pid, raw_arguments, received_args);
    case SYSN_OPENAT:
        return process_syscall_openat(pid, raw_arguments, received_args);
    case SYSN_IOCTL:
        return process_syscall_ioctl(pid, raw_arguments, received_args);
    case SYSN_PRCTL:
        return process_syscall_prctl(pid, raw_arguments, received_args);
    case SYSN_FUTEX:
        return process_syscall_futex(pid, raw_arguments, received_args);
    case SYSN_MREMAP:
        return process_syscall_mremap(pid, raw_arguments, received_args);    
    case SYSN_GETSOCKOPT:
        return process_syscall_getsockopt(pid, raw_arguments, received_args);
    case SYSN_SEMCTL:
        return process_syscall_semctl(pid, raw_arguments, received_args);
    case SYSN_FCNTL:
        return process_syscall_fcntl(pid, raw_arguments, received_args);
    }
}

#endif