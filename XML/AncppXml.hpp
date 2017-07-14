#pragma once
#include "../Core/Promise.hpp"
#include "tinyxml2.h"

namespace ancpp
{
  using XMLDocumentPtr = std::shared_ptr<tinyxml2::XMLDocument>;
  class XmlHandler
  {
    static void rejectWithError(PromisePtr<XMLDocumentPtr> promise, tinyxml2::XMLError errorCode, std::wstring filename=L"");
  public:
    static PromisePtr<XMLDocumentPtr> readXmlFromFile(const std::wstring& filename);
    static PromisePtr<XMLDocumentPtr> readXmlFromString(const std::wstring& filename);
  };
}