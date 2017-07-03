#pragma once
#include <vector>
#include <future>
#include <functional>
#include <unordered_set>
#include <memory>
#include "Core/Promise.hpp"

namespace ancpp {

  class TimeTrigger
  {
  private:
    std::mutex timerIdsMutex;
    std::unordered_set<long> timerIds;
    std::unordered_set<std::shared_ptr<IPromise>> promises;
    long lastTimerId = 0;
  private:
    bool isActive(long triggerId);
    long createTriggerId();
  public:
    TimeTrigger();
    ~TimeTrigger();
    long onTimer(std::function<void()> callable, long timeout);
    void destroyTimer(long timerId);
  };

};