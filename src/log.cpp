#include "../include/log.h"
Log::Log()
{
    fp_ = nullptr;
    deque_ = nullptr;
    writeThread_ = nullptr;
    toDay_ = 0;
    lineCount_ = 0;
    isOpen_ = false;
    level_ = 0;
    isAsync_ = false;
    MAX_LINES_ = MAX_LINES;
}
Log::~Log()
{
    if (isAsync_)
    {
        // 先完成队列内的任务
        while (!deque_->empty())
        {
            deque_->flush();
        }
    }

    // 清空队列元素，关闭队列
    if (deque_)
        deque_->close();
    isOpen_ = false;
    // 等待写线程完成任务
    if (writeThread_)
        writeThread_->join();
    {
        // 冲洗文件缓冲区再关闭文件描述符
        lock_guard<mutex> locker(mtx_);
        if (fp_)
        {
            flush();
            fclose(fp_);
        }
    }
}
void Log::flush()
{
    if (isAsync_ && !deque_->getClose())
    {
        deque_->flush();
    }
    fflush(fp_);
}
// init(日志级别，路径名，后缀名，队列最大容量)
void Log::init(int level, const char *path, const char *suffix, int maxQueueCapacity)
{
    isOpen_ = true;
    level_ = level;
    path_ = path;
    suffix_ = suffix;
    if (maxQueueCapacity)
    {
        isAsync_ = true;
        // 队列不存在则创建
        if (!deque_)
        {
            deque_ = std::make_unique<BlockQueue<std::string>>(maxQueueCapacity);
            writeThread_ = std::make_unique<std::thread>(FlushLogThread);
        }
    }
    else
    {
        isAsync_ = false;
    }
    // 初始化today_,创建目录文件,打开文件
    lineCount_ = 0;
    time_t timer = time(0);
    struct tm *systime = localtime(&timer);
    char newFileName[LOG_NAME_LEN] = {0};
    snprintf(newFileName, LOG_NAME_LEN, "%s/%04d_%02d_%02d%s", path_, systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday, suffix_);
    toDay_ = systime->tm_mday;
    {
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_)
        {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(newFileName, "a");
        if (!fp_)
        {
            mkdir(path_, 0777);
            fp_ = fopen(newFileName, "a");
        }
    }
    assert(fp_ != nullptr);
}
Log *Log::Instance()
{
    static Log log;
    return &log;
}
// 异步日志写线程函数,静态接口调用单例成员函数
void Log::FlushLogThread()
{
    Log::Instance()->AsyncWrite_();
}
// 异步写线程执行函数
void Log::AsyncWrite_()
{
    string str = "";
    while (deque_->pop(str))
    {
        {
            lock_guard<mutex> locker(mtx_);
            fputs(str.c_str(), fp_);
            if (!isOpen_)
            {
                break;
            }
        }
            cout << "异步写调试" << endl;
    }
    cout << "退出异步写" << endl;
}
// 把消息写入阻塞队列，先判断是否要创建新文件(不是当天或行数过多),再创建消息
void Log::write(int level, const char *format, ...)
{
    struct timeval tv = {0, 0};
    gettimeofday(&tv, NULL);
    time_t tSec = tv.tv_sec;
    struct tm *systime = localtime(&tSec);
    va_list vaList;
    if (systime->tm_mday != toDay_ || (lineCount_ &&( (lineCount_ %MAX_LINES)==0) ))
    {
        // 需要切换文件
        char newFileName[LOG_NAME_LEN];
        char tail[36];
        snprintf(tail, 36, "%04d_%02d_%02d", systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday);
        if (systime->tm_mday != toDay_)
        {
            snprintf(newFileName, LOG_NAME_LEN, "%s/%s%s", path_, tail, suffix_);
            toDay_ = systime->tm_mday;
            lineCount_ = 0;
        }
        else
        {
            snprintf(newFileName, LOG_NAME_LEN, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);
        }
        // 加锁
        lock_guard<mutex> locker(mtx_);
        // 刷新并关闭原有描述符
        if (fp_)
        {
            flush();
            fclose(fp_);
        }
        if (access(path_, F_OK) == -1)
        {
            mkdir(path_, 0777);
        }
        fp_ = fopen(newFileName, "a");
    }
    // 创建一行日志信息并放入队列
    {
        lock_guard<mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), buff_.WritableBytes(), "%04d-%02d-%02d %02dH:%02dM:%02dS:%06ldUS ",
                         systime->tm_year + 1900, systime->tm_mon + 1, systime->tm_mday, systime->tm_hour, systime->tm_min, systime->tm_sec, tv.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevelTitle_(level_);
        va_start(vaList, format);
        string temp = "";
        n = vsnprintf(nullptr, 0, format, vaList);
        va_end(vaList);
        va_start(vaList, format);
        if (n > 0)
        {
            temp.resize(n + 1);
            vsnprintf(&temp[0], n + 1, format, vaList);
            temp.resize(n);
            // temp="调试日志";
            buff_.Append(temp);
        }
        va_end(vaList);
        // fputs写尽头
        buff_.Append("\n\0", 2);
        if (deque_&& isAsync_&&!deque_->full())
        {
            deque_->push_back(buff_.RetrieveAllToStr());
        }
        else if (!isAsync_)
        {
            fputs(buff_.peek(), fp_);
        }
        buff_.RetrieveAll();
    }
}
// 添加日志等级标题
void Log::AppendLogLevelTitle_(int level)
{
    switch (level)
    {
    case 0:
        buff_.Append("[DEBUG]: ", 9);
        break;
    case 1:
        buff_.Append("[INFO] : ", 9);
        break;
    case 2:
        buff_.Append("[WARN] : ", 9);
        break;
    case 3:
        buff_.Append("[ERROR]: ", 9);
        break;
    default:
        buff_.Append("[INFO] : ", 9);
    }
}
int Log::GetLevel()
{
    lock_guard<mutex> locker(mtx_);
    return level_;
}
void Log::SetLevel(int level)
{
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}
bool Log::IsOpen()
{
    lock_guard<mutex> locker(mtx_);
    return isOpen_;
}
