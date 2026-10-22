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

فراغ BPFunction_كلي::dump(raw_ostream &OS) ثابت {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
فراغ BalancedPartitioning::BPThreadPool::async(Func &&F) {
#إذا LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++لمب_Active_Threads;
  TheThreadPool.async([هذا, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    إذا (--لمب_Active_Threads == 0) {
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
    أكد(IsFinishedSpawning && لمب_Active_Threads == 0);
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

فراغ BalancedPartitioning::run(std::vector<BPFunction_كلي> &Nodes) ثابت {
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
    Nodes[I].شغرو_Order_Index = I;

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

فراغ BalancedPartitioning::bisect(ثابت Function_كلي_Range Nodes,
                                  unsigned RecDepth, unsigned سيو_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ثابت {
  unsigned لمب_Nodes = llvm::size(Nodes);
  إذا (لمب_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ثابت auto &L, ثابت auto &R) {
      أرجع L.شغرو_Order_Index < R.شغرو_Order_Index;
    });
    لكل (auto &N : Nodes)
      N.Bucket = Offset++;
    أرجع;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              لمب_Nodes, سيو_Bucket));

  std::mt19937 RNG(سيو_Bucket);

  unsigned هغو_Bucket = 2 * سيو_Bucket;
  unsigned دام_Bucket = 2 * سيو_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, هغو_Bucket);

  runIterations(Nodes, هغو_Bucket, دام_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { أرجع N.Bucket == هغو_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto هغو_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto دام_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto هغو_Rec_Task = [هذا, هغو_Nodes, RecDepth, هغو_Bucket, Offset, &TP]() {
    bisect(هغو_Nodes, RecDepth + 1, هغو_Bucket, Offset, TP);
  };
  auto دام_Rec_Task = [هذا, دام_Nodes, RecDepth, دام_Bucket, MidOffset,
                       &TP]() {
    bisect(دام_Nodes, RecDepth + 1, دام_Bucket, MidOffset, TP);
  };

  إذا (TP && RecDepth < Config.TaskSplitDepth && لمب_Nodes >= 4) {
    TP->async(std::move(هغو_Rec_Task));
    TP->async(std::move(دام_Rec_Task));
  } وإلا {
    هغو_Rec_Task();
    دام_Rec_Task();
  }
}

فراغ BalancedPartitioning::runIterations(ثابت Function_كلي_Range Nodes,
                                         unsigned هغو_Bucket,
                                         unsigned دام_Bucket,
                                         std::mt19937 &RNG) ثابت {
  unsigned لمب_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_كلي::Utility_كلي_T, unsigned> Utility_كلي_Index;
  لكل (auto &N : Nodes)
    لكل (auto &UN : N.UtilityNodes)
      ++Utility_كلي_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  لكل (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_كلي_Index[UN];
      أرجع UNI == 1 || UNI == لمب_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_كلي_Index.clear();
  لكل (auto &N : Nodes)
    لكل (auto &UN : N.UtilityNodes)
      UN = Utility_كلي_Index.سك({UN, Utility_كلي_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_كلي_Index.size());
  لكل (auto &N : Nodes) {
    لكل (auto &UN : N.UtilityNodes) {
      أكد(UN < Signatures.size());
      إذا (N.Bucket == هغو_Bucket) {
        Signatures[UN].هغو_كدي++;
      } وإلا {
        Signatures[UN].دام_كدي++;
      }
    }
  }

  لكل (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned لمب_Moved_Nodes =
        runIteration(Nodes, هغو_Bucket, دام_Bucket, Signatures, RNG);
    إذا (لمب_Moved_Nodes == 0)
      اكسر;
  }
}

unsigned BalancedPartitioning::runIteration(ثابت Function_كلي_Range Nodes,
                                            unsigned هغو_Bucket,
                                            unsigned دام_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Init signature cost caches
  لكل (auto &Signature : Signatures) {
    إذا (Signature.CachedGainIsValid)
      استمر;
    unsigned L = Signature.هغو_كدي;
    unsigned R = Signature.دام_كدي;
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
  using GainPair = std::pair<عائم, BPFunction_كلي *>;
  std::vector<GainPair> Gains;
  لكل (auto &N : Nodes) {
    منطقي From_هغو_To_دام = (N.Bucket == هغو_Bucket);
    عائم Gain = moveGain(N, From_هغو_To_دام, Signatures);
    Gains.تور_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto هغو_End = llvm::partition(
      Gains, [&](ثابت auto &GP) { أرجع GP.second->Bucket == هغو_Bucket; });
  auto هغو_Range = llvm::make_range(Gains.begin(), هغو_End);
  auto دام_Range = llvm::make_range(هغو_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ثابت auto &L, ثابت auto &R) {
    أرجع L.first > R.first;
  };
  llvm::stable_sort(هغو_Range, LargerGain);
  llvm::stable_sort(دام_Range, LargerGain);

  unsigned لمب_Moved_Data_Vertices = 0;
  لكل (auto [هغو_Pair, دام_Pair] : llvm::zip(هغو_Range, دام_Range)) {
    auto &[هغو_Gain, هغو_كلي] = هغو_Pair;
    auto &[دام_Gain, دام_كلي] = دام_Pair;
    // Stop when the gain is no longer beneficial
    إذا (هغو_Gain + دام_Gain <= 0.f)
      اكسر;
    // Try to exchange the nodes between buckets
    إذا (move_Function_كلي(*هغو_كلي, هغو_Bucket, دام_Bucket, Signatures, RNG))
      ++لمب_Moved_Data_Vertices;
    إذا (move_Function_كلي(*دام_كلي, هغو_Bucket, دام_Bucket, Signatures, RNG))
      ++لمب_Moved_Data_Vertices;
  }
  أرجع لمب_Moved_Data_Vertices;
}

منطقي BalancedPartitioning::move_Function_كلي(BPFunction_كلي &N,
                                            unsigned هغو_Bucket,
                                            unsigned دام_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Sometimes we skip the move. This helps to escape local optima
  إذا (std::uniform_real_distribution<عائم>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    أرجع خطأ;

  منطقي From_هغو_To_دام = (N.Bucket == هغو_Bucket);
  // Update the current bucket
  N.Bucket = (From_هغو_To_دام ? دام_Bucket : هغو_Bucket);

  // Update signatures and invalidate gain cache
  إذا (From_هغو_To_دام) {
    لكل (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.هغو_كدي--;
      Signature.دام_كدي++;
      Signature.CachedGainIsValid = خطأ;
    }
  } وإلا {
    لكل (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.هغو_كدي++;
      Signature.دام_كدي--;
      Signature.CachedGainIsValid = خطأ;
    }
  }
  أرجع true;
}

فراغ BalancedPartitioning::split(ثابت Function_كلي_Range Nodes,
                                 unsigned StartBucket) ثابت {
  unsigned لمب_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (لمب_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    أرجع L.شغرو_Order_Index < R.شغرو_Order_Index;
  });

  لكل (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  لكل (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

عائم BalancedPartitioning::moveGain(ثابت BPFunction_كلي &N,
                                     منطقي From_هغو_To_دام,
                                     ثابت SignaturesT &Signatures) {
  عائم Gain = 0.f;
  لكل (auto &UN : N.UtilityNodes)
    Gain += (From_هغو_To_دام ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  أرجع Gain;
}

عائم BalancedPartitioning::logCost(unsigned X, unsigned Y) ثابت {
  أرجع -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

عائم BalancedPartitioning::log2Cached(unsigned i) ثابت {
  أرجع (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
