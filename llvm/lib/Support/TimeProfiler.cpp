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

#شامل "llvm/Support/TimeProfiler.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/STLFunctionalExtras.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/Support/Compression.h"
#شامل "llvm/Support/JSON.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Threading.h"
#شامل <algorithm>
#شامل <cassert>
#شامل <chrono>
#شامل <memory>
#شامل <mutex>
#شامل <string>
#شامل <vector>

استعمال نامخلا llvm;

نامخلا {

استعمال std::chrono::duration;
استعمال std::chrono::duration_cast;
استعمال std::chrono::microseconds;
استعمال std::chrono::steady_clock;
استعمال std::chrono::system_clock;
استعمال std::chrono::وقت_نقطہ_2;
استعمال std::chrono::وقت_نقطہ_cast;

ساخت وقت_Trace_Profiler_Instances {
  std::mutex تالا_3;
  std::سمتیہ<وقت_Trace_Profiler *> List;
};

وقت_Trace_Profiler_Instances &get_وقت_Trace_Profiler_Instances() {
  ساکن وقت_Trace_Profiler_Instances Instances;
  واپس Instances;
}

ساکن ثابت حرف وقت_Trace_File_Extension[] = ".time-trace.json";

} // anonymous namespace

// Per Thread instance
ساکن LLVM_THREAD_LOCAL وقت_Trace_Profiler *وقت_Trace_Profiler_Instance = خالی_اشاریہ;

وقت_Trace_Profiler *llvm::get_وقت_Trace_Profiler_Instance() {
  واپس وقت_Trace_Profiler_Instance;
}

نامخلا {

استعمال ClockType = steady_clock;
استعمال وقت_نقطہ_Type = وقت_نقطہ_2<ClockType>;
استعمال DurationType = duration<ClockType::rep, ClockType::period>;
استعمال گنتی_And_Duration_Type = std::pair<size_t, DurationType>;
استعمال Name_And_گنتی_And_Duration_Type =
    std::pair<std::string, گنتی_And_Duration_Type>;

} // anonymous namespace

/// Represents an open or completed time section entry to be captured.
ساخت llvm::وقت_Trace_Profiler_اندراج {
  ثابت وقت_نقطہ_Type آغاز;
  وقت_نقطہ_Type انتہا_2;
  ثابت std::string Name;
  وقت_Trace_Metadata Metadata;

  ثابت وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent;
  وقت_Trace_Profiler_اندراج(وقت_نقطہ_Type &&S, وقت_نقطہ_Type &&E, std::string &&N,
                         std::string &&Dt, وقت_Trace_Event_Type Et)
      : آغاز(std::move(S)), انتہا_2(std::move(E)), Name(std::move(N)), Metadata(),
        EventType(Et) {
    Metadata.Detail = std::move(Dt);
  }

  وقت_Trace_Profiler_اندراج(وقت_نقطہ_Type &&S, وقت_نقطہ_Type &&E, std::string &&N,
                         وقت_Trace_Metadata &&Mt, وقت_Trace_Event_Type Et)
      : آغاز(std::move(S)), انتہا_2(std::move(E)), Name(std::move(N)),
        Metadata(std::move(Mt)), EventType(Et) {}

  // Calculate timings for FlameGraph. Cast time points to microsecond precision
  // rather than casting duration. This avoids truncation issues causing inner
  // scopes overruning outer scopes.
  ClockType::rep get_Flame_گراف_آغاز_Us(وقت_نقطہ_Type آغاز_وقت) ثابت {
    واپس (وقت_نقطہ_cast<microseconds>(آغاز) -
            وقت_نقطہ_cast<microseconds>(آغاز_وقت))
        .گنتی();
  }

  ClockType::rep get_Flame_گراف_Dur_Us() ثابت {
    واپس (وقت_نقطہ_cast<microseconds>(انتہا_2) -
            وقت_نقطہ_cast<microseconds>(آغاز))
        .گنتی();
  }
};

