#ifndef BUFFER_H
#define BUFFER_H
#include <cstring>   //perror
#include <iostream>
#include <string>
#include <unistd.h>  // write
#include <sys/uio.h> //readv
#include <vector> //readv
#include <atomic>
#include <assert.h>
class Buffer {
public:
// 读写下标初始化，vector<char>初始化
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;
    // 可写的数量：buffer大小 - 写下标
    size_t WritableBytes() const;
    // 可读的数量：写下标 - 读下标
    size_t ReadableBytes() const ;
    // 可预留空间：已经读过的就没用了，等于读下标
    size_t PrependableBytes() const;
    
    
    const char* Peek() const;
    // 确保可写的长度
    void EnsureWriteable(size_t len);
    // 移动写下标，在Append中使用
    void HasWritten(size_t len);

    // 读取len长度，移动读下标
    void Retrieve(size_t len);
    // 读取到end位置
    void RetrieveUntil(const char* end);
    
    // 取出所有数据，buffer归零，读写下标归零,在别的函数中会用到
    void RetrieveAll();
    // 取出剩余可读的str
    std::string RetrieveAllToStr();
    
    // 写指针的位置
    const char* BeginWriteConst() const;
    char* BeginWrite();
    
    // 添加str到缓冲区
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);
    
    // 将fd的内容读到缓冲区，即writable的位置
    ssize_t ReadFd(int fd, int* Errno);
    // 将buffer中可读的区域写入fd中
    ssize_t WriteFd(int fd, int* Errno);

private:
    char* BeginPtr_();  // buffer开头
    const char* BeginPtr_() const;
    // 扩展空间
    void MakeSpace_(size_t len);

    std::vector<char> buffer_;  
    std::atomic<std::size_t> readPos_;  // 读的下标
    std::atomic<std::size_t> writePos_; // 写的下标
};

#endif //BUFFER_H
