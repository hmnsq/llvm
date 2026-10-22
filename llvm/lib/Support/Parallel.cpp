//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Parallel.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/ExponentialBackoff.h"
#شامل "llvm/Support/Jobserver.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/Threading.h"

#شامل <atomic>
#شامل <future>
#شامل <memory>
#شامل <mutex>
#شامل <thread>
#شامل <vector>

استعمال نامخلا llvm;
استعمال نامخلا llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#اگر LLVM_ENABLE_دھاگے

ساکن thread_local غیرعلامتی thread_اشاریہ = UINT_MAX;

نامخلا {

/// Runs closures on a thread pool in filo order.
جماعت ThreadPoolExecutor {
عوامی:
  صریح ThreadPoolExecutor(ThreadPoolStrategy S) {
    اگر (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_گنتی = S.compute_thread_گنتی();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    دھاگے.reserve(Thread_گنتی);
    دھاگے.resize(1);
    std::تالا_guard<std::mutex> تالا_3(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    خودکار &Thread0 = دھاگے[0];
    Thread0 = std::thread([یہ, S] {
      برائے (غیرعلامتی I = 1; I < Thread_گنتی; ++I) {
        دھاگے.emplace_عقب([یہ, S, I] { work(S, I); });
        اگر (Stop)
          توڑو;
      }
      دھاگے_Created.سیٹ_قدر();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = حذف;

  خالی stop() {
    {
      std::تالا_guard<std::mutex> تالا_3(Mutex);
      اگر (Stop)
        واپس;
      Stop = سچ;
    }
    Cond.notify_all();
    دھاگے_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    برائے (std::thread &T : دھاگے)
      اگر (T.get_id() == CurrentThreadId)
        T.detach();
      ورنہ
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  ساخت Creator {
    ساکن خالی *call() { واپس نیا ThreadPoolExecutor(strategy); }
  };
  ساخت Deleter {
    ساکن خالی call(خالی *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  ساخت Work_شے {
    std::function<خالی()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    خالی عامل()() {
      F();
      L.get().dec();
    }
  };

  خالی جوڑو(std::function<خالی()> F, parallel::detail::Latch &L) {
    {
      std::تالا_guard<std::mutex> تالا_3(Mutex);
      Work_ڈھیر.ڈالو_عقب({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  خالی helpSync(ثابت parallel::detail::Latch &L) {
    جب (L.get_گنتی() != 0) {
      std::unique_تالا<std::mutex> تالا_3(Mutex);
      اگر (Stop || Work_ڈھیر.خالی_خانے())
        واپس;
      نکالو_And_تسلسل(تالا_3);
    }
  }

  size_t get_Thread_گنتی() ثابت { واپس Thread_گنتی; }

نجی:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  خالی نکالو_And_تسلسل(std::unique_تالا<std::mutex> &تالا_3) {
    خودکار شے = std::move(Work_ڈھیر.عقب());
    Work_ڈھیر.نکالو_عقب();
    تالا_3.unlock();
    شے();
  }

  خالی work(ThreadPoolStrategy S, غیرعلامتی ThreadID) {
    thread_اشاریہ = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    جب (سچ) {
      اگر (TheJobserver) {
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
        کام_خانہ خانہ;
        کرو {
          اگر (Stop)
            واپس;
          خانہ = TheJobserver->tryAcquire();
          اگر (خانہ.isValid())
            توڑو;
        } جب (Backoff.wait_For_اگلا_Attempt());

        llvm::scope_exit خانہ_Releaser(
            [&] { TheJobserver->release(std::move(خانہ)); });

        جب (سچ) {
          std::unique_تالا<std::mutex> تالا_3(Mutex);
          Cond.wait(تالا_3, [&] { واپس Stop || !Work_ڈھیر.خالی_خانے(); });
          اگر (Stop && Work_ڈھیر.خالی_خانے())
            واپس;
          اگر (Work_ڈھیر.خالی_خانے())
            توڑو;
          نکالو_And_تسلسل(تالا_3);
        }
      } ورنہ {
        std::unique_تالا<std::mutex> تالا_3(Mutex);
        Cond.wait(تالا_3, [&] { واپس Stop || !Work_ڈھیر.خالی_خانے(); });
        اگر (Stop)
          توڑو;
        نکالو_And_تسلسل(تالا_3);
      }
    }
  }

  std::atomic<منطقی> Stop{جھوٹ};
  std::سمتیہ<Work_شے> Work_ڈھیر;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<خالی> دھاگے_Created;
  std::سمتیہ<std::thread> دھاگے;
  غیرعلامتی Thread_گنتی;

  JobserverClient *TheJobserver = خالی_اشاریہ;
};
} // namespace

ساکن ThreadPoolExecutor *getDefaultExecutor() {
#اگر_تعریف_شدہ _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  ساکن ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  ساکن std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  واپس Exec.get();
#ورنہ
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  ساکن ThreadPoolExecutor Exec(strategy);
  واپس &Exec;
#اگر_ختم
}

size_t parallel::get_Thread_گنتی() {
  واپس getDefaultExecutor()->get_Thread_گنتی();
}
#اگر_ختم

Task_گروہ::Task_گروہ()
    : Parallel(
#اگر LLVM_ENABLE_دھاگے
          strategy.دھاگے_Requested != 1
#ورنہ
          جھوٹ
#اگر_ختم
      ) {
}

Task_گروہ::~Task_گروہ() {
#اگر LLVM_ENABLE_دھاگے
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  منطقی IsNested = thread_اشاریہ != UINT_MAX;
  اگر (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#اگر_ختم
  L.ہمآہنگ();
}

خالی Task_گروہ::spawn(std::function<خالی()> F) {
#اگر LLVM_ENABLE_دھاگے
  اگر (Parallel) {
    L.inc();
    getDefaultExecutor()->جوڑو(std::move(F), L);
    واپس;
  }
#اگر_ختم
  F();
}

خالی llvm::parallelFor(size_t Begin, size_t انتہا_2,
                       function_ref<خالی(size_t)> Fn) {
#اگر LLVM_ENABLE_دھاگے
  اگر (strategy.دھاگے_Requested != 1) {
    size_t عدد_اشیاء = انتہا_2 - Begin;
    اگر (عدد_اشیاء == 0)
      واپس;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t عدد_Workers = std::min<size_t>(عدد_اشیاء, get_Thread_گنتی());
    size_t Chunk_سائز = std::max(size_t(1), عدد_اشیاء / (عدد_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    خودکار کارکن = [&] {
      جب (سچ) {
        size_t I = Idx.fetch_جوڑو(Chunk_سائز, std::memory_order_relaxed);
        اگر (I >= انتہا_2)
          توڑو;
        size_t IEnd = std::min(I + Chunk_سائز, انتہا_2);
        برائے (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_گروہ TG;
    جب (--عدد_Workers)
      TG.spawn(کارکن);
    کارکن();
    واپس;
  }
#اگر_ختم

  برائے (; Begin != انتہا_2; ++Begin)
    Fn(Begin);
}
