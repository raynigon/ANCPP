#include "stdafx.h"
#include "Files.h"
#include <vector>
#include <iterator>
#include <fstream>
#include <future>
#include <ios>
#include <string>
#include <iostream>

using namespace ancpp;

template<typename T, typename U>
class basic_line {
  T data;
public:
  operator T () const
  {
    return data;
  }

  friend U & operator>>(U & is, basic_line<T, U> & line)
  {
    return std::getline(is, line.data);
  }
};

using Line = basic_line<std::wstring, std::wistream>;
using wLine = basic_line<std::wstring, std::wistream>;

Files::Files()
{
}


Files::~Files()
{
}

std::shared_ptr<Promise<std::vector<std::wstring>>> Files::readAllLines(const std::wstring& path)
{
  auto pPromise = std::make_shared<Promise<std::vector<std::wstring>>>();
  EventQueue::getInstance().launchExternal(std::dynamic_pointer_cast<ancpp::IPromise>(pPromise), [=]() {
    std::vector<std::wstring> data;
    std::wifstream fis(path);
    if (!fis) 
    {
      pPromise->reject(L"Unable to open File");
    }
    std::copy(std::istream_iterator<Line, wchar_t>(fis),
      std::istream_iterator<Line, wchar_t>(),
      std::back_inserter(data));
    fis.close();
    pPromise->resolve(data);
  });
  return std::move(pPromise);
}
