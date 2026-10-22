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

#अन्तर्भाव "llvm/Support/ThreadPool.h"

#अन्तर्भाव "llvm/Config/llvm-config.h"

#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Support/ExponentialBackoff.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/Threading.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

उपयुज्य नामाकाश llvm;

ThreadPoolInterface::~ThreadPoolInterface() = स्वभाव;

// A note on thread groups: Tasks are by default in no group (represented
// by nullptr ThreadPoolTaskGroup pointer in the Tasks queue) and functionality
// here normally works on all tasks regardless of their group (functions
// in that case receive nullptr ThreadPoolTaskGroup pointer as argument).
// A task in a group has a pointer to that ThreadPoolTaskGroup in the Tasks
// queue, and functions called to work only on tasks from one group take that
// pointer.

#यदि LLVM_ENABLE_तन्तवः

StdThreadPool::StdThreadPool(ThreadPoolStrategy S)
    : Strategy(S), Max_Thread_गणनम्(S.compute_thread_गणनम्()) {
  यदि (Strategy.UseJobserver)
    TheJobserver = JobserverClient::getInstance();
}

शून्य StdThreadPool::grow(पूर्णाङ्क requested) {
  llvm::sys::ScopedWriter तालकम्_Guard(तन्तवः_तालकम्);
  यदि (तन्तवः.आकार_2() >= Max_Thread_गणनम्)
    प्रत्यागम; // Already hit the max thread pool size.
  पूर्णाङ्क new_Thread_गणनम् = std::min<पूर्णाङ्क>(requested, Max_Thread_गणनम्);
  यावत् (static_cast<पूर्णाङ्क>(तन्तवः.आकार_2()) < new_Thread_गणनम्) {
    पूर्णाङ्क ThreadID = तन्तवः.आकार_2();
    तन्तवः.emplace_पृष्ठभागः([एतत्, ThreadID] {
      समुच्चयः_thread_name(formatv("llvm-worker-{0}", ThreadID));
      Strategy.apply_thread_strategy(ThreadID);
      // Note on jobserver deadlock avoidance:
      // GNU Make grants each invoked process one implicit job slot.
      // JobserverClient::tryAcquire() returns that implicit slot on the first
      // successful call in a process, ensuring forward progress without a
      // dedicated "always-on" thread.
      यदि (TheJobserver)
        processTasksWithJobserver();
      अन्यथा
        processTasks(शून्यसूचकः);
    });
  }
}

#यदि_अपरिभाषितम् NDEBUG
// The group of the tasks run by the current thread.
स्थिर LLVM_THREAD_LOCAL std::सदिशम्<Thread_Pool_Task_गणः *>
    *CurrentThreadTaskGroups = शून्यसूचकः;
#यदि_अन्तः

// WaitingForGroup == nullptr means all tasks regardless of their group.
शून्य StdThreadPool::processTasks(Thread_Pool_Task_गणः *प्रतीक्षा_For_गणः) {
  यावत् (सत्य) {
    llvm::unique_function<शून्य()> Task;
    Thread_Pool_Task_गणः *गणः_सम्बन्धि_Task;
    {
      std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);
      तार्किक work_Completed_For_गणः = असत्य; // Result of workCompletedUnlocked()
      // Wait for tasks to be pushed in the queue
      QueueCondition.प्रतीक्षस्व(तालकम्_Guard, [&] {
        प्रत्यागम !EnableFlag || !Tasks.रिक्तानि() ||
               (प्रतीक्षा_For_गणः != शून्यसूचकः &&
                (work_Completed_For_गणः =
                     workCompletedUnlocked(प्रतीक्षा_For_गणः)));
      });
      // Exit condition
      यदि (!EnableFlag && Tasks.रिक्तानि())
        प्रत्यागम;
      यदि (प्रतीक्षा_For_गणः != शून्यसूचकः && work_Completed_For_गणः)
        प्रत्यागम;
      // Yeah, we have a task, grab it and release the lock on the queue

      // We first need to signal that we are active before popping the queue
      // in order for wait() to properly detect that even if the queue is
      // empty, there is still a task in flight.
      ++सक्रिय_तन्तवः;
      Task = std::move(Tasks.front().first);
      गणः_सम्बन्धि_Task = Tasks.front().second;
      // Need to count active threads in each group separately, ActiveThreads
      // would never be 0 if waiting for another group inside a wait.
      यदि (गणः_सम्बन्धि_Task != शून्यसूचकः)
        ++सक्रिय_Groups[गणः_सम्बन्धि_Task]; // Increment or set to 1 if new item
      Tasks.निष्कासय_front();
    }
#यदि_अपरिभाषितम् NDEBUG
    यदि (CurrentThreadTaskGroups == शून्यसूचकः)
      CurrentThreadTaskGroups = नूतन std::सदिशम्<Thread_Pool_Task_गणः *>;
    CurrentThreadTaskGroups->प्रक्षिप_पृष्ठभागः(गणः_सम्बन्धि_Task);
#यदि_अन्तः

    // Run the task we just grabbed. This also destroys the task once run to
    // release any resources held by it through RAII captured objects.
    //
    // It is particularly important to do this here so that we're not holding
    // any lock and any further operations on the thread or `ThreadPool` take
    // place here, at the same point as the task itself is executed.
    std::exchange(Task, {})();

#यदि_अपरिभाषितम् NDEBUG
    CurrentThreadTaskGroups->निष्कासय_पृष्ठभागः();
    यदि (CurrentThreadTaskGroups->रिक्तानि()) {
      विलोपय CurrentThreadTaskGroups;
      CurrentThreadTaskGroups = शून्यसूचकः;
    }
#यदि_अन्तः

    तार्किक Notify;
    तार्किक Notify_गणः;
    {
      // Adjust `ActiveThreads`, in case someone waits on StdThreadPool::wait()
      std::तालकम्_guard<std::mutex> तालकम्_Guard(Queue_तालकम्);
      --सक्रिय_तन्तवः;
      यदि (गणः_सम्बन्धि_Task != शून्यसूचकः) {
        स्वचालित A = सक्रिय_Groups.find(गणः_सम्बन्धि_Task);
        यदि (--(A->second) == 0)
          सक्रिय_Groups.erase(A);
      }
      Notify = workCompletedUnlocked(गणः_सम्बन्धि_Task);
      Notify_गणः = गणः_सम्बन्धि_Task != शून्यसूचकः && Notify;
    }
    // Notify task completion if this is the last active thread, in case
    // someone waits on StdThreadPool::wait().
    यदि (Notify)
      CompletionCondition.notify_all();
    // If this was a task in a group, notify also threads waiting for tasks
    // in this function on QueueCondition, to make a recursive wait() return
    // after the group it's been waiting for has finished.
    यदि (Notify_गणः)
      QueueCondition.notify_all();
  }
}

