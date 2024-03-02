#include "pch.h"
#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(uint32 _bufferSize)
{
    capacity = _bufferSize * 10;
    buffer.resize(capacity);
}

CircularBuffer::~CircularBuffer()
{
    buffer.clear();
}

void CircularBuffer::Clear()
{
    buffer.clear();
    readPos = 0;
    writePos = 0;
}

//데이터가 없으면 비워주고, 끝까지 있으면 [][][][r][][w] 맨 앞으로 이동
void CircularBuffer::Clean()
{
    uint32 dataSize = DataSize();
    if (dataSize == 0)
    {
        readPos = 0;
        writePos = 0;
    }

    else
    {
        if (FreeSize() < bufferSize)
        {
            memcpy(&buffer[0], &buffer[readPos], dataSize);
            readPos = 0;
            writePos = dataSize;
        }
    }
}

bool CircularBuffer::OnRead(uint32 bytes)
{
    if (bytes > DataSize())
        return false;

    readPos += bytes;
    return true;
}

bool CircularBuffer::OnWrite(uint32 bytes)
{
    if (bytes > FreeSize())
        return false;

    writePos += bytes;
    return true;
}

char* CircularBuffer::ReadPos()
{
    return &buffer[readPos];
}

char* CircularBuffer::WritePos()
{
    return &buffer[writePos];
}

uint32 CircularBuffer::DataSize()
{
    return writePos - readPos;
}

uint32 CircularBuffer::FreeSize()
{
    return capacity - writePos;
}

void CircularBuffer::WriteBuf(char* Buf)
{
    char* destData = WritePos();
    uint32 writeSize = (uint32)(sizeof(char) * strlen(Buf));
    memcpy(destData, Buf, writeSize);
    OnWrite(writeSize);
    Clean();
}

char* CircularBuffer::ReadBuf(uint32 bytes)
{
    char* subBuffer = new char[bytes];
    uint32 j = 0;
    for(uint32 i = readPos; i <= readPos + bytes; i++)
    {
        subBuffer[j] = buffer[i];
        j++;
    }
    return subBuffer;
}
