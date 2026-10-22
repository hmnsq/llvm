//===- BalancedPartitioning.cpp -------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements BalancedPartitioning, a recursive balanced graph
// partitioning algorithm.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/BalancedPartitioning.h"
#ضمن "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/ThreadPool.h"

#ضمن <cmath>

استفاده نطاق llvm;
#تعریف_کن DEBUG_TYPE "balanced-partitioning"

فراغ BPFunction_گره::dump(raw_ostream &OS) ثابت {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.انتها()), سطل);
}

قالب <نام_نوع Func>
فراغ متوازن_Partitioning::BPThreadPool::async(Func &&F) {
#إذا LLVM_ENABLE_نخها
  // This new thread could spawn more threads, so mark it as active
  ++رقم_فعال_نخها;
  TheThreadPool.async([هذا, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    إذا (--رقم_فعال_نخها == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_قفل<std::mutex> قفل_2(mtx);
        أكد(!Is_پایانیافته_Spawning);
        Is_پایانیافته_Spawning = درست;
      }
      cv.notify_one();
    }
  });
#وإلا
  llvm_unreachable("threads are disabled");
#پایان_اگر
}

فراغ متوازن_Partitioning::BPThreadPool::wait() {
#إذا LLVM_ENABLE_نخها
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_قفل<std::mutex> قفل_2(mtx);
    cv.wait(قفل_2, [&]() { أرجع Is_پایانیافته_Spawning; });
    أكد(Is_پایانیافته_Spawning && رقم_فعال_نخها == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#وإلا
  llvm_unreachable("threads are disabled");
#پایان_اگر
}

متوازن_Partitioning::متوازن_Partitioning(
    ثابت متوازن_Partitioning_Config &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  لكل (بیعلامت I = 1; I < LOG_CACHE_اندازه; I++)
    Log2Cache[I] = std::log2(I);
}

فراغ متوازن_Partitioning::پیاپی(std::وکتور<BPFunction_گره> &Nodes) ثابت {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.اندازه_2(), Config.تقسیم_عمق, Config.Iterations_در_هر_تقسیم));
  std::optional<BPThreadPool> TP;
#إذا LLVM_ENABLE_نخها
  DefaultThreadPool TheThreadPool;
  إذا (Config.Task_تقسیم_عمق > 1)
    TP.emplace(TheThreadPool);
