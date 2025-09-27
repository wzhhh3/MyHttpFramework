#include "../include/main.h"
#include "../include/buffer.h"
int main()
{
    Buffer buffer(1024);
    buffer.Append("19aksiwesd1",12);
    std::cout<<buffer.RetrieveAllToStr()<<std::endl;
    return 0;
}