#pragma once
#include <vector>
#include <future>
#include <functional>
class TimeTrigger
{
private:
  std::mutex timerIdsMutex;
  std::unordered_set<long> timerIds;
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