#پایان_اگر

  // Record the input order
  لكل (بیعلامت I = 0; I < Nodes.اندازه_2(); I++)
    Nodes[I].درون_داد_ترتیب_شاخص = I;

  خودکار NodesRange = llvm::make_range(Nodes.begin(), Nodes.انتها());
  خودکار BisectTask = [هذا, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  إذا (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } وإلا {
    BisectTask();
  }

  llvm::stable_مرتب_سازی(NodesRange, [](ثابت خودکار &L, ثابت خودکار &R) {
    أرجع L.سطل < R.سطل;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

فراغ متوازن_Partitioning::bisect(ثابت Function_گره_Range Nodes,
                                  بیعلامت Rec_عمق, بیعلامت ریشه_سطل,
                                  بیعلامت جابه_جایی,
                                  std::optional<BPThreadPool> &TP) ثابت {
  بیعلامت رقم_Nodes = llvm::اندازه_2(Nodes);
  إذا (رقم_Nodes <= 1 || Rec_عمق >= Config.تقسیم_عمق) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::مرتب_سازی(Nodes, [](ثابت خودکار &L, ثابت خودکار &R) {
      أرجع L.درون_داد_ترتیب_شاخص < R.درون_داد_ترتیب_شاخص;
    });
    لكل (خودکار &N : Nodes)
      N.سطل = جابه_جایی++;
    أرجع;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              رقم_Nodes, ریشه_سطل));

  std::mt19937 RNG(ریشه_سطل);

  بیعلامت چپ_سطل = 2 * ریشه_سطل;
  بیعلامت راست_سطل = 2 * ریشه_سطل + 1;

  // Split into two and assign to the left and right buckets
  تقسیم(Nodes, چپ_سطل);

  پیاپی_Iterations(Nodes, چپ_سطل, راست_سطل, RNG);

  // Split nodes wrt the resulting buckets
  خودکار NodesMid =
      llvm::افراز(Nodes, [&](خودکار &N) { أرجع N.سطل == چپ_سطل; });
  بیعلامت Mid_جابه_جایی = جابه_جایی + std::فاصله(Nodes.begin(), NodesMid);

  خودکار چپ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  خودکار راست_Nodes = llvm::make_range(NodesMid, Nodes.انتها());

  خودکار چپ_Rec_Task = [هذا, چپ_Nodes, Rec_عمق, چپ_سطل, جابه_جایی, &TP]() {
    bisect(چپ_Nodes, Rec_عمق + 1, چپ_سطل, جابه_جایی, TP);
  };
  خودکار راست_Rec_Task = [هذا, راست_Nodes, Rec_عمق, راست_سطل, Mid_جابه_جایی,
                       &TP]() {
    bisect(راست_Nodes, Rec_عمق + 1, راست_سطل, Mid_جابه_جایی, TP);
  };

  إذا (TP && Rec_عمق < Config.Task_تقسیم_عمق && رقم_Nodes >= 4) {
    TP->async(std::move(چپ_Rec_Task));
    TP->async(std::move(راست_Rec_Task));
  } وإلا {
    چپ_Rec_Task();
    راست_Rec_Task();
  }
}

فراغ متوازن_Partitioning::پیاپی_Iterations(ثابت Function_گره_Range Nodes,
                                         بیعلامت چپ_سطل,
                                         بیعلامت راست_سطل,
                                         std::mt19937 &RNG) ثابت {
  بیعلامت رقم_Nodes = llvm::اندازه_2(Nodes);
  DenseMap<BPFunction_گره::Utility_گره_T, بیعلامت> Utility_گره_شاخص;
  لكل (خودکار &N : Nodes)
    لكل (خودکار &UN : N.UtilityNodes)
      ++Utility_گره_شاخص[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  لكل (خودکار &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](خودکار &UN) {
      بیعلامت UNI = Utility_گره_شاخص[UN];
      أرجع UNI == 1 || UNI == رقم_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_گره_شاخص.clear();
  لكل (خودکار &N : Nodes)
    لكل (خودکار &UN : N.UtilityNodes)
      UN = Utility_گره_شاخص.درج({UN, Utility_گره_شاخص.اندازه_2()}).اولی_2->دومی_2;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_گره_شاخص.اندازه_2());
  لكل (خودکار &N : Nodes) {
    لكل (خودکار &UN : N.UtilityNodes) {
      أكد(UN < Signatures.اندازه_2());
      إذا (N.سطل == چپ_سطل) {
        Signatures[UN].چپ_شمار++;
      } وإلا {
        Signatures[UN].راست_شمار++;
      }
    }
  }

  لكل (بیعلامت I = 0; I < Config.Iterations_در_هر_تقسیم; I++) {
    بیعلامت رقم_Moved_Nodes =
        پیاپی_Iteration(Nodes, چپ_سطل, راست_سطل, Signatures, RNG);
    إذا (رقم_Moved_Nodes == 0)
      اكسر;
  }
}

بیعلامت متوازن_Partitioning::پیاپی_Iteration(ثابت Function_گره_Range Nodes,
                                            بیعلامت چپ_سطل,
                                            بیعلامت راست_سطل,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Init signature cost caches
  لكل (خودکار &Signature : Signatures) {
    إذا (Signature.CachedGainIsValid)
      استمر;
    بیعلامت L = Signature.چپ_شمار;
    بیعلامت R = Signature.راست_شمار;
    أكد((L > 0 || R > 0) && "incorrect signature");
    عائم هزینه = log_هزینه(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    إذا (L > 0)
      Signature.CachedGainLR = هزینه - log_هزینه(L - 1, R + 1);
    إذا (R > 0)
      Signature.CachedGainRL = هزینه - log_هزینه(L + 1, R - 1);
    Signature.CachedGainIsValid = درست;
  }

  // Compute move gains
  استفاده GainPair = std::pair<عائم, BPFunction_گره *>;
  std::وکتور<GainPair> Gains;
  لكل (خودکار &N : Nodes) {
    منطقي From_چپ_مقصد_راست = (N.سطل == چپ_سطل);
    عائم Gain = moveGain(N, From_چپ_مقصد_راست, Signatures);
    Gains.افزودن_پشت(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  خودکار چپ_انتها = llvm::افراز(
      Gains, [&](ثابت خودکار &GP) { أرجع GP.دومی_2->سطل == چپ_سطل; });
  خودکار چپ_Range = llvm::make_range(Gains.begin(), چپ_انتها);
  خودکار راست_Range = llvm::make_range(چپ_انتها, Gains.انتها());

  // Sort gains in descending order
  خودکار LargerGain = [](ثابت خودکار &L, ثابت خودکار &R) {
    أرجع L.اولی_2 > R.اولی_2;
  };
  llvm::stable_مرتب_سازی(چپ_Range, LargerGain);
  llvm::stable_مرتب_سازی(راست_Range, LargerGain);

  بیعلامت رقم_Moved_داده_Vertices = 0;
  لكل (خودکار [چپ_Pair, راست_Pair] : llvm::zip(چپ_Range, راست_Range)) {
    خودکار &[چپ_Gain, چپ_گره] = چپ_Pair;
    خودکار &[راست_Gain, راست_گره] = راست_Pair;
    // Stop when the gain is no longer beneficial
    إذا (چپ_Gain + راست_Gain <= 0.f)
      اكسر;
    // Try to exchange the nodes between buckets
    إذا (move_Function_گره(*چپ_گره, چپ_سطل, راست_سطل, Signatures, RNG))
      ++رقم_Moved_داده_Vertices;
    إذا (move_Function_گره(*راست_گره, چپ_سطل, راست_سطل, Signatures, RNG))
      ++رقم_Moved_داده_Vertices;
  }
  أرجع رقم_Moved_داده_Vertices;
}

منطقي متوازن_Partitioning::move_Function_گره(BPFunction_گره &N,
                                            بیعلامت چپ_سطل,
                                            بیعلامت راست_سطل,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Sometimes we skip the move. This helps to escape local optima
  إذا (std::یکنواخت_real_distribution<عائم>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    أرجع خطأ;

  منطقي From_چپ_مقصد_راست = (N.سطل == چپ_سطل);
  // Update the current bucket
  N.سطل = (From_چپ_مقصد_راست ? راست_سطل : چپ_سطل);

  // Update signatures and invalidate gain cache
  إذا (From_چپ_مقصد_راست) {
    لكل (خودکار &UN : N.UtilityNodes) {
      خودکار &Signature = Signatures[UN];
      Signature.چپ_شمار--;
      Signature.راست_شمار++;
      Signature.CachedGainIsValid = خطأ;
    }
  } وإلا {
    لكل (خودکار &UN : N.UtilityNodes) {
      خودکار &Signature = Signatures[UN];
      Signature.چپ_شمار++;
      Signature.راست_شمار--;
      Signature.CachedGainIsValid = خطأ;
    }
  }
  أرجع درست;
}

فراغ متوازن_Partitioning::تقسیم(ثابت Function_گره_Range Nodes,
                                 بیعلامت شروع_سطل) ثابت {
  بیعلامت رقم_Nodes = llvm::اندازه_2(Nodes);
  خودکار NodesMid = Nodes.begin() + (رقم_Nodes + 1) / 2;

  llvm::مرتب_سازی(Nodes, [](خودکار &L, خودکار &R) {
    أرجع L.درون_داد_ترتیب_شاخص < R.درون_داد_ترتیب_شاخص;
  });

  لكل (خودکار &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.سطل = شروع_سطل;
  لكل (خودکار &N : llvm::make_range(NodesMid, Nodes.انتها()))
    N.سطل = شروع_سطل + 1;
}

عائم متوازن_Partitioning::moveGain(ثابت BPFunction_گره &N,
                                     منطقي From_چپ_مقصد_راست,
                                     ثابت SignaturesT &Signatures) {
  عائم Gain = 0.f;
  لكل (خودکار &UN : N.UtilityNodes)
    Gain += (From_چپ_مقصد_راست ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  أرجع Gain;
}

عائم متوازن_Partitioning::log_هزینه(بیعلامت X, بیعلامت Y) ثابت {
  أرجع -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

عائم متوازن_Partitioning::log2Cached(بیعلامت i) ثابت {
  أرجع (i < LOG_CACHE_اندازه) ? Log2Cache[i] : std::log2(i);
}
