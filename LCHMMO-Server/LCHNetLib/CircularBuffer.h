#pragma once
#define MAX_BUFFER_SIZE 65535

class CircularBuffer
{
public:
	CircularBuffer(uint32 _bufferSize);
	~CircularBuffer();

	void Clear();
	void Clean();
	bool OnRead(uint32 bytes);
	bool OnWrite(uint32 bytes);

	char* ReadPos();
	char* WritePos();
	uint32 DataSize();
	uint32 FreeSize();
	void WriteBuf(char* Buf);
	char* ReadBuf(uint32 bytes);

	char* data()
	{
		return buffer.data();
	}

private:
	uint32 capacity = 0;
	uint32 bufferSize = 0;
	uint32 readPos = 0;
	uint32 writePos = 0;

private:
	std::vector<char> buffer;
};

