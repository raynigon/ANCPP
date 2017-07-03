// ANCPP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include "Files.h"
#include <future>
#include <thread>
#include <chrono>
#include "Promise.h"
#include <memory>
#include "TimeTrigger.h"

using namespace ancpp;

void start(int i) 
{
  //std::cout << "Thread id:" << std::this_thread::get_id() << std::endl;
  std::vector<std::shared_ptr<ancpp::Promise<std::vector<std::wstring>>>> promises;
  auto promise = ancpp::Files::readAllLines(i%2==0 ? L"ReadMe.txt" : L"Troll.txt");
  promise->onResolve([=](std::vector<std::wstring> data)
  {
    //std::cout << "Thread id:" << std::this_thread::get_id() << std::endl;
    std::wcout << L"Read File " << i << std::endl;
  });
  promises.push_back(promise);
}

int main()
{
  using namespace std::chrono_literals;
  for (int i = 0; i < 10; i++)
  {
    EventQueue::getInstance().push([=]()
    {
      start(i);
    });
  }
  TimeTrigger tt;
  tt.onTimer([]() {
    std::cout << "Every 1000ms this text will show up" << std::endl;
  }, 1000);
  std::wcout << L" - - - End of Main - - - " << std::endl;
  std::this_thread::sleep_for(10000ms);
  EventQueue::getInstance().exit();
  return 0;
}

