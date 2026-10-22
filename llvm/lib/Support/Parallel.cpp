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

ورتوں نطاق llvm;
ورتوں نطاق llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#إذا LLVM_ENABLE_دھاگے

ساكن thread_local بنانشانی thread_اشاریہ = UINT_MAX;

نطاق {

/// Runs closures on a thread pool in filo order.
صنف ThreadPoolExecutor {
عام:
  واضح ThreadPoolExecutor(ThreadPoolStrategy S) {
    إذا (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_گنتی = S.compute_thread_گنتی();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    دھاگے.reserve(Thread_گنتی);
    دھاگے.resize(1);
    std::جندرا_guard<std::mutex> جندرا_2(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    خودکار &Thread0 = دھاگے[0];
    Thread0 = std::thread([هذا, S] {
      لكل (بنانشانی I = 1; I < Thread_گنتی; ++I) {
        دھاگے.emplace_پٹھ([هذا, S, I] { work(S, I); });
        إذا (Stop)
          اكسر;
      }
      دھاگے_Created.سیٹ_قدر();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = مٹاؤ;

  فراغ stop() {
    {
      std::جندرا_guard<std::mutex> جندرا_2(Mutex);
      إذا (Stop)
        أرجع;
      Stop = سچ;
    }
    Cond.notify_all();
    دھاگے_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    لكل (std::thread &T : دھاگے)
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

  بنية Work_شے {
    std::function<فراغ()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    فراغ عامل()() {
      F();
      L.get().dec();
    }
  };

  فراغ جوڑو(std::function<فراغ()> F, parallel::detail::Latch &L) {
    {
      std::جندرا_guard<std::mutex> جندرا_2(Mutex);
      Work_ڈھیر.ڈالو_پٹھ({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  فراغ helpSync(ثابت parallel::detail::Latch &L) {
    طالما (L.get_گنتی() != 0) {
      std::unique_جندرا<std::mutex> جندرا_2(Mutex);
      إذا (Stop || Work_ڈھیر.خالی_تھاواں())
        أرجع;
      نکالو_And_لگاتار(جندرا_2);
    }
  }

  size_t get_Thread_گنتی() ثابت { أرجع Thread_گنتی; }

خاص:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  فراغ نکالو_And_لگاتار(std::unique_جندرا<std::mutex> &جندرا_2) {
    خودکار شے = std::move(Work_ڈھیر.پٹھ());
    Work_ڈھیر.نکالو_پٹھ();
    جندرا_2.unlock();
    شے();
  }

  فراغ work(ThreadPoolStrategy S, بنانشانی ThreadID) {
    thread_اشاریہ = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    طالما (سچ) {
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
        قوت_نمائی_Backoff Backoff(std::chrono::hours(24));
        کم_تھاں تھاں;
        افعل {
          إذا (Stop)
            أرجع;
          تھاں = TheJobserver->tryAcquire();
          إذا (تھاں.isValid())
            اكسر;
        } طالما (Backoff.wait_For_اگلا_Attempt());

        llvm::scope_exit تھاں_Releaser(
            [&] { TheJobserver->release(std::move(تھاں)); });

        طالما (سچ) {
          std::unique_جندرا<std::mutex> جندرا_2(Mutex);
          Cond.wait(جندرا_2, [&] { أرجع Stop || !Work_ڈھیر.خالی_تھاواں(); });
          إذا (Stop && Work_ڈھیر.خالی_تھاواں())
            أرجع;
          إذا (Work_ڈھیر.خالی_تھاواں())
            اكسر;
          نکالو_And_لگاتار(جندرا_2);
        }
      } وإلا {
        std::unique_جندرا<std::mutex> جندرا_2(Mutex);
        Cond.wait(جندرا_2, [&] { أرجع Stop || !Work_ڈھیر.خالی_تھاواں(); });
        إذا (Stop)
          اكسر;
        نکالو_And_لگاتار(جندرا_2);
      }
    }
  }

  std::atomic<منطقي> Stop{خطأ};
  std::سمتیہ<Work_شے> Work_ڈھیر;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<فراغ> دھاگے_Created;
  std::سمتیہ<std::thread> دھاگے;
  بنانشانی Thread_گنتی;

  JobserverClient *TheJobserver = خالی_سوچک;
};
} // namespace

ساكن ThreadPoolExecutor *getDefaultExecutor() {
#جے_تعریف_شدہ _WIN32
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
#جے_انت
}

size_t parallel::get_Thread_گنتی() {
  أرجع getDefaultExecutor()->get_Thread_گنتی();
}
#جے_انت

Task_گروہ::Task_گروہ()
    : Parallel(
#إذا LLVM_ENABLE_دھاگے
          strategy.دھاگے_Requested != 1
#وإلا
          خطأ
#جے_انت
      ) {
}

Task_گروہ::~Task_گروہ() {
#إذا LLVM_ENABLE_دھاگے
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقي IsNested = thread_اشاریہ != UINT_MAX;
  إذا (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#جے_انت
  L.متزامن();
}

فراغ Task_گروہ::spawn(std::function<فراغ()> F) {
#إذا LLVM_ENABLE_دھاگے
  إذا (Parallel) {
    L.inc();
    getDefaultExecutor()->جوڑو(std::move(F), L);
    أرجع;
  }
#جے_انت
  F();
}

فراغ llvm::parallelFor(size_t Begin, size_t سرا_2,
                       function_ref<فراغ(size_t)> Fn) {
#إذا LLVM_ENABLE_دھاگے
  إذا (strategy.دھاگے_Requested != 1) {
    size_t عدد_اشیاء = سرا_2 - Begin;
    إذا (عدد_اشیاء == 0)
      أرجع;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t عدد_Workers = std::min<size_t>(عدد_اشیاء, get_Thread_گنتی());
    size_t Chunk_ناپ = std::max(size_t(1), عدد_اشیاء / (عدد_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    خودکار کامہ = [&] {
      طالما (سچ) {
        size_t I = Idx.fetch_جوڑو(Chunk_ناپ, std::memory_order_relaxed);
        إذا (I >= سرا_2)
          اكسر;
        size_t IEnd = std::min(I + Chunk_ناپ, سرا_2);
        لكل (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_گروہ TG;
    طالما (--عدد_Workers)
      TG.spawn(کامہ);
    کامہ();
    أرجع;
  }
#جے_انت

  لكل (; Begin != سرا_2; ++Begin)
    Fn(Begin);
}
