#pragma once
using MessageFuncType = std::function<void()>;

class MessageQueue
{
public:
	void Push(MessageFuncType message);
	MessageFuncType Pop();
	void FlushAll(std::vector<MessageFuncType>& messages);
private:
	std::queue<MessageFuncType> _queue;
	std::mutex messageLock;
	std::atomic<int32> messageCnt;
};

