//==-- llvm/Support/ThreadPool.cpp - A ThreadPool implementation -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
// This file implements a crude C++11 based thread pool.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ThreadPool.h"

#అన్తర్భావ "llvm/Config/llvm-config.h"

#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/Support/ExponentialBackoff.h"
#అన్తర్భావ "llvm/Support/FormatVariadic.h"
#అన్తర్భావ "llvm/Support/Threading.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;

ThreadPoolInterface::~ThreadPoolInterface() = అప్రమేయ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#ఒకవేళ LLVM_ENABLE_దారాలు

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_లెక్క(S.compute_thread_లెక్క()) {
  ఒకవేళ (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

శూన్య StdThreadPool::grow(పూర్ణాంకం requested) {
  llvm::sys::ScopedWriter తాళం_Guard(దారాలు_తాళం);
  ఒకవేళ (దారాలు.కొలత_2() >= Max_Thread_లెక్క)
    తిరిగి; // Already hit the max thread pool size.
  పూర్ణాంకం new_Thread_లెక్క = std::min<పూర్ణాంకం>(requested, Max_Thread_లెక్క);
  అయితే (static_cast<పూర్ణాంకం>(దారాలు.కొలత_2()) < new_Thread_లెక్క) {
    పూర్ణాంకం ThreadID = దారాలు.కొలత_2();
    దారాలు.emplace_వెనుక([ఇది, ThreadID] {
      సమితి_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      ఒకవేళ (TheJobserver)
        processTasksWithJobserver();
      లేకపోతే
        processTasks(శూన్యసూచిక);
    });
  }
}

#నిర్వచితం_కాకపోతే NDEBUG
// The group of the tasks run by the current thread.
స్థిర LLVM_THREAD_LOCAL std::సదిశ<Thread_Pool_Task_సమూహం *>
    *CurrentThreadTaskGroups = శూన్యసూచిక;
#అయితే_ముగింపు

// WaitingForGroup == nullptr means all tasks regardless of their group.
శూన్య StdThreadPool::processTasks(Thread_Pool_Task_సమూహం *నిరీక్షణ_For_సమూహం) {
  అయితే (నిజం) {
    llvm::unique_function<శూన్య()> Task;
    Thread_Pool_Task_సమూహం *సమూహం_యొక్క_Task;
    {
      std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);
      తార్కిక work_Completed_For_సమూహం = అబద్ధం; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      వరుస_Condition.wait(తాళం_Guard, [&] {
        తిరిగి !EnableFlag || !Tasks.ఖాళీలు() ||
               (నిరీక్షణ_For_సమూహం != శూన్యసూచిక &&
                (work_Completed_For_సమూహం =
                     workCompletedUnlocked(నిరీక్షణ_For_సమూహం)));
      });
      // Exit condition
      ఒకవేళ (!EnableFlag && Tasks.ఖాళీలు())
        తిరిగి;
      ఒకవేళ (నిరీక్షణ_For_సమూహం != శూన్యసూచిక && work_Completed_For_సమూహం)
        తిరిగి;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++చురుకైన_దారాలు;
      Task = std::move(Tasks.front().మొదటి_2);
      సమూహం_యొక్క_Task = Tasks.front().రెండవ_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      ఒకవేళ (సమూహం_యొక్క_Task != శూన్యసూచిక)
        ++చురుకైన_Groups[సమూహం_యొక్క_Task]; // Increment or set to 1 if new item
      Tasks.తీయి_front();
    }
#నిర్వచితం_కాకపోతే NDEBUG
    ఒకవేళ (CurrentThreadTaskGroups == శూన్యసూచిక)
      CurrentThreadTaskGroups = కొత్త std::సదిశ<Thread_Pool_Task_సమూహం *>;
    CurrentThreadTaskGroups->నెట్టు_వెనుక(సమూహం_యొక్క_Task);
#అయితే_ముగింపు

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#నిర్వచితం_కాకపోతే NDEBUG
    CurrentThreadTaskGroups->తీయి_వెనుక();
    ఒకవేళ (CurrentThreadTaskGroups->ఖాళీలు()) {
      తొలగించు CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = శూన్యసూచిక;
    }
#అయితే_ముగింపు

    తార్కిక Notify;
    తార్కిక Notify_సమూహం;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::తాళం_guard<std::mutex> తాళం_Guard(వరుస_తాళం);
      --చురుకైన_దారాలు;
      ఒకవేళ (సమూహం_యొక్క_Task != శూన్యసూచిక) {
        స్వయంచాలక A = చురుకైన_Groups.కనుగొను(సమూహం_యొక్క_Task);
        ఒకవేళ (--(A->రెండవ_2) == 0)
          చురుకైన_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(సమూహం_యొక్క_Task);
      Notify_సమూహం = సమూహం_యొక్క_Task != శూన్యసూచిక && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    ఒకవేళ (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    ఒకవేళ (Notify_సమూహం)
      వరుస_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
శూన్య StdThreadPool::processTasksWithJobserver() {
  అయితే (నిజం) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    పని_స్థానం స్థానం;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    ఘాతాంక_Backoff Backoff(std::chrono::hours(24));
    తార్కిక AcquiredToken = అబద్ధం;
    చేయి {
      // Return if the thread pool is shutting down.
      {
        std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);
        ఒకవేళ (!EnableFlag)
          తిరిగి;
      }

      స్థానం = TheJobserver->tryAcquire();
      ఒకవేళ (స్థానం.isValid()) {
        AcquiredToken = నిజం;
        ఆపు;
      }
    } అయితే (Backoff.wait_For_తదుపరి_Attempt());

    ఒకవేళ (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit స్థానం_Releaser(
        [&] { TheJobserver->release(std::move(స్థానం)); });

    // While we hold a job slot, process tasks from the internal queue.
    అయితే (నిజం) {
      llvm::unique_function<శూన్య()> Task;
      Thread_Pool_Task_సమూహం *సమూహం_యొక్క_Task = శూన్యసూచిక;

      {
        std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);

        // Wait until a task is available or the pool is shutting down.
        వరుస_Condition.wait(తాళం_Guard,
                            [&] { తిరిగి !EnableFlag || !Tasks.ఖాళీలు(); });

        // If shutting down and the queue is empty, the thread can terminate.
        ఒకవేళ (!EnableFlag && Tasks.ఖాళీలు())
          తిరిగి;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        ఒకవేళ (Tasks.ఖాళీలు())
          ఆపు;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++చురుకైన_దారాలు;
        Task = std::move(Tasks.front().మొదటి_2);
        సమూహం_యొక్క_Task = Tasks.front().రెండవ_2;
        ఒకవేళ (సమూహం_యొక్క_Task != శూన్యసూచిక)
          ++చురుకైన_Groups[సమూహం_యొక్క_Task];
        Tasks.తీయి_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::తాళం_guard<std::mutex> తాళం_Guard(వరుస_తాళం);
        --చురుకైన_దారాలు;
        ఒకవేళ (సమూహం_యొక్క_Task != శూన్యసూచిక) {
          స్వయంచాలక A = చురుకైన_Groups.కనుగొను(సమూహం_యొక్క_Task);
          ఒకవేళ (--(A->రెండవ_2) == 0)
            చురుకైన_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        ఒకవేళ (workCompletedUnlocked(శూన్యసూచిక))
          CompletionCondition.notify_all();
      }
    }
  }
}
తార్కిక StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_సమూహం *సమూహం) స్థిరరాశి {
  ఒకవేళ (సమూహం == శూన్యసూచిక)
    తిరిగి !చురుకైన_దారాలు && Tasks.ఖాళీలు();
  తిరిగి చురుకైన_Groups.లెక్క(సమూహం) == 0 &&
         !llvm::is_contained(llvm::make_రెండవ_range(Tasks), సమూహం);
}

శూన్య StdThreadPool::wait() {
  ప్రతిజ్ఞా(!is_కార్మికుడు_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);
  CompletionCondition.wait(తాళం_Guard,
                           [&] { తిరిగి workCompletedUnlocked(శూన్యసూచిక); });
}

శూన్య StdThreadPool::wait(Thread_Pool_Task_సమూహం &సమూహం) {
  // Wait for all threads in the group to complete.
  ఒకవేళ (!is_కార్మికుడు_Thread()) {
    std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);
    CompletionCondition.wait(తాళం_Guard,
                             [&] { తిరిగి workCompletedUnlocked(&సమూహం); });
    తిరిగి;
  }
  // Make sure to not deadlock waiting for oneself.
  ప్రతిజ్ఞా(CurrentThreadTaskGroups == శూన్యసూచిక ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &సమూహం));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&సమూహం);
}