/// Main loop for worker threads when using a jobserver.
/// This function uses a two-level queue; it first acquires a job slot from the
/// external jobserver, then retrieves a task from the internal queue.
/// This allows the thread pool to cooperate with build systems like `make -j`.
शून्य StdThreadPool::processTasksWithJobserver() {
  यावत् (सत्य) {
    // Acquire a job slot from the external jobserver.
    // This polls for a slot and yields the thread to avoid a high-CPU wait.
    JobSlot Slot;
    // The timeout for the backoff can be very long, as the shutdown
    // is checked on each iteration. The sleep duration is capped by MaxWait
    // in ExponentialBackoff, so shutdown latency is not a problem.
    घातीयम्_Backoff Backoff(std::chrono::hours(24));
    तार्किक AcquiredToken = असत्य;
    कुरु {
      // Return if the thread pool is shutting down.
      {
        std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);
        यदि (!EnableFlag)
          प्रत्यागम;
      }

      Slot = TheJobserver->tryAcquire();
      यदि (Slot.isValid()) {
        AcquiredToken = सत्य;
        भङ्ग;
      }
    } यावत् (Backoff.wait_For_अग्र_Attempt());

    यदि (!AcquiredToken) {
      // This is practically unreachable with a 24h timeout and indicates a
      // deeper problem if hit.
      report_fatal_error("Timed out waiting for jobserver token.");
    }

    // `llvm::scope_exit` guarantees the job slot is released, even if the
    // task throws or we exit early. This prevents deadlocking the build.
    llvm::scope_exit SlotReleaser(
        [&] { TheJobserver->release(std::move(Slot)); });

    // While we hold a job slot, process tasks from the internal queue.
    यावत् (सत्य) {
      llvm::unique_function<शून्य()> Task;
      Thread_Pool_Task_गणः *गणः_सम्बन्धि_Task = शून्यसूचकः;

      {
        std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);

        // Wait until a task is available or the pool is shutting down.
        QueueCondition.प्रतीक्षस्व(तालकम्_Guard,
                            [&] { प्रत्यागम !EnableFlag || !Tasks.रिक्तानि(); });

        // If shutting down and the queue is empty, the thread can terminate.
        यदि (!EnableFlag && Tasks.रिक्तानि())
          प्रत्यागम;

        // If the queue is empty, we're done processing tasks for now.
        // Break the inner loop to release the job slot.
        यदि (Tasks.रिक्तानि())
          भङ्ग;

        // A task is available. Mark it as active before releasing the lock
        // to prevent race conditions with `wait()`.
        ++सक्रिय_तन्तवः;
        Task = std::move(Tasks.front().first);
        गणः_सम्बन्धि_Task = Tasks.front().second;
        यदि (गणः_सम्बन्धि_Task != शून्यसूचकः)
          ++सक्रिय_Groups[गणः_सम्बन्धि_Task];
        Tasks.निष्कासय_front();
      } // The queue lock is released.

      // Run the task. The job slot remains acquired during execution.
      Task();

      // The task has finished. Update the active count and notify any waiters.
      {
        std::तालकम्_guard<std::mutex> तालकम्_Guard(Queue_तालकम्);
        --सक्रिय_तन्तवः;
        यदि (गणः_सम्बन्धि_Task != शून्यसूचकः) {
          स्वचालित A = सक्रिय_Groups.find(गणः_सम्बन्धि_Task);
          यदि (--(A->second) == 0)
            सक्रिय_Groups.erase(A);
        }
        // If all tasks are complete, notify any waiting threads.
        यदि (workCompletedUnlocked(शून्यसूचकः))
          CompletionCondition.notify_all();
      }
    }
  }
}
तार्किक StdThreadPool::workCompletedUnlocked(Thread_Pool_Task_गणः *गणः) ध्रुव {
  यदि (गणः == शून्यसूचकः)
    प्रत्यागम !सक्रिय_तन्तवः && Tasks.रिक्तानि();
  प्रत्यागम सक्रिय_Groups.गणनम्(गणः) == 0 &&
         !llvm::is_contained(llvm::make_second_range(Tasks), गणः);
}

