#pragma once
#include <functional>
#include <mutex>
#include <memory>
#include <atomic>
#include "EventQueue.hpp"

namespace ancpp 
{
  namespace core 
  {
    template<typename ResultType>
    class PromiseImpl : public Promise<ResultType>
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
      PromiseImpl();
      ~PromiseImpl();
      virtual bool isResolved() const override;
      virtual bool isRejected() const override;
      void resolve(const ResultType& result);
      virtual void reject(const std::wstring& reason) override;
      void onResolve(helpers::ResolveFct<ResultType> fct);
      virtual void onReject(helpers::RejectFct fct) override;
    };
  }
}

namespace ancpp 
{
  namespace core
  {
    template<typename ResultType>
    PromiseImpl<ResultType>::PromiseImpl()
    {
    }

    template<typename ResultType>
    PromiseImpl<ResultType>::~PromiseImpl()
    {
      if (!resolved && !rejected && rejectFct != nullptr)
        rejectFct(L"Unresolved Promise");
    }


    template<typename ResultType>
    bool PromiseImpl<ResultType>::isResolved() const
    {
      return resolved;
    }

    template<typename ResultType>
    bool PromiseImpl<ResultType>::isRejected() const
    {
      return rejected;
    }

    template<typename ResultType>
    void PromiseImpl<ResultType>::resolve(const ResultType& result)
    {
      resolve(std::make_unique<ResultType>(result));
    }

    template<typename ResultType>
    void PromiseImpl<ResultType>::resolve(std::unique_ptr<ResultType>&& result)
    {
      std::lock_guard<std::mutex> lg(mutex);
      pResult = std::move(result);
      if (resolveFct != nullptr)
      {
        EventQueue::getInstance().push([&]()
        {
          resolveFct(*pResult);
          resolved = true;
        });
      }
    }

    template<typename ResultType>
    void PromiseImpl<ResultType>::reject(const std::wstring& reason)
    {
      std::lock_guard<std::mutex> lg(mutex);
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
    void PromiseImpl<ResultType>::onResolve(helpers::ResolveFct<ResultType> fct)
    {
      std::lock_guard<std::mutex> lg(mutex);
      resolveFct = fct;
      if (pResult != nullptr)
      {
        fct(*pResult);
        resolved = true;
      }
    }

    template<typename ResultType>
    void PromiseImpl<ResultType>::onReject(helpers::RejectFct fct)
    {
      std::lock_guard<std::mutex> lg(mutex);
      rejectFct = fct;
      if (rejected)
      {
        fct(rejectReason);
        resolved = true;
      }
    }
  }
}