#ifndef _SPECIAL_SYSCALLS_
#define _SPECIAL_SYSCALLS_


#include <sys/ioctl.h>


#include "sandbox.h"
#include "syscalls_table.h"


#define I_POP 21251
#define I_FLUSH 21253
#define I_SETSIG 21257
#define I_SRDOPT 21254
#define I_SWROPT 21264
#define I_SENDFD 21261
#define I_ATMARK 21268
#define I_CKBAND 21270
#define I_CANPUT 21271
#define I_LINK 21272
#define I_UNLINK 21273
#define I_PLINK 21274
#define I_PUNLINK 21275
#define I_GETCLTIME 21276
#define I_SETCLTIME 21277
#define I_GETBAND 21278
#define I_GWROPT 21265
#define I_NREAD 21249
#define I_GRDOPT 21255
#define I_GETSIG 21258
#define I_PUSH 21250
#define I_LOOK 21252
#define I_FIND 21259
#define I_PEEK 21260
#define I_STR 21256
#define I_FLUSHBAND 21263
#define I_FDINSERT 21267
#define I_RECVFD 21262
#define I_LIST 21279

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
        received_args[2].int_ = temp_arr[0];
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
}

u_int8_t process_special_syscall(reg_t syscall_num, pid_t pid, const reg_t raw_arguments[MAX_SYSCALL_ARGS_NUM], Syscall_argument received_args[MAX_SYSCALL_ARGS_NUM]){
    // dispatcher
    switch (syscall_num) {
    case SYSN_OPEN:
        return process_syscall_open(pid, raw_arguments, received_args);
    //case SYSN_OPENAT:
    //    return process_syscall_openat(pid, raw_arguments, received_args);
    case SYSN_IOCTL:
        return process_syscall_ioctl(pid, raw_arguments, received_args);
}
}

#endif