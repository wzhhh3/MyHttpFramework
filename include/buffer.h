#ifndef BUFFER_H
#define BUFFER_H
#include<cstring>
#include<iostream>
#include<unistd.h>
#include<sys/uio.h>
#include<vector>
#include<atomic>
#include<assert.h>
class Buffer{
public:
     Buffer(int initBuffSize=1024);
     ~Buffer();
     size_t WritableBytes() const;
     size_t ReadableBytes() const;
     size_t PrependableBytes() const;
     const char* peek() const;
     void EnsureWritable(size_t len);
     void HasWritten(size_t len);

     void Retrieve(size_t len);
     void RetrieveUntil(const char* end);
     void RetrieveAll();
     std::string RetrieveAllToStr();
     
     char* BeginWrite();
     const char* BeginWriteConst() const;
     void Append(const std::string &str);
     void Append(const char* str,size_t len);
     void Append(const void* str,size_t len);
     void Append(const Buffer& buffer);

     ssize_t ReadFd(int fd,int *Errno);
     ssize_t WriteFd(int fd,int* Errno);
private:
     char* BeginPtr_();
     const char* BeginPtr_()const;
     void MakeSpace(size_t len);
     std::vector<char> buffer_;
     std::atomic<size_t> readpos_;
     std::atomic<size_t> writepos_;
};
#endif