// Represents a currently open (in-progress) time trace entry. InstantEvents
// that happen during an open event are associated with the duration of this
// parent event and they are dropped if parent duration is shorter than
// the granularity.
ساخت اندر_پیش_رفت_اندراج {
  وقت_Trace_Profiler_اندراج Event;
  std::سمتیہ<وقت_Trace_Profiler_اندراج> InstantEvents;

  اندر_پیش_رفت_اندراج(وقت_نقطہ_Type S, وقت_نقطہ_Type E, std::string N,
                  std::string Dt, وقت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Dt), Et),
        InstantEvents() {}

  اندر_پیش_رفت_اندراج(وقت_نقطہ_Type S, وقت_نقطہ_Type E, std::string N,
                  وقت_Trace_Metadata Mt, وقت_Trace_Event_Type Et)
      : Event(std::move(S), std::move(E), std::move(N), std::move(Mt), Et),
        InstantEvents() {}
};

ساخت llvm::وقت_Trace_Profiler {
  وقت_Trace_Profiler(
      غیرعلامتی وقت_Trace_Granularity = 0, StringRef ProcName = "",
      منطقی وقت_Trace_Verbose = جھوٹ,
      وقت_Trace_Compression وقت_Trace_Compress = وقت_Trace_Compression::Infer)
      : Beginning_کا_وقت(system_clock::now()), آغاز_وقت(ClockType::now()),
        ProcName(ProcName), Pid(sys::Process::getProcessId()),
        Tid(llvm::get_threadid()), وقت_Trace_Granularity(وقت_Trace_Granularity),
        وقت_Trace_Verbose(وقت_Trace_Verbose),
        وقت_Trace_Compress(وقت_Trace_Compress) {
    llvm::get_thread_name(ThreadName);
  }

  وقت_Trace_Profiler_اندراج *
  begin(std::string Name, llvm::function_ref<std::string()> Detail,
        وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وقت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڈھیر.emplace_عقب(std::make_unique<اندر_پیش_رفت_اندراج>(
        ClockType::now(), وقت_نقطہ_Type(), std::move(Name), Detail(),
        EventType));
    واپس &ڈھیر.عقب()->Event;
  }

  وقت_Trace_Profiler_اندراج *
  begin(std::string Name, llvm::function_ref<وقت_Trace_Metadata()> Metadata,
        وقت_Trace_Event_Type EventType = وقت_Trace_Event_Type::CompleteEvent) {
    أكد(EventType != وقت_Trace_Event_Type::InstantEvent &&
           "Instant Events don't have begin and end.");
    ڈھیر.emplace_عقب(std::make_unique<اندر_پیش_رفت_اندراج>(
        ClockType::now(), وقت_نقطہ_Type(), std::move(Name), Metadata(),
        EventType));
    واپس &ڈھیر.عقب()->Event;
  }

  خالی داخل_کرو(std::string Name, llvm::function_ref<std::string()> Detail) {
    اگر (ڈھیر.خالی_خانے())
      واپس;

    ڈھیر.عقب()->InstantEvents.emplace_عقب(وقت_Trace_Profiler_اندراج(
        ClockType::now(), وقت_نقطہ_Type(), std::move(Name), Detail(),
        وقت_Trace_Event_Type::InstantEvent));
  }

  خالی انتہا() {
    أكد(!ڈھیر.خالی_خانے() && "Must call begin() first");
    انتہا(ڈھیر.عقب()->Event);
  }

  خالی انتہا(وقت_Trace_Profiler_اندراج &E) {
    أكد(!ڈھیر.خالی_خانے() && "Must call begin() first");
    E.انتہا_2 = ClockType::now();

    // Calculate duration at full precision for overall counts.
    DurationType Duration = E.انتہا_2 - E.آغاز;

    ثابت خودکار *Iter =
        llvm::ڈھونڈو_if(ڈھیر, [&](ثابت std::unique_ptr<اندر_پیش_رفت_اندراج> &Val) {
          واپس &Val->Event == &E;
        });
    أكد(Iter != ڈھیر.انتہا() && "Event not in the Stack");

    // Only include sections longer or equal to TimeTraceGranularity msec.
    اگر (duration_cast<microseconds>(Duration).گنتی() >= وقت_Trace_Granularity) {
      Entries.emplace_عقب(E);
      برائے (خودکار &IE : Iter->get()->InstantEvents) {
        Entries.emplace_عقب(IE);
      }
    }

    // Track total time taken by each "name", but only the topmost levels of
    // them; e.g. if there's a template instantiation that instantiates other
    // templates from within, we only want to add the topmost one. "topmost"
    // happens to be the ones that don't have any currently open entries above
    // itself.
    اگر (llvm::none_کا(llvm::drop_begin(llvm::الٹ(ڈھیر)),
                      [&](ثابت std::unique_ptr<اندر_پیش_رفت_اندراج> &Val) {
                        واپس Val->Event.Name == E.Name;
                      })) {
      خودکار &گنتی_And_کل = گنتی_And_کل_فی_Name[E.Name];
      گنتی_And_کل.پہلا_2++;
      گنتی_And_کل.دوسرا_2 += Duration;
    };

    ڈھیر.erase(Iter);
  }

  // Write events from this TimeTraceProfilerInstance and
  // ThreadTimeTraceProfilerInstances.
  خالی اكتب(raw_ostream &OS) {
    // Acquire Mutex as reading ThreadTimeTraceProfilerInstances.
    خودکار &Instances = get_وقت_Trace_Profiler_Instances();
    std::تالا_guard<std::mutex> تالا_3(Instances.تالا_3);
    أكد(ڈھیر.خالی_خانے() &&
           "All profiler sections should be ended when calling write");
    أكد(llvm::all_کا(Instances.List,
                        [](ثابت خودکار &TTP) { واپس TTP->ڈھیر.خالی_خانے(); }) &&
           "All profiler sections should be ended when calling write");

    json::OStream J(OS);
    J.objectBegin();
    J.attributeBegin("traceEvents");
    J.arrayBegin();

    // Emit all events for the main flame graph.
    خودکار writeEvent = [&](ثابت خودکار &E, uint64_t Tid) {
      خودکار آغاز_Us = E.get_Flame_گراف_آغاز_Us(آغاز_وقت);
      خودکار DurUs = E.get_Flame_گراف_Dur_Us();

      J.object([&] {
        J.attribute("pid", Pid);
        J.attribute("tid", int64_t(Tid));
        J.attribute("ts", آغاز_Us);
        اگر (E.EventType == وقت_Trace_Event_Type::AsyncEvent) {
          J.attribute("cat", E.Name);
          J.attribute("ph", "b");
          J.attribute("id", 0);
        } ورنہ اگر (E.EventType == وقت_Trace_Event_Type::CompleteEvent) {
          J.attribute("ph", "X");
          J.attribute("dur", DurUs);
        } ورنہ { // instant event
          أكد(E.EventType == وقت_Trace_Event_Type::InstantEvent &&
                 "InstantEvent expected");
          J.attribute("ph", "i");
        }
        J.attribute("name", E.Name);
        اگر (!E.Metadata.is_خالی_خانے()) {
          J.attributeObject("args", [&] {
            اگر (!E.Metadata.Detail.خالی_خانے())
              J.attribute("detail", E.Metadata.Detail);
            اگر (!E.Metadata.File.خالی_خانے())
              J.attribute("file", E.Metadata.File);
            اگر (E.Metadata.سطر > 0)
              J.attribute("line", E.Metadata.سطر);
          });
        }
      });

      اگر (E.EventType == وقت_Trace_Event_Type::AsyncEvent) {
        J.object([&] {
          J.attribute("pid", Pid);
          J.attribute("tid", int64_t(Tid));
          J.attribute("ts", آغاز_Us + DurUs);
          J.attribute("cat", E.Name);
          J.attribute("ph", "e");
          J.attribute("id", 0);
          J.attribute("name", E.Name);
        });
      }
    };
    برائے (ثابت وقت_Trace_Profiler_اندراج &E : Entries)
      writeEvent(E, یہ->Tid);
    برائے (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      برائے (ثابت وقت_Trace_Profiler_اندراج &E : TTP->Entries)
        writeEvent(E, TTP->Tid);

    // Emit totals by section name as additional "thread" events, sorted from
    // longest one.
    // Find highest used thread id.
    uint64_t MaxTid = یہ->Tid;
    برائے (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      MaxTid = std::max(MaxTid, TTP->Tid);

    // Combine all CountAndTotalPerName from threads into one.
    StringMap<گنتی_And_Duration_Type> All_گنتی_And_کل_فی_Name;
    خودکار combineStat = [&](ثابت خودکار &Stat) {
      StringRef کلید = Stat.get_کلید();
      خودکار قدر = Stat.get_قدر();
      خودکار &گنتی_And_کل = All_گنتی_And_کل_فی_Name[کلید];
      گنتی_And_کل.پہلا_2 += قدر.پہلا_2;
      گنتی_And_کل.دوسرا_2 += قدر.دوسرا_2;
    };
    برائے (ثابت خودکار &Stat : گنتی_And_کل_فی_Name)
      combineStat(Stat);
    برائے (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      برائے (ثابت خودکار &Stat : TTP->گنتی_And_کل_فی_Name)
        combineStat(Stat);

    std::سمتیہ<Name_And_گنتی_And_Duration_Type> SortedTotals;
    SortedTotals.reserve(All_گنتی_And_کل_فی_Name.سائز_2());
    برائے (ثابت خودکار &کل : All_گنتی_And_کل_فی_Name)
      SortedTotals.emplace_عقب(std::string(کل.get_کلید()), کل.get_قدر());

    llvm::ترتیب_دو(SortedTotals, [](ثابت Name_And_گنتی_And_Duration_Type &A,
                                ثابت Name_And_گنتی_And_Duration_Type &B) {
      واپس A.دوسرا_2.دوسرا_2 > B.دوسرا_2.دوسرا_2;
    });

    // Report totals on separate threads of tracing file.
    uint64_t کل_Tid = MaxTid + 1;
    برائے (ثابت Name_And_گنتی_And_Duration_Type &کل : SortedTotals) {
      خودکار DurUs = duration_cast<microseconds>(کل.دوسرا_2.دوسرا_2).گنتی();
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
    برائے (ثابت وقت_Trace_Profiler *TTP : Instances.List)
      writeMetadataEvent("thread_name", TTP->Tid, TTP->ThreadName);

    J.array_انتہا();
    J.attribute_انتہا();

    // Emit the absolute time when this TimeProfiler started.
    // This can be used to combine the profiling data from
    // multiple processes and preserve actual time intervals.
    J.attribute("beginningOfTime",
                وقت_نقطہ_cast<microseconds>(Beginning_کا_وقت)
                    .وقت_since_epoch()
                    .گنتی());

    J.object_انتہا();
  }

  Small_سمتیہ<std::unique_ptr<اندر_پیش_رفت_اندراج>, 16> ڈھیر;
  Small_سمتیہ<وقت_Trace_Profiler_اندراج, 128> Entries;
  StringMap<گنتی_And_Duration_Type> گنتی_And_کل_فی_Name;
  // System clock time when the session was begun.
  ثابت وقت_نقطہ_2<system_clock> Beginning_کا_وقت;
  // Profiling clock time when the session was begun.
  ثابت وقت_نقطہ_Type آغاز_وقت;
  ثابت std::string ProcName;
  ثابت sys::Process::Pid Pid;
  SmallString<0> ThreadName;
  ثابت uint64_t Tid;

  // Minimum time granularity (in microseconds)
  ثابت غیرعلامتی وقت_Trace_Granularity;

  // Make time trace capture verbose event details (e.g. source filenames). This
  // can increase the size of the output by 2-3 times.
  ثابت منطقی وقت_Trace_Verbose;

  // Compression mode for output.
  وقت_Trace_Compression وقت_Trace_Compress;
};

منطقی llvm::is_وقت_Trace_Verbose() {
  واپس get_وقت_Trace_Profiler_Instance() &&
         get_وقت_Trace_Profiler_Instance()->وقت_Trace_Verbose;
}

وقت_Trace_Compression llvm::infer_وقت_Trace_Compression_From_راستہ(StringRef راستہ) {
  StringRef Ext = llvm::sys::راستہ_2::extension(راستہ);
  اگر (Ext.equals_insensitive(".zst") || Ext.equals_insensitive(".zstd"))
    واپس وقت_Trace_Compression::Zstd;
  واپس وقت_Trace_Compression::None;
}

خالی llvm::وقت_Trace_Profiler_Initialize(غیرعلامتی وقت_Trace_Granularity,
                                       StringRef ProcName,
                                       منطقی وقت_Trace_Verbose,
                                       وقت_Trace_Compression وقت_Trace_Compress) {
  أكد(وقت_Trace_Profiler_Instance == خالی_اشاریہ &&
         "Profiler should not be initialized");
  وقت_Trace_Profiler_Instance = نیا وقت_Trace_Profiler(
      وقت_Trace_Granularity, llvm::sys::راستہ_2::filename(ProcName),
      وقت_Trace_Verbose, وقت_Trace_Compress);
}

// Removes all TimeTraceProfilerInstances.
// Called from main thread.
خالی llvm::وقت_Trace_Profiler_Cleanup() {
  حذف وقت_Trace_Profiler_Instance;
  وقت_Trace_Profiler_Instance = خالی_اشاریہ;

  خودکار &Instances = get_وقت_Trace_Profiler_Instances();
  std::تالا_guard<std::mutex> تالا_3(Instances.تالا_3);
  برائے (خودکار *TTP : Instances.List)
    حذف TTP;
  Instances.List.clear();
}

// Finish TimeTraceProfilerInstance on a worker thread.
// This doesn't remove the instance, just moves the pointer to global vector.
خالی llvm::وقت_Trace_Profiler_اختتام_Thread() {
  خودکار &Instances = get_وقت_Trace_Profiler_Instances();
  std::تالا_guard<std::mutex> تالا_3(Instances.تالا_3);
  Instances.List.ڈالو_عقب(وقت_Trace_Profiler_Instance);
  وقت_Trace_Profiler_Instance = خالی_اشاریہ;
}

خالی llvm::وقت_Trace_Profiler_Write(raw_pwrite_دھارا &OS) {
  أكد(وقت_Trace_Profiler_Instance != خالی_اشاریہ &&
         "Profiler object can't be null");
  وقت_Trace_Compression CompressType =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  اگر (CompressType != وقت_Trace_Compression::Zstd) {
    وقت_Trace_Profiler_Instance->اكتب(OS);
    واپس;
  }

  compression::Format F = compression::Format::Zstd;
  اگر (ثابت حرف *Reason = compression::getReasonIfUnsupported(F))
    report_fatal_error(Reason);

  Small_سمتیہ<حرف, 0> Uncompressed;
  {
    raw_svector_ostream UncompressedOS(Uncompressed);
    وقت_Trace_Profiler_Instance->اكتب(UncompressedOS);
  }
  Small_سمتیہ<uint8_t, 0> Compressed;
  compression::compress(
      compression::Params(F),
      ArrayRef(reinterpret_cast<ثابت uint8_t *>(Uncompressed.ڈیٹا()),
               Uncompressed.سائز_2()),
      Compressed);
  OS.اكتب(reinterpret_cast<ثابت حرف *>(Compressed.ڈیٹا()),
           Compressed.سائز_2());
}

Error llvm::وقت_Trace_Profiler_Write(StringRef PreferredFileName,
                                   StringRef FallbackFileName) {
  أكد(وقت_Trace_Profiler_Instance != خالی_اشاریہ &&
         "Profiler object can't be null");

  وقت_Trace_Compression CompressType =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  std::string راستہ = PreferredFileName.str();
  اگر (راستہ.خالی_خانے()) {
    راستہ = FallbackFileName == "-" ? "out" : FallbackFileName.str();
    راستہ += وقت_Trace_File_Extension;
    اگر (CompressType == وقت_Trace_Compression::Zstd)
      راستہ += ".zst";
  } ورنہ اگر (CompressType == وقت_Trace_Compression::Infer) {
    CompressType = infer_وقت_Trace_Compression_From_راستہ(راستہ);
  }

  اگر (CompressType == وقت_Trace_Compression::Zstd) {
    اگر (ثابت حرف *Reason =
            compression::getReasonIfUnsupported(compression::Format::Zstd))
      واپس createStringError(inconvertible_Error_رمز(), Reason);
  }

  std::error_رمز EC;
  sys::fs::OpenFlags Flags = CompressType == وقت_Trace_Compression::Zstd
                                 ? sys::fs::کا_None
                                 : sys::fs::کا_متن_With_CRLF;
  raw_fd_ostream OS(راستہ, EC, Flags);
  اگر (EC)
    واپس createStringError(EC, "Could not open " + راستہ);

  وقت_Trace_Compression SavedCompress =
      وقت_Trace_Profiler_Instance->وقت_Trace_Compress;
  وقت_Trace_Profiler_Instance->وقت_Trace_Compress = CompressType;
  وقت_Trace_Profiler_Write(OS);
  وقت_Trace_Profiler_Instance->وقت_Trace_Compress = SavedCompress;
  واپس Error::success();
}

وقت_Trace_Profiler_اندراج *llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                                                     StringRef Detail) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    واپس وقت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { واپس std::string(Detail); },
        وقت_Trace_Event_Type::CompleteEvent);
  واپس خالی_اشاریہ;
}

وقت_Trace_Profiler_اندراج *
llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<std::string()> Detail) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    واپس وقت_Trace_Profiler_Instance->begin(std::string(Name), Detail,
                                            وقت_Trace_Event_Type::CompleteEvent);
  واپس خالی_اشاریہ;
}

