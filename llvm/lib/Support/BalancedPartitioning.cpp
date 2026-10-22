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

using نطاق llvm;
#define DEBUG_TYPE "balanced-partitioning"

فراغ BPFunction_عقدة::dump(raw_ostream &OS) ثابت {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
فراغ BalancedPartitioning::BPThreadPool::async(Func &&F) {
#إذا LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++رقم_Active_Threads;
  TheThreadPool.async([هذا, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    إذا (--رقم_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        أكد(!IsFinishedSpawning);
        IsFinishedSpawning = true;
      }
      cv.notify_one();
    }
  });
#وإلا
  llvm_unreachable("threads are disabled");
#endif
}

فراغ BalancedPartitioning::BPThreadPool::wait() {
#إذا LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { أرجع IsFinishedSpawning; });
    أكد(IsFinishedSpawning && رقم_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#وإلا
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ثابت BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  لكل (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

فراغ BalancedPartitioning::run(std::vector<BPFunction_عقدة> &Nodes) ثابت {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#إذا LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  إذا (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  لكل (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].مدخل_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [هذا, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  إذا (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } وإلا {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ثابت auto &L, ثابت auto &R) {
    أرجع L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

فراغ BalancedPartitioning::bisect(ثابت Function_عقدة_Range Nodes,
                                  unsigned RecDepth, unsigned جذر_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  إذا (رقم_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ثابت auto &L, ثابت auto &R) {
      أرجع L.مدخل_Order_Index < R.مدخل_Order_Index;
    });
    لكل (auto &N : Nodes)
      N.Bucket = Offset++;
    أرجع;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              رقم_Nodes, جذر_Bucket));

  std::mt19937 RNG(جذر_Bucket);

  unsigned يسار_Bucket = 2 * جذر_Bucket;
  unsigned يمين_Bucket = 2 * جذر_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, يسار_Bucket);

  runIterations(Nodes, يسار_Bucket, يمين_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { أرجع N.Bucket == يسار_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto يسار_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto يمين_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto يسار_Rec_Task = [هذا, يسار_Nodes, RecDepth, يسار_Bucket, Offset, &TP]() {
    bisect(يسار_Nodes, RecDepth + 1, يسار_Bucket, Offset, TP);
  };
  auto يمين_Rec_Task = [هذا, يمين_Nodes, RecDepth, يمين_Bucket, MidOffset,
                       &TP]() {
    bisect(يمين_Nodes, RecDepth + 1, يمين_Bucket, MidOffset, TP);
  };

  إذا (TP && RecDepth < Config.TaskSplitDepth && رقم_Nodes >= 4) {
    TP->async(std::move(يسار_Rec_Task));
    TP->async(std::move(يمين_Rec_Task));
  } وإلا {
    يسار_Rec_Task();
    يمين_Rec_Task();
  }
}

فراغ BalancedPartitioning::runIterations(ثابت Function_عقدة_Range Nodes,
                                         unsigned يسار_Bucket,
                                         unsigned يمين_Bucket,
                                         std::mt19937 &RNG) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_عقدة::Utility_عقدة_T, unsigned> Utility_عقدة_Index;
  لكل (auto &N : Nodes)
    لكل (auto &UN : N.UtilityNodes)
      ++Utility_عقدة_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  لكل (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_عقدة_Index[UN];
      أرجع UNI == 1 || UNI == رقم_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_عقدة_Index.clear();
  لكل (auto &N : Nodes)
    لكل (auto &UN : N.UtilityNodes)
      UN = Utility_عقدة_Index.أدرج({UN, Utility_عقدة_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_عقدة_Index.size());
  لكل (auto &N : Nodes) {
    لكل (auto &UN : N.UtilityNodes) {
      أكد(UN < Signatures.size());
      إذا (N.Bucket == يسار_Bucket) {
        Signatures[UN].يسار_عدد++;
      } وإلا {
        Signatures[UN].يمين_عدد++;
      }
    }
  }

  لكل (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned رقم_Moved_Nodes =
        runIteration(Nodes, يسار_Bucket, يمين_Bucket, Signatures, RNG);
    إذا (رقم_Moved_Nodes == 0)
      اكسر;
  }
}

unsigned BalancedPartitioning::runIteration(ثابت Function_عقدة_Range Nodes,
                                            unsigned يسار_Bucket,
                                            unsigned يمين_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Init signature cost caches
  لكل (auto &Signature : Signatures) {
    إذا (Signature.CachedGainIsValid)
      استمر;
    unsigned L = Signature.يسار_عدد;
    unsigned R = Signature.يمين_عدد;
    أكد((L > 0 || R > 0) && "incorrect signature");
    عائم Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    إذا (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    إذا (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = true;
  }

  // Compute move gains
  using GainPair = std::pair<عائم, BPFunction_عقدة *>;
  std::vector<GainPair> Gains;
  لكل (auto &N : Nodes) {
    منطقي From_يسار_To_يمين = (N.Bucket == يسار_Bucket);
    عائم Gain = moveGain(N, From_يسار_To_يمين, Signatures);
    Gains.ادفع_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto يسار_End = llvm::partition(
      Gains, [&](ثابت auto &GP) { أرجع GP.second->Bucket == يسار_Bucket; });
  auto يسار_Range = llvm::make_range(Gains.begin(), يسار_End);
  auto يمين_Range = llvm::make_range(يسار_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ثابت auto &L, ثابت auto &R) {
    أرجع L.first > R.first;
  };
  llvm::stable_sort(يسار_Range, LargerGain);
  llvm::stable_sort(يمين_Range, LargerGain);

  unsigned رقم_Moved_Data_Vertices = 0;
  لكل (auto [يسار_Pair, يمين_Pair] : llvm::zip(يسار_Range, يمين_Range)) {
    auto &[يسار_Gain, يسار_عقدة] = يسار_Pair;
    auto &[يمين_Gain, يمين_عقدة] = يمين_Pair;
    // Stop when the gain is no longer beneficial
    إذا (يسار_Gain + يمين_Gain <= 0.f)
      اكسر;
    // Try to exchange the nodes between buckets
    إذا (move_Function_عقدة(*يسار_عقدة, يسار_Bucket, يمين_Bucket, Signatures, RNG))
      ++رقم_Moved_Data_Vertices;
    إذا (move_Function_عقدة(*يمين_عقدة, يسار_Bucket, يمين_Bucket, Signatures, RNG))
      ++رقم_Moved_Data_Vertices;
  }
  أرجع رقم_Moved_Data_Vertices;
}

منطقي BalancedPartitioning::move_Function_عقدة(BPFunction_عقدة &N,
                                            unsigned يسار_Bucket,
                                            unsigned يمين_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Sometimes we skip the move. This helps to escape local optima
  إذا (std::uniform_real_distribution<عائم>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    أرجع خطأ;

  منطقي From_يسار_To_يمين = (N.Bucket == يسار_Bucket);
  // Update the current bucket
  N.Bucket = (From_يسار_To_يمين ? يمين_Bucket : يسار_Bucket);

  // Update signatures and invalidate gain cache
  إذا (From_يسار_To_يمين) {
    لكل (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.يسار_عدد--;
      Signature.يمين_عدد++;
      Signature.CachedGainIsValid = خطأ;
    }
  } وإلا {
    لكل (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.يسار_عدد++;
      Signature.يمين_عدد--;
      Signature.CachedGainIsValid = خطأ;
    }
  }
  أرجع true;
}

فراغ BalancedPartitioning::split(ثابت Function_عقدة_Range Nodes,
                                 unsigned StartBucket) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (رقم_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    أرجع L.مدخل_Order_Index < R.مدخل_Order_Index;
  });

  لكل (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  لكل (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

عائم BalancedPartitioning::moveGain(ثابت BPFunction_عقدة &N,
                                     منطقي From_يسار_To_يمين,
                                     ثابت SignaturesT &Signatures) {
  عائم Gain = 0.f;
  لكل (auto &UN : N.UtilityNodes)
    Gain += (From_يسار_To_يمين ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  أرجع Gain;
}

عائم BalancedPartitioning::logCost(unsigned X, unsigned Y) ثابت {
  أرجع -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

عائم BalancedPartitioning::log2Cached(unsigned i) ثابت {
  أرجع (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
