#include <future>
#include <thread>
#include <unordered_set>
#include <vector>
#include "Timing/TimeTrigger.hpp"
#include "Core/EventQueue.hpp"

using namespace ancpp;


TimeTrigger::TimeTrigger()
{
}


TimeTrigger::~TimeTrigger()
{
  {
    std::lock_guard<std::mutex> lg(timerIdsMutex);
    timerIds.clear();
  }
  // TODO: Caution some stuff could still running here, wait until its finished
  std::vector<std::shared_future<void>> futures;
  {
    std::lock_guard<std::recursive_mutex> lg(EventQueue::getInstance().futures_mutex);
    for (auto& futurePair : EventQueue::getInstance().futures)
    {
      if (promises.find(futurePair.first) != promises.end())
        futures.push_back(futurePair.second.share());
    }
  }
  for (auto& futureFct : futures) 
  {
    futureFct.wait();
  }
}

long TimeTrigger::onTimer(std::function<void()> callable, long timeout)
{
  long triggerId = createTriggerId();
  auto promisePtr = EventQueue::getInstance().launchExternal([&,callable,timeout,triggerId]() {
    while(isActive(triggerId)){
      std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
      //Implement a condition variable which is notified if the Time Trigger is killed
      EventQueue::getInstance().push(callable);
    }
  });
  promises.insert(promisePtr);
  return triggerId;
}

void TimeTrigger::destroyTimer(long timerId)
{
  std::lock_guard<std::mutex> lg(timerIdsMutex);
  auto it = timerIds.find(timerId);
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