وقت_Trace_Profiler_اندراج *
llvm::وقت_Trace_Profiler_Begin(StringRef Name,
                             llvm::function_ref<وقت_Trace_Metadata()> Metadata) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    واپس وقت_Trace_Profiler_Instance->begin(std::string(Name), Metadata,
                                            وقت_Trace_Event_Type::CompleteEvent);
  واپس خالی_اشاریہ;
}

وقت_Trace_Profiler_اندراج *llvm::وقت_Trace_Async_Profiler_Begin(StringRef Name,
                                                          StringRef Detail) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    واپس وقت_Trace_Profiler_Instance->begin(
        std::string(Name), [&]() { واپس std::string(Detail); },
        وقت_Trace_Event_Type::AsyncEvent);
  واپس خالی_اشاریہ;
}

خالی llvm::وقت_Trace_جوڑو_Instant_Event(StringRef Name,
                                    llvm::function_ref<std::string()> Detail) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    وقت_Trace_Profiler_Instance->داخل_کرو(std::string(Name), Detail);
}

خالی llvm::وقت_Trace_Profiler_انتہا() {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    وقت_Trace_Profiler_Instance->انتہا();
}

خالی llvm::وقت_Trace_Profiler_انتہا(وقت_Trace_Profiler_اندراج *E) {
  اگر (وقت_Trace_Profiler_Instance != خالی_اشاریہ)
    وقت_Trace_Profiler_Instance->انتہا(*E);
}
