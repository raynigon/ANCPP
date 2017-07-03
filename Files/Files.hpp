#pragma once
#include <string>
#include <vector>
#include "Core/Promise.hpp"

namespace ancpp
{
  class Files
  {
  public:
    Files();
    ~Files();
    static std::shared_ptr<Promise<std::vector<std::wstring>>> readAllLines(const std::wstring& path);
  };
}

