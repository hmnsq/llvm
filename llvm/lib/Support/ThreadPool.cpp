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

#ضمن "llvm/Support/ThreadPool.h"

#ضمن "llvm/Config/llvm-config.h"

#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Support/ExponentialBackoff.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/Threading.h"
#ضمن "llvm/Support/raw_ostream.h"

استفاده نطاق llvm;

ThreadPoolInterface::~ThreadPoolInterface() = افتراضي;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#إذا LLVM_ENABLE_نخها

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_شمار(S.compute_thread_شمار()) {
  إذا (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

فراغ StdThreadPool::grow(صحيح requested) {
  llvm::sys::ScopedWriter قفل_Guard(نخها_قفل);
  إذا (نخها.اندازه_2() >= Max_Thread_شمار)
    أرجع; // Already hit the max thread pool size.
  صحيح new_Thread_شمار = std::min<صحيح>(requested, Max_Thread_شمار);
  طالما (static_cast<صحيح>(نخها.اندازه_2()) < new_Thread_شمار) {
    صحيح ThreadID = نخها.اندازه_2();
    نخها.emplace_پشت([هذا, ThreadID] {
      مجموعه_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      إذا (TheJobserver)
        processTasksWithJobserver();
      وإلا
        processTasks(اشاره_تهی);
    });
  }
}

#اگر_تعریف_نشده NDEBUG
// The group of the tasks run by the current thread.
ساكن LLVM_THREAD_LOCAL std::وکتور<Thread_Pool_Task_گروه *>
    *CurrentThreadTaskGroups = اشاره_تهی;
#پایان_اگر

// WaitingForGroup == nullptr means all tasks regardless of their group.
فراغ StdThreadPool::processTasks(Thread_Pool_Task_گروه *انتظار_For_گروه) {
  طالما (درست) {
    llvm::unique_function<فراغ()> Task;
    Thread_Pool_Task_گروه *گروه_از_Task;
    {
      std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);
      منطقي work_Completed_For_گروه = خطأ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      صف_Condition.wait(قفل_Guard, [&] {
        أرجع !EnableFlag || !Tasks.خالی() ||
               (انتظار_For_گروه != اشاره_تهی &&
                (work_Completed_For_گروه =
                     workCompletedUnlocked(انتظار_For_گروه)));
      });
      // Exit condition
      إذا (!EnableFlag && Tasks.خالی())
        أرجع;
      إذا (انتظار_For_گروه != اشاره_تهی && work_Completed_For_گروه)
        أرجع;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++فعال_نخها;
      Task = std::move(Tasks.front().اولی_2);
      گروه_از_Task = Tasks.front().دومی_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      إذا (گروه_از_Task != اشاره_تهی)
        ++فعال_Groups[گروه_از_Task]; // Increment or set to 1 if new item
      Tasks.برداشت_front();
    }
#اگر_تعریف_نشده NDEBUG
    إذا (CurrentThreadTaskGroups == اشاره_تهی)
      CurrentThreadTaskGroups = جديد std::وکتور<Thread_Pool_Task_گروه *>;
    CurrentThreadTaskGroups->افزودن_پشت(گروه_از_Task);
#پایان_اگر

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#اگر_تعریف_نشده NDEBUG
    CurrentThreadTaskGroups->برداشت_پشت();
    إذا (CurrentThreadTaskGroups->خالی()) {
      حذف CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = اشاره_تهی;
    }
#پایان_اگر

    منطقي Notify;
    منطقي Notify_گروه;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::قفل_guard<std::mutex> قفل_Guard(صف_قفل);
      --فعال_نخها;
      إذا (گروه_از_Task != اشاره_تهی) {
        خودکار A = فعال_Groups.پیدا_کن(گروه_از_Task);
        إذا (--(A->دومی_2) == 0)
          فعال_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(گروه_از_Task);
      Notify_گروه = گروه_از_Task != اشاره_تهی && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    إذا (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    إذا (Notify_گروه)
      صف_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
فراغ StdThreadPool::processTasksWithJobserver() {
  طالما (درست) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    کار_جایگاه جایگاه;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    نمایی_Backoff Backoff(std::chrono::hours(24));
    منطقي AcquiredToken = خطأ;
    افعل {
      // Return if the thread pool is shutting down.
      {
        std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);
        إذا (!EnableFlag)
          أرجع;
      }

      جایگاه = TheJobserver->tryAcquire();
      إذا (جایگاه.isValid()) {
        AcquiredToken = درست;
        اكسر;
      }
    } طالما (Backoff.wait_For_بعدی_Attempt());

    إذا (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit جایگاه_Releaser(
        [&] { TheJobserver->release(std::move(جایگاه)); });

    // While we hold a job slot, process tasks from the internal queue.
    طالما (درست) {
      llvm::unique_function<فراغ()> Task;
      Thread_Pool_Task_گروه *گروه_از_Task = اشاره_تهی;

      {
        std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);

        // Wait until a task is available or the pool is shutting down.
        صف_Condition.wait(قفل_Guard,
                            [&] { أرجع !EnableFlag || !Tasks.خالی(); });

        // If shutting down and the queue is empty, the thread can terminate.
        إذا (!EnableFlag && Tasks.خالی())
          أرجع;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        إذا (Tasks.خالی())
          اكسر;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++فعال_نخها;
        Task = std::move(Tasks.front().اولی_2);
        گروه_از_Task = Tasks.front().دومی_2;
        إذا (گروه_از_Task != اشاره_تهی)
          ++فعال_Groups[گروه_از_Task];
        Tasks.برداشت_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::قفل_guard<std::mutex> قفل_Guard(صف_قفل);
        --فعال_نخها;
        إذا (گروه_از_Task != اشاره_تهی) {
          خودکار A = فعال_Groups.پیدا_کن(گروه_از_Task);
          إذا (--(A->دومی_2) == 0)
            فعال_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        إذا (workCompletedUnlocked(اشاره_تهی))
          CompletionCondition.notify_all();
      }
    }
  }
}
منطقي StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_گروه *گروه) ثابت {
  إذا (گروه == اشاره_تهی)
    أرجع !فعال_نخها && Tasks.خالی();
  أرجع فعال_Groups.شمار(گروه) == 0 &&
         !llvm::is_contained(llvm::make_دومی_range(Tasks), گروه);
}

فراغ StdThreadPool::wait() {
  أكد(!is_کارگر_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);
  CompletionCondition.wait(قفل_Guard,
                           [&] { أرجع workCompletedUnlocked(اشاره_تهی); });
}

فراغ StdThreadPool::wait(Thread_Pool_Task_گروه &گروه) {
  // Wait for all threads in the group to complete.
  إذا (!is_کارگر_Thread()) {
    std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);
    CompletionCondition.wait(قفل_Guard,
                             [&] { أرجع workCompletedUnlocked(&گروه); });
    أرجع;
  }
  // Make sure to not deadlock waiting for oneself.
  أكد(CurrentThreadTaskGroups == اشاره_تهی ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &گروه));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&گروه);
}

منطقي StdThreadPool::is_کارگر_Thread() ثابت {
  llvm::sys::ScopedReader قفل_Guard(نخها_قفل);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  لكل (ثابت llvm::thread &Thread : نخها)
    إذا (CurrentThreadId == Thread.get_id())
      أرجع درست;
  أرجع خطأ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_قفل<std::mutex> قفل_Guard(صف_قفل);
    EnableFlag = خطأ;
  }
  صف_Condition.notify_all();
  llvm::sys::ScopedReader قفل_Guard(نخها_قفل);
  لكل (خودکار &کارگر : نخها)
    کارگر.join();
}

#پایان_اگر // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  صحيح Thread_شمار = S.compute_thread_شمار();
  إذا (Thread_شمار != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_شمار
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

فراغ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  طالما (!Tasks.خالی()) {
    خودکار Task = std::move(Tasks.front().اولی_2);
    Tasks.برداشت_front();
    Task();
  }
}

فراغ SingleThreadExecutor::wait(Thread_Pool_Task_گروه &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

منطقي SingleThreadExecutor::is_کارگر_Thread() ثابت {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
