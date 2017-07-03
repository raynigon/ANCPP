#pragma once
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include "EventQueue.hpp"

namespace ancpp
{
  namespace helpers 
  {
    template<typename ResultType>
    using ResolveFct = std::function<void(ResultType)>;
    using RejectFct = std::function<void(const std::wstring& reason)>;
  }

  class IPromise
  {
  public:
    virtual void onReject(helpers::RejectFct fct) = 0;
    virtual void reject(const std::wstring& exception) = 0;
    virtual bool isResolved() const = 0;
    virtual bool isRejected() const = 0;
  };

  template<typename ResultType>
  class Promise : public IPromise
  {
  public:
    virtual void resolve(const ResultType& result) = 0;
    virtual void onResolve(helpers::ResolveFct<ResultType> fct) = 0;
  public:
    static std::shared_ptr<Promise<ResultType>> create();
  };
}

#include "PromiseImpl.hpp"


template<typename ResultType>
std::shared_ptr<ancpp::Promise<ResultType>> ancpp::Promise<ResultType>::create()
{
  return std::dynamic_pointer_cast<ancpp::Promise<ResultType>>(std::make_shared<ancpp::core::PromiseImpl<ResultType>>());
}
