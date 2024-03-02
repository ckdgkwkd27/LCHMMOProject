#include "pch.h"
#include "MessageQueue.h"

void MessageQueue::Push(MessageFuncType message)
{
    LockGuard guard(messageLock);

    _queue.push(message);
}

MessageFuncType MessageQueue::Pop()
{
    LockGuard guard(messageLock);

    auto ret = _queue.front();
    _queue.pop();
    return ret;
}

void MessageQueue::FlushAll(std::vector<MessageFuncType>& messages)
{
    LockGuard guard(messageLock);

    while (_queue.empty() == false)
    {
        auto _message = _queue.front();
        messages.push_back(_message);
        _queue.pop();
    }
}
