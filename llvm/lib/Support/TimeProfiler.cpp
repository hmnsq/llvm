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

#ಅನ್ತರ್ಭಾವ "llvm/Support/TimeProfiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLFunctionalExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compression.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/JSON.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Threading.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <chrono>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <vector>

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::time_point_cast;

ಸಂರಚನಾ TimeTraceProfilerInstances {
  std::mutex Lock;
  std::vector<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  ಸ್ಥಿರ TimeTraceProfilerInstances Instances;
  ಪ್ರತ್ಯಾಗಮ Instances;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ಸ್ಥಿರ LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = nullptr;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  ಪ್ರತ್ಯಾಗಮ TimeTraceProfilerInstance;
}

ನಾಮಾಕಾಶ {

using ClockType = steady_clock;
using TimePointType = time_point<ClockType>;
using DurationType = duration<ClockType::rep, ClockType::period>;
using ಎಣಿಕೆ_And_Duration_Type = std::pair<size_t, DurationType>;
using Name_And_ಎಣಿಕೆ_And_Duration_Type =
    std::pair<std::string, ಎಣಿಕೆ_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
ಸಂರಚನಾ llvm::TimeTraceProfilerEntry {
  ಧ್ರುವ TimePointType Start;
  TimePointType End;
  ಧ್ರುವ std::string Name;
  TimeTraceMetadata Metadata;

  ಧ್ರುವ TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
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
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) ಧ್ರುವ {
    ಪ್ರತ್ಯಾಗಮ (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .ಎಣಿಕೆ();
  }

  ClockType::rep getFlameGraphDurUs() ಧ್ರುವ {
    ಪ್ರತ್ಯಾಗಮ (time_point_cast<microseconds>(End) -
            time_point_cast<microseconds>(Start))
        .ಎಣಿಕೆ();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
ಸಂರಚನಾ InProgressEntry {
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

ಸಂರಚನಾ llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      unsigned TimeTraceGranularity = 0, StringRef ProcName = "",
      ತಾರ್ಕಿಕ TimeTraceVerbose = ಅಸತ್ಯ,
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
    ಪ್ರತಿಜ್ಞಾ(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    ರಾಶಿ.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    ಪ್ರತ್ಯಾಗಮ &ರಾಶಿ.back()->Event;
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    ಪ್ರತಿಜ್ಞಾ(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    ರಾಶಿ.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    ಪ್ರತ್ಯಾಗಮ &ರಾಶಿ.back()->Event;
  }

  ಶೂನ್ಯ ಸೇರಿಸು(std::string Name, llvm::function_ref<std::string()> Detail) {
    ಯದಿ (ರಾಶಿ.empty())
      ಪ್ರತ್ಯಾಗಮ;

    ರಾಶಿ.back()->InstantEvents.emplace_back(TimeTraceProfilerEntry(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  ಶೂನ್ಯ end() {
    ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.empty() && "Must call begin() first");
    end(ರಾಶಿ.back()->Event);
  }

  ಶೂನ್ಯ end(TimeTraceProfilerEntry &E) {
    ಪ್ರತಿಜ್ಞಾ(!ರಾಶಿ.empty() && "Must call begin() first");
    E.End = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.End - E.Start;

    ಧ್ರುವ auto *Iter =
        llvm::find_if(ರಾಶಿ, [&](ಧ್ರುವ std::unique_ptr<InProgressEntry> &Val) {
          ಪ್ರತ್ಯಾಗಮ &Val->Event == &E;
        });
    ಪ್ರತಿಜ್ಞಾ(Iter != ರಾಶಿ.end() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    ಯದಿ (duration_cast<microseconds>(Duration).ಎಣಿಕೆ() >= TimeTraceGranularity) {
      Entries.emplace_back(E);
      ಪ್ರತಿ (auto &IE : Iter->get()->InstantEvents) {
        Entries.emplace_back(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    ಯದಿ (llvm::none_of(llvm::drop_begin(llvm::reverse(ರಾಶಿ)),
                      [&](ಧ್ರುವ std::unique_ptr<InProgressEntry> &Val) {
                        ಪ್ರತ್ಯಾಗಮ Val->Event.Name == E.Name;
                      })) {
      auto &ಎಣಿಕೆ_And_Total = ಎಣಿಕೆ_And_Total_Per_Name[E.Name];
      ಎಣಿಕೆ_And_Total.first++;
      ಎಣಿಕೆ_And_Total.second += Duration;
    };

    ರಾಶಿ.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  ಶೂನ್ಯ ಲಿಖ(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    auto &Instances = getTimeTraceProfilerInstances();
    std::lock_guard<std::mutex> Lock(Instances.Lock);
    ಪ್ರತಿಜ್ಞಾ(ರಾಶಿ.empty() &&
           "All profiler sections should be ended when calling write");
    ಪ್ರತಿಜ್ಞಾ(llvm::all_of(Instances.List,
                        [](ಧ್ರುವ auto &TTP) { ಪ್ರತ್ಯಾಗಮ TTP->ರಾಶಿ.empty(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    auto writeEvent = [&](ಧ್ರುವ auto &E, uint64_t Tid) {
      auto StartUs = E.getFlameGraphStartUs(StartTime);
      auto DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        ಯದಿ (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } ಅನ್ಯಥಾ ಯದಿ (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } ಅನ್ಯಥಾ { // instant event
          ಪ್ರತಿಜ್ಞಾ(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        ಯದಿ (!E.Metadata.isEmpty()) {
          J.attributeObject("args", [&] {
            ಯದಿ (!E.Metadata.Detail.empty())
              J.attribute("detail", E.Metadata.Detail);
            ಯದಿ (!E.Metadata.File.empty())
              J.attribute("file", E.Metadata.File);
            ಯದಿ (E.Metadata.ಸಾಲು > 0)
              J.attribute("line", E.Metadata.ಸಾಲು);
          });
        }
      });

      ಯದಿ (E.EventType == TimeTraceEventType::AsyncEvent) {
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
    ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfilerEntry &E : Entries)
      writeEvent(E, ಏತತ್->Tid);
    ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfiler *TTP : Instances.List)
      ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfilerEntry &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = ಏತತ್->Tid;
    ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<ಎಣಿಕೆ_And_Duration_Type> All_ಎಣಿಕೆ_And_Total_Per_Name;
    auto combineStat = [&](ಧ್ರುವ auto &Stat) {
      StringRef ಕೀಲಿ = Stat.get_ಕೀಲಿ();
      auto ಮೌಲ್ಯ = Stat.get_ಮೌಲ್ಯ();
      auto &ಎಣಿಕೆ_And_Total = All_ಎಣಿಕೆ_And_Total_Per_Name[ಕೀಲಿ];
      ಎಣಿಕೆ_And_Total.first += ಮೌಲ್ಯ.first;
      ಎಣಿಕೆ_And_Total.second += ಮೌಲ್ಯ.second;
    };
    ಪ್ರತಿ (ಧ್ರುವ auto &Stat : ಎಣಿಕೆ_And_Total_Per_Name)
      combineStat(Stat);
    ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfiler *TTP : Instances.List)
      ಪ್ರತಿ (ಧ್ರುವ auto &Stat : TTP->ಎಣಿಕೆ_And_Total_Per_Name)
        combineStat(Stat);

    std::vector<Name_And_ಎಣಿಕೆ_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_ಎಣಿಕೆ_And_Total_Per_Name.size());
    ಪ್ರತಿ (ಧ್ರುವ auto &Total : All_ಎಣಿಕೆ_And_Total_Per_Name)
      SortedTotals.emplace_back(std::string(Total.get_ಕೀಲಿ()), Total.get_ಮೌಲ್ಯ());

    llvm::sort(SortedTotals, [](ಧ್ರುವ Name_And_ಎಣಿಕೆ_And_Duration_Type &A,
                                ಧ್ರುವ Name_And_ಎಣಿಕೆ_And_Duration_Type &B) {
      ಪ್ರತ್ಯಾಗಮ A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t TotalTid = MaxTid + 1;
    ಪ್ರತಿ (ಧ್ರುವ Name_And_ಎಣಿಕೆ_And_Duration_Type &Total : SortedTotals) {
      auto DurUs = duration_cast<microseconds>(Total.second.second).ಎಣಿಕೆ();
      auto ಎಣಿಕೆ_2 = All_ಎಣಿಕೆ_And_Total_Per_Name[Total.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(TotalTid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + Total.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(ಎಣಿಕೆ_2));
          J.attribute("avg ms", int64_t(DurUs / ಎಣಿಕೆ_2 / 1000));
        });
      });

      ++TotalTid;
    }

    auto writeMetadataEvent = [&](ಧ್ರುವ ವರ್ಣ *Name, uint64_t Tid,
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
    ಪ್ರತಿ (ಧ್ರುವ TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.arrayEnd();
    J.attributeEnd();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(BeginningOfTime)
                    .time_since_epoch()
                    .ಎಣಿಕೆ());

    J.objectEnd();
  }

  SmallVector<std::unique_ptr<InProgressEntry>, 16> ರಾಶಿ;
  SmallVector<TimeTraceProfilerEntry, 128> Entries;
  StringMap<ಎಣಿಕೆ_And_Duration_Type> ಎಣಿಕೆ_And_Total_Per_Name;
  // System clock time when the session was begun.
  ಧ್ರುವ time_point<system_clock> BeginningOfTime;
  // Profiling clock time when the session was begun.
  ಧ್ರುವ TimePointType StartTime;
  ಧ್ರುವ std::string ProcName;
  ಧ್ರುವ sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ಧ್ರುವ uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ಧ್ರುವ unsigned TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ಧ್ರುವ ತಾರ್ಕಿಕ TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

ತಾರ್ಕಿಕ llvm::isTimeTraceVerbose() {
  ಪ್ರತ್ಯಾಗಮ getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::inferTimeTraceCompressionFromPath(StringRef Path) {
  StringRef Ext = llvm::sys::path::extension(Path);
  ಯದಿ (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    ಪ್ರತ್ಯಾಗಮ TimeTraceCompression::Zstd;
  ಪ್ರತ್ಯಾಗಮ TimeTraceCompression::None;
}

ಶೂನ್ಯ llvm::timeTraceProfilerInitialize(unsigned TimeTraceGranularity,
                                       StringRef ProcName,
                                       ತಾರ್ಕಿಕ TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  ಪ್ರತಿಜ್ಞಾ(TimeTraceProfilerInstance == nullptr &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = ನೂತನ TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::path::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
ಶೂನ್ಯ llvm::timeTraceProfilerCleanup() {
  delete TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = nullptr;

  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  ಪ್ರತಿ (auto *TTP : Instances.List)
    delete TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
ಶೂನ್ಯ llvm::timeTraceProfilerFinishThread() {
  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  Instances.List.ತಳ್ಳು_back(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = nullptr;
}

ಶೂನ್ಯ llvm::timeTraceProfilerWrite(raw_pwrite_stream &OS) {
  ಪ್ರತಿಜ್ಞಾ(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  ಯದಿ (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->ಲಿಖ(OS);
    ಪ್ರತ್ಯಾಗಮ;
  }

  compression::Format F = compression::Format::Zstd;
  ಯದಿ (ಧ್ರುವ ವರ್ಣ *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  SmallVector<ವರ್ಣ, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->ಲಿಖ(UncompressedOS);
  }
  SmallVector<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ಧ್ರುವ uint8_t *>(Uncompressed.data()),
               Uncompressed.size()),
      Compressed);
  OS.ಲಿಖ(reinterpret_cast<ಧ್ರುವ ವರ್ಣ *>(Compressed.data()),
           Compressed.size());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  ಪ್ರತಿಜ್ಞಾ(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string Path = PreferredFileName.str();
  ಯದಿ (Path.empty()) {
    Path = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    Path += TimeTraceFileExtension;
    ಯದಿ (CompressType == TimeTraceCompression::Zstd)
      Path += ".zst";
  } ಅನ್ಯಥಾ ಯದಿ (CompressType == TimeTraceCompression::Infer) {
    CompressType = inferTimeTraceCompressionFromPath(Path);
  }

  ಯದಿ (CompressType == TimeTraceCompression::Zstd) {
    ಯದಿ (ಧ್ರುವ ವರ್ಣ *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      ಪ್ರತ್ಯಾಗಮ createStringError(inconvertibleErrorCode(), Reason);
  }

  std::error_code EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::OF_None
                                 : sys::fs::OF_TextWithCRLF;
  raw_fd_ostream OS(Path, EC, Flags);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ createStringError(EC, "Could not open " + Path);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

TimeTraceProfilerEntry *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    ಪ್ರತ್ಯಾಗಮ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { ಪ್ರತ್ಯಾಗಮ std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    ಪ್ರತ್ಯಾಗಮ TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    ಪ್ರತ್ಯಾಗಮ TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

TimeTraceProfilerEntry *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    ಪ್ರತ್ಯಾಗಮ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { ಪ್ರತ್ಯಾಗಮ std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

ಶೂನ್ಯ llvm::timeTraceAddInstantEvent(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->ಸೇರಿಸು(std::string(Name), Detail);
}

ಶೂನ್ಯ llvm::timeTraceProfilerEnd() {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end();
}

ಶೂನ್ಯ llvm::timeTraceProfilerEnd(TimeTraceProfilerEntry *E) {
  ಯದಿ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end(*E);
}