शून्य StdThreadPool::प्रतीक्षस्व() {
  प्रतिज्ञा(!is_कार्यकर्ता_Thread()); // Would deadlock waiting for itself.
  // Wait for all threads to complete and the queue to be empty
  std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);
  CompletionCondition.प्रतीक्षस्व(तालकम्_Guard,
                           [&] { प्रत्यागम workCompletedUnlocked(शून्यसूचकः); });
}

शून्य StdThreadPool::प्रतीक्षस्व(Thread_Pool_Task_गणः &गणः) {
  // Wait for all threads in the group to complete.
  यदि (!is_कार्यकर्ता_Thread()) {
    std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);
    CompletionCondition.प्रतीक्षस्व(तालकम्_Guard,
                             [&] { प्रत्यागम workCompletedUnlocked(&गणः); });
    प्रत्यागम;
  }
  // Make sure to not deadlock waiting for oneself.
  प्रतिज्ञा(CurrentThreadTaskGroups == शून्यसूचकः ||
         !llvm::is_contained(*CurrentThreadTaskGroups, &गणः));
  // Handle the case of recursive call from another task in a different group,
  // in which case process tasks while waiting to keep the thread busy and avoid
  // possible deadlock.
  processTasks(&गणः);
}

तार्किक StdThreadPool::is_कार्यकर्ता_Thread() ध्रुव {
  llvm::sys::ScopedReader तालकम्_Guard(तन्तवः_तालकम्);
  llvm::thread::id CurrentThreadId = llvm::this_thread::get_id();
  प्रति (ध्रुव llvm::thread &Thread : तन्तवः)
    यदि (CurrentThreadId == Thread.get_id())
      प्रत्यागम सत्य;
  प्रत्यागम असत्य;
}

// The destructor joins all threads, waiting for completion.
StdThreadPool::~StdThreadPool() {
  {
    std::unique_तालकम्<std::mutex> तालकम्_Guard(Queue_तालकम्);
    EnableFlag = असत्य;
  }
  QueueCondition.notify_all();
  llvm::sys::ScopedReader तालकम्_Guard(तन्तवः_तालकम्);
  प्रति (स्वचालित &कार्यकर्ता : तन्तवः)
    कार्यकर्ता.join();
}

#यदि_अन्तः // LLVM_ENABLE_THREADS Disabled

// No threads are launched, issue a warning if ThreadCount is not 0
SingleThreadExecutor::SingleThreadExecutor(ThreadPoolStrategy S) {
  पूर्णाङ्क Thread_गणनम् = S.compute_thread_गणनम्();
  यदि (Thread_गणनम् != 1) {
    errs() << "Warning: request a ThreadPool with " << Thread_गणनम्
           << " threads, but LLVM_ENABLE_THREADS has been turned off\n";
  }
}

शून्य SingleThreadExecutor::प्रतीक्षस्व() {
  // Sequential implementation running the tasks
  यावत् (!Tasks.रिक्तानि()) {
    स्वचालित Task = std::move(Tasks.front().first);
    Tasks.निष्कासय_front();
    Task();
  }
}

शून्य SingleThreadExecutor::प्रतीक्षस्व(Thread_Pool_Task_गणः &) {
  // Simply wait for all, this works even if recursive (the running task
  // is already removed from the queue).
  प्रतीक्षस्व();
}

तार्किक SingleThreadExecutor::is_कार्यकर्ता_Thread() ध्रुव {
  report_fatal_error("LLVM compiled without multithreading");
}

SingleThreadExecutor::~SingleThreadExecutor() { प्रतीक्षस्व(); }
