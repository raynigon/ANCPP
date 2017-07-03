#pragma once
#include <functional>
#include <vcruntime_exception.h>
#include <mutex>
#include <memory>
#include "EventQueue.h"
#include <atomic>

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
  private:
    std::atomic<bool> resolved = false;
    std::atomic<bool> rejected = false;
    std::mutex mutex;
    std::unique_ptr<ResultType> pResult = nullptr;
    helpers::ResolveFct<ResultType> resolveFct = nullptr;
    helpers::RejectFct rejectFct = nullptr;
    std::wstring rejectReason;
  protected:
    void resolve(std::unique_ptr<ResultType>&& result);
  public:
    Promise();
    ~Promise();
    virtual bool isResolved() const override;
    virtual bool isRejected() const override;
    void resolve(const ResultType& result);
    virtual void reject(const std::wstring& reason) override;
    void onResolve(helpers::ResolveFct<ResultType> fct);
    virtual void onReject(helpers::RejectFct fct) override;
  };

  using lock_guard = std::lock_guard<std::mutex>;

  template<typename ResultType>
  Promise<ResultType>::Promise()
  {
  }

  template<typename ResultType>
  Promise<ResultType>::~Promise()
  {
    if (!resolved && !rejected && rejectFct != nullptr)
      rejectFct(L"Unresolved Promise");
  }


  template<typename ResultType>
  bool ancpp::Promise<ResultType>::isResolved() const
  {
    return resolved;
  }

  template<typename ResultType>
  bool ancpp::Promise<ResultType>::isRejected() const
  {
    return rejected;
  }

  template<typename ResultType>
  void ancpp::Promise<ResultType>::resolve(const ResultType& result)
  {
    resolve(std::make_unique<ResultType>(result));
  }

  template<typename ResultType>
  void ancpp::Promise<ResultType>::resolve(std::unique_ptr<ResultType>&& result)
  {
    std::lock_guard<std::mutex> lg(mutex);
    pResult = std::move(result);
    if (resolveFct != nullptr) 
    {
      EventQueue::getInstance().push([&]() {
        resolveFct(*pResult);
        resolved = true;
      });
    }
  }

  template<typename ResultType>
  void Promise<ResultType>::reject(const std::wstring& reason)
  {
    lock_guard lg(mutex);
    rejected = true;
    rejectReason = reason;
    if (rejected)
    {
      EventQueue::getInstance().push([&]()
      {
        rejectFct(rejectReason);
        resolved = true;
      });
    }
  }

  template<typename ResultType>
  void Promise<ResultType>::onResolve(helpers::ResolveFct<ResultType> fct)
  {
    lock_guard lg(mutex);
    resolveFct = fct;
    if (pResult != nullptr)
    {
      fct(*pResult);
      resolved = true;
    }
  }

  template<typename ResultType>
  void Promise<ResultType>::onReject(helpers::RejectFct fct)
  {
    lock_guard lg(mutex);
    rejectFct = fct;
    if (rejected)
    {
      fct(rejectReason);
      resolved = true;
    }
  }
}
