#define RETURN_FALSE_ON_FAIL(cond) if(cond == false) return false;
#define RETURN_ON_FAIL(cond) if(cond == false) return;
#define ARRAY_CNT(arr) (sizeof(arr) / sizeof(arr[0]))
#define CURRENT_TIMESTAMP() std::chrono::duration_cast<milliseconds>(system_clock::now().time_since_epoch())
#define MACRO_NEW(T) (std::make_shared<T>())