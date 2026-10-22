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

#അന്തര്ഭാവ "llvm/Support/TimeProfiler.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/STLFunctionalExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/Support/Compression.h"
#അന്തര്ഭാവ "llvm/Support/JSON.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Threading.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <chrono>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <mutex>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <vector>

using നാമാകാശ llvm;

നാമാകാശ {

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;
using std::chrono::time_point;
using std::chrono::time_point_cast;

ഘടന TimeTraceProfilerInstances {
  std::mutex Lock;
  std::vector<TimeTraceProfiler *> List;
};

TimeTraceProfilerInstances &getTimeTraceProfilerInstances() {
  സ്ഥിതം TimeTraceProfilerInstances Instances;
  തിരികെ Instances;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം TimeTraceFileExtension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
സ്ഥിതം LLVM_THREAD_LOCAL TimeTraceProfiler *TimeTraceProfilerInstance = nullptr;

TimeTraceProfiler *llvm::getTimeTraceProfilerInstance() {
  തിരികെ TimeTraceProfilerInstance;
}

നാമാകാശ {

using ClockType = steady_clock;
using TimePointType = time_point<ClockType>;
using DurationType = duration<ClockType::rep, ClockType::period>;
using എണ്ണം_And_Duration_Type = std::pair<size_t, DurationType>;
using Name_And_എണ്ണം_And_Duration_Type =
    std::pair<std::string, എണ്ണം_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
ഘടന llvm::TimeTraceProfilerEntry {
  സ്ഥിരം TimePointType Start;
  TimePointType End;
  സ്ഥിരം std::string Name;
  TimeTraceMetadata Metadata;

  സ്ഥിരം TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent;
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
  ClockType::rep getFlameGraphStartUs(TimePointType StartTime) സ്ഥിരം {
    തിരികെ (time_point_cast<microseconds>(Start) -
            time_point_cast<microseconds>(StartTime))
        .എണ്ണം();
  }

  ClockType::rep getFlameGraphDurUs() സ്ഥിരം {
    തിരികെ (time_point_cast<microseconds>(End) -
            time_point_cast<microseconds>(Start))
        .എണ്ണം();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
ഘടന InProgressEntry {
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

ഘടന llvm::TimeTraceProfiler {
  TimeTraceProfiler(
      unsigned TimeTraceGranularity = 0, StringRef ProcName = "",
      തർക്കം TimeTraceVerbose = അസത്യം,
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
    പ്രതിജ്ഞാ(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    അട്ടി.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        EventType));
    തിരികെ &അട്ടി.back()->Event;
  }

  TimeTraceProfilerEntry *
  begin(std::string Name, llvm::function_ref<TimeTraceMetadata()> Metadata,
        TimeTraceEventType EventType = TimeTraceEventType::CompleteEvent) {
    പ്രതിജ്ഞാ(EventType != TimeTraceEventType::InstantEvent &&
           "Instant Events don't have begin and end.");
    അട്ടി.emplace_back(std::make_unique<InProgressEntry>(
        ClockType::now(), TimePointType(), std::move(Name), Metadata(),
        EventType));
    തിരികെ &അട്ടി.back()->Event;
  }

  ശൂന്യം തിരുകുക(std::string Name, llvm::function_ref<std::string()> Detail) {
    എങ്കിൽ (അട്ടി.empty())
      തിരികെ;

    അട്ടി.back()->InstantEvents.emplace_back(TimeTraceProfilerEntry(
        ClockType::now(), TimePointType(), std::move(Name), Detail(),
        TimeTraceEventType::InstantEvent));
  }

  ശൂന്യം end() {
    പ്രതിജ്ഞാ(!അട്ടി.empty() && "Must call begin() first");
    end(അട്ടി.back()->Event);
  }

  ശൂന്യം end(TimeTraceProfilerEntry &E) {
    പ്രതിജ്ഞാ(!അട്ടി.empty() && "Must call begin() first");
    E.End = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.End - E.Start;

    സ്ഥിരം auto *Iter =
        llvm::find_if(അട്ടി, [&](സ്ഥിരം std::unique_ptr<InProgressEntry> &Val) {
          തിരികെ &Val->Event == &E;
        });
    പ്രതിജ്ഞാ(Iter != അട്ടി.end() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    എങ്കിൽ (duration_cast<microseconds>(Duration).എണ്ണം() >= TimeTraceGranularity) {
      Entries.emplace_back(E);
      വേണ്ടി (auto &IE : Iter->get()->InstantEvents) {
        Entries.emplace_back(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    എങ്കിൽ (llvm::none_of(llvm::drop_begin(llvm::reverse(അട്ടി)),
                      [&](സ്ഥിരം std::unique_ptr<InProgressEntry> &Val) {
                        തിരികെ Val->Event.Name == E.Name;
                      })) {
      auto &എണ്ണം_And_Total = എണ്ണം_And_Total_Per_Name[E.Name];
      എണ്ണം_And_Total.first++;
      എണ്ണം_And_Total.second += Duration;
    };

    അട്ടി.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  ശൂന്യം ലിഖ(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    auto &Instances = getTimeTraceProfilerInstances();
    std::lock_guard<std::mutex> Lock(Instances.Lock);
    പ്രതിജ്ഞാ(അട്ടി.empty() &&
           "All profiler sections should be ended when calling write");
    പ്രതിജ്ഞാ(llvm::all_of(Instances.List,
                        [](സ്ഥിരം auto &TTP) { തിരികെ TTP->അട്ടി.empty(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    auto writeEvent = [&](സ്ഥിരം auto &E, uint64_t Tid) {
      auto StartUs = E.getFlameGraphStartUs(StartTime);
      auto DurUs = E.getFlameGraphDurUs();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", StartUs);
        എങ്കിൽ (E.EventType == TimeTraceEventType::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } അല്ലെങ്കിൽ എങ്കിൽ (E.EventType == TimeTraceEventType::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } അല്ലെങ്കിൽ { // instant event
          പ്രതിജ്ഞാ(E.EventType == TimeTraceEventType::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        എങ്കിൽ (!E.Metadata.isEmpty()) {
          J.attributeObject("args", [&] {
            എങ്കിൽ (!E.Metadata.Detail.empty())
              J.attribute("detail", E.Metadata.Detail);
            എങ്കിൽ (!E.Metadata.File.empty())
              J.attribute("file", E.Metadata.File);
            എങ്കിൽ (E.Metadata.വരി > 0)
              J.attribute("line", E.Metadata.വരി);
          });
        }
      });

      എങ്കിൽ (E.EventType == TimeTraceEventType::AsyncEvent) {
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
    വേണ്ടി (സ്ഥിരം TimeTraceProfilerEntry &E : Entries)
      writeEvent(E, ഇത്->Tid);
    വേണ്ടി (സ്ഥിരം TimeTraceProfiler *TTP : Instances.List)
      വേണ്ടി (സ്ഥിരം TimeTraceProfilerEntry &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = ഇത്->Tid;
    വേണ്ടി (സ്ഥിരം TimeTraceProfiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<എണ്ണം_And_Duration_Type> All_എണ്ണം_And_Total_Per_Name;
    auto combineStat = [&](സ്ഥിരം auto &Stat) {
      StringRef താക്കോൽ = Stat.get_താക്കോൽ();
      auto മൂല്യം = Stat.get_മൂല്യം();
      auto &എണ്ണം_And_Total = All_എണ്ണം_And_Total_Per_Name[താക്കോൽ];
      എണ്ണം_And_Total.first += മൂല്യം.first;
      എണ്ണം_And_Total.second += മൂല്യം.second;
    };
    വേണ്ടി (സ്ഥിരം auto &Stat : എണ്ണം_And_Total_Per_Name)
      combineStat(Stat);
    വേണ്ടി (സ്ഥിരം TimeTraceProfiler *TTP : Instances.List)
      വേണ്ടി (സ്ഥിരം auto &Stat : TTP->എണ്ണം_And_Total_Per_Name)
        combineStat(Stat);

    std::vector<Name_And_എണ്ണം_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_എണ്ണം_And_Total_Per_Name.size());
    വേണ്ടി (സ്ഥിരം auto &Total : All_എണ്ണം_And_Total_Per_Name)
      SortedTotals.emplace_back(std::string(Total.get_താക്കോൽ()), Total.get_മൂല്യം());

    llvm::sort(SortedTotals, [](സ്ഥിരം Name_And_എണ്ണം_And_Duration_Type &A,
                                സ്ഥിരം Name_And_എണ്ണം_And_Duration_Type &B) {
      തിരികെ A.second.second > B.second.second;
    });

    // Report totals on separate threads of tracing file.
    uint64_t TotalTid = MaxTid + 1;
    വേണ്ടി (സ്ഥിരം Name_And_എണ്ണം_And_Duration_Type &Total : SortedTotals) {
      auto DurUs = duration_cast<microseconds>(Total.second.second).എണ്ണം();
      auto എണ്ണം_2 = All_എണ്ണം_And_Total_Per_Name[Total.first].first;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(TotalTid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + Total.first);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(എണ്ണം_2));
          J.attribute("avg ms", int64_t(DurUs / എണ്ണം_2 / 1000));
        });
      });

      ++TotalTid;
    }

    auto writeMetadataEvent = [&](സ്ഥിരം അക്ഷരം *Name, uint64_t Tid,
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
    വേണ്ടി (സ്ഥിരം TimeTraceProfiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.arrayEnd();
    J.attributeEnd();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                time_point_cast<microseconds>(BeginningOfTime)
                    .time_since_epoch()
                    .എണ്ണം());

    J.objectEnd();
  }

  SmallVector<std::unique_ptr<InProgressEntry>, 16> അട്ടി;
  SmallVector<TimeTraceProfilerEntry, 128> Entries;
  StringMap<എണ്ണം_And_Duration_Type> എണ്ണം_And_Total_Per_Name;
  // System clock time when the session was begun.
  സ്ഥിരം time_point<system_clock> BeginningOfTime;
  // Profiling clock time when the session was begun.
  സ്ഥിരം TimePointType StartTime;
  സ്ഥിരം std::string ProcName;
  സ്ഥിരം sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  സ്ഥിരം uint64_t Tid;

  // Minimum time granularity (in microseconds)
  സ്ഥിരം unsigned TimeTraceGranularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  സ്ഥിരം തർക്കം TimeTraceVerbose;

  // Compression mode for output.
  TimeTraceCompression TimeTraceCompress;
};

തർക്കം llvm::isTimeTraceVerbose() {
  തിരികെ getTimeTraceProfilerInstance() &&
         getTimeTraceProfilerInstance()->TimeTraceVerbose;
}

TimeTraceCompression llvm::inferTimeTraceCompressionFromPath(StringRef Path) {
  StringRef Ext = llvm::sys::path::extension(Path);
  എങ്കിൽ (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    തിരികെ TimeTraceCompression::Zstd;
  തിരികെ TimeTraceCompression::None;
}

ശൂന്യം llvm::timeTraceProfilerInitialize(unsigned TimeTraceGranularity,
                                       StringRef ProcName,
                                       തർക്കം TimeTraceVerbose,
                                       TimeTraceCompression TimeTraceCompress) {
  പ്രതിജ്ഞാ(TimeTraceProfilerInstance == nullptr &&
         "Profiler should not be initialized");
  TimeTraceProfilerInstance = പുതിയ TimeTraceProfiler(
      TimeTraceGranularity, llvm::sys::path::filename(ProcName),
      TimeTraceVerbose, TimeTraceCompress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
ശൂന്യം llvm::timeTraceProfilerCleanup() {
  delete TimeTraceProfilerInstance;
  TimeTraceProfilerInstance = nullptr;

  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  വേണ്ടി (auto *TTP : Instances.List)
    delete TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
ശൂന്യം llvm::timeTraceProfilerFinishThread() {
  auto &Instances = getTimeTraceProfilerInstances();
  std::lock_guard<std::mutex> Lock(Instances.Lock);
  Instances.List.തള്ളുക_back(TimeTraceProfilerInstance);
  TimeTraceProfilerInstance = nullptr;
}

ശൂന്യം llvm::timeTraceProfilerWrite(raw_pwrite_stream &OS) {
  പ്രതിജ്ഞാ(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");
  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  എങ്കിൽ (CompressType != TimeTraceCompression::Zstd) {
    TimeTraceProfilerInstance->ലിഖ(OS);
    തിരികെ;
  }

  compression::Format F = compression::Format::Zstd;
  എങ്കിൽ (സ്ഥിരം അക്ഷരം *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  SmallVector<അക്ഷരം, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    TimeTraceProfilerInstance->ലിഖ(UncompressedOS);
  }
  SmallVector<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<സ്ഥിരം uint8_t *>(Uncompressed.data()),
               Uncompressed.size()),
      Compressed);
  OS.ലിഖ(reinterpret_cast<സ്ഥിരം അക്ഷരം *>(Compressed.data()),
           Compressed.size());
}

Error llvm::timeTraceProfilerWrite(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  പ്രതിജ്ഞാ(TimeTraceProfilerInstance != nullptr &&
         "Profiler object can't be null");

  TimeTraceCompression CompressType =
      TimeTraceProfilerInstance->TimeTraceCompress;
  std::string Path = PreferredFileName.str();
  എങ്കിൽ (Path.empty()) {
    Path = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    Path += TimeTraceFileExtension;
    എങ്കിൽ (CompressType == TimeTraceCompression::Zstd)
      Path += ".zst";
  } അല്ലെങ്കിൽ എങ്കിൽ (CompressType == TimeTraceCompression::Infer) {
    CompressType = inferTimeTraceCompressionFromPath(Path);
  }

  എങ്കിൽ (CompressType == TimeTraceCompression::Zstd) {
    എങ്കിൽ (സ്ഥിരം അക്ഷരം *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      തിരികെ createStringError(inconvertibleErrorCode(), Reason);
  }

  std::error_code EC;
  sys::fs::OpenFlags Flags = CompressType == TimeTraceCompression::Zstd
                                 ? sys::fs::OF_None
                                 : sys::fs::OF_TextWithCRLF;
  raw_fd_ostream OS(Path, EC, Flags);
  എങ്കിൽ (EC)
    തിരികെ createStringError(EC, "Could not open " + Path);

  TimeTraceCompression SavedCompress =
      TimeTraceProfilerInstance->TimeTraceCompress;
  TimeTraceProfilerInstance->TimeTraceCompress = CompressType;
  timeTraceProfilerWrite(OS);
  TimeTraceProfilerInstance->TimeTraceCompress = SavedCompress;
  തിരികെ Error::success();
}

TimeTraceProfilerEntry *llvm::timeTraceProfilerBegin(StringRef Name,
                                                     StringRef Detail) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    തിരികെ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { തിരികെ std::string(Detail); },
        TimeTraceEventType::CompleteEvent);
  തിരികെ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    തിരികെ TimeTraceProfilerInstance->begin(std::string(Name), Detail,
                                            TimeTraceEventType::CompleteEvent);
  തിരികെ nullptr;
}

TimeTraceProfilerEntry *
llvm::timeTraceProfilerBegin(StringRef Name,
                             llvm::function_ref<TimeTraceMetadata()> Metadata) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    തിരികെ TimeTraceProfilerInstance->begin(std::string(Name), Metadata,
                                            TimeTraceEventType::CompleteEvent);
  തിരികെ nullptr;
}

TimeTraceProfilerEntry *llvm::timeTraceAsyncProfilerBegin(StringRef Name,
                                                          StringRef Detail) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    തിരികെ TimeTraceProfilerInstance->begin(
        std::string(Name), [&]() { തിരികെ std::string(Detail); },
        TimeTraceEventType::AsyncEvent);
  തിരികെ nullptr;
}

ശൂന്യം llvm::timeTraceAddInstantEvent(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->തിരുകുക(std::string(Name), Detail);
}

ശൂന്യം llvm::timeTraceProfilerEnd() {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end();
}

ശൂന്യം llvm::timeTraceProfilerEnd(TimeTraceProfilerEntry *E) {
  എങ്കിൽ (TimeTraceProfilerInstance != nullptr)
    TimeTraceProfilerInstance->end(*E);
}
