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

ورتوں نطاق llvm;

ThreadPoolInterface::~ThreadPoolInterface() = افتراضي;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#إذا LLVM_ENABLE_دھاگے

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_گنتی(S.compute_thread_گنتی()) {
  إذا (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

فراغ StdThreadPool::grow(صحيح requested) {
  llvm::sys::ScopedWriter جندرا_Guard(دھاگے_جندرا);
  إذا (دھاگے.ناپ_2() >= Max_Thread_گنتی)
    أرجع; // Already hit the max thread pool size.
  صحيح new_Thread_گنتی = std::min<صحيح>(requested, Max_Thread_گنتی);
  طالما (static_cast<صحيح>(دھاگے.ناپ_2()) < new_Thread_گنتی) {
    صحيح ThreadID = دھاگے.ناپ_2();
    دھاگے.emplace_پٹھ([هذا, ThreadID] {
      سیٹ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      إذا (TheJobserver)
        processTasksWithJobserver();
      وإلا
        processTasks(خالی_سوچک);
    });
  }
}

#جے_بنا_تعریف NDEBUG
// The group of the tasks run by the current thread.
ساكن LLVM_THREAD_LOCAL std::سمتیہ<Thread_Pool_Task_گروہ *>
    *CurrentThreadTaskGroups = خالی_سوچک;
#جے_انت

// WaitingForGroup == nullptr means all tasks regardless of their group.
فراغ StdThreadPool::processTasks(Thread_Pool_Task_گروہ *اڈیک_For_گروہ) {
  طالما (سچ) {
    llvm::unique_function<فراغ()> Task;
    Thread_Pool_Task_گروہ *گروہ_دا_Task;
    {
      std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);
      منطقي work_Completed_For_گروہ = خطأ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      قطار_Condition.wait(جندرا_Guard, [&] {
        أرجع !EnableFlag || !Tasks.خالی_تھاواں() ||
               (اڈیک_For_گروہ != خالی_سوچک &&
                (work_Completed_For_گروہ =
                     workCompletedUnlocked(اڈیک_For_گروہ)));
      });
      // Exit condition
      إذا (!EnableFlag && Tasks.خالی_تھاواں())
        أرجع;
      إذا (اڈیک_For_گروہ != خالی_سوچک && work_Completed_For_گروہ)
        أرجع;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++سرگرم_دھاگے;
      Task = std::move(Tasks.front().پہلا_2);
      گروہ_دا_Task = Tasks.front().دوجا_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      إذا (گروہ_دا_Task != خالی_سوچک)
        ++سرگرم_Groups[گروہ_دا_Task]; // Increment or set to 1 if new item
      Tasks.نکالو_front();
    }
#جے_بنا_تعریف NDEBUG
    إذا (CurrentThreadTaskGroups == خالی_سوچک)
      CurrentThreadTaskGroups = جديد std::سمتیہ<Thread_Pool_Task_گروہ *>;
    CurrentThreadTaskGroups->ڈالو_پٹھ(گروہ_دا_Task);
#جے_انت

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#جے_بنا_تعریف NDEBUG
    CurrentThreadTaskGroups->نکالو_پٹھ();
    إذا (CurrentThreadTaskGroups->خالی_تھاواں()) {
      مٹاؤ CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = خالی_سوچک;
    }
#جے_انت

    منطقي Notify;
    منطقي Notify_گروہ;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::جندرا_guard<std::mutex> جندرا_Guard(قطار_جندرا);
      --سرگرم_دھاگے;
      إذا (گروہ_دا_Task != خالی_سوچک) {
        خودکار A = سرگرم_Groups.لبھو(گروہ_دا_Task);
        إذا (--(A->دوجا_2) == 0)
          سرگرم_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(گروہ_دا_Task);
      Notify_گروہ = گروہ_دا_Task != خالی_سوچک && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    إذا (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    إذا (Notify_گروہ)
      قطار_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
فراغ StdThreadPool::processTasksWithJobserver() {
  طالما (سچ) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    کم_تھاں تھاں;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    قوت_نمائی_Backoff Backoff(std::chrono::hours(24));
    منطقي AcquiredToken = خطأ;
    افعل {
      // Return if the thread pool is shutting down.
      {
        std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);
        إذا (!EnableFlag)
          أرجع;
      }

      تھاں = TheJobserver->tryAcquire();
      إذا (تھاں.isValid()) {
        AcquiredToken = سچ;
        اكسر;
      }
    } طالما (Backoff.wait_For_اگلا_Attempt());

    إذا (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit تھاں_Releaser(
        [&] { TheJobserver->release(std::move(تھاں)); });

    // While we hold a job slot, process tasks from the internal queue.
    طالما (سچ) {
      llvm::unique_function<فراغ()> Task;
      Thread_Pool_Task_گروہ *گروہ_دا_Task = خالی_سوچک;

      {
        std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);

        // Wait until a task is available or the pool is shutting down.
        قطار_Condition.wait(جندرا_Guard,
                            [&] { أرجع !EnableFlag || !Tasks.خالی_تھاواں(); });

        // If shutting down and the queue is empty, the thread can terminate.
        إذا (!EnableFlag && Tasks.خالی_تھاواں())
          أرجع;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        إذا (Tasks.خالی_تھاواں())
          اكسر;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++سرگرم_دھاگے;
        Task = std::move(Tasks.front().پہلا_2);
        گروہ_دا_Task = Tasks.front().دوجا_2;
        إذا (گروہ_دا_Task != خالی_سوچک)
          ++سرگرم_Groups[گروہ_دا_Task];
        Tasks.نکالو_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::جندرا_guard<std::mutex> جندرا_Guard(قطار_جندرا);
        --سرگرم_دھاگے;
        إذا (گروہ_دا_Task != خالی_سوچک) {
          خودکار A = سرگرم_Groups.لبھو(گروہ_دا_Task);
          إذا (--(A->دوجا_2) == 0)
            سرگرم_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        إذا (workCompletedUnlocked(خالی_سوچک))
          CompletionCondition.notify_all();
      }
    }
  }
}
منطقي StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_گروہ *گروہ) ثابت {
  إذا (گروہ == خالی_سوچک)
    أرجع !سرگرم_دھاگے && Tasks.خالی_تھاواں();
  أرجع سرگرم_Groups.گنتی(گروہ) == 0 &&
         !llvm::is_contained(llvm::make_دوجا_range(Tasks), گروہ);
}

فراغ StdThreadPool::wait() {
  أكد(!is_کامہ_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);
  CompletionCondition.wait(جندرا_Guard,
                           [&] { أرجع workCompletedUnlocked(خالی_سوچک); });
}

فراغ StdThreadPool::wait(Thread_Pool_Task_گروہ &گروہ) {
  // Wait for all threads in the group to complete.
  إذا (!is_کامہ_Thread()) {
    std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);
    CompletionCondition.wait(جندرا_Guard,
                             [&] { أرجع workCompletedUnlocked(&گروہ); });
    أرجع;
  }
  // Make sure to not deadlock waiting for oneself.
  أكد(CurrentThreadTaskGroups == خالی_سوچک ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &گروہ));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&گروہ);
}

منطقي StdThreadPool::is_کامہ_Thread() ثابت {
  llvm::sys::ScopedReader جندرا_Guard(دھاگے_جندرا);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  لكل (ثابت llvm::thread &Thread : دھاگے)
    إذا (CurrentThreadId == Thread.get_id())
      أرجع سچ;
  أرجع خطأ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_جندرا<std::mutex> جندرا_Guard(قطار_جندرا);
    EnableFlag = خطأ;
  }
  قطار_Condition.notify_all();
  llvm::sys::ScopedReader جندرا_Guard(دھاگے_جندرا);
  لكل (خودکار &کامہ : دھاگے)
    کامہ.join();
}

#جے_انت // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  صحيح Thread_گنتی = S.compute_thread_گنتی();
  إذا (Thread_گنتی != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_گنتی
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

فراغ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  طالما (!Tasks.خالی_تھاواں()) {
    خودکار Task = std::move(Tasks.front().پہلا_2);
    Tasks.نکالو_front();
    Task();
  }
}

فراغ SingleThreadExecutor::wait(Thread_Pool_Task_گروہ &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

منطقي SingleThreadExecutor::is_کامہ_Thread() ثابت {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
