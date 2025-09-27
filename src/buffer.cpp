#include "../include/buffer.h"
Buffer::Buffer(int initBuffSize ) : buffer_(initBuffSize), readpos_(0), writepos_(0)
{
}
Buffer::~Buffer()
{
}
size_t Buffer::WritableBytes() const
{
    return buffer_.size() - writepos_;
}
size_t Buffer::ReadableBytes() const
{
    return writepos_ - readpos_;
}
size_t Buffer::PrependableBytes() const
{
    return readpos_;
}
const char *Buffer::peek() const
{
    return &buffer_[readpos_];
}
void Buffer::EnsureWritable(size_t len)
{
    if (WritableBytes() < len)
    {
        MakeSpace(len);
    }
    assert(len <= WritableBytes());
}
void Buffer::HasWritten(size_t len)
{
    writepos_ += len;
}
void Buffer::Retrieve(size_t len)
{
    readpos_ += len;
}
void Buffer::RetrieveUntil(const char *end)
{
    assert(peek() <= end);
    Retrieve(end - peek());
}
void Buffer::RetrieveAll()
{
    bzero(&buffer_[0], buffer_.size());
    readpos_ = writepos_ = 0;
}
std::string Buffer::RetrieveAllToStr()
{
    std::string str(peek(), ReadableBytes());
    RetrieveAll();
    return str;
}
char *Buffer::BeginWrite()
{
    return &buffer_[writepos_];
}
const char *Buffer::BeginWriteConst() const
{
    return &buffer_[writepos_];
}
void Buffer::Append(const char *str, size_t len)
{
    assert(str);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}
void Buffer::Append(const std::string &str)
{
    Append(str.c_str(), str.size());
}
void Buffer::Append(const void *str, size_t len)
{
    Append(static_cast<const char *>( str), len);
}
void Buffer::Append(const Buffer &buffer)
{
    Append(buffer.peek(), buffer.ReadableBytes());
}
ssize_t Buffer::ReadFd(int fd, int *Errno)
{
    char buff[65535];
    struct iovec iov[2];
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = WritableBytes();
    iov[1].iov_base = buff;
    iov[1].iov_len = 65535;
    ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(len) <= WritableBytes())
    {
        writepos_ += len;
    }
    else
    {
        writepos_ = buffer_.size();
        Append(buff, len - WritableBytes());
    }
    return len;
}
ssize_t Buffer::WriteFd(int fd, int *Errno)
{
    ssize_t len = write(fd,peek(),ReadableBytes());
    if (len < 0)
    {
        *Errno = errno;
        return len;
    }
    else
    {
        Retrieve(len);
        return len;
    }
}
char *Buffer::BeginPtr_()
{
    return &buffer_[0];
}
const char *Buffer::BeginPtr_() const
{
    return &buffer_[0];
}
void Buffer::MakeSpace(size_t len)
{
    assert(len);
    if((WritableBytes()+PrependableBytes())<len)
    {
        buffer_.resize(writepos_+len+1);
    }
    else{
        size_t readable=ReadableBytes();
        std::copy(BeginPtr_()+readpos_,BeginPtr_()+writepos_,BeginPtr_());
        readpos_=0;
        writepos_=readable;
        assert(readable=ReadableBytes());
    }
}