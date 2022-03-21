#pragma once

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

class process_info {
public:
    enum class data{PID,COMM,STATE,PPID,PGRP,SESSION,TTY_NR,TPGID,FLAGS,MINFLT,CMINFLT,MAJFLT,
        CMAJFLT,UTIME,STIME,CUTIME,CSTIME,PRIORITY,NICE,NUM_THREADS,ITREALVALUE,STARTTIME,
        VSIZE,RSS,RSSLIM,STARTCODE,ENDCODE,STARTSTACK,KSTKESP,KSTKEIP,SIGNAL,BLOCKED,
        SIGIGNORE,SIGCATCH,WCHAN,NSWAP,CNSWAP,EXIT_SIGNAL,PROCESSOR,_SIZE};
private:
    std::string dat[static_cast<size_t>(data::_SIZE)];
public:
    void generate();

    const std::string& get(const data&);
};