#include <windows.h>
#include <iostream>
#include <queue>
#include <functional>
#include <thread>

class JobQueue {
public:
    JobQueue() : stopFlag(false) {
        queueMutex = CreateMutex(NULL, FALSE, NULL);
        if (queueMutex == NULL) {
            throw std::runtime_error("Failed to create mutex");
        }

        queueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (queueEvent == NULL) {
            CloseHandle(queueMutex);
            throw std::runtime_error("Failed to create event");
        }

        workerThread = std::thread(&JobQueue::workerFunction, this);
    }

    ~JobQueue() {
        {
            stopFlag = true;
            SetEvent(queueEvent);
        }

        if (workerThread.joinable()) {
            workerThread.join();
        }

        CloseHandle(queueMutex);
        CloseHandle(queueEvent);
    }

    void addJob(std::function<void()> job) {
        WaitForSingleObject(queueMutex, INFINITE);

        jobQueue.push(job);

        ReleaseMutex(queueMutex);

        SetEvent(queueEvent);
    }

private:
    std::queue<std::function<void()>> jobQueue;
    HANDLE queueMutex;
    HANDLE queueEvent;
    std::thread workerThread;
    volatile bool stopFlag;

    void workerFunction() {
        while (true) {
            WaitForSingleObject(queueEvent, INFINITE);

            while (true) {
                std::function<void()> job;

                WaitForSingleObject(queueMutex, INFINITE);

                if (!jobQueue.empty()) {
                    job = std::move(jobQueue.front());
                    jobQueue.pop();
                } else {
                    ReleaseMutex(queueMutex);
                    break;
                }

                ReleaseMutex(queueMutex);

                job();
            }

            if (stopFlag) {
                break;
            }
        }
    }
};

