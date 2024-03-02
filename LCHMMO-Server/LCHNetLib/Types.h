#pragma once
class Session;
class CircularBuffer;

using int64 = __int64;
using int32 = __int32;
using int16 = __int16;
using int8 = __int8;

using uint64 = unsigned __int64;
using uint32 = unsigned __int32;
using uint16 = unsigned __int16;
using uint8 = unsigned __int8;

using SessionPtr = std::shared_ptr<Session>;
using CircularBufferPtr = std::shared_ptr<CircularBuffer>;

using AtomicBool = std::atomic<bool>;
using RecursiveLockGuard = std::lock_guard<std::recursive_mutex>;
using LockGuard = std::lock_guard<std::mutex>;
using Wstring = std::wstring;
using RecursiveMutex = std::recursive_mutex;
using SessionFactory = std::function<std::shared_ptr<Session>(void)>;


