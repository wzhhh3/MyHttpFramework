#include "../include/main.h"
#include "../include/buffer.h"
#include "../include/blockqueue.h"
#include "../include/log.h"
#include "../include/threadpool.h"
int main()
{
    string s1="缓冲区测试";
    Buffer buffer(1024);
    buffer.Append(s1);
    std::cout<<buffer.RetrieveAllToStr()<<std::endl;
    BlockQueue<int> q;
    Log *log=Log::Instance();
    log->init(0,"../log",".log",1024);
    for(int i=0;i<60000;i++)
    {
            s1="我在写日志";
            s1+=to_string(i);
            LOG_DEBUG("%s",s1.c_str());
    }
    ThreadPool tp8(8);
    return 0;
}