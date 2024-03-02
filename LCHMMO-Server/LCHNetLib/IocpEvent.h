#pragma once
class CircularBuffer;

enum class EventType : uint8
{
	ACCEPT,
	CONNECT,
	DISCONNECT,
	SEND,
	RECV,
};

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType _type) : eventType(_type)
	{
	}

	void Init() {
		OVERLAPPED::hEvent = 0;
		OVERLAPPED::Internal = 0;
		OVERLAPPED::InternalHigh = 0;
		OVERLAPPED::Offset = 0;
		OVERLAPPED::OffsetHigh = 0;
	}

	EventType GetType() {
		return eventType;
	}

public:
	SessionPtr sessionRef;

private:
	EventType eventType;
};

class AcceptEvent : public IocpEvent 
{
public:
	AcceptEvent() : IocpEvent(EventType::ACCEPT) {}
};

class ConnectEvent : public IocpEvent 
{
public:
	ConnectEvent() : IocpEvent(EventType::CONNECT) {}
};

class DisconnectEvent : public IocpEvent {
public:
	DisconnectEvent() : IocpEvent(EventType::DISCONNECT) {}
};

class SendEvent : public IocpEvent 
{
public:
	SendEvent() : IocpEvent(EventType::SEND)
	{
	}

public:
	CircularBufferPtr sendBuffer;
};

class RecvEvent : public IocpEvent 
{
public:
	RecvEvent() : IocpEvent(EventType::RECV) {}
};