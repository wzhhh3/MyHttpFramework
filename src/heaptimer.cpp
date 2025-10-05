#include "../include/heaptimer.h"
void HeapTimer::SwapNode_(size_t i, size_t j)
{
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = j;
    ref_[heap_[j].id] = i;
}
void HeapTimer::siftup_(size_t i)
{
    assert(i >= 0 && i < heap_.size());
    ssize_t parent = (i - 1) / 2;
    while (parent >= 0)
    {
        if (heap_[parent] > heap_[i])
        {
            SwapNode_(parent, i);
            i = parent;
            parent = (i - 1) / 2;
        }
        else
        {
            break;
        }
    }
}
bool HeapTimer::siftdown_(size_t i, size_t n)
{
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t index = i;
    size_t child = (index * 2) + 1;
    while (child < n)
    {
        if (child + 1 < n && heap_[child + 1] < heap_[child])
        {
            child++;
        }
        if (heap_[child] < heap_[index])
        {
            SwapNode_(child, index);
            index = child;
            child = (index * 2) + 1;
        }
        else
        {
            break;
        }
    }
    return index>i;
}
void HeapTimer::del_(size_t index)
{
    assert(index >= 0 && index < heap_.size());
    size_t tmp=index;
    size_t n=heap_.size()-1;
    if(tmp<n)
    {
        SwapNode_(tmp,n);
        if(!siftdown_(tmp,n))
        {
            siftup_(tmp);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}
void HeapTimer::adjust(int id, int newExpires)
{
    assert(!heap_.empty() && ref_.count(id));
    heap_[ref_[id]].expires=Clock::now()+MS(newExpires);
    siftdown_(id,heap_.size());
}
void HeapTimer::add(int id, int timeOut, const TimeoutCallBack& cb)
{
    assert(id >= 0);
    if(ref_.count(id))
    {
        adjust(id,timeOut);
    }
    else{
        size_t n=heap_.size();
        ref_[id]=n;
        struct TimerNode temp;
        temp.id=id;
        temp.expires=Clock::now()+MS(timeOut);
        temp.cb=cb;
        heap_.push_back(temp);
        siftup_(n);
    }
}
void HeapTimer::doWork(int id)
{
    if(heap_.empty()||ref_.count(id)==0)
    return;
    auto node=heap_[ref_[id]];
    node.cb();
    del_(ref_[id]);
}
void HeapTimer::tick()
{
    if(heap_.empty()) {
        return;
    }
    while(!heap_.empty())
    {
        struct TimerNode temp =heap_.front();
        if(std::chrono::duration_cast<MS>(temp.expires-Clock::now()).count()>0)
        {
            break;
        }
        temp.cb();
        pop();
    }
}
int HeapTimer::GetNextTick()
{
    tick();
    int res=-1;
    if(!heap_.empty())
    {
        res=std::chrono::duration_cast<MS>(heap_.front().expires-Clock::now()).count();
        if(res<0)res=0;
    }
    return res;
}
void HeapTimer::pop() {
    assert(!heap_.empty());
    del_(0);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}