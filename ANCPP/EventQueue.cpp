#include "stdafx.h"
#include "EventQueue.h"
#include <mutex>
#include <iostream>
#include <chrono>
#include "Promise.h"

using namespace ancpp;
using namespace std::chrono_literals;

template<typename R>
bool is_ready(std::future<R> const& f)
{
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

const std::chrono::milliseconds EventQueue::QUEUE_TIMEOUT = std::chrono::milliseconds(100);

EventQueue::EventQueue() : t([&]() { run(); })
{
}


EventQueue::~EventQueue()
{
  if (exit_queue == false)
    exit();
  if (t.joinable())
    t.join();
}

void EventQueue::exit()
{
  exit_queue = true;
  monitor.notify_all();
  t.join();
}

void EventQueue::run()
{
  while (exit_queue == false)
  {
    if (queue.empty())
      waitForEvent();
    else
      handleEvent();
    if(queue.empty())
      cleanUpFutures();
  }
  cleanUpFutures();
}
void EventQueue::push(std::function<void()> callable)
{
  std::lock_guard<std::recursive_mutex> lg(queue_mutex);
  queue.push(callable);
}

void EventQueue::launchExternal(std::shared_ptr<ancpp::IPromise> promise, std::function<void()> callable)
{
  std::lock_guard<std::recursive_mutex> lg(futures_mutex);
  auto future_pair = std::make_pair(promise, std::async(std::launch::async, callable));
  futures.emplace_back(std::move(future_pair));
}

std::shared_ptr<Promise<int>> EventQueue::launchExternal(std::function<void()> callable)
{
  std::lock_guard<std::recursive_mutex> lg(futures_mutex);
  auto promise = std::make_shared<ancpp::Promise<int>>();
  auto future_pair = std::make_pair(promise, std::async(std::launch::async, [=]() {
    callable();
    promise->resolve(0);
  }));
  futures.emplace_back(std::move(future_pair));
  return promise;
}

void ancpp::EventQueue::waitForEvent()
{
  try{
    std::unique_lock<std::mutex> lk(monitor_mutex);
    monitor.wait_until(lk, std::chrono::system_clock::now() + EventQueue::QUEUE_TIMEOUT, [&]() { return exit_queue || !queue.empty(); });
    lk.unlock();
  }
  catch (std::exception& e) 
  {
    std::cout << e.what() << std::endl;
    exit_queue = true;
  }
}

void ancpp::EventQueue::handleEvent()
{
  std::function<void()> callable = nullptr;
  {
    std::lock_guard<std::recursive_mutex> lg(queue_mutex);
    callable = queue.front();
    queue.pop();
  }
  if (callable != nullptr)
  {
    callable();
  }
}

void ancpp::EventQueue::cleanUpFutures()
{
  std::lock_guard<std::recursive_mutex> lg(futures_mutex);
  auto it = std::remove_if(futures.begin(), futures.end(), [](std::pair<std::shared_ptr<IPromise>, std::future<void>>& dataPair) {
    return dataPair.first->isResolved() && is_ready(dataPair.second);
  });
  if (it != futures.end())
  {
    size_t dist = std::distance(it, futures.end());
    futures.erase(it, futures.end());
    std::cout << "Removed " << dist << " Futures" << std::endl;
  }
}

