#pragma once
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include "EventQueue.hpp"

namespace ancpp
{

  class IPromise;

  template<typename T>
  class Promise;

  using IPromisePtr = std::shared_ptr<IPromise>;

  template<typename T>
  using PromisePtr = std::shared_ptr < Promise<T>>;

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
    static PromisePtr<ResultType> create();
  };
}

#include "PromiseImpl.hpp"


template<typename ResultType>
ancpp::PromisePtr<ResultType> ancpp::Promise<ResultType>::create()
{
  return std::dynamic_pointer_cast<ancpp::Promise<ResultType>>(std::make_shared<ancpp::core::PromiseImpl<ResultType>>());
}
