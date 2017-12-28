#ifndef _SERVER_H_
#define _SERVER_H_

#define DEFAULT_BUFLEN          4096
#define DEFAULT_MAX_PASS_LEN    32
#define DEFAULT_MAX_FILENAME	64
#define DEFAULT_PROGRESS_INC    10
#define DEFAULT_MAX_THREADS     64
#define WORKLOAD_PER_THREAD		13	

void
Log(
    const char *Format,
    ...
    );

#endif _SERVER_H_