తార్కిక StdThreadPool::is_కార్మికుడు_Thread() స్థిరరాశి {
  llvm::sys::ScopedReader తాళం_Guard(దారాలు_తాళం);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  కొరకు (స్థిరరాశి llvm::thread &Thread : దారాలు)
    ఒకవేళ (CurrentThreadId == Thread.get_id())
      తిరిగి నిజం;
  తిరిగి అబద్ధం;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_తాళం<std::mutex> తాళం_Guard(వరుస_తాళం);
    EnableFlag = అబద్ధం;
  }
  వరుస_Condition.notify_all();
  llvm::sys::ScopedReader తాళం_Guard(దారాలు_తాళం);
  కొరకు (స్వయంచాలక &కార్మికుడు : దారాలు)
    కార్మికుడు.join();
}

#అయితే_ముగింపు // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  పూర్ణాంకం Thread_లెక్క = S.compute_thread_లెక్క();
  ఒకవేళ (Thread_లెక్క != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_లెక్క
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

శూన్య SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  అయితే (!Tasks.ఖాళీలు()) {
    స్వయంచాలక Task = std::move(Tasks.front().మొదటి_2);
    Tasks.తీయి_front();
    Task();
  }
}

శూన్య SingleThreadExecutor::wait(Thread_Pool_Task_సమూహం &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

తార్కిక SingleThreadExecutor::is_కార్మికుడు_Thread() స్థిరరాశి {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
