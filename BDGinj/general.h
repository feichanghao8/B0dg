#pragma once

#include <Windows.h>
#include <stdio.h>
#include <cstdarg>
#include <string>
#include <sstream>

#include <iostream>
#include <random>
#include <iomanip>

#include <chrono>
#include <mutex>


// TODO: Move this out into utils
template <typename T>
std::string vectorToString(const std::vector<T>& vec, const std::string& delimiter = ", ") {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i < vec.size() - 1) {
            oss << delimiter; // Add delimiter except after the last element
        }
    }
    return oss.str();
}


std::string getDllPath() {
    char path[MAX_PATH] = {0};

    HMODULE hModule = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                          reinterpret_cast<LPCSTR>(&getDllPath), &hModule)) {
        if (GetModuleFileNameA(hModule, path, MAX_PATH)) {
            return std::string(path);
        }
    }

    return "";
}

std::string getDllBaseDir() {
    char fullPath[MAX_PATH] = { 0 };

    HMODULE hModule = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, 
                          reinterpret_cast<LPCSTR>(&getDllBaseDir), &hModule)) {
        if (GetModuleFileNameA(hModule, fullPath, MAX_PATH)) {
            std::string path(fullPath);
            size_t lastSlash = path.find_last_of("\\/");
            if (lastSlash != std::string::npos) {
                return path.substr(0, lastSlash); // Return directory part
            }
        }
    }

    return "";
}

// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

long long getCurrentTimeMillis() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}

long long getTimeDifferenceMillis(long long pastTimeMillis) {
    long long currentTimeMillis = getCurrentTimeMillis();
    return currentTimeMillis - pastTimeMillis;
}


std::string getTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    std::ostringstream oss;
    oss << seconds_since_epoch;
    return oss.str();
}


FILE* logFileGeneral = 0;

void initLog() {
    // TODO: This is just a initialization used during test.
    // if we continue using this logging then convert it to a proper
    // singleton class to reduce error chances in the future
    std::string dllBaseDir = getDllBaseDir();
    std::string dllLog = dllBaseDir + "\\program-" + getTimestampString() + ".log";
    logFileGeneral = _fsopen(dllLog.c_str(), "w", _SH_DENYNO);

    // TODO: verify file was opened.
}

void log(char* message) {
    if (logFileGeneral == 0) { initLog(); }

    fprintf(logFileGeneral, message);
    fflush(logFileGeneral);
}

HANDLE loggingMutex;
template <typename IdType>
void logfff(IdType id, const char* format, ...) {
    if (logFileGeneral == 0) { 
        loggingMutex = CreateMutexA(NULL, FALSE, "loggingMutex");
        initLog(); 

    }

    WaitForSingleObject(loggingMutex, 2*1000);

    if((int)id >= 0) {
        fprintf(logFileGeneral, "[%8x|", (int)id);
        printf("[%8x|", (int)id);

        std::time_t currentTime = std::time(nullptr);
        std::tm localTime;
        int seconds = -1;
        if (localtime_s(&localTime, &currentTime) == 0) { // Returns 0 on success
            seconds = localTime.tm_sec; // Get the seconds (0-59)
        }
        fprintf(logFileGeneral, "%02d]: ", seconds);
        printf("%02d]: ", seconds);
    }

    va_list args;
    va_start(args, format);

    vfprintf(logFileGeneral, format, args);
    vprintf(format, args);

    fflush(logFileGeneral); // TODO: No need to flush all the time but here as precaution for now

    // TODO: Process/Log if amount reached boundary?
    va_end(args);

    ReleaseMutex(loggingMutex);
}


// void logError(int id, const char* format, ...) {
//     // TODO: Switch error reporting to use this
//     const char* prefix = "ERROR: ";
//     size_t prefixLen = strlen(prefix);
//     size_t formatLen = strlen(format);
//     size_t combinedLen = prefixLen + formatLen;
//     constexpr size_t maxBufferLen = 1024;
//
//     if(combinedLen >= maxBufferLen) {
//         logfff(id, "ERROR: Couldn't print error log because format[%d] + prefix[%d] is larger than max[%d]", formatLen, prefixLen, maxBufferLen);
//     }
//
//     char newFormat[maxBufferLen];
//     strcpy_s(newFormat, prefixLen, prefix);
//     strcat_s(newFormat, formatLen, format);
//
//     va_list args;
//     va_start(args, format);
//     logfff(id, newFormat, args);
//     va_end(args);
// }


int charToNumber(char value) {
    if (value >= 'a' && value <= 'f') { return value - 'a' + 10; }
    else if (value >= 'A' && value <= 'F') { return value - 'A' + 10; }
    else if (value >= '0' && value <= '9') { return value - '0'; }
    return 0;
}
std::vector<int> patterTextToInts(const char* signatureString) {
    int sigTextLength = strlen(signatureString);
    std::vector<int> items = {};

    const char* startP = signatureString;
    const char* endP = signatureString + sigTextLength;
    char* p = (char*)startP;

    while (p < endP) {
        if ((*p == ' ') || (*p == '?')) {
            if (*p == '?') {
                items.push_back(-1);
            }
            p++;
            continue;
        }

        char nible1 = charToNumber(*p);
        char nible2 = charToNumber(*(p + 1)); // TODO: Possible overflow. Add check conditions
        int value = nible1 << 4 | nible2;
        items.push_back(value);
        p += 2; // TODO: Possible overflow. Add check conditions
    }

    return items;
}

