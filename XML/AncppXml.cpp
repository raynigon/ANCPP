#include "AncppXml.hpp"
#include "tinyxml2.h"
#include <sstream>

using namespace ancpp;
using namespace tinyxml2;

void ancpp::XmlHandler::rejectWithError(PromisePtr<XMLDocumentPtr> promise, tinyxml2::XMLError errorCode, std::wstring filename)
{
  std::wstringstream stream;
  if (!filename.empty()) 
  {
    stream << L"Unable to load XMLFile "
      << filename
      << L"\n";
  }
  else 
  {
    stream << L"Unable to parse string\n";
  }
  stream  << L"Error with Id: "
          << errorCode
          << L" occurred";
  promise->reject(stream.str());
}

PromisePtr<XMLDocumentPtr> XmlHandler::readXmlFromFile(const std::wstring & filename)
{
  auto xmldoc = std::make_shared<XMLDocument>();
  auto prmse = Promise<XMLDocumentPtr>::create();
  EventQueue::getInstance().launchExternal([=]()
  {
    std::string filenameCStr(filename.begin(), filename.end());
    XMLError xmlError = xmldoc->LoadFile(filenameCStr.c_str());
    if (xmlError == XMLError::XML_SUCCESS) 
      prmse->resolve(xmldoc);
    else 
      rejectWithError(prmse, xmlError, filename);
  });
  return prmse;
}

PromisePtr<XMLDocumentPtr> ancpp::XmlHandler::readXmlFromString(const std::wstring & xmltext)
{
  auto xmldoc = std::make_shared<XMLDocument>();
  auto prmse = Promise<XMLDocumentPtr>::create();
  EventQueue::getInstance().launchExternal([=]()
  {
    std::string xmltextCStr(xmltext.begin(), xmltext.end());
    XMLError xmlError = xmldoc->Parse(xmltextCStr.c_str(), xmltextCStr.size());
    if (xmlError == XMLError::XML_SUCCESS)
      prmse->resolve(xmldoc);
    else
      rejectWithError(prmse, xmlError);
  });
  return prmse;
}
