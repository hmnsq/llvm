//===- llvm/Support/Parallel.cpp - Parallel algorithms --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/Parallel.h"
#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/ExponentialBackoff.h"
#అన్తర్భావ "llvm/Support/Jobserver.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/Threading.h"

#అన్తర్భావ <atomic>
#అన్తర్భావ <future>
#అన్తర్భావ <memory>
#అన్తర్భావ <mutex>
#అన్తర్భావ <thread>
#అన్తర్భావ <vector>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::parallel;

llvm::ThreadPoolStrategy parallel::strategy;

#ఒకవేళ LLVM_ENABLE_దారాలు

స్థిర thread_local గుర్తులేని thread_సూచీ = UINT_MAX;

నామాకాశ {

/// Runs closures on a thread pool in filo order.
తరగతి ThreadPoolExecutor {
బహిరంగ:
  స్పష్టమైన ThreadPoolExecutor(ThreadPoolStrategy S) {
    ఒకవేళ (S.UseJobserver)
      TheJobserver = JobserverClient::getInstance();

    Thread_లెక్క = S.compute_thread_లెక్క();
    // Spawn all but one of the threads in another thread as spawning threads
    // can take a while.
    దారాలు.reserve(Thread_లెక్క);
    దారాలు.resize(1);
    std::తాళం_guard<std::mutex> తాళం_3(Mutex);
    // Use operator[] before creating the thread to avoid data race in .size()
    // in 'safe libc++' mode.
    స్వయంచాలక &Thread0 = దారాలు[0];
    Thread0 = std::thread([ఇది, S] {
      కొరకు (గుర్తులేని I = 1; I < Thread_లెక్క; ++I) {
        దారాలు.emplace_వెనుక([ఇది, S, I] { work(S, I); });
        ఒకవేళ (Stop)
          ఆపు;
      }
      దారాలు_Created.సమితి_విలువ();
      work(S, 0);
    });
  }

  // To make sure the thread pool executor can only be created with a parallel
  // strategy.
  ThreadPoolExecutor() = తొలగించు;

  శూన్య stop() {
    {
      std::తాళం_guard<std::mutex> తాళం_3(Mutex);
      ఒకవేళ (Stop)
        తిరిగి;
      Stop = నిజం;
    }
    Cond.notify_all();
    దారాలు_Created.get_future().wait();

    std::thread::id CurrentThreadId = std::this_thread::get_id();
    కొరకు (std::thread &T : దారాలు)
      ఒకవేళ (T.get_id() == CurrentThreadId)
        T.detach();
      లేకపోతే
        T.join();
  }

  ~ThreadPoolExecutor() { stop(); }

  నిర్మాణం Creator {
    స్థిర శూన్య *call() { తిరిగి కొత్త ThreadPoolExecutor(strategy); }
  };
  నిర్మాణం Deleter {
    స్థిర శూన్య call(శూన్య *Ptr) { ((ThreadPoolExecutor *)Ptr)->stop(); }
  };

  నిర్మాణం Work_అంశం {
    std::function<శూన్య()> F;
    std::reference_wrapper<parallel::detail::Latch> L;
    శూన్య క్రియాకారకం()() {
      F();
      L.get().dec();
    }
  };

  శూన్య కూడు(std::function<శూన్య()> F, parallel::detail::Latch &L) {
    {
      std::తాళం_guard<std::mutex> తాళం_3(Mutex);
      Work_దొంతర.నెట్టు_వెనుక({std::move(F), std::ref(L)});
    }
    Cond.notify_one();
  }

  // Execute tasks from the work queue until the latch reaches zero.
  // Used by nested TaskGroups (on worker threads) to prevent deadlock:
  // instead of blocking in sync(), actively help drain the queue.
  శూన్య helpSync(స్థిరరాశి parallel::detail::Latch &L) {
    అయితే (L.get_లెక్క() != 0) {
      std::unique_తాళం<std::mutex> తాళం_3(Mutex);
      ఒకవేళ (Stop || Work_దొంతర.ఖాళీలు())
        తిరిగి;
      తీయి_And_నిరంతరం(తాళం_3);
    }
  }

  size_t get_Thread_లెక్క() స్థిరరాశి { తిరిగి Thread_లెక్క; }

అంతరంగ:
  // Pop one task from the queue and run it. Must be called with Lock held;
  // releases Lock before executing the task.
  శూన్య తీయి_And_నిరంతరం(std::unique_తాళం<std::mutex> &తాళం_3) {
    స్వయంచాలక అంశం = std::move(Work_దొంతర.వెనుక());
    Work_దొంతర.తీయి_వెనుక();
    తాళం_3.unlock();
    అంశం();
  }

  శూన్య work(ThreadPoolStrategy S, గుర్తులేని ThreadID) {
    thread_సూచీ = ThreadID;
    S.apply_thread_strategy(ThreadID);
    // Note on jobserver deadlock avoidance:
    // GNU Make grants each invoked process one implicit job slot. Our
    // JobserverClient models this by returning an implicit JobSlot on the
    // first successful tryAcquire() in a process. This guarantees forward
    // progress without requiring a dedicated "always-on" thread here.

    అయితే (నిజం) {
      ఒకవేళ (TheJobserver) {
        // Jobserver-mode scheduling:
        // - Acquire one job slot (with exponential backoff to avoid busy-wait).
        // - While holding the slot, drain and run tasks from the local queue.
        // - Release the slot when the queue is empty or when shutting down.
        // Rationale: Holding a slot amortizes acquire/release overhead over
        // multiple tasks and avoids requeue/yield churn, while still enforcing
        // the jobserver’s global concurrency limit. With K available slots,
        // up to K workers run tasks in parallel; within each worker tasks run
        // sequentially until the local queue is empty.
        ఘాతాంక_Backoff Backoff(std::chrono::hours(24));
        పని_స్థానం స్థానం;
        చేయి {
          ఒకవేళ (Stop)
            తిరిగి;
          స్థానం = TheJobserver->tryAcquire();
          ఒకవేళ (స్థానం.isValid())
            ఆపు;
        } అయితే (Backoff.wait_For_తదుపరి_Attempt());

        llvm::scope_exit స్థానం_Releaser(
            [&] { TheJobserver->release(std::move(స్థానం)); });

        అయితే (నిజం) {
          std::unique_తాళం<std::mutex> తాళం_3(Mutex);
          Cond.wait(తాళం_3, [&] { తిరిగి Stop || !Work_దొంతర.ఖాళీలు(); });
          ఒకవేళ (Stop && Work_దొంతర.ఖాళీలు())
            తిరిగి;
          ఒకవేళ (Work_దొంతర.ఖాళీలు())
            ఆపు;
          తీయి_And_నిరంతరం(తాళం_3);
        }
      } లేకపోతే {
        std::unique_తాళం<std::mutex> తాళం_3(Mutex);
        Cond.wait(తాళం_3, [&] { తిరిగి Stop || !Work_దొంతర.ఖాళీలు(); });
        ఒకవేళ (Stop)
          ఆపు;
        తీయి_And_నిరంతరం(తాళం_3);
      }
    }
  }

  std::atomic<తార్కిక> Stop{అబద్ధం};
  std::సదిశ<Work_అంశం> Work_దొంతర;
  std::mutex Mutex;
  std::condition_variable Cond;
  std::promise<శూన్య> దారాలు_Created;
  std::సదిశ<std::thread> దారాలు;
  గుర్తులేని Thread_లెక్క;

  JobserverClient *TheJobserver = శూన్యసూచిక;
};
} // namespace

