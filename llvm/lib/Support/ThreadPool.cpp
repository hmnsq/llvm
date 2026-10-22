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

استعمال نطاق llvm;

ThreadPoolInterface::~ThreadPoolInterface() = افتراضي;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#إذا LLVM_ENABLE_ڌاڳا

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_ڳڻپ(S.compute_thread_ڳڻپ()) {
  إذا (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

فراغ StdThreadPool::grow(صحيح requested) {
  llvm::sys::ScopedWriter تالو_Guard(ڌاڳا_تالو);
  إذا (ڌاڳا.سائيز_2() >= Max_Thread_ڳڻپ)
    أرجع; // Already hit the max thread pool size.
  صحيح new_Thread_ڳڻپ = std::min<صحيح>(requested, Max_Thread_ڳڻپ);
  طالما (static_cast<صحيح>(ڌاڳا.سائيز_2()) < new_Thread_ڳڻپ) {
    صحيح ThreadID = ڌاڳا.سائيز_2();
    ڌاڳا.emplace_پٺ([هذا, ThreadID] {
      سيٽ_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      إذا (TheJobserver)
        processTasksWithJobserver();
      وإلا
        processTasks(خالي_اشارو);
    });
  }
}

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
// The group of the tasks run by the current thread.
ساكن LLVM_THREAD_LOCAL std::ويڪٽر<Thread_Pool_Task_گروهه *>
    *CurrentThreadTaskGroups = خالي_اشارو;
#جيڪڏهن_پورو

// WaitingForGroup == nullptr means all tasks regardless of their group.
فراغ StdThreadPool::processTasks(Thread_Pool_Task_گروهه *انتظار_For_گروهه) {
  طالما (سچ) {
    llvm::unique_function<فراغ()> Task;
    Thread_Pool_Task_گروهه *گروهه_جو_Task;
    {
      std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);
      منطقي work_Completed_For_گروهه = خطأ; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      قطار_Condition.wait(تالو_Guard, [&] {
        أرجع !EnableFlag || !Tasks.خالي_جايون() ||
               (انتظار_For_گروهه != خالي_اشارو &&
                (work_Completed_For_گروهه =
                     workCompletedUnlocked(انتظار_For_گروهه)));
      });
      // Exit condition
      إذا (!EnableFlag && Tasks.خالي_جايون())
        أرجع;
      إذا (انتظار_For_گروهه != خالي_اشارو && work_Completed_For_گروهه)
        أرجع;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++سرگرم_ڌاڳا;
      Task = std::move(Tasks.front().پهريون_2);
      گروهه_جو_Task = Tasks.front().ٻيو_2;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      إذا (گروهه_جو_Task != خالي_اشارو)
        ++سرگرم_Groups[گروهه_جو_Task]; // Increment or set to 1 if new item
      Tasks.ڪڍو_front();
    }
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
    إذا (CurrentThreadTaskGroups == خالي_اشارو)
      CurrentThreadTaskGroups = جديد std::ويڪٽر<Thread_Pool_Task_گروهه *>;
    CurrentThreadTaskGroups->ڌڪو_پٺ(گروهه_جو_Task);
#جيڪڏهن_پورو

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#جيڪڏهن_وصف_نه_ٿيل NDEBUG
    CurrentThreadTaskGroups->ڪڍو_پٺ();
    إذا (CurrentThreadTaskGroups->خالي_جايون()) {
      ميٽيو CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = خالي_اشارو;
    }
#جيڪڏهن_پورو

    منطقي Notify;
    منطقي Notify_گروهه;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::تالو_guard<std::mutex> تالو_Guard(قطار_تالو);
      --سرگرم_ڌاڳا;
      إذا (گروهه_جو_Task != خالي_اشارو) {
        خودڪار A = سرگرم_Groups.ڳوليو(گروهه_جو_Task);
        إذا (--(A->ٻيو_2) == 0)
          سرگرم_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(گروهه_جو_Task);
      Notify_گروهه = گروهه_جو_Task != خالي_اشارو && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    إذا (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    إذا (Notify_گروهه)
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
    ڪم_جڳهه جڳهه;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    قوت_نمائي_Backoff Backoff(std::chrono::hours(24));
    منطقي AcquiredToken = خطأ;
    افعل {
      // Return if the thread pool is shutting down.
      {
        std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);
        إذا (!EnableFlag)
          أرجع;
      }

      جڳهه = TheJobserver->tryAcquire();
      إذا (جڳهه.isValid()) {
        AcquiredToken = سچ;
        اكسر;
      }
    } طالما (Backoff.wait_For_ايندڙ_Attempt());

    إذا (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit جڳهه_Releaser(
        [&] { TheJobserver->release(std::move(جڳهه)); });

    // While we hold a job slot, process tasks from the internal queue.
    طالما (سچ) {
      llvm::unique_function<فراغ()> Task;
      Thread_Pool_Task_گروهه *گروهه_جو_Task = خالي_اشارو;

      {
        std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);

        // Wait until a task is available or the pool is shutting down.
        قطار_Condition.wait(تالو_Guard,
                            [&] { أرجع !EnableFlag || !Tasks.خالي_جايون(); });

        // If shutting down and the queue is empty, the thread can terminate.
        إذا (!EnableFlag && Tasks.خالي_جايون())
          أرجع;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        إذا (Tasks.خالي_جايون())
          اكسر;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++سرگرم_ڌاڳا;
        Task = std::move(Tasks.front().پهريون_2);
        گروهه_جو_Task = Tasks.front().ٻيو_2;
        إذا (گروهه_جو_Task != خالي_اشارو)
          ++سرگرم_Groups[گروهه_جو_Task];
        Tasks.ڪڍو_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::تالو_guard<std::mutex> تالو_Guard(قطار_تالو);
        --سرگرم_ڌاڳا;
        إذا (گروهه_جو_Task != خالي_اشارو) {
          خودڪار A = سرگرم_Groups.ڳوليو(گروهه_جو_Task);
          إذا (--(A->ٻيو_2) == 0)
            سرگرم_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        إذا (workCompletedUnlocked(خالي_اشارو))
          CompletionCondition.notify_all();
      }
    }
  }
}
منطقي StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_گروهه *گروهه) ثابت {
  إذا (گروهه == خالي_اشارو)
    أرجع !سرگرم_ڌاڳا && Tasks.خالي_جايون();
  أرجع سرگرم_Groups.ڳڻپ(گروهه) == 0 &&
         !llvm::is_contained(llvm::make_ٻيو_range(Tasks), گروهه);
}

فراغ StdThreadPool::wait() {
  أكد(!is_ڪم_ڪندڙ_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);
  CompletionCondition.wait(تالو_Guard,
                           [&] { أرجع workCompletedUnlocked(خالي_اشارو); });
}

فراغ StdThreadPool::wait(Thread_Pool_Task_گروهه &گروهه) {
  // Wait for all threads in the group to complete.
  إذا (!is_ڪم_ڪندڙ_Thread()) {
    std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);
    CompletionCondition.wait(تالو_Guard,
                             [&] { أرجع workCompletedUnlocked(&گروهه); });
    أرجع;
  }
  // Make sure to not deadlock waiting for oneself.
  أكد(CurrentThreadTaskGroups == خالي_اشارو ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &گروهه));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&گروهه);
}

