//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Parallel.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن "llvm/Support/Jobserver.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/Threading.h"

#ضمن <atomic>
#ضمن <future>
#ضمن <memory>
#ضمن <mutex>
#ضمن <thread>
#ضمن <vector>

استفاده نطاق llvm;
استفاده نطاق llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#إذا LLVM_ENABLE_نخها

ساكن thread_local بیعلامت thread_شاخص = UINT_MAX;

نطاق {

/// Runs closures on a thread pool in filo order.
صنف ThreadPoolExecutor {
عام:
  صریح ThreadPoolExecutor(ThreadPoolStrategy S) {
    إذا (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_شمار = S.compute_thread_شمار();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    نخها.reserve(Thread_شمار);
    نخها.resize(1);
    std::قفل_guard<std::mutex> قفل_3(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    خودکار &Thread0 = نخها[0];
    Thread0 = std::thread([هذا, S] {
      لكل (بیعلامت I = 1; I < Thread_شمار; ++I) {
        نخها.emplace_پشت([هذا, S, I] { work(S, I); });
        إذا (Stop)
          اكسر;
      }
      نخها_Created.مجموعه_مقدار();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = حذف;

  فراغ stop() {
    {
      std::قفل_guard<std::mutex> قفل_3(Mutex);
      إذا (Stop)
        أرجع;
      Stop = درست;
    }
    Cond.notify_all();
    نخها_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    لكل (std::thread &T : نخها)
      إذا (T.get_id() == CurrentThreadId)
        T.detach();
      وإلا
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  بنية Creator {
    ساكن فراغ *call() { أرجع جديد ThreadPoolExecutor(strategy); }
  };
  بنية Deleter {
    ساكن فراغ call(فراغ *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  بنية Work_قلم {
    std::function<فراغ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    فراغ عملگر()() {
      F();
      L.get().dec();
    }
  };

  فراغ جمع_کن(std::function<فراغ()> F, parallel::detail::Latch &L) {
    {
      std::قفل_guard<std::mutex> قفل_3(Mutex);
      Work_پشته.افزودن_پشت({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  فراغ helpSync(ثابت parallel::detail::Latch &L) {
    طالما (L.get_شمار() != 0) {
      std::unique_قفل<std::mutex> قفل_3(Mutex);
      إذا (Stop || Work_پشته.خالی())
        أرجع;
      برداشت_And_پیاپی(قفل_3);
    }
  }

  size_t get_Thread_شمار() ثابت { أرجع Thread_شمار; }

خاص:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  فراغ برداشت_And_پیاپی(std::unique_قفل<std::mutex> &قفل_3) {
    خودکار قلم = std::move(Work_پشته.پشت());
    Work_پشته.برداشت_پشت();
    قفل_3.unlock();
    قلم();
  }

  فراغ work(ThreadPoolStrategy S, بیعلامت ThreadID) {
    thread_شاخص = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    طالما (درست) {
      إذا (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        نمایی_Backoff Backoff(std::chrono::hours(24));
        کار_جایگاه جایگاه;
        افعل {
          إذا (Stop)
            أرجع;
          جایگاه = TheJobserver->tryAcquire();
          إذا (جایگاه.isValid())
            اكسر;
        } طالما (Backoff.wait_For_بعدی_Attempt());

        llvm::scope_exit جایگاه_Releaser(
            [&] { TheJobserver->release(std::move(جایگاه)); });

        طالما (درست) {
          std::unique_قفل<std::mutex> قفل_3(Mutex);
          Cond.wait(قفل_3, [&] { أرجع Stop || !Work_پشته.خالی(); });
          إذا (Stop && Work_پشته.خالی())
            أرجع;
          إذا (Work_پشته.خالی())
            اكسر;
          برداشت_And_پیاپی(قفل_3);
        }
      } وإلا {
        std::unique_قفل<std::mutex> قفل_3(Mutex);
        Cond.wait(قفل_3, [&] { أرجع Stop || !Work_پشته.خالی(); });
        إذا (Stop)
          اكسر;
        برداشت_And_پیاپی(قفل_3);
      }
    }
  }

  std::atomic<منطقي> Stop{خطأ};
  std::وکتور<Work_قلم> Work_پشته;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<فراغ> نخها_Created;
  std::وکتور<std::thread> نخها;
  بیعلامت Thread_شمار;

  JobserverClient *TheJobserver = اشاره_تهی;
};
} // namespace

ساكن ThreadPoolExecutor *getDefaultExecutor() {
#اگر_تعریف_شده _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ساكن ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ساكن std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  أرجع Exec.get();
#وإلا
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ساكن ThreadPoolExecutor Exec(strategy);
  أرجع &Exec;
#پایان_اگر
}

size_t parallel::get_Thread_شمار() {
  أرجع getDefaultExecutor()->get_Thread_شمار();
}
#پایان_اگر

Task_گروه::Task_گروه()
    : Parallel(
#إذا LLVM_ENABLE_نخها
          strategy.نخها_Requested != 1
#وإلا
          خطأ
#پایان_اگر
      ) {
}

Task_گروه::~Task_گروه() {
#إذا LLVM_ENABLE_نخها
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقي IsNested = thread_شاخص != UINT_MAX;
  إذا (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#پایان_اگر
  L.متزامن();
}

فراغ Task_گروه::spawn(std::function<فراغ()> F) {
#إذا LLVM_ENABLE_نخها
  إذا (Parallel) {
    L.inc();
    getDefaultExecutor()->جمع_کن(std::move(F), L);
    أرجع;
  }
#پایان_اگر
  F();
}

فراغ llvm::parallelFor(size_t Begin, size_t انتها_2,
                       function_ref<فراغ(size_t)> Fn) {
#إذا LLVM_ENABLE_نخها
  إذا (strategy.نخها_Requested != 1) {
    size_t رقم_اقلام = انتها_2 - Begin;
    إذا (رقم_اقلام == 0)
      أرجع;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t رقم_Workers = std::min<size_t>(رقم_اقلام, get_Thread_شمار());
    size_t Chunk_اندازه = std::max(size_t(1), رقم_اقلام / (رقم_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    خودکار کارگر = [&] {
      طالما (درست) {
        size_t I = Idx.fetch_جمع_کن(Chunk_اندازه, std::memory_order_relaxed);
        إذا (I >= انتها_2)
          اكسر;
        size_t IEnd = std::min(I + Chunk_اندازه, انتها_2);
        لكل (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_گروه TG;
    طالما (--رقم_Workers)
      TG.spawn(کارگر);
    کارگر();
    أرجع;
  }
#پایان_اگر

  لكل (; Begin != انتها_2; ++Begin)
    Fn(Begin);
}
