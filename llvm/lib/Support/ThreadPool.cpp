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

#شامل "llvm/Support/ThreadPool.h"

#شامل "llvm/Config/llvm-config.h"

#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/Support/ExponentialBackoff.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/Threading.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;

ThreadPoolInterface::~ThreadPoolInterface() = طےشدہ;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#اگر LLVM_ENABLE_دھاگے

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_گنتی(S.compute_thread_گنتی()) {
  اگر (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

خالی StdThreadPool::grow(صحیح requested) {
  llvm::sys::ScopedWriter تالا_Guard(دھاگے_تالا);
  اگر (دھاگے.سائز_2() >= Max_Thread_گنتی)
    واپس; // Already hit the max thread pool size.
  صحیح new_Thread_گنتی = std::min<صحیح>(requested, Max_Thread_گنتی);
  جب (static_cast<صحیح>(دھاگے.سائز_2()) < new_Thread_گنتی) {
    صحیح ThreadID = دھاگے.سائز_2();
    دھاگے.emplace_عقب([یہ, ThreadID] {
      سیٹ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      اگر (TheJobserver)
        processTasksWithJobserver();
      ورنہ
        processTasks(خالی_اشاریہ);
    });
  }
}

#اگر_غیر_تعریف_شدہ NDEBUG
// The group of the tasks run by the current thread.
ساکن LLVM_THREAD_LOCAL std::سمتیہ<Thread_Pool_Task_گروہ *>
    *CurrentThreadTaskGroups = خالی_اشاریہ;
#اگر_ختم

// WaitingForGroup == nullptr means all tasks regardless of their group.
خالی StdThreadPool::processTasks(Thread_Pool_Task_گروہ *انتظار_For_گروہ) {
  جب (سچ) {
    llvm::unique_function<خالی()> Task;
    Thread_Pool_Task_گروہ *گروہ_کا_Task;
    {
      std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);
      منطقی work_Completed_For_گروہ = جھوٹ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      قطار_Condition.wait(تالا_Guard, [&] {
        واپس !EnableFlag || !Tasks.خالی_خانے() ||
               (انتظار_For_گروہ != خالی_اشاریہ &&
                (work_Completed_For_گروہ =
                     workCompletedUnlocked(انتظار_For_گروہ)));
      });
      // Exit condition
      اگر (!EnableFlag && Tasks.خالی_خانے())
        واپس;
      اگر (انتظار_For_گروہ != خالی_اشاریہ && work_Completed_For_گروہ)
        واپس;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++فعال_دھاگے;
      Task = std::move(Tasks.front().پہلا_2);
      گروہ_کا_Task = Tasks.front().دوسرا_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      اگر (گروہ_کا_Task != خالی_اشاریہ)
        ++فعال_Groups[گروہ_کا_Task]; // Increment or set to 1 if new item
      Tasks.نکالو_front();
    }
#اگر_غیر_تعریف_شدہ NDEBUG
    اگر (CurrentThreadTaskGroups == خالی_اشاریہ)
      CurrentThreadTaskGroups = نیا std::سمتیہ<Thread_Pool_Task_گروہ *>;
    CurrentThreadTaskGroups->ڈالو_عقب(گروہ_کا_Task);
#اگر_ختم

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#اگر_غیر_تعریف_شدہ NDEBUG
    CurrentThreadTaskGroups->نکالو_عقب();
    اگر (CurrentThreadTaskGroups->خالی_خانے()) {
      حذف CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = خالی_اشاریہ;
    }
#اگر_ختم

    منطقی Notify;
    منطقی Notify_گروہ;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::تالا_guard<std::mutex> تالا_Guard(قطار_تالا);
      --فعال_دھاگے;
      اگر (گروہ_کا_Task != خالی_اشاریہ) {
        خودکار A = فعال_Groups.ڈھونڈو(گروہ_کا_Task);
        اگر (--(A->دوسرا_2) == 0)
          فعال_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(گروہ_کا_Task);
      Notify_گروہ = گروہ_کا_Task != خالی_اشاریہ && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    اگر (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    اگر (Notify_گروہ)
      قطار_Condition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
خالی StdThreadPool::processTasksWithJobserver() {
  جب (سچ) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    کام_خانہ خانہ;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    قوت_نمائی_Backoff Backoff(std::chrono::hours(24));
    منطقی AcquiredToken = جھوٹ;
    کرو {
      // Return if the thread pool is shutting down.
      {
        std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);
        اگر (!EnableFlag)
          واپس;
      }

      خانہ = TheJobserver->tryAcquire();
      اگر (خانہ.isValid()) {
        AcquiredToken = سچ;
        توڑو;
      }
    } جب (Backoff.wait_For_اگلا_Attempt());

    اگر (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit خانہ_Releaser(
        [&] { TheJobserver->release(std::move(خانہ)); });

    // While we hold a job slot, process tasks from the internal queue.
    جب (سچ) {
      llvm::unique_function<خالی()> Task;
      Thread_Pool_Task_گروہ *گروہ_کا_Task = خالی_اشاریہ;

      {
        std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);

        // Wait until a task is available or the pool is shutting down.
        قطار_Condition.wait(تالا_Guard,
                            [&] { واپس !EnableFlag || !Tasks.خالی_خانے(); });

        // If shutting down and the queue is empty, the thread can terminate.
        اگر (!EnableFlag && Tasks.خالی_خانے())
          واپس;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        اگر (Tasks.خالی_خانے())
          توڑو;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++فعال_دھاگے;
        Task = std::move(Tasks.front().پہلا_2);
        گروہ_کا_Task = Tasks.front().دوسرا_2;
        اگر (گروہ_کا_Task != خالی_اشاریہ)
          ++فعال_Groups[گروہ_کا_Task];
        Tasks.نکالو_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::تالا_guard<std::mutex> تالا_Guard(قطار_تالا);
        --فعال_دھاگے;
        اگر (گروہ_کا_Task != خالی_اشاریہ) {
          خودکار A = فعال_Groups.ڈھونڈو(گروہ_کا_Task);
          اگر (--(A->دوسرا_2) == 0)
            فعال_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        اگر (workCompletedUnlocked(خالی_اشاریہ))
          CompletionCondition.notify_all();
      }
    }
  }
}
منطقی StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_گروہ *گروہ) ثابت {
  اگر (گروہ == خالی_اشاریہ)
    واپس !فعال_دھاگے && Tasks.خالی_خانے();
  واپس فعال_Groups.گنتی(گروہ) == 0 &&
         !llvm::is_contained(llvm::make_دوسرا_range(Tasks), گروہ);
}

خالی StdThreadPool::wait() {
  أكد(!is_کارکن_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);
  CompletionCondition.wait(تالا_Guard,
                           [&] { واپس workCompletedUnlocked(خالی_اشاریہ); });
}

خالی StdThreadPool::wait(Thread_Pool_Task_گروہ &گروہ) {
  // Wait for all threads in the group to complete.
  اگر (!is_کارکن_Thread()) {
    std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);
    CompletionCondition.wait(تالا_Guard,
                             [&] { واپس workCompletedUnlocked(&گروہ); });
    واپس;
  }
  // Make sure to not deadlock waiting for oneself.
  أكد(CurrentThreadTaskGroups == خالی_اشاریہ ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &گروہ));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&گروہ);
}

منطقی StdThreadPool::is_کارکن_Thread() ثابت {
  llvm::sys::ScopedReader تالا_Guard(دھاگے_تالا);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  برائے (ثابت llvm::thread &Thread : دھاگے)
    اگر (CurrentThreadId == Thread.get_id())
      واپس سچ;
  واپس جھوٹ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_تالا<std::mutex> تالا_Guard(قطار_تالا);
    EnableFlag = جھوٹ;
  }
  قطار_Condition.notify_all();
  llvm::sys::ScopedReader تالا_Guard(دھاگے_تالا);
  برائے (خودکار &کارکن : دھاگے)
    کارکن.join();
}

#اگر_ختم // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  صحیح Thread_گنتی = S.compute_thread_گنتی();
  اگر (Thread_گنتی != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_گنتی
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

خالی SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  جب (!Tasks.خالی_خانے()) {
    خودکار Task = std::move(Tasks.front().پہلا_2);
    Tasks.نکالو_front();
    Task();
  }
}

خالی SingleThreadExecutor::wait(Thread_Pool_Task_گروہ &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

منطقی SingleThreadExecutor::is_کارکن_Thread() ثابت {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
