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

استفاده نطاق llvm;

نطاق {

استفاده std::chrono::duration;
استفاده std::chrono::duration_cast;
استفاده std::chrono::microseconds;
استفاده std::chrono::steady_clock;
استفاده std::chrono::system_clock;
استفاده std::chrono::زمان_نقطه_2;
استفاده std::chrono::زمان_نقطه_cast;

بنية زمان_Trace_Profiler_Instances {
  std::mutex قفل_3;
  std::وکتور<زمان_Trace_Profiler *> List;
};

زمان_Trace_Profiler_Instances &get_زمان_Trace_Profiler_Instances() {
  ساكن زمان_Trace_Profiler_Instances Instances;
  أرجع Instances;
}

ساكن ثابت حرف زمان_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساكن LLVM_THREAD_LOCAL زمان_Trace_Profiler *زمان_Trace_Profiler_Instance = اشاره_تهی;

زمان_Trace_Profiler *llvm::get_زمان_Trace_Profiler_Instance() {
  أرجع زمان_Trace_Profiler_Instance;
}

نطاق {

استفاده ClockType = steady_clock;
استفاده زمان_نقطه_Type = زمان_نقطه_2<ClockType>;
استفاده DurationType = duration<ClockType::rep, ClockType::period>;
استفاده شمار_And_Duration_Type = std::pair<size_t, DurationType>;
استفاده Name_And_شمار_And_Duration_Type =
    std::pair<std::string, شمار_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
بنية llvm::زمان_Trace_Profiler_مدخل {
  ثابت زمان_نقطه_Type شروع;
  زمان_نقطه_Type انتها_2;
  ثابت std::string Name;
  زمان_Trace_Metadata Metadata;

  ثابت زمان_Trace_Event_Type EventType = زمان_Trace_Event_Type::CompleteEvent;
  زمان_Trace_Profiler_مدخل(زمان_نقطه_Type &&S, زمان_نقطه_Type &&E, std::string &&N,
                         std::string &&Dt, زمان_Trace_Event_Type Et)
      : شروع(std::move(S)), انتها_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  زمان_Trace_Profiler_مدخل(زمان_نقطه_Type &&S, زمان_نقطه_Type &&E, std::string &&N,
                         زمان_Trace_Metadata &&Mt, زمان_Trace_Event_Type Et)
      : شروع(std::move(S)), انتها_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_گراف_شروع_Us(زمان_نقطه_Type شروع_زمان) ثابت {
    أرجع (زمان_نقطه_cast<microseconds>(شروع) -
            زمان_نقطه_cast<microseconds>(شروع_زمان))
        .شمار();
  }

  ClockType::rep get_Flame_گراف_Dur_Us() ثابت {
    أرجع (زمان_نقطه_cast<microseconds>(انتها_2) -
            زمان_نقطه_cast<microseconds>(شروع))
        .شمار();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
بنية درون_پیشرفت_مدخل {
  زمان_Trace_Profiler_مدخل Event;
  std::وکتور<زمان_Trace_Profiler_مدخل> InstantEvents;

  درون_پیشرفت_مدخل(زمان_نقطه_Type S, زمان_نقطه_Type E, std::string N,
                  std::string Dt, زمان_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  درون_پیشرفت_مدخل(زمان_نقطه_Type S, زمان_نقطه_Type E, std::string N,
                  زمان_Trace_Metadata Mt, زمان_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

بنية llvm::زمان_Trace_Profiler {
  زمان_Trace_Profiler(
      بیعلامت زمان_Trace_Granularity = 0, StringRef ProcName = "",
      منطقي زمان_Trace_Verbose = خطأ,
      زمان_Trace_Compression زمان_Trace_Compress = زمان_Trace_Compression::Infer)
      : Beginning_از_زمان(system_clock::now()), شروع_زمان(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), زمان_Trace_Granularity(زمان_Trace_Granularity),
        زمان_Trace_Verbose(زمان_Trace_Verbose),
        زمان_Trace_Compress(زمان_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  زمان_Trace_Profiler_مدخل *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        زمان_Trace_Event_Type EventType = زمان_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != زمان_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    پشته.emplace_پشت(std::make_unique<درون_پیشرفت_مدخل>(
        ClockType::now(), زمان_نقطه_Type(), std::move(Name), Detail(),
        EventType));
    أرجع &پشته.پشت()->Event;
  }

  زمان_Trace_Profiler_مدخل *
  begin(std::string Name, llvm::function_ref<زمان_Trace_Metadata()> Metadata,
        زمان_Trace_Event_Type EventType = زمان_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != زمان_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    پشته.emplace_پشت(std::make_unique<درون_پیشرفت_مدخل>(
        ClockType::now(), زمان_نقطه_Type(), std::move(Name), Metadata(),
        EventType));
    أرجع &پشته.پشت()->Event;
  }

  فراغ درج(std::string Name, llvm::function_ref<std::string()> Detail) {
    إذا (پشته.خالی())
      أرجع;

    پشته.پشت()->InstantEvents.emplace_پشت(زمان_Trace_Profiler_مدخل(
        ClockType::now(), زمان_نقطه_Type(), std::move(Name), Detail(),
        زمان_Trace_Event_Type::InstantEvent));
  }

  فراغ انتها() {
    أكد(!پشته.خالی() && "Must call begin() first");
    انتها(پشته.پشت()->Event);
  }

  فراغ انتها(زمان_Trace_Profiler_مدخل &E) {
    أكد(!پشته.خالی() && "Must call begin() first");
    E.انتها_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.انتها_2 - E.شروع;

    ثابت خودکار *Iter =
        llvm::پیدا_کن_if(پشته, [&](ثابت std::unique_ptr<درون_پیشرفت_مدخل> &Val) {
          أرجع &Val->Event == &E;
        });
    أكد(Iter != پشته.انتها() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    إذا (duration_cast<microseconds>(Duration).شمار() >= زمان_Trace_Granularity) {
      Entries.emplace_پشت(E);
      لكل (خودکار &IE : Iter->get()->InstantEvents) {
        Entries.emplace_پشت(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    إذا (llvm::none_از(llvm::drop_begin(llvm::معکوس(پشته)),
                      [&](ثابت std::unique_ptr<درون_پیشرفت_مدخل> &Val) {
                        أرجع Val->Event.Name == E.Name;
                      })) {
      خودکار &شمار_And_مجموع = شمار_And_مجموع_در_هر_Name[E.Name];
      شمار_And_مجموع.اولی_2++;
      شمار_And_مجموع.دومی_2 += Duration;
    };

    پشته.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  فراغ اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    خودکار &Instances = get_زمان_Trace_Profiler_Instances();
    std::قفل_guard<std::mutex> قفل_3(Instances.قفل_3);
    أكد(پشته.خالی() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_از(Instances.List,
                        [](ثابت خودکار &TTP) { أرجع TTP->پشته.خالی(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    خودکار writeEvent = [&](ثابت خودکار &E, uint64_t Tid) {
      خودکار شروع_Us = E.get_Flame_گراف_شروع_Us(شروع_زمان);
      خودکار DurUs = E.get_Flame_گراف_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", شروع_Us);
        إذا (E.EventType == زمان_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } وإلا إذا (E.EventType == زمان_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } وإلا { // instant event
          أكد(E.EventType == زمان_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        إذا (!E.Metadata.is_خالی()) {
          J.attributeObject("args", [&] {
            إذا (!E.Metadata.Detail.خالی())
              J.attribute("detail", E.Metadata.Detail);
            إذا (!E.Metadata.File.خالی())
              J.attribute("file", E.Metadata.File);
            إذا (E.Metadata.سطر > 0)
              J.attribute("line", E.Metadata.سطر);
          });
        }
      });

      إذا (E.EventType == زمان_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", شروع_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    لكل (ثابت زمان_Trace_Profiler_مدخل &E : Entries)
      writeEvent(E, هذا->Tid);
    لكل (ثابت زمان_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت زمان_Trace_Profiler_مدخل &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = هذا->Tid;
    لكل (ثابت زمان_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<شمار_And_Duration_Type> All_شمار_And_مجموع_در_هر_Name;
    خودکار combineStat = [&](ثابت خودکار &Stat) {
      StringRef کلید = Stat.get_کلید();
      خودکار مقدار = Stat.get_مقدار();
      خودکار &شمار_And_مجموع = All_شمار_And_مجموع_در_هر_Name[کلید];
      شمار_And_مجموع.اولی_2 += مقدار.اولی_2;
      شمار_And_مجموع.دومی_2 += مقدار.دومی_2;
    };
    لكل (ثابت خودکار &Stat : شمار_And_مجموع_در_هر_Name)
      combineStat(Stat);
    لكل (ثابت زمان_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت خودکار &Stat : TTP->شمار_And_مجموع_در_هر_Name)
        combineStat(Stat);

    std::وکتور<Name_And_شمار_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_شمار_And_مجموع_در_هر_Name.اندازه_2());
    لكل (ثابت خودکار &مجموع : All_شمار_And_مجموع_در_هر_Name)
      SortedTotals.emplace_پشت(std::string(مجموع.get_کلید()), مجموع.get_مقدار());

    llvm::مرتب_سازی(SortedTotals, [](ثابت Name_And_شمار_And_Duration_Type &A,
                                ثابت Name_And_شمار_And_Duration_Type &B) {
      أرجع A.دومی_2.دومی_2 > B.دومی_2.دومی_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t مجموع_Tid = MaxTid + 1;
    لكل (ثابت Name_And_شمار_And_Duration_Type &مجموع : SortedTotals) {
      خودکار DurUs = duration_cast<microseconds>(مجموع.دومی_2.دومی_2).شمار();
      خودکار شمار_2 = All_شمار_And_مجموع_در_هر_Name[مجموع.اولی_2].اولی_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(مجموع_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + مجموع.اولی_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(شمار_2));
          J.attribute("avg ms", int64_t(DurUs / شمار_2 / 1000));
        });
      });

      ++مجموع_Tid;
    }

    خودکار writeMetadataEvent = [&](ثابت حرف *Name, uint64_t Tid,
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
    لكل (ثابت زمان_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_انتها();
    J.attribute_انتها();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                زمان_نقطه_cast<microseconds>(Beginning_از_زمان)
                    .زمان_since_epoch()
                    .شمار());

    J.object_انتها();
  }

  Small_وکتور<std::unique_ptr<درون_پیشرفت_مدخل>, 16> پشته;
  Small_وکتور<زمان_Trace_Profiler_مدخل, 128> Entries;
  StringMap<شمار_And_Duration_Type> شمار_And_مجموع_در_هر_Name;
  // System clock time when the session was begun.
  ثابت زمان_نقطه_2<system_clock> Beginning_از_زمان;
  // Profiling clock time when the session was begun.
  ثابت زمان_نقطه_Type شروع_زمان;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت بیعلامت زمان_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقي زمان_Trace_Verbose;

  // Compression mode for output.
  زمان_Trace_Compression زمان_Trace_Compress;
};

منطقي llvm::is_زمان_Trace_Verbose() {
  أرجع get_زمان_Trace_Profiler_Instance() &&
         get_زمان_Trace_Profiler_Instance()->زمان_Trace_Verbose;
}

زمان_Trace_Compression llvm::infer_زمان_Trace_Compression_From_مسیر(StringRef مسیر) {
  StringRef Ext = llvm::sys::مسیر_2::extension(مسیر);
  إذا (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    أرجع زمان_Trace_Compression::Zstd;
  أرجع زمان_Trace_Compression::None;
}

فراغ llvm::زمان_Trace_Profiler_Initialize(بیعلامت زمان_Trace_Granularity,
                                       StringRef ProcName,
                                       منطقي زمان_Trace_Verbose,
                                       زمان_Trace_Compression زمان_Trace_Compress) {
  أكد(زمان_Trace_Profiler_Instance == اشاره_تهی &&
         "Profiler should not be initialized");
  زمان_Trace_Profiler_Instance = جديد زمان_Trace_Profiler(
      زمان_Trace_Granularity, llvm::sys::مسیر_2::filename(ProcName),
      زمان_Trace_Verbose, زمان_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
فراغ llvm::زمان_Trace_Profiler_Cleanup() {
  حذف زمان_Trace_Profiler_Instance;
  زمان_Trace_Profiler_Instance = اشاره_تهی;

  خودکار &Instances = get_زمان_Trace_Profiler_Instances();
  std::قفل_guard<std::mutex> قفل_3(Instances.قفل_3);
  لكل (خودکار *TTP : Instances.List)
    حذف TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
فراغ llvm::زمان_Trace_Profiler_پایان_Thread() {
  خودکار &Instances = get_زمان_Trace_Profiler_Instances();
  std::قفل_guard<std::mutex> قفل_3(Instances.قفل_3);
  Instances.List.افزودن_پشت(زمان_Trace_Profiler_Instance);
  زمان_Trace_Profiler_Instance = اشاره_تهی;
}

فراغ llvm::زمان_Trace_Profiler_Write(raw_pwrite_جریان &OS) {
  أكد(زمان_Trace_Profiler_Instance != اشاره_تهی &&
         "Profiler object can't be null");
  زمان_Trace_Compression CompressType =
      زمان_Trace_Profiler_Instance->زمان_Trace_Compress;
  إذا (CompressType != زمان_Trace_Compression::Zstd) {
    زمان_Trace_Profiler_Instance->اكتب(OS);
    أرجع;
  }

  compression::Format F = compression::Format::Zstd;
  إذا (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_وکتور<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    زمان_Trace_Profiler_Instance->اكتب(UncompressedOS);
  }
  Small_وکتور<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.داده()),
               Uncompressed.اندازه_2()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.داده()),
           Compressed.اندازه_2());
}

Error llvm::زمان_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(زمان_Trace_Profiler_Instance != اشاره_تهی &&
         "Profiler object can't be null");

  زمان_Trace_Compression CompressType =
      زمان_Trace_Profiler_Instance->زمان_Trace_Compress;
  std::string مسیر = PreferredFileName.str();
  إذا (مسیر.خالی()) {
    مسیر = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    مسیر += زمان_Trace_File_Extension;
    إذا (CompressType == زمان_Trace_Compression::Zstd)
      مسیر += ".zst";
  } وإلا إذا (CompressType == زمان_Trace_Compression::Infer) {
    CompressType = infer_زمان_Trace_Compression_From_مسیر(مسیر);
  }

  إذا (CompressType == زمان_Trace_Compression::Zstd) {
    إذا (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      أرجع createStringError(inconvertible_Error_کد(), Reason);
  }

  std::error_کد EC;
  sys::fs::OpenFlags Flags = CompressType == زمان_Trace_Compression::Zstd
                                 ? sys::fs::از_None
                                 : sys::fs::از_متن_With_CRLF;
  raw_fd_ostream OS(مسیر, EC, Flags);
  إذا (EC)
    أرجع createStringError(EC, "Could not open " + مسیر);

  زمان_Trace_Compression SavedCompress =
      زمان_Trace_Profiler_Instance->زمان_Trace_Compress;
  زمان_Trace_Profiler_Instance->زمان_Trace_Compress = CompressType;
  زمان_Trace_Profiler_Write(OS);
  زمان_Trace_Profiler_Instance->زمان_Trace_Compress = SavedCompress;
  أرجع Error::success();
}

زمان_Trace_Profiler_مدخل *llvm::زمان_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    أرجع زمان_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        زمان_Trace_Event_Type::CompleteEvent);
  أرجع اشاره_تهی;
}

زمان_Trace_Profiler_مدخل *
llvm::زمان_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    أرجع زمان_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            زمان_Trace_Event_Type::CompleteEvent);
  أرجع اشاره_تهی;
}

زمان_Trace_Profiler_مدخل *
llvm::زمان_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<زمان_Trace_Metadata()> Metadata) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    أرجع زمان_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            زمان_Trace_Event_Type::CompleteEvent);
  أرجع اشاره_تهی;
}

زمان_Trace_Profiler_مدخل *llvm::زمان_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    أرجع زمان_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        زمان_Trace_Event_Type::AsyncEvent);
  أرجع اشاره_تهی;
}

فراغ llvm::زمان_Trace_جمع_کن_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    زمان_Trace_Profiler_Instance->درج(std::string(Name), Detail);
}

فراغ llvm::زمان_Trace_Profiler_انتها() {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    زمان_Trace_Profiler_Instance->انتها();
}

فراغ llvm::زمان_Trace_Profiler_انتها(زمان_Trace_Profiler_مدخل *E) {
  إذا (زمان_Trace_Profiler_Instance != اشاره_تهی)
    زمان_Trace_Profiler_Instance->انتها(*E);
}