منطقي StdThreadPool::is_ڪم_ڪندڙ_Thread() ثابت {
  llvm::sys::ScopedReader تالو_Guard(ڌاڳا_تالو);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  لكل (ثابت llvm::thread &Thread : ڌاڳا)
    إذا (CurrentThreadId == Thread.get_id())
      أرجع سچ;
  أرجع خطأ;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_تالو<std::mutex> تالو_Guard(قطار_تالو);
    EnableFlag = خطأ;
  }
  قطار_Condition.notify_all();
  llvm::sys::ScopedReader تالو_Guard(ڌاڳا_تالو);
  لكل (خودڪار &ڪم_ڪندڙ : ڌاڳا)
    ڪم_ڪندڙ.join();
}

#جيڪڏهن_پورو // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  صحيح Thread_ڳڻپ = S.compute_thread_ڳڻپ();
  إذا (Thread_ڳڻپ != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_ڳڻپ
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

فراغ SingleThreadExecutor::wait() {
  // Sequential implementation running the tasks
  طالما (!Tasks.خالي_جايون()) {
    خودڪار Task = std::move(Tasks.front().پهريون_2);
    Tasks.ڪڍو_front();
    Task();
  }
}

فراغ SingleThreadExecutor::wait(Thread_Pool_Task_گروهه &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  wait();
}

منطقي SingleThreadExecutor::is_ڪم_ڪندڙ_Thread() ثابت {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { wait(); }
