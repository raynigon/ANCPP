#pragma once
#include <thread>
#include <mutex>
#include <chrono>
#include <queue>
#include <future>

namespace ancpp{
  class IPromise;
  template<typename T>
  class Promise;

  class EventQueue
  {
    friend class TimeTrigger;
  public:
    static const std::chrono::milliseconds QUEUE_TIMEOUT;
  private:
    std::thread t;
    // Monitor
    std::condition_variable monitor;
    std::mutex monitor_mutex;
    // Event Queue
    std::queue<std::function<void()>> queue;
    std::recursive_mutex queue_mutex;
    // External Futures
    std::vector<std::pair<std::shared_ptr<IPromise>, std::future<void>>> futures;
    std::recursive_mutex futures_mutex;
    // State Flag
    bool exit_queue = false;
    void waitForEvent();
    void handleEvent();
    void cleanUpFutures();
  protected:
    void run();
  public:
    static EventQueue& getInstance()
    {
      static EventQueue    instance;
      return instance;
    }
    EventQueue();
    ~EventQueue();
    void exit();
    void push(std::function<void()> callable);
    void launchExternal(std::shared_ptr<IPromise> promise, std::function<void()> callable);
    std::shared_ptr<ancpp::Promise<int>> launchExternal(std::function<void()> callable);
  };

}