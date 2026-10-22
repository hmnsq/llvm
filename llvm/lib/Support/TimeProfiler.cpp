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

ورتوں نطاق llvm;

نطاق {

ورتوں std::chrono::duration;
ورتوں std::chrono::duration_cast;
ورتوں std::chrono::microseconds;
ورتوں std::chrono::steady_clock;
ورتوں std::chrono::system_clock;
ورتوں std::chrono::ویلا_نقطہ_2;
ورتوں std::chrono::ویلا_نقطہ_cast;

بنية ویلا_Trace_Profiler_Instances {
  std::mutex جندرا_2;
  std::سمتیہ<ویلا_Trace_Profiler *> List;
};

ویلا_Trace_Profiler_Instances &get_ویلا_Trace_Profiler_Instances() {
  ساكن ویلا_Trace_Profiler_Instances Instances;
  أرجع Instances;
}

ساكن ثابت حرف ویلا_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساكن LLVM_THREAD_LOCAL ویلا_Trace_Profiler *ویلا_Trace_Profiler_Instance = خالی_سوچک;

ویلا_Trace_Profiler *llvm::get_ویلا_Trace_Profiler_Instance() {
  أرجع ویلا_Trace_Profiler_Instance;
}

نطاق {

ورتوں ClockType = steady_clock;
ورتوں ویلا_نقطہ_Type = ویلا_نقطہ_2<ClockType>;
ورتوں DurationType = duration<ClockType::rep, ClockType::period>;
ورتوں گنتی_And_Duration_Type = std::pair<size_t, DurationType>;
ورتوں Name_And_گنتی_And_Duration_Type =
    std::pair<std::string, گنتی_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
بنية llvm::ویلا_Trace_Profiler_اندراج {
  ثابت ویلا_نقطہ_Type شروع;
  ویلا_نقطہ_Type سرا_2;
  ثابت std::string Name;
  ویلا_Trace_Metadata Metadata;

  ثابت ویلا_Trace_Event_Type EventType = ویلا_Trace_Event_Type::CompleteEvent;
  ویلا_Trace_Profiler_اندراج(ویلا_نقطہ_Type &&S, ویلا_نقطہ_Type &&E, std::string &&N,
                         std::string &&Dt, ویلا_Trace_Event_Type Et)
      : شروع(std::move(S)), سرا_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  ویلا_Trace_Profiler_اندراج(ویلا_نقطہ_Type &&S, ویلا_نقطہ_Type &&E, std::string &&N,
                         ویلا_Trace_Metadata &&Mt, ویلا_Trace_Event_Type Et)
      : شروع(std::move(S)), سرا_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_گراف_شروع_Us(ویلا_نقطہ_Type شروع_ویلا) ثابت {
    أرجع (ویلا_نقطہ_cast<microseconds>(شروع) -
            ویلا_نقطہ_cast<microseconds>(شروع_ویلا))
        .گنتی();
  }

  ClockType::rep get_Flame_گراف_Dur_Us() ثابت {
    أرجع (ویلا_نقطہ_cast<microseconds>(سرا_2) -
            ویلا_نقطہ_cast<microseconds>(شروع))
        .گنتی();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
بنية وچ_ترقی_اندراج {
  ویلا_Trace_Profiler_اندراج Event;
  std::سمتیہ<ویلا_Trace_Profiler_اندراج> InstantEvents;

  وچ_ترقی_اندراج(ویلا_نقطہ_Type S, ویلا_نقطہ_Type E, std::string N,
                  std::string Dt, ویلا_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  وچ_ترقی_اندراج(ویلا_نقطہ_Type S, ویلا_نقطہ_Type E, std::string N,
                  ویلا_Trace_Metadata Mt, ویلا_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

بنية llvm::ویلا_Trace_Profiler {
  ویلا_Trace_Profiler(
      بنانشانی ویلا_Trace_Granularity = 0, StringRef ProcName = "",
      منطقي ویلا_Trace_Verbose = خطأ,
      ویلا_Trace_Compression ویلا_Trace_Compress = ویلا_Trace_Compression::Infer)
      : Beginning_دا_ویلا(system_clock::now()), شروع_ویلا(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), ویلا_Trace_Granularity(ویلا_Trace_Granularity),
        ویلا_Trace_Verbose(ویلا_Trace_Verbose),
        ویلا_Trace_Compress(ویلا_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  ویلا_Trace_Profiler_اندراج *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        ویلا_Trace_Event_Type EventType = ویلا_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != ویلا_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڈھیر.emplace_پٹھ(std::make_unique<وچ_ترقی_اندراج>(
        ClockType::now(), ویلا_نقطہ_Type(), std::move(Name), Detail(),
        EventType));
    أرجع &ڈھیر.پٹھ()->Event;
  }

  ویلا_Trace_Profiler_اندراج *
  begin(std::string Name, llvm::function_ref<ویلا_Trace_Metadata()> Metadata,
        ویلا_Trace_Event_Type EventType = ویلا_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != ویلا_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڈھیر.emplace_پٹھ(std::make_unique<وچ_ترقی_اندراج>(
        ClockType::now(), ویلا_نقطہ_Type(), std::move(Name), Metadata(),
        EventType));
    أرجع &ڈھیر.پٹھ()->Event;
  }

  فراغ داخل_کرو(std::string Name, llvm::function_ref<std::string()> Detail) {
    إذا (ڈھیر.خالی_تھاواں())
      أرجع;

    ڈھیر.پٹھ()->InstantEvents.emplace_پٹھ(ویلا_Trace_Profiler_اندراج(
        ClockType::now(), ویلا_نقطہ_Type(), std::move(Name), Detail(),
        ویلا_Trace_Event_Type::InstantEvent));
  }

  فراغ سرا() {
    أكد(!ڈھیر.خالی_تھاواں() && "Must call begin() first");
    سرا(ڈھیر.پٹھ()->Event);
  }

  فراغ سرا(ویلا_Trace_Profiler_اندراج &E) {
    أكد(!ڈھیر.خالی_تھاواں() && "Must call begin() first");
    E.سرا_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.سرا_2 - E.شروع;

    ثابت خودکار *Iter =
        llvm::لبھو_if(ڈھیر, [&](ثابت std::unique_ptr<وچ_ترقی_اندراج> &Val) {
          أرجع &Val->Event == &E;
        });
    أكد(Iter != ڈھیر.سرا() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    إذا (duration_cast<microseconds>(Duration).گنتی() >= ویلا_Trace_Granularity) {
      Entries.emplace_پٹھ(E);
      لكل (خودکار &IE : Iter->get()->InstantEvents) {
        Entries.emplace_پٹھ(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    إذا (llvm::none_دا(llvm::drop_begin(llvm::الٹ(ڈھیر)),
                      [&](ثابت std::unique_ptr<وچ_ترقی_اندراج> &Val) {
                        أرجع Val->Event.Name == E.Name;
                      })) {
      خودکار &گنتی_And_کل = گنتی_And_کل_فی_Name[E.Name];
      گنتی_And_کل.پہلا_2++;
      گنتی_And_کل.دوجا_2 += Duration;
    };

    ڈھیر.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  فراغ اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    خودکار &Instances = get_ویلا_Trace_Profiler_Instances();
    std::جندرا_guard<std::mutex> جندرا_2(Instances.جندرا_2);
    أكد(ڈھیر.خالی_تھاواں() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_دا(Instances.List,
                        [](ثابت خودکار &TTP) { أرجع TTP->ڈھیر.خالی_تھاواں(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    خودکار writeEvent = [&](ثابت خودکار &E, uint64_t Tid) {
      خودکار شروع_Us = E.get_Flame_گراف_شروع_Us(شروع_ویلا);
      خودکار DurUs = E.get_Flame_گراف_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", شروع_Us);
        إذا (E.EventType == ویلا_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } وإلا إذا (E.EventType == ویلا_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } وإلا { // instant event
          أكد(E.EventType == ویلا_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        إذا (!E.Metadata.is_خالی_تھاواں()) {
          J.attributeObject("args", [&] {
            إذا (!E.Metadata.Detail.خالی_تھاواں())
              J.attribute("detail", E.Metadata.Detail);
            إذا (!E.Metadata.File.خالی_تھاواں())
              J.attribute("file", E.Metadata.File);
            إذا (E.Metadata.سطر > 0)
              J.attribute("line", E.Metadata.سطر);
          });
        }
      });

      إذا (E.EventType == ویلا_Trace_Event_Type::AsyncEvent) {
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
    لكل (ثابت ویلا_Trace_Profiler_اندراج &E : Entries)
      writeEvent(E, هذا->Tid);
    لكل (ثابت ویلا_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت ویلا_Trace_Profiler_اندراج &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = هذا->Tid;
    لكل (ثابت ویلا_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<گنتی_And_Duration_Type> All_گنتی_And_کل_فی_Name;
    خودکار combineStat = [&](ثابت خودکار &Stat) {
      StringRef کلید = Stat.get_کلید();
      خودکار قدر = Stat.get_قدر();
      خودکار &گنتی_And_کل = All_گنتی_And_کل_فی_Name[کلید];
      گنتی_And_کل.پہلا_2 += قدر.پہلا_2;
      گنتی_And_کل.دوجا_2 += قدر.دوجا_2;
    };
    لكل (ثابت خودکار &Stat : گنتی_And_کل_فی_Name)
      combineStat(Stat);
    لكل (ثابت ویلا_Trace_Profiler *TTP : Instances.List)
      لكل (ثابت خودکار &Stat : TTP->گنتی_And_کل_فی_Name)
        combineStat(Stat);

    std::سمتیہ<Name_And_گنتی_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_گنتی_And_کل_فی_Name.ناپ_2());
    لكل (ثابت خودکار &کل : All_گنتی_And_کل_فی_Name)
      SortedTotals.emplace_پٹھ(std::string(کل.get_کلید()), کل.get_قدر());

    llvm::ترتیب_دیو(SortedTotals, [](ثابت Name_And_گنتی_And_Duration_Type &A,
                                ثابت Name_And_گنتی_And_Duration_Type &B) {
      أرجع A.دوجا_2.دوجا_2 > B.دوجا_2.دوجا_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t کل_Tid = MaxTid + 1;
    لكل (ثابت Name_And_گنتی_And_Duration_Type &کل : SortedTotals) {
      خودکار DurUs = duration_cast<microseconds>(کل.دوجا_2.دوجا_2).گنتی();
      خودکار گنتی_2 = All_گنتی_And_کل_فی_Name[کل.پہلا_2].پہلا_2;

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(کل_Tid));
        J.attribute("ph", "X");
        J.attribute("ts", 0);
        J.attribute("dur", DurUs);
        J.attribute("name", "Total " + کل.پہلا_2);
        J.attributeObject("args", [&] {
          J.attribute("count", int64_t(گنتی_2));
          J.attribute("avg ms", int64_t(DurUs / گنتی_2 / 1000));
        });
      });

      ++کل_Tid;
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
    لكل (ثابت ویلا_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_سرا();
    J.attribute_سرا();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                ویلا_نقطہ_cast<microseconds>(Beginning_دا_ویلا)
                    .ویلا_since_epoch()
                    .گنتی());

    J.object_سرا();
  }

  Small_سمتیہ<std::unique_ptr<وچ_ترقی_اندراج>, 16> ڈھیر;
  Small_سمتیہ<ویلا_Trace_Profiler_اندراج, 128> Entries;
  StringMap<گنتی_And_Duration_Type> گنتی_And_کل_فی_Name;
  // System clock time when the session was begun.
  ثابت ویلا_نقطہ_2<system_clock> Beginning_دا_ویلا;
  // Profiling clock time when the session was begun.
  ثابت ویلا_نقطہ_Type شروع_ویلا;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت بنانشانی ویلا_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقي ویلا_Trace_Verbose;

  // Compression mode for output.
  ویلا_Trace_Compression ویلا_Trace_Compress;
};

منطقي llvm::is_ویلا_Trace_Verbose() {
  أرجع get_ویلا_Trace_Profiler_Instance() &&
         get_ویلا_Trace_Profiler_Instance()->ویلا_Trace_Verbose;
}

ویلا_Trace_Compression llvm::infer_ویلا_Trace_Compression_From_رستہ(StringRef رستہ) {
  StringRef Ext = llvm::sys::رستہ_2::extension(رستہ);
  إذا (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    أرجع ویلا_Trace_Compression::Zstd;
  أرجع ویلا_Trace_Compression::None;
}

فراغ llvm::ویلا_Trace_Profiler_Initialize(بنانشانی ویلا_Trace_Granularity,
                                       StringRef ProcName,
                                       منطقي ویلا_Trace_Verbose,
                                       ویلا_Trace_Compression ویلا_Trace_Compress) {
  أكد(ویلا_Trace_Profiler_Instance == خالی_سوچک &&
         "Profiler should not be initialized");
  ویلا_Trace_Profiler_Instance = جديد ویلا_Trace_Profiler(
      ویلا_Trace_Granularity, llvm::sys::رستہ_2::filename(ProcName),
      ویلا_Trace_Verbose, ویلا_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
فراغ llvm::ویلا_Trace_Profiler_Cleanup() {
  مٹاؤ ویلا_Trace_Profiler_Instance;
  ویلا_Trace_Profiler_Instance = خالی_سوچک;

  خودکار &Instances = get_ویلا_Trace_Profiler_Instances();
  std::جندرا_guard<std::mutex> جندرا_2(Instances.جندرا_2);
  لكل (خودکار *TTP : Instances.List)
    مٹاؤ TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
فراغ llvm::ویلا_Trace_Profiler_انت_Thread() {
  خودکار &Instances = get_ویلا_Trace_Profiler_Instances();
  std::جندرا_guard<std::mutex> جندرا_2(Instances.جندرا_2);
  Instances.List.ڈالو_پٹھ(ویلا_Trace_Profiler_Instance);
  ویلا_Trace_Profiler_Instance = خالی_سوچک;
}

فراغ llvm::ویلا_Trace_Profiler_Write(raw_pwrite_دھارا &OS) {
  أكد(ویلا_Trace_Profiler_Instance != خالی_سوچک &&
         "Profiler object can't be null");
  ویلا_Trace_Compression CompressType =
      ویلا_Trace_Profiler_Instance->ویلا_Trace_Compress;
  إذا (CompressType != ویلا_Trace_Compression::Zstd) {
    ویلا_Trace_Profiler_Instance->اكتب(OS);
    أرجع;
  }

  compression::Format F = compression::Format::Zstd;
  إذا (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_سمتیہ<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    ویلا_Trace_Profiler_Instance->اكتب(UncompressedOS);
  }
  Small_سمتیہ<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.انکڑے()),
               Uncompressed.ناپ_2()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.انکڑے()),
           Compressed.ناپ_2());
}

Error llvm::ویلا_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(ویلا_Trace_Profiler_Instance != خالی_سوچک &&
         "Profiler object can't be null");

  ویلا_Trace_Compression CompressType =
      ویلا_Trace_Profiler_Instance->ویلا_Trace_Compress;
  std::string رستہ = PreferredFileName.str();
  إذا (رستہ.خالی_تھاواں()) {
    رستہ = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    رستہ += ویلا_Trace_File_Extension;
    إذا (CompressType == ویلا_Trace_Compression::Zstd)
      رستہ += ".zst";
  } وإلا إذا (CompressType == ویلا_Trace_Compression::Infer) {
    CompressType = infer_ویلا_Trace_Compression_From_رستہ(رستہ);
  }

  إذا (CompressType == ویلا_Trace_Compression::Zstd) {
    إذا (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      أرجع createStringError(inconvertible_Error_کوڈ(), Reason);
  }

  std::error_کوڈ EC;
  sys::fs::OpenFlags Flags = CompressType == ویلا_Trace_Compression::Zstd
                                 ? sys::fs::دا_None
                                 : sys::fs::دا_متن_With_CRLF;
  raw_fd_ostream OS(رستہ, EC, Flags);
  إذا (EC)
    أرجع createStringError(EC, "Could not open " + رستہ);

  ویلا_Trace_Compression SavedCompress =
      ویلا_Trace_Profiler_Instance->ویلا_Trace_Compress;
  ویلا_Trace_Profiler_Instance->ویلا_Trace_Compress = CompressType;
  ویلا_Trace_Profiler_Write(OS);
  ویلا_Trace_Profiler_Instance->ویلا_Trace_Compress = SavedCompress;
  أرجع Error::success();
}

ویلا_Trace_Profiler_اندراج *llvm::ویلا_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    أرجع ویلا_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        ویلا_Trace_Event_Type::CompleteEvent);
  أرجع خالی_سوچک;
}

ویلا_Trace_Profiler_اندراج *
llvm::ویلا_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    أرجع ویلا_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            ویلا_Trace_Event_Type::CompleteEvent);
  أرجع خالی_سوچک;
}

ویلا_Trace_Profiler_اندراج *
llvm::ویلا_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<ویلا_Trace_Metadata()> Metadata) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    أرجع ویلا_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            ویلا_Trace_Event_Type::CompleteEvent);
  أرجع خالی_سوچک;
}

ویلا_Trace_Profiler_اندراج *llvm::ویلا_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    أرجع ویلا_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { أرجع std::string(Detail); },
        ویلا_Trace_Event_Type::AsyncEvent);
  أرجع خالی_سوچک;
}

فراغ llvm::ویلا_Trace_جوڑو_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    ویلا_Trace_Profiler_Instance->داخل_کرو(std::string(Name), Detail);
}

فراغ llvm::ویلا_Trace_Profiler_سرا() {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    ویلا_Trace_Profiler_Instance->سرا();
}

فراغ llvm::ویلا_Trace_Profiler_سرا(ویلا_Trace_Profiler_اندراج *E) {
  إذا (ویلا_Trace_Profiler_Instance != خالی_سوچک)
    ویلا_Trace_Profiler_Instance->سرا(*E);
}
