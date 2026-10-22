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

کارول نطاق llvm;

ThreadPoolInterface::~ThreadPoolInterface() = افتراضي;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#إذا LLVM_ENABLE_تارونه

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_شمېر(S.compute_thread_شمېر()) {
  إذا (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

فراغ StdThreadPool::grow(صحيح requested) {
  llvm::sys::ScopedWriter قلف_Guard(تارونه_قلف);
  إذا (تارونه.اندازه_2() >= Max_Thread_شمېر)
    أرجع; // Already hit the max thread pool size.
  صحيح new_Thread_شمېر = std::min<صحيح>(requested, Max_Thread_شمېر);
  طالما (static_cast<صحيح>(تارونه.اندازه_2()) < new_Thread_شمېر) {
    صحيح ThreadID = تارونه.اندازه_2();
    تارونه.emplace_شا([هذا, ThreadID] {
      سېټ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      إذا (TheJobserver)
        processTasksWithJobserver();
      وإلا
        processTasks(تش_اشاره);
    });
  }
}

#که_نه_تعریف_شوی NDEBUG
// The group of the tasks run by the current thread.
ساكن LLVM_THREAD_LOCAL std::ویکټور<Thread_Pool_Task_ډله *>
    *CurrentThreadTaskGroups = تش_اشاره;
#د_که_پای

// WaitingForGroup == nullptr means all tasks regardless of their group.
فراغ StdThreadPool::processTasks(Thread_Pool_Task_ډله *انتظار_For_ډله) {
  طالما (ریښتیا) {
    llvm::unique_function<فراغ()> Task;
    Thread_Pool_Task_ډله *ډله_د_Task;
    {
      std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);
      منطقي work_Completed_For_ډله = خطأ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      کتار_Condition.wait(قلف_Guard, [&] {
        أرجع !EnableFlag || !Tasks.تشه() ||
               (انتظار_For_ډله != تش_اشاره &&
                (work_Completed_For_ډله =
                     workCompletedUnlocked(انتظار_For_ډله)));
      });
      // Exit condition
      إذا (!EnableFlag && Tasks.تشه())
        أرجع;
      إذا (انتظار_For_ډله != تش_اشاره && work_Completed_For_ډله)
        أرجع;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++فعال_تارونه;
      Task = std::move(Tasks.front().لومړی_2);
      ډله_د_Task = Tasks.front().دوهم_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      إذا (ډله_د_Task != تش_اشاره)
        ++فعال_Groups[ډله_د_Task]; // Increment or set to 1 if new item
      Tasks.راوباسه_front();
    }
#که_نه_تعریف_شوی NDEBUG
    إذا (CurrentThreadTaskGroups == تش_اشاره)
      CurrentThreadTaskGroups = جديد std::ویکټور<Thread_Pool_Task_ډله *>;
    CurrentThreadTaskGroups->ټېل_شا(ډله_د_Task);
#د_که_پای

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#که_نه_تعریف_شوی NDEBUG
    CurrentThreadTaskGroups->راوباسه_شا();
    إذا (CurrentThreadTaskGroups->تشه()) {
      ړنګول CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = تش_اشاره;
    }
#د_که_پای

    منطقي Notify;
    منطقي Notify_ډله;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::قلف_guard<std::mutex> قلف_Guard(کتار_قلف);
      --فعال_تارونه;
      إذا (ډله_د_Task != تش_اشاره) {
        خپلکاری A = فعال_Groups.پیدا_کړه(ډله_د_Task);
        إذا (--(A->دوهم_2) == 0)
          فعال_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(ډله_د_Task);
      Notify_ډله = ډله_د_Task != تش_اشاره && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    إذا (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    إذا (Notify_ډله)
      کتار_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
فراغ StdThreadPool::processTasksWithJobserver() {
  طالما (ریښتیا) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    دنده_خونه خونه;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    توانیز_Backoff Backoff(std::chrono::hours(24));
    منطقي AcquiredToken = خطأ;
    افعل {
      // Return if the thread pool is shutting down.
      {
        std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);
        إذا (!EnableFlag)
          أرجع;
      }

      خونه = TheJobserver->tryAcquire();
      إذا (خونه.isValid()) {
        AcquiredToken = ریښتیا;
        اكسر;
      }
    } طالما (Backoff.wait_For_بل_Attempt());

    إذا (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit خونه_Releaser(
        [&] { TheJobserver->release(std::move(خونه)); });

    // While we hold a job slot, process tasks from the internal queue.
    طالما (ریښتیا) {
      llvm::unique_function<فراغ()> Task;
      Thread_Pool_Task_ډله *ډله_د_Task = تش_اشاره;

      {
        std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);

        // Wait until a task is available or the pool is shutting down.
        کتار_Condition.wait(قلف_Guard,
                            [&] { أرجع !EnableFlag || !Tasks.تشه(); });

        // If shutting down and the queue is empty, the thread can terminate.
        إذا (!EnableFlag && Tasks.تشه())
          أرجع;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        إذا (Tasks.تشه())
          اكسر;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++فعال_تارونه;
        Task = std::move(Tasks.front().لومړی_2);
        ډله_د_Task = Tasks.front().دوهم_2;
        إذا (ډله_د_Task != تش_اشاره)
          ++فعال_Groups[ډله_د_Task];
        Tasks.راوباسه_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::قلف_guard<std::mutex> قلف_Guard(کتار_قلف);
        --فعال_تارونه;
        إذا (ډله_د_Task != تش_اشاره) {
          خپلکاری A = فعال_Groups.پیدا_کړه(ډله_د_Task);
          إذا (--(A->دوهم_2) == 0)
            فعال_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        إذا (workCompletedUnlocked(تش_اشاره))
          CompletionCondition.notify_all();
      }
    }
  }
}
منطقي StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_ډله *ډله) ثابت {
  إذا (ډله == تش_اشاره)
    أرجع !فعال_تارونه && Tasks.تشه();
  أرجع فعال_Groups.شمېر(ډله) == 0 &&
         !llvm::is_contained(llvm::make_دوهم_range(Tasks), ډله);
}

فراغ StdThreadPool::wait() {
  أكد(!is_کارګر_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);
  CompletionCondition.wait(قلف_Guard,
                           [&] { أرجع workCompletedUnlocked(تش_اشاره); });
}

فراغ StdThreadPool::wait(Thread_Pool_Task_ډله &ډله) {
  // Wait for all threads in the group to complete.
  إذا (!is_کارګر_Thread()) {
    std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);
    CompletionCondition.wait(قلف_Guard,
                             [&] { أرجع workCompletedUnlocked(&ډله); });
    أرجع;
  }
  // Make sure to not deadlock waiting for oneself.
  أكد(CurrentThreadTaskGroups == تش_اشاره ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &ډله));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&ډله);
}

منطقي StdThreadPool::is_کارګر_Thread() ثابت {
  llvm::sys::ScopedReader قلف_Guard(تارونه_قلف);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  لكل (ثابت llvm::thread &Thread : تارونه)
    إذا (CurrentThreadId == Thread.get_id())
      أرجع ریښتیا;
  أرجع خطأ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_قلف<std::mutex> قلف_Guard(کتار_قلف);
    EnableFlag = خطأ;
  }
  کتار_Condition.notify_all();
  llvm::sys::ScopedReader قلف_Guard(تارونه_قلف);
  لكل (خپلکاری &کارګر : تارونه)
    کارګر.join();
}

#د_که_پای // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  صحيح Thread_شمېر = S.compute_thread_شمېر();
  إذا (Thread_شمېر != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_شمېر
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

فراغ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  طالما (!Tasks.تشه()) {
    خپلکاری Task = std::move(Tasks.front().لومړی_2);
    Tasks.راوباسه_front();
    Task();
  }
}

فراغ SingleThreadExecutor::wait(Thread_Pool_Task_ډله &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

منطقي SingleThreadExecutor::is_کارګر_Thread() ثابت {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
