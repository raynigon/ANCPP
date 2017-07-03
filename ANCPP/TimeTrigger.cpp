#include "stdafx.h"
#include "TimeTrigger.h"
#include <future>
#include <thread>
#include "EventQueue.h"


TimeTrigger::TimeTrigger()
{
}


TimeTrigger::~TimeTrigger()
{
  active = false;
}

long TimeTrigger::onTimer(std::function<void()> callable, long timeout)
{
  long triggerId = createTriggerId();
  EventQueue::launchExternal([&,callable,timeout,triggerId]() {
    while(isActive(triggerId)){
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
      ancpp::EventQueue::getInstance().push(callable);
    }
  });
  return triggerId;
}

void TimeTrigger::destroyTimer(long timerId)
{
  std::lock_guard<std::mutex> lg(timerIdsMutex);
  auto it = timerIds.find(triggerId);
  if (it == timerIds.end())
    return;
  timerIds.erase(it);
}

bool TimeTrigger::isActive(long triggerId)
{
  std::lock_guard<std::mutex> lg(timerIdsMutex);
  return timerIds.find(triggerId)!=timerIds.end();
}

long TimeTrigger::createTriggerId()
{
  std::lock_guard<std::mutex> lg(timerIdsMutex);
  long triggerId = 0;
  while (timerIds.find(triggerId) != timerIds.end())
  {
    triggerId = ++lastTimerId;
  }
  timerIds.emplace(triggerId);
  return triggerId;
}