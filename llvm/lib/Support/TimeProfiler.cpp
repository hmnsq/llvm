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

کارول نطاق llvm;

نطاق {

کارول std::chrono::duration;
کارول std::chrono::duration_cast;
کارول std::chrono::microseconds;
کارول std::chrono::steady_clock;
کارول std::chrono::system_clock;
کارول std::chrono::وخت_ټکی_2;
کارول std::chrono::وخت_ټکی_cast;

بنية وخت_Trace_Profiler_Instances {
  std::mutex قلف_2;
  std::ویکټور<وخت_Trace_Profiler *> List;
};

وخت_Trace_Profiler_Instances &get_وخت_Trace_Profiler_Instances() {
  ساكن وخت_Trace_Profiler_Instances Instances;
  أرجع Instances;
}

ساكن ثابت حرف وخت_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساكن LLVM_THREAD_LOCAL وخت_Trace_Profiler *وخت_Trace_Profiler_Instance = تش_اشاره;

وخت_Trace_Profiler *llvm::get_وخت_Trace_Profiler_Instance() {
  أرجع وخت_Trace_Profiler_Instance;
}

نطاق {

کارول ClockType = steady_clock;
کارول وخت_ټکی_Type = وخت_ټکی_2<ClockType>;
کارول DurationType = duration<ClockType::rep, ClockType::period>;
کارول شمېر_And_Duration_Type = std::pair<size_t, DurationType>;
کارول Name_And_شمېر_And_Duration_Type =
    std::pair<std::string, شمېر_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
بنية llvm::وخت_Trace_Profiler_ثبت {
  ثابت وخت_ټکی_Type پیل;
  وخت_ټکی_Type پای_ټکی_2;
  ثابت std::string Name;
  وخت_Trace_Metadata Metadata;

  ثابت وخت_Trace_Event_Type EventType = وخت_Trace_Event_Type::CompleteEvent;
  وخت_Trace_Profiler_ثبت(وخت_ټکی_Type &&S, وخت_ټکی_Type &&E, std::string &&N,
                         std::string &&Dt, وخت_Trace_Event_Type Et)
      : پیل(std::move(S)), پای_ټکی_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  وخت_Trace_Profiler_ثبت(وخت_ټکی_Type &&S, وخت_ټکی_Type &&E, std::string &&N,
                         وخت_Trace_Metadata &&Mt, وخت_Trace_Event_Type Et)
      : پیل(std::move(S)), پای_ټکی_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_ګراف_پیل_Us(وخت_ټکی_Type پیل_وخت) ثابت {
    أرجع (وخت_ټکی_cast<microseconds>(پیل) -
            وخت_ټکی_cast<microseconds>(پیل_وخت))
        .شمېر();
  }

  ClockType::rep get_Flame_ګراف_Dur_Us() ثابت {
    أرجع (وخت_ټکی_cast<microseconds>(پای_ټکی_2) -
            وخت_ټکی_cast<microseconds>(پیل))
        .شمېر();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
بنية دننه_پرمختګ_ثبت {
  وخت_Trace_Profiler_ثبت Event;
  std::ویکټور<وخت_Trace_Profiler_ثبت> InstantEvents;

  دننه_پرمختګ_ثبت(وخت_ټکی_Type S, وخت_ټکی_Type E, std::string N,
                  std::string Dt, وخت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  دننه_پرمختګ_ثبت(وخت_ټکی_Type S, وخت_ټکی_Type E, std::string N,
                  وخت_Trace_Metadata Mt, وخت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

بنية llvm::وخت_Trace_Profiler {
  وخت_Trace_Profiler(
      بینښه وخت_Trace_Granularity = 0, StringRef ProcName = "",
      منطقي وخت_Trace_Verbose = خطأ,
      وخت_Trace_Compression وخت_Trace_Compress = وخت_Trace_Compression::Infer)
      : Beginning_د_وخت(system_clock::now()), پیل_وخت(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), وخت_Trace_Granularity(وخت_Trace_Granularity),
        وخت_Trace_Verbose(وخت_Trace_Verbose),
        وخت_Trace_Compress(وخت_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  وخت_Trace_Profiler_ثبت *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        وخت_Trace_Event_Type EventType = وخت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وخت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ډېری.emplace_شا(std::make_unique<دننه_پرمختګ_ثبت>(
        ClockType::now(), وخت_ټکی_Type(), std::move(Name), Detail(),
        EventType));
    أرجع &ډېری.شا()->Event;
  }

  وخت_Trace_Profiler_ثبت *
  begin(std::string Name, llvm::function_ref<وخت_Trace_Metadata()> Metadata,
        وخت_Trace_Event_Type EventType = وخت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وخت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ډېری.emplace_شا(std::make_unique<دننه_پرمختګ_ثبت>(
        ClockType::now(), وخت_ټکی_Type(), std::move(Name), Metadata(),
        EventType));
    أرجع &ډېری.شا()->Event;
  }

  فراغ ورننباسه(std::string Name, llvm::function_ref<std::string()> Detail) {
    إذا (ډېری.تشه())
      أرجع;

    ډېری.شا()->InstantEvents.emplace_شا(وخت_Trace_Profiler_ثبت(
        ClockType::now(), وخت_ټکی_Type(), std::move(Name), Detail(),
        وخت_Trace_Event_Type::InstantEvent));
  }

  فراغ پای_ټکی() {
    أكد(!ډېری.تشه() && "Must call begin() first");
    پای_ټکی(ډېری.شا()->Event);
  }

  فراغ پای_ټکی(وخت_Trace_Profiler_ثبت &E) {
    أكد(!ډېری.تشه() && "Must call begin() first");
    E.پای_ټکی_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.پای_ټکی_2 - E.پیل;

    ثابت خپلکاری *Iter =
        llvm::پیدا_کړه_if(ډېری, [&](ثابت std::unique_ptr<دننه_پرمختګ_ثبت> &Val) {
          أرجع &Val->Event == &E;
        });
    أكد(Iter != ډېری.پای_ټکی() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    إذا (duration_cast<microseconds>(Duration).شمېر() >= وخت_Trace_Granularity) {
      Entries.emplace_شا(E);
      لكل (خپلکاری &IE : Iter->get()->InstantEvents) {
        Entries.emplace_شا(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    إذا (llvm::none_د(llvm::drop_begin(llvm::سرچپه(ډېری)),
                      [&](ثابت std::unique_ptr<دننه_پرمختګ_ثبت> &Val) {
                        أرجع Val->Event.Name == E.Name;
                      })) {
      خپلکاری &شمېر_And_ټول = شمېر_And_ټول_فی_Name[E.Name];
      شمېر_And_ټول.لومړی_2++;
      شمېر_And_ټول.دوهم_2 += Duration;
    };

    ډېری.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  فراغ اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    خپلکاری &Instances = get_وخت_Trace_Profiler_Instances();
    std::قلف_guard<std::mutex> قلف_2(Instances.قلف_2);
    أكد(ډېری.تشه() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_د(Instances.List,
                        [](ثابت خپلکاری &TTP) { أرجع TTP->ډېری.تشه(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    خپلکاری writeEvent = [&](ثابت خپلکاری &E, uint64_t Tid) {
      خپلکاری پیل_Us = E.get_Flame_ګراف_پیل_Us(پیل_وخت);
      خپلکاری DurUs = E.get_Flame_ګراف_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", پیل_Us);
        إذا (E.EventType == وخت_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } وإلا إذا (E.EventType == وخت_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } وإلا { // instant event
          أكد(E.EventType == وخت_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        إذا (!E.Metadata.is_تشه()) {
          J.attributeObject("args", [&] {
            إذا (!E.Metadata.Detail.تشه())
              J.attribute("detail", E.Metadata.Detail);
            إذا (!E.Metadata.File.تشه())
              J.attribute("file", E.Metadata.File);
            إذا (E.Metadata.کرښه > 0)
              J.attribute("line", E.Metadata.کرښه);
          });
        }
      });

      إذا (E.EventType == وخت_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", پیل_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    لكل (ثابت وخت_Trace_Profiler_ثبت &E : Entries)
      writeEvent(E, هذا->Tid);
    لكل (ثابت وخت_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت وخت_Trace_Profiler_ثبت &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = هذا->Tid;
    لكل (ثابت وخت_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<شمېر_And_Duration_Type> All_شمېر_And_ټول_فی_Name;
    خپلکاری combineStat = [&](ثابت خپلکاری &Stat) {
      StringRef کلی = Stat.get_کلی();
      خپلکاری ارزښت = Stat.get_ارزښت();
      خپلکاری &شمېر_And_ټول = All_شمېر_And_ټول_فی_Name[کلی];
      شمېر_And_ټول.لومړی_2 += ارزښت.لومړی_2;
      شمېر_And_ټول.دوهم_2 += ارزښت.دوهم_2;
    };
    لكل (ثابت خپلکاری &Stat : شمېر_And_ټول_فی_Name)
      combineStat(Stat);
    لكل (ثابت وخت_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت خپلکاری &Stat : TTP->شمېر_And_ټول_فی_Name)
        combineStat(Stat);

    std::ویکټور<Name_And_شمېر_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_شمېر_And_ټول_فی_Name.اندازه_2());
    لكل (ثابت خپلکاری &ټول : All_شمېر_And_ټول_فی_Name)
      SortedTotals.emplace_شا(std::string(ټول.get_کلی()), ټول.get_ارزښت());

    llvm::ترتیبول(SortedTotals, [](ثابت Name_And_شمېر_And_Duration_Type &A,
                                ثابت Name_And_شمېر_And_Duration_Type &B) {
      أرجع A.دوهم_2.دوهم_2 > B.دوهم_2.دوهم_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t ټول_Tid = MaxTid + 1;
    لكل (ثابت Name_And_شمېر_And_Duration_Type &ټول : SortedTotals) {
      خپلکاری DurUs = duration_cast<microseconds>(ټول.دوهم_2.دوهم_2).شمېر();
      خپلکاری شمېر_2 = All_شمېر_And_ټول_فی_Name[ټول.لومړی_2].لومړی_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(ټول_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + ټول.لومړی_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(شمېر_2));
          J.attribute("avg ms", int64_t(DurUs / شمېر_2 / 1000));
        });
      });

      ++ټول_Tid;
    }

    خپلکاری writeMetadataEvent = [&](ثابت حرف *Name, uint64_t Tid,
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
    لكل (ثابت وخت_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_پای_ټکی();
    J.attribute_پای_ټکی();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                وخت_ټکی_cast<microseconds>(Beginning_د_وخت)
                    .وخت_since_epoch()
                    .شمېر());

    J.object_پای_ټکی();
  }

  Small_ویکټور<std::unique_ptr<دننه_پرمختګ_ثبت>, 16> ډېری;
  Small_ویکټور<وخت_Trace_Profiler_ثبت, 128> Entries;
  StringMap<شمېر_And_Duration_Type> شمېر_And_ټول_فی_Name;
  // System clock time when the session was begun.
  ثابت وخت_ټکی_2<system_clock> Beginning_د_وخت;
  // Profiling clock time when the session was begun.
  ثابت وخت_ټکی_Type پیل_وخت;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت بینښه وخت_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقي وخت_Trace_Verbose;

  // Compression mode for output.
  وخت_Trace_Compression وخت_Trace_Compress;
};

منطقي llvm::is_وخت_Trace_Verbose() {
  أرجع get_وخت_Trace_Profiler_Instance() &&
         get_وخت_Trace_Profiler_Instance()->وخت_Trace_Verbose;
}

وخت_Trace_Compression llvm::infer_وخت_Trace_Compression_From_لار(StringRef لار) {
  StringRef Ext = llvm::sys::لار_2::extension(لار);
  إذا (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    أرجع وخت_Trace_Compression::Zstd;
  أرجع وخت_Trace_Compression::None;
}

فراغ llvm::وخت_Trace_Profiler_Initialize(بینښه وخت_Trace_Granularity,
                                       StringRef ProcName,
                                       منطقي وخت_Trace_Verbose,
                                       وخت_Trace_Compression وخت_Trace_Compress) {
  أكد(وخت_Trace_Profiler_Instance == تش_اشاره &&
         "Profiler should not be initialized");
  وخت_Trace_Profiler_Instance = جديد وخت_Trace_Profiler(
      وخت_Trace_Granularity, llvm::sys::لار_2::filename(ProcName),
      وخت_Trace_Verbose, وخت_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
فراغ llvm::وخت_Trace_Profiler_Cleanup() {
  ړنګول وخت_Trace_Profiler_Instance;
  وخت_Trace_Profiler_Instance = تش_اشاره;

  خپلکاری &Instances = get_وخت_Trace_Profiler_Instances();
  std::قلف_guard<std::mutex> قلف_2(Instances.قلف_2);
  لكل (خپلکاری *TTP : Instances.List)
    ړنګول TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
فراغ llvm::وخت_Trace_Profiler_پای_Thread() {
  خپلکاری &Instances = get_وخت_Trace_Profiler_Instances();
  std::قلف_guard<std::mutex> قلف_2(Instances.قلف_2);
  Instances.List.ټېل_شا(وخت_Trace_Profiler_Instance);
  وخت_Trace_Profiler_Instance = تش_اشاره;
}

فراغ llvm::وخت_Trace_Profiler_Write(raw_pwrite_بهیر &OS) {
  أكد(وخت_Trace_Profiler_Instance != تش_اشاره &&
         "Profiler object can't be null");
  وخت_Trace_Compression CompressType =
      وخت_Trace_Profiler_Instance->وخت_Trace_Compress;
  إذا (CompressType != وخت_Trace_Compression::Zstd) {
    وخت_Trace_Profiler_Instance->اكتب(OS);
    أرجع;
  }

  compression::Format F = compression::Format::Zstd;
  إذا (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_ویکټور<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    وخت_Trace_Profiler_Instance->اكتب(UncompressedOS);
  }
  Small_ویکټور<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.معلومات()),
               Uncompressed.اندازه_2()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.معلومات()),
           Compressed.اندازه_2());
}

Error llvm::وخت_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(وخت_Trace_Profiler_Instance != تش_اشاره &&
         "Profiler object can't be null");

  وخت_Trace_Compression CompressType =
      وخت_Trace_Profiler_Instance->وخت_Trace_Compress;
  std::string لار = PreferredFileName.str();
  إذا (لار.تشه()) {
    لار = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    لار += وخت_Trace_File_Extension;
    إذا (CompressType == وخت_Trace_Compression::Zstd)
      لار += ".zst";
  } وإلا إذا (CompressType == وخت_Trace_Compression::Infer) {
    CompressType = infer_وخت_Trace_Compression_From_لار(لار);
  }

  إذا (CompressType == وخت_Trace_Compression::Zstd) {
    إذا (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      أرجع createStringError(inconvertible_Error_کوډ(), Reason);
  }

  std::error_کوډ EC;
  sys::fs::OpenFlags Flags = CompressType == وخت_Trace_Compression::Zstd
                                 ? sys::fs::د_None
                                 : sys::fs::د_متن_With_CRLF;
  raw_fd_ostream OS(لار, EC, Flags);
  إذا (EC)
    أرجع createStringError(EC, "Could not open " + لار);

  وخت_Trace_Compression SavedCompress =
      وخت_Trace_Profiler_Instance->وخت_Trace_Compress;
  وخت_Trace_Profiler_Instance->وخت_Trace_Compress = CompressType;
  وخت_Trace_Profiler_Write(OS);
  وخت_Trace_Profiler_Instance->وخت_Trace_Compress = SavedCompress;
  أرجع Error::success();
}

وخت_Trace_Profiler_ثبت *llvm::وخت_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    أرجع وخت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        وخت_Trace_Event_Type::CompleteEvent);
  أرجع تش_اشاره;
}

وخت_Trace_Profiler_ثبت *
llvm::وخت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    أرجع وخت_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            وخت_Trace_Event_Type::CompleteEvent);
  أرجع تش_اشاره;
}

وخت_Trace_Profiler_ثبت *
llvm::وخت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<وخت_Trace_Metadata()> Metadata) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    أرجع وخت_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            وخت_Trace_Event_Type::CompleteEvent);
  أرجع تش_اشاره;
}

وخت_Trace_Profiler_ثبت *llvm::وخت_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    أرجع وخت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        وخت_Trace_Event_Type::AsyncEvent);
  أرجع تش_اشاره;
}

فراغ llvm::وخت_Trace_جمع_کړه_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    وخت_Trace_Profiler_Instance->ورننباسه(std::string(Name), Detail);
}

فراغ llvm::وخت_Trace_Profiler_پای_ټکی() {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    وخت_Trace_Profiler_Instance->پای_ټکی();
}

فراغ llvm::وخت_Trace_Profiler_پای_ټکی(وخت_Trace_Profiler_ثبت *E) {
  إذا (وخت_Trace_Profiler_Instance != تش_اشاره)
    وخت_Trace_Profiler_Instance->پای_ټکی(*E);
}