స్థిర ThreadPoolExecutor *getDefaultExecutor() {
#నిర్వచితమైతే _WIN32
  // The ManagedStatic enables the ThreadPoolExecutor to be stopped via
  // llvm_shutdown() on Windows. This is important to avoid various race
  // conditions at process exit that can cause crashes or deadlocks.

  స్థిర ManagedStatic<ThreadPoolExecutor, ThreadPoolExecutor::Creator,
                       ThreadPoolExecutor::Deleter>
      ManagedExec;
  స్థిర std::unique_ptr<ThreadPoolExecutor> Exec(&(*ManagedExec));
  తిరిగి Exec.get();
#లేకపోతే
  // ManagedStatic is not desired on other platforms. When `Exec` is destroyed
  // by llvm_shutdown(), worker threads will clean up and invoke TLS
  // destructors. This can lead to race conditions if other threads attempt to
  // access TLS objects that have already been destroyed.
  స్థిర ThreadPoolExecutor Exec(strategy);
  తిరిగి &Exec;
#అయితే_ముగింపు
}

size_t parallel::get_Thread_లెక్క() {
  తిరిగి getDefaultExecutor()->get_Thread_లెక్క();
}
#అయితే_ముగింపు

Task_సమూహం::Task_సమూహం()
    : Parallel(
#ఒకవేళ LLVM_ENABLE_దారాలు
          strategy.దారాలు_Requested != 1
#లేకపోతే
          అబద్ధం
#అయితే_ముగింపు
      ) {
}