unsigned char* findSignature(const char* signatureStr) {
    // Get .text region
    HMODULE base = GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)base;
    IMAGE_NT_HEADERS* pNTHeaders = (IMAGE_NT_HEADERS*)((BYTE*)base + pDOSHeader->e_lfanew);

    uintptr_t codeStartP = reinterpret_cast<uintptr_t>((unsigned char*)base + pNTHeaders->OptionalHeader.BaseOfCode);
    size_t codeBlockLength = pNTHeaders->OptionalHeader.SizeOfCode;
    uintptr_t codeEndP = (codeStartP + codeBlockLength);

    size_t codeSectionSize = codeEndP - codeStartP;

    std::vector<int> signature = patterTextToInts(signatureStr);
    size_t signatureLength = signature.size();

    size_t index = 0;
    bool isFound = false;

    unsigned char* location = 0;

    while (!isFound && index < codeBlockLength) {
        unsigned char* searchBlockStart = (unsigned char*)codeStartP + index;
        unsigned char* p = searchBlockStart;

        for (int i = 0; i < signatureLength; i++) {
            int value = signature[i];

            if (value == -1) {
                p++;
                continue;
            }

            if (value != *p) {
                break;
            }

            p++;
        }

        if (searchBlockStart + signatureLength == p) {
            isFound = true;
            location = searchBlockStart;
        }

        index++;
    }

    char Buffer2[1024] = {};
    log(Buffer2);

    return location;
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

template <typename T>
class scoped_refptr {
public:
    explicit scoped_refptr(T* ptr = nullptr) : ptr_(ptr) {
        if (ptr_) {
            ptr_->AddRef();  
        }
    }

    ~scoped_refptr() {
        if (ptr_) {
            ptr_->Release();  
        }
    }

    scoped_refptr(const scoped_refptr& other) : ptr_(other.ptr_) {
        if (ptr_) {
            ptr_->AddRef();  
        }
    }

    scoped_refptr(scoped_refptr&& other) noexcept : ptr_(other.ptr_) {
        other.ptr_ = nullptr;  
    }

    scoped_refptr& operator=(const scoped_refptr& other) {
        if (this != &other) {
            if (ptr_) {
                ptr_->Release();
            }
            ptr_ = other.ptr_;
            if (ptr_) {
                ptr_->AddRef();
            }
        }
        return *this;
    }

    scoped_refptr& operator=(scoped_refptr&& other) noexcept {
        if (this != &other) {
            if (ptr_) {
                ptr_->Release();
            }
            ptr_ = other.ptr_;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    T* get() const { return ptr_; }

    T& operator*() const { return *ptr_; }

    T* operator->() const { return ptr_; }

private:
    T* ptr_;
};


template <typename T>
class RefCountedThreadSafe {
public:
    // Constructor: Initializes the object and sets reference count to 1
    RefCountedThreadSafe() : refCount(1) {}

    // Increment the reference count atomically
    void AddRef() {
        refCount.fetch_add(1, std::memory_order_relaxed);
    }

    // Decrement the reference count atomically and delete if it reaches 0
    void Release() {
        if (refCount.fetch_sub(1, std::memory_order_release) == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete static_cast<T*>(this);  // Cast to T and call its destructor
        }
    }

    // Get the current reference count
    int GetRefCount() const {
        return refCount.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> refCount;  // Atomic reference count
    // mutable std::atomic<int> ref_count_;
    // mutable AtomicRefCount ref_count_{0};
};


class IOBuffer : public RefCountedThreadSafe<IOBuffer> {
public:
    IOBuffer();

    IOBuffer(size_t size) : _size(size), data(new char[size]) {}

    IOBuffer(const std::string& str)
    {  
        _size = str.size() + 1;
        data = new char[_size];
        std::memcpy(data, str.c_str(), _size);
    }
    
    virtual ~IOBuffer() { delete[] data; }


    char* getData() { return data; }
    size_t getSize() { return _size; }
    size_t size() { return _size; }

    void fillData(const std::string& str) {
        int a = _size;
        int b = str.size() + 1; // null terminated
        size_t len = (a < b) ? a : b;
        memcpy(data, str.c_str(), len);
    }


protected:
    friend class RefCountedThreadSafe<IOBuffer>;

    char* data;
    size_t _size;
};




std::shared_ptr<IOBuffer> CreateIOBufferFromString(const std::string& str) {
    // NOTE: Create and return a shared_ptr to IOBuffer initialized with the string content
    return std::make_shared<IOBuffer>(str);
}

scoped_refptr<IOBuffer> stringToIOBuffer(const std::string& str) {
    auto x = new IOBuffer(str);
    return scoped_refptr<IOBuffer>(x);
}


// ----------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

typedef void (__thiscall *SendFramePtr)(void* thisptr, bool fin, int op_code, scoped_refptr<IOBuffer> data, size_t size);
SendFramePtr SendFrame = 0;


std::string generateRandomHexString(size_t length) {
    const char hex_chars[] = "0123456789abcdef";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        result += hex_chars[dis(gen)];
    }

    return result;
}

std::string getRandomTransparentId() {
    // NOTE: 00-4361c510d51c359af131ecc6fa403fad-f8e500deae6f3abc-00
    std::string result = "00-" + generateRandomHexString(32) + "-" + generateRandomHexString(16) + "-00";
    return result;
}

std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // UUID version 4
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen); // UUID variant
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

bool isBitPresent(int value, int mask) {
    if((value & mask) == 0) { return false; }
    else { return true; }
}
