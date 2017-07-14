// ANCPP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <string>
#include <future>
#include <thread>
#include <chrono>
#include <memory>
#include "Files/Files.hpp"
#include "Core/Promise.hpp"
#include "Timing/TimeTrigger.hpp"
#include "XML/AncppXml.hpp"

using namespace ancpp;
using namespace std::chrono_literals;

void start(int i) 
{
  auto promise = ancpp::Files::readAllLines(i%2==0 ? L"ReadMe.txt" : L"Troll.txt");
  promise->onResolve([=](std::vector<std::wstring> data)
  {
    std::wcout << L"Read File " << i << std::endl;
  });
  promise->onReject([=](const std::wstring& reason) 
  {
    std::wcerr << L"Error: " << reason << std::endl;
  });
}

void readXml() 
{
  auto promisePtr = ancpp::XmlHandler::readXmlFromFile(L"Test.xml");
  promisePtr->onResolve([](ancpp::XMLDocumentPtr doc) {
    std::wcout << L"XML Text:" << std::endl;
    std::cout << doc->FirstChildElement("PLAY")->FirstChildElement("TITLE")->GetText() << std::endl;
  });
  promisePtr->onReject([](const std::wstring& reason) {
    std::wcerr << L"XML-Error: " << reason << std::endl;
  });
}

int main()
{
  for (int i = 0; i < 10; i++)
  {
    EventQueue::getInstance().push([=]()
    {
      start(i);
    });
  }

  EventQueue::getInstance().push(readXml);
  
  TimeTrigger tt;
  long timerId = tt.onTimer([]() {
    std::cout << "Every 1000ms this text will show up" << std::endl;
  }, 1000);

  std::wcout << L" - - - Started all Events - - - " << std::endl;
  std::this_thread::sleep_for(10000ms);
  tt.destroyTimer(timerId);
  std::wcout << L" - - - Destroyed Timer - - - " << std::endl;

  std::this_thread::sleep_for(2500ms);
  EventQueue::getInstance().exit();
  return 0;
}