Task_సమూహం::~Task_సమూహం() {
#ఒకవేళ LLVM_ENABLE_దారాలు
  // In a nested TaskGroup (threadIndex != -1u), actively help drain the queue.
  తార్కిక IsNested = thread_సూచీ != UINT_MAX;
  ఒకవేళ (Parallel && IsNested)
    getDefaultExecutor()->helpSync(L);
#అయితే_ముగింపు
  L.సమకాలీకరణ();
}

శూన్య Task_సమూహం::spawn(std::function<శూన్య()> F) {
#ఒకవేళ LLVM_ENABLE_దారాలు
  ఒకవేళ (Parallel) {
    L.inc();
    getDefaultExecutor()->కూడు(std::move(F), L);
    తిరిగి;
  }
#అయితే_ముగింపు
  F();
}

శూన్య llvm::parallelFor(size_t Begin, size_t చివర_2,
                       function_ref<శూన్య(size_t)> Fn) {
#ఒకవేళ LLVM_ENABLE_దారాలు
  ఒకవేళ (strategy.దారాలు_Requested != 1) {
    size_t అంకె_వస్తువులు = చివర_2 - Begin;
    ఒకవేళ (అంకె_వస్తువులు == 0)
      తిరిగి;
    // Distribute work via an atomic counter shared by NumWorkers threads,
    // keeping the task count (and thus Linux futex calls) at O(ThreadCount)
    // For lld, per-file work is somewhat uneven, so a multipler > 1 is safer.
    // While 2 vs 4 vs 8 makes no measurable difference, 4 is used as a
    // reasonable default.
    size_t అంకె_Workers = std::min<size_t>(అంకె_వస్తువులు, get_Thread_లెక్క());
    size_t Chunk_కొలత = std::max(size_t(1), అంకె_వస్తువులు / (అంకె_Workers * 4));
    std::atomic<size_t> Idx{Begin};
    స్వయంచాలక కార్మికుడు = [&] {
      అయితే (నిజం) {
        size_t I = Idx.fetch_కూడు(Chunk_కొలత, std::memory_order_relaxed);
        ఒకవేళ (I >= చివర_2)
          ఆపు;
        size_t IEnd = std::min(I + Chunk_కొలత, చివర_2);
        కొరకు (; I < IEnd; ++I)
          Fn(I);
      }
    };

    // Run one worker on the calling thread: starts working immediately and
    // avoids an idle thread.
    Task_సమూహం TG;
    అయితే (--అంకె_Workers)
      TG.spawn(కార్మికుడు);
    కార్మికుడు();
    తిరిగి;
  }
#అయితే_ముగింపు

  కొరకు (; Begin != చివర_2; ++Begin)
    Fn(Begin);
}
