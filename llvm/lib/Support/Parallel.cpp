//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Parallel.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ExponentialBackoff.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Jobserver.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Threading.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>
#ਅਨ੍ਤਰ੍ਭਾਵ <future>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <mutex>
#ਅਨ੍ਤਰ੍ਭਾਵ <thread>
#ਅਨ੍ਤਰ੍ਭਾਵ <vector>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ਜੇ LLVM_ENABLE_ਧਾਗੇ

ਸਥਿਰ thread_local ਬਿਨਾਨਿਸ਼ਾਨੀ thread_ਸੂਚਕਾਂਕ = UINT_MAX;

ਨਾਮਾਕਾਸ਼ {

/// Runs closures on a thread pool in filo order.
ਵਰਗ ThreadPoolExecutor {
ਜਨਤਕ:
  ਸਪਸ਼ਟ ThreadPoolExecutor(ThreadPoolStrategy S) {
    ਜੇ (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_ਗਿਣਤੀ = S.compute_thread_ਗਿਣਤੀ();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    ਧਾਗੇ.reserve(Thread_ਗਿਣਤੀ);
    ਧਾਗੇ.resize(1);
    std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    ਸਵੈਚਾਲਿਤ &Thread0 = ਧਾਗੇ[0];
    Thread0 = std::thread([ਇਹ, S] {
      ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 1; I < Thread_ਗਿਣਤੀ; ++I) {
        ਧਾਗੇ.emplace_ਪਿੱਠ([ਇਹ, S, I] { work(S, I); });
        ਜੇ (Stop)
          ਰੋਕੋ;
      }
      ਧਾਗੇ_Created.ਸੈੱਟ_ਮੁੱਲ();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = ਮਿਟਾਓ;

  ਖਾਲੀ stop() {
    {
      std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Mutex);
      ਜੇ (Stop)
        ਵਾਪਸ;
      Stop = ਸੱਚ;
    }
    Cond.notify_all();
    ਧਾਗੇ_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    ਲਈ (std::thread &T : ਧਾਗੇ)
      ਜੇ (T.get_id() == CurrentThreadId)
        T.detach();
      ਨਹੀਂਤਾਂ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ਬਣਤਰ Creator {
    ਸਥਿਰ ਖਾਲੀ *call() { ਵਾਪਸ ਨਵਾਂ ThreadPoolExecutor(strategy); }
  };
  ਬਣਤਰ Deleter {
    ਸਥਿਰ ਖਾਲੀ call(ਖਾਲੀ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ਬਣਤਰ Work_ਵਸਤੂ {
    std::function<ਖਾਲੀ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    ਖਾਲੀ ਸੰਚਾਲਕ()() {
      F();
      L.get().dec();
    }
  };

  ਖਾਲੀ ਜੋੜੋ(std::function<ਖਾਲੀ()> F, parallel::detail::Latch &L) {
    {
      std::ਤਾਲਾ_guard<std::mutex> ਤਾਲਾ_2(Mutex);
      Work_ਢੇਰ.ਧੱਕੋ_ਪਿੱਠ({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  ਖਾਲੀ helpSync(ਅਚਲ parallel::detail::Latch &L) {
    ਜਦੋਂ (L.get_ਗਿਣਤੀ() != 0) {
      std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_2(Mutex);
      ਜੇ (Stop || Work_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
        ਵਾਪਸ;
      ਕੱਢੋ_And_ਲੜੀਵਾਰ(ਤਾਲਾ_2);
    }
  }

  size_t get_Thread_ਗਿਣਤੀ() ਅਚਲ { ਵਾਪਸ Thread_ਗਿਣਤੀ; }

ਨਿਜੀ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  ਖਾਲੀ ਕੱਢੋ_And_ਲੜੀਵਾਰ(std::unique_ਤਾਲਾ<std::mutex> &ਤਾਲਾ_2) {
    ਸਵੈਚਾਲਿਤ ਵਸਤੂ = std::move(Work_ਢੇਰ.ਪਿੱਠ());
    Work_ਢੇਰ.ਕੱਢੋ_ਪਿੱਠ();
    ਤਾਲਾ_2.unlock();
    ਵਸਤੂ();
  }

  ਖਾਲੀ work(ThreadPoolStrategy S, ਬਿਨਾਨਿਸ਼ਾਨੀ ThreadID) {
    thread_ਸੂਚਕਾਂਕ = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    ਜਦੋਂ (ਸੱਚ) {
      ਜੇ (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        ਘਾਤਾਂਕੀ_Backoff Backoff(std::chrono::hours(24));
        ਕੰਮ_ਥਾਂ ਥਾਂ;
        ਕਰੋ {
          ਜੇ (Stop)
            ਵਾਪਸ;
          ਥਾਂ = TheJobserver->tryAcquire();
          ਜੇ (ਥਾਂ.isValid())
            ਰੋਕੋ;
        } ਜਦੋਂ (Backoff.wait_For_ਅਗਲਾ_Attempt());

        llvm::scope_exit ਥਾਂ_Releaser(
            [&] { TheJobserver->release(std::move(ਥਾਂ)); });

        ਜਦੋਂ (ਸੱਚ) {
          std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_2(Mutex);
          Cond.wait(ਤਾਲਾ_2, [&] { ਵਾਪਸ Stop || !Work_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ(); });
          ਜੇ (Stop && Work_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
            ਵਾਪਸ;
          ਜੇ (Work_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ())
            ਰੋਕੋ;
          ਕੱਢੋ_And_ਲੜੀਵਾਰ(ਤਾਲਾ_2);
        }
      } ਨਹੀਂਤਾਂ {
        std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ_2(Mutex);
        Cond.wait(ਤਾਲਾ_2, [&] { ਵਾਪਸ Stop || !Work_ਢੇਰ.ਖਾਲੀ_ਥਾਵਾਂ(); });
        ਜੇ (Stop)
          ਰੋਕੋ;
        ਕੱਢੋ_And_ਲੜੀਵਾਰ(ਤਾਲਾ_2);
      }
    }
  }

  std::atomic<ਤਾਰਕਿਕ> Stop{ਝੂਠ};
  std::ਸਦਿਸ਼<Work_ਵਸਤੂ> Work_ਢੇਰ;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<ਖਾਲੀ> ਧਾਗੇ_Created;
  std::ਸਦਿਸ਼<std::thread> ਧਾਗੇ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ Thread_ਗਿਣਤੀ;

  JobserverClient *TheJobserver = ਖਾਲੀ_ਸੂਚਕ;
};
} // namespace

ਸਥਿਰ ThreadPoolExecutor *getDefaultExecutor() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ਸਥਿਰ ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ਸਥਿਰ std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  ਵਾਪਸ Exec.get();
#ਨਹੀਂਤਾਂ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ਸਥਿਰ ThreadPoolExecutor Exec(strategy);
  ਵਾਪਸ &Exec;
#ਜੇ_ਅੰਤ
}

size_t parallel::get_Thread_ਗਿਣਤੀ() {
  ਵਾਪਸ getDefaultExecutor()->get_Thread_ਗਿਣਤੀ();
}
#ਜੇ_ਅੰਤ

Task_ਸਮੂਹ::Task_ਸਮੂਹ()
    : Parallel(
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
          strategy.ਧਾਗੇ_Requested != 1
#ਨਹੀਂਤਾਂ
          ਝੂਠ
#ਜੇ_ਅੰਤ
      ) {
}

Task_ਸਮੂਹ::~Task_ਸਮੂਹ() {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  ਤਾਰਕਿਕ IsNested = thread_ਸੂਚਕਾਂਕ != UINT_MAX;
  ਜੇ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#ਜੇ_ਅੰਤ
  L.ਸਮਕਾਲੀ();
}

ਖਾਲੀ Task_ਸਮੂਹ::spawn(std::function<ਖਾਲੀ()> F) {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  ਜੇ (Parallel) {
    L.inc();
    getDefaultExecutor()->ਜੋੜੋ(std::move(F), L);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ
  F();
}

ਖਾਲੀ llvm::parallelFor(size_t Begin, size_t ਸਿਰਾ_2,
                       function_ref<ਖਾਲੀ(size_t)> Fn) {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  ਜੇ (strategy.ਧਾਗੇ_Requested != 1) {
    size_t ਅੰਕ_ਚੀਜ਼ਾਂ = ਸਿਰਾ_2 - Begin;
    ਜੇ (ਅੰਕ_ਚੀਜ਼ਾਂ == 0)
      ਵਾਪਸ;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t ਅੰਕ_Workers = std::min<size_t>(ਅੰਕ_ਚੀਜ਼ਾਂ, get_Thread_ਗਿਣਤੀ());
    size_t Chunk_ਮਾਪ = std::max(size_t(1), ਅੰਕ_ਚੀਜ਼ਾਂ / (ਅੰਕ_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    ਸਵੈਚਾਲਿਤ ਕਾਮਾ = [&] {
      ਜਦੋਂ (ਸੱਚ) {
        size_t I = Idx.fetch_ਜੋੜੋ(Chunk_ਮਾਪ, std::memory_order_relaxed);
        ਜੇ (I >= ਸਿਰਾ_2)
          ਰੋਕੋ;
        size_t IEnd = std::min(I + Chunk_ਮਾਪ, ਸਿਰਾ_2);
        ਲਈ (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_ਸਮੂਹ TG;
    ਜਦੋਂ (--ਅੰਕ_Workers)
      TG.spawn(ਕਾਮਾ);
    ਕਾਮਾ();
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ

  ਲਈ (; Begin != ਸਿਰਾ_2; ++Begin)
    Fn(Begin);
}
