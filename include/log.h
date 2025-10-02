#ifndef LOG_H
#define LOG_H
#include <mutex>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <sys/time.h>
#include <condition_variable>
#include <string.h>
#include <stdarg.h>           // vastart va_end
#include <assert.h>
#include <sys/stat.h>         // mkdir
#include "blockqueue.h"
#include "../include/buffer.h"

class Log
{
public:
    void init(int level, const char *path = "./log", const char *suffix = ".log",
              int maxQueueCapacity = 1024);
    static Log *Instance();
    static void FlushLogThread();
    void write(int level, const char *format, ...);
    void flush();
    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();

private:
    Log();
    void AppendLogLevelTitle_(int level);
    ~Log();
    void AsyncWrite_();

    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;
    const char* path_;
    const char * suffix_;
    int MAX_LINES_;
    int toDay_;
    int lineCount_;
    Buffer buff_;
    bool isOpen_;
    int level_;
    int isAsync_;
    FILE* fp_; 
    std::unique_ptr<BlockQueue<std::string>> deque_;
    std::unique_ptr<std::thread>writeThread_;
    std::mutex mtx_;
};

#define LOG_BASE(level,format,...)\
do\
{\
    Log* log=Log::Instance();\
    if(log->IsOpen()&&log->GetLevel()<=level)\
    {\
        log->write(level,format,##__VA_ARGS__);\
        log->flush();\
    }\
} while (0);

#define LOG_DEBUG(format,...) do{LOG_BASE(0,format,##__VA_ARGS__)}while(0);
#define LOG_INFO(format,...) do{LOG_BASE(1,format,##__VA_ARGS__)}while(0);
#define LOG_WAIN(format,...) do{LOG_BASE(2,format,##__VA_ARGS__)}while(0);
#define LOG_ERROR(format,...) do{LOG_BASE(3,format,##__VA_ARGS__)}while(0);
#endif