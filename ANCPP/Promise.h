#pragma once
#include <functional>
#include <vcruntime_exception.h>
#include <mutex>
#include <memory>
#include "EventQueue.h"

namespace ancpp
{
  namespace helpers 
  {
    template<typename ResultType>
    using ResolveFct = std::function<void(ResultType)>;
    using RejectFct = std::function<void(const std::exception& exception)>;
  }

  class IPromise
  {
  public:
    virtual void onReject(helpers::RejectFct fct) = 0;
    virtual void reject(const std::exception& exception) = 0;
    virtual bool isResolved() const = 0;
  };

  template<typename ResultType>
  class Promise : public IPromise
  {
  private:
    bool resolved = false;
    std::mutex mutex;
    std::unique_ptr<ResultType> pResult;
    helpers::ResolveFct<ResultType> resolveFct;
    helpers::RejectFct rejectFct;
  protected:
    void resolve(std::unique_ptr<ResultType>&& result);
  public:
    Promise();
    ~Promise();
    virtual bool isResolved() const override;
    void resolve(const ResultType& result);
    virtual void reject(const std::exception& exception) override;
    void onResolve(helpers::ResolveFct<ResultType> fct);
    virtual void onReject(std::function<void(const std::exception& exception)> fct) override;
  };

  using lock_guard = std::lock_guard<std::mutex>;

  template<typename ResultType>
  Promise<ResultType>::Promise()
  {
  }

  template<typename ResultType>
  Promise<ResultType>::~Promise()
  {
    if (!resolved && rejectFct != nullptr)
      rejectFct(std::exception("Unresolved Promise"));
  }


  template<typename ResultType>
  bool ancpp::Promise<ResultType>::isResolved() const
  {
    return resolved;
  }


  template<typename ResultType>
  void ancpp::Promise<ResultType>::resolve(const ResultType& result)
  {
    resolve(std::make_unique<ResultType>(result));
  }

  template<typename ResultType>
  void ancpp::Promise<ResultType>::resolve(std::unique_ptr<ResultType>&& result)
  {
    //lock_guard(mutex);
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
  void Promise<ResultType>::reject(const std::exception& exception)
  {
    resolved = true;
    throw std::logic_error("The method or operation is not implemented.");
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
  }
}
