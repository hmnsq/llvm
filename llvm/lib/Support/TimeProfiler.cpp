//===-- TimeProfiler.cpp - Hierarchical Time Profiler ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements hierarchical time profiler.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/TimeProfiler.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/STLFunctionalExtras.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/Support/Compression.h"
#ضمن "llvm/Support/JSON.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Threading.h"
#ضمن <algorithm>
#ضمن <cassert>
#ضمن <chrono>
#ضمن <memory>
#ضمن <mutex>
#ضمن <string>
#ضمن <vector>

using نطاق llvm;

نطاق {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::time_point_cast;

بنية TimeTraceProfilerInstances {
  std::mutex Lock;
  std::vector<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  ساكن TimeTraceProfilerInstances Instances;
  החזר Instances;
}

ساكن ثابت حرف TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساكن LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = nullptr;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  החזר TimeTraceProfilerInstance;
}

نطاق {

using ClockType = steady_clock;
using TimePointType = time_point<ClockType>;
using DurationType = duration<ClockType::rep, ClockType::period>;
using CountAndDurationType = std::pair<size_t, DurationType>;
using NameAndCountAndDurationType =
    std::pair<std::string, CountAndDurationType>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
بنية llvm::TimeTraceProfilerEntry {
  ثابت TimePointType Start;
  TimePointType End;
  ثابت std::string Name;
  TimeTraceMetadata Metadata;

  ثابت TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
  TimeTraceProfilerEntry(TimePointType &&S, TimePointType &&E, std::string &&N,
                         std::string &&Dt, TimeTraceEventType Et)
      : Start(std::move(S)), End(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  TimeTraceProfilerEntry(TimePointType &&S, TimePointType &&E, std::string &&N,
                         TimeTraceMetadata &&Mt, TimeTraceEventType Et)
      : Start(std::move(S)), End(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) ثابت {
    החזר (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .count();
  }

  ClockType::rep getFlameGraphDurUs() ثابت {
    החזר (time_point_cast<microseconds>(End) -
            time_point_cast<microseconds>(Start))
        .count();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
بنية InProgressEntry {
  TimeTraceProfilerEntry Event;
  std::vector<TimeTraceProfilerEntry> InstantEvents;

  InProgressEntry(TimePointType S, TimePointType E, std::string N,
                  std::string Dt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  InProgressEntry(TimePointType S, TimePointType E, std::string N,
                  TimeTraceMetadata Mt, TimeTraceEventType Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

بنية llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      unsigned TimeTraceGranularity = 0, StringRef ProcName = "",
      منطقي TimeTraceVerbose = שקר,
      TimeTraceCompression TimeTraceCompress = TimeTraceCompression::Infer)
      : BeginningOfTime(system_clock::now()), StartTime(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), TimeTraceGranularity(TimeTraceGranularity),
        TimeTraceVerbose(TimeTraceVerbose),
        TimeTraceCompress(TimeTraceCompress) {
    llvm::get_thread_name(ThreadName);
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    أكد(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    Stack.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    החזר &Stack.back()->Event;
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    أكد(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    Stack.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    החזר &Stack.back()->Event;
  }

  فراغ insert(std::string Name, llvm::function_ref<std::string()> Detail) {
    אם (Stack.empty())
      החזר;

    Stack.back()->InstantEvents.emplace_back(TimeTraceProfilerEntry(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  فراغ end() {
    أكد(!Stack.empty() && "Must call begin() first");
    end(Stack.back()->Event);
  }

  فراغ end(TimeTraceProfilerEntry &E) {
    أكد(!Stack.empty() && "Must call begin() first");
    E.End = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.End - E.Start;

    ثابت auto *Iter =
        llvm::find_if(Stack, [&](ثابت std::unique_ptr<InProgressEntry> &Val) {
          החזר &Val->Event == &E;
        });
    أكد(Iter != Stack.end() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    אם (duration_cast<microseconds>(Duration).count() >= TimeTraceGranularity) {
      Entries.emplace_back(E);
      עבור (auto &IE : Iter->get()->InstantEvents) {
        Entries.emplace_back(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    אם (llvm::none_of(llvm::drop_begin(llvm::reverse(Stack)),
                      [&](ثابت std::unique_ptr<InProgressEntry> &Val) {
                        החזר Val->Event.Name == E.Name;
                      })) {
      auto &CountAndTotal = CountAndTotalPerName[E.Name];
      CountAndTotal.first++;
      CountAndTotal.second += Duration;
    };

    Stack.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  فراغ اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    auto &Instances = getTimeTraceProfilerInstances();
    std::lock_guard<std::mutex> Lock(Instances.Lock);
    أكد(Stack.empty() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_of(Instances.List,
                        [](ثابت auto &TTP) { החזר TTP->Stack.empty(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    auto writeEvent = [&](ثابت auto &E, uint64_t Tid) {
      auto StartUs = E.getFlameGraphStartUs(StartTime);
      auto DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        אם (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } אחרת אם (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } אחרת { // instant event
          أكد(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        אם (!E.Metadata.isEmpty()) {
          J.attributeObject("args", [&] {
            אם (!E.Metadata.Detail.empty())
              J.attribute("detail", E.Metadata.Detail);
            אם (!E.Metadata.File.empty())
              J.attribute("file", E.Metadata.File);
            אם (E.Metadata.Line > 0)
              J.attribute("line", E.Metadata.Line);
          });
        }
      });

      אם (E.EventType == TimeTraceEventType::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", StartUs + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    עבור (ثابت TimeTraceProfilerEntry &E : Entries)
      writeEvent(E, هذا->Tid);
    עבור (ثابت TimeTraceProfiler *TTP : Instances.List)
      עבור (ثابت TimeTraceProfilerEntry &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = هذا->Tid;
    עבור (ثابت TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<CountAndDurationType> AllCountAndTotalPerName;
    auto combineStat = [&](ثابت auto &Stat) {
      StringRef Key = Stat.getKey();
      auto Value = Stat.getValue();
      auto &CountAndTotal = AllCountAndTotalPerName[Key];
      CountAndTotal.first += Value.first;
      CountAndTotal.second += Value.second;
    };
    עבור (ثابت auto &Stat : CountAndTotalPerName)
      combineStat(Stat);
    עבור (ثابت TimeTraceProfiler *TTP : Instances.List)
      עבור (ثابت auto &Stat : TTP->CountAndTotalPerName)
        combineStat(Stat);

    std::vector<NameAndCountAndDurationType> SortedTotals;
    SortedTotals.reserve(AllCountAndTotalPerName.size());
    עבור (ثابت auto &Total : AllCountAndTotalPerName)
      SortedTotals.emplace_back(std::string(Total.getKey()), Total.getValue());

    llvm::sort(SortedTotals, [](ثابت NameAndCountAndDurationType &A,
                                ثابت NameAndCountAndDurationType &B) {
      החזר A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t TotalTid = MaxTid + 1;
    עבור (ثابت NameAndCountAndDurationType &Total : SortedTotals) {
      auto DurUs = duration_cast<microseconds>(Total.second.second).count();
      auto Count = AllCountAndTotalPerName[Total.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(TotalTid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + Total.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(Count));
          J.attribute("avg ms", int64_t(DurUs / Count / 1000));
        });
      });

      ++TotalTid;
    }

    auto writeMetadataEvent = [&](ثابت حرف *Name, uint64_t Tid,
                                  StringRef arg) {
      J.object([&] {
        J.attribute("cat", "");
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", 0);
        J.attribute("ph", "M");
        J.attribute("name", Name);
        J.attributeObject("args", [&] { J.attribute("name", arg); });
      });
    };

    writeMetadataEvent("process_name", Tid, ProcName);
    writeMetadataEvent("thread_name", Tid, ThreadName);
    עבור (ثابت TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.arrayEnd();
    J.attributeEnd();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(BeginningOfTime)
                    .time_since_epoch()
                    .count());

    J.objectEnd();
  }

  SmallVector<std::unique_ptr<InProgressEntry>, 16> Stack;
  SmallVector<TimeTraceProfilerEntry, 128> Entries;
  StringMap<CountAndDurationType> CountAndTotalPerName;
  // System clock time when the session was begun.
  ثابت time_point<system_clock> BeginningOfTime;
  // Profiling clock time when the session was begun.
  ثابت TimePointType StartTime;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت unsigned TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقي TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

منطقي llvm::isTimeTraceVerbose() {
  החזר getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::inferTimeTraceCompressionFromPath(StringRef Path) {
  StringRef Ext = llvm::sys::path::extension(Path);
  אם (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    החזר TimeTraceCompression::Zstd;
  החזר TimeTraceCompression::None;
}

فراغ llvm::timeTraceProfilerInitialize(unsigned TimeTraceGranularity,
                                       StringRef ProcName,
                                       منطقي TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  أكد(TimeTraceProfilerInstance == nullptr &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = جديد TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::path::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
فراغ llvm::timeTraceProfilerCleanup() {
  delete TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = nullptr;

  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  עבור (auto *TTP : Instances.List)
    delete TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
فراغ llvm::timeTraceProfilerFinishThread() {
  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  Instances.List.push_back(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = nullptr;
}

فراغ llvm::timeTraceProfilerWrite(raw_pwrite_stream &OS) {
  أكد(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  אם (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->اكتب(OS);
    החזר;
  }

  compression::Format F = compression::Format::Zstd;
  אם (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  SmallVector<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->اكتب(UncompressedOS);
  }
  SmallVector<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.data()),
               Uncompressed.size()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.data()),
           Compressed.size());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string Path = PreferredFileName.str();
  אם (Path.empty()) {
    Path = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    Path += TimeTraceFileExtension;
    אם (CompressType == TimeTraceCompression::Zstd)
      Path += ".zst";
  } אחרת אם (CompressType == TimeTraceCompression::Infer) {
    CompressType = inferTimeTraceCompressionFromPath(Path);
  }

  אם (CompressType == TimeTraceCompression::Zstd) {
    אם (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      החזר createStringError(inconvertibleErrorCode(), Reason);
  }

  std::error_code EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::OF_None
                                 : sys::fs::OF_TextWithCRLF;
  raw_fd_ostream OS(Path, EC, Flags);
  אם (EC)
    החזר createStringError(EC, "Could not open " + Path);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  החזר Error::success();
}

TimeTraceProfilerEntry *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  אם (TimeTraceProfilerInstance != nullptr)
    החזר TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { החזר std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  החזר nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  אם (TimeTraceProfilerInstance != nullptr)
    החזר TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  החזר nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  אם (TimeTraceProfilerInstance != nullptr)
    החזר TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  החזר nullptr;
}

TimeTraceProfilerEntry *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  אם (TimeTraceProfilerInstance != nullptr)
    החזר TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { החזר std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  החזר nullptr;
}

فراغ llvm::timeTraceAddInstantEvent(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  אם (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->insert(std::string(Name), Detail);
}

فراغ llvm::timeTraceProfilerEnd() {
  אם (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end();
}

فراغ llvm::timeTraceProfilerEnd(TimeTraceProfilerEntry *E) {
  אם (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end(*E);
}
