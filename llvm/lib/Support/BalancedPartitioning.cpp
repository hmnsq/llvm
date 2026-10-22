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

#অন্তর্ভাব "llvm/Support/BalancedPartitioning.h"
#অন্তর্ভাব "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Format.h"
#অন্তর্ভাব "llvm/Support/FormatVariadic.h"
#অন্তর্ভাব "llvm/Support/ThreadPool.h"

#অন্তর্ভাব <cmath>

using নামাকাশ llvm;
#define DEBUG_TYPE "balanced-partitioning"

শূন্য BPFunction_গ্রন্থি::dump(raw_ostream &OS) ধ্রুব {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
শূন্য BalancedPartitioning::BPThreadPool::async(Func &&F) {
#যদি LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++অঙ্ক_Active_Threads;
  TheThreadPool.async([এতত্, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    যদি (--অঙ্ক_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        প্রতিজ্ঞা(!IsFinishedSpawning);
        IsFinishedSpawning = সত্য;
      }
      cv.notify_one();
    }
  });
#অন্যথা
  llvm_unreachable("threads are disabled");
#endif
}

শূন্য BalancedPartitioning::BPThreadPool::wait() {
#যদি LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { প্রত্যাগম IsFinishedSpawning; });
    প্রতিজ্ঞা(IsFinishedSpawning && অঙ্ক_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#অন্যথা
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ধ্রুব BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  প্রতি (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

শূন্য BalancedPartitioning::run(std::vector<BPFunction_গ্রন্থি> &Nodes) ধ্রুব {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#যদি LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  যদি (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  প্রতি (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].প্রবেশ_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [এতত্, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  যদি (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } অন্যথা {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ধ্রুব auto &L, ধ্রুব auto &R) {
    প্রত্যাগম L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

শূন্য BalancedPartitioning::bisect(ধ্রুব Function_গ্রন্থি_Range Nodes,
                                  unsigned RecDepth, unsigned মূল_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ধ্রুব {
  unsigned অঙ্ক_Nodes = llvm::size(Nodes);
  যদি (অঙ্ক_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ধ্রুব auto &L, ধ্রুব auto &R) {
      প্রত্যাগম L.প্রবেশ_Order_Index < R.প্রবেশ_Order_Index;
    });
    প্রতি (auto &N : Nodes)
      N.Bucket = Offset++;
    প্রত্যাগম;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              অঙ্ক_Nodes, মূল_Bucket));

  std::mt19937 RNG(মূল_Bucket);

  unsigned বাম_Bucket = 2 * মূল_Bucket;
  unsigned ডান_Bucket = 2 * মূল_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, বাম_Bucket);

  runIterations(Nodes, বাম_Bucket, ডান_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { প্রত্যাগম N.Bucket == বাম_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto বাম_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto ডান_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto বাম_Rec_Task = [এতত্, বাম_Nodes, RecDepth, বাম_Bucket, Offset, &TP]() {
    bisect(বাম_Nodes, RecDepth + 1, বাম_Bucket, Offset, TP);
  };
  auto ডান_Rec_Task = [এতত্, ডান_Nodes, RecDepth, ডান_Bucket, MidOffset,
                       &TP]() {
    bisect(ডান_Nodes, RecDepth + 1, ডান_Bucket, MidOffset, TP);
  };

  যদি (TP && RecDepth < Config.TaskSplitDepth && অঙ্ক_Nodes >= 4) {
    TP->async(std::move(বাম_Rec_Task));
    TP->async(std::move(ডান_Rec_Task));
  } অন্যথা {
    বাম_Rec_Task();
    ডান_Rec_Task();
  }
}

শূন্য BalancedPartitioning::runIterations(ধ্রুব Function_গ্রন্থি_Range Nodes,
                                         unsigned বাম_Bucket,
                                         unsigned ডান_Bucket,
                                         std::mt19937 &RNG) ধ্রুব {
  unsigned অঙ্ক_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_গ্রন্থি::Utility_গ্রন্থি_T, unsigned> Utility_গ্রন্থি_Index;
  প্রতি (auto &N : Nodes)
    প্রতি (auto &UN : N.UtilityNodes)
      ++Utility_গ্রন্থি_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  প্রতি (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_গ্রন্থি_Index[UN];
      প্রত্যাগম UNI == 1 || UNI == অঙ্ক_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_গ্রন্থি_Index.clear();
  প্রতি (auto &N : Nodes)
    প্রতি (auto &UN : N.UtilityNodes)
      UN = Utility_গ্রন্থি_Index.ঢোকাও({UN, Utility_গ্রন্থি_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_গ্রন্থি_Index.size());
  প্রতি (auto &N : Nodes) {
    প্রতি (auto &UN : N.UtilityNodes) {
      প্রতিজ্ঞা(UN < Signatures.size());
      যদি (N.Bucket == বাম_Bucket) {
        Signatures[UN].বাম_গুনতি++;
      } অন্যথা {
        Signatures[UN].ডান_গুনতি++;
      }
    }
  }

  প্রতি (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned অঙ্ক_Moved_Nodes =
        runIteration(Nodes, বাম_Bucket, ডান_Bucket, Signatures, RNG);
    যদি (অঙ্ক_Moved_Nodes == 0)
      ভঙ্গ;
  }
}

unsigned BalancedPartitioning::runIteration(ধ্রুব Function_গ্রন্থি_Range Nodes,
                                            unsigned বাম_Bucket,
                                            unsigned ডান_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ধ্রুব {
  // Init signature cost caches
  প্রতি (auto &Signature : Signatures) {
    যদি (Signature.CachedGainIsValid)
      অনুবর্ত;
    unsigned L = Signature.বাম_গুনতি;
    unsigned R = Signature.ডান_গুনতি;
    প্রতিজ্ঞা((L > 0 || R > 0) && "incorrect signature");
    ভিন্ন Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    যদি (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    যদি (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = সত্য;
  }

  // Compute move gains
  using GainPair = std::pair<ভিন্ন, BPFunction_গ্রন্থি *>;
  std::vector<GainPair> Gains;
  প্রতি (auto &N : Nodes) {
    তার্কিক From_বাম_To_ডান = (N.Bucket == বাম_Bucket);
    ভিন্ন Gain = moveGain(N, From_বাম_To_ডান, Signatures);
    Gains.ঠেলো_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto বাম_End = llvm::partition(
      Gains, [&](ধ্রুব auto &GP) { প্রত্যাগম GP.second->Bucket == বাম_Bucket; });
  auto বাম_Range = llvm::make_range(Gains.begin(), বাম_End);
  auto ডান_Range = llvm::make_range(বাম_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ধ্রুব auto &L, ধ্রুব auto &R) {
    প্রত্যাগম L.first > R.first;
  };
  llvm::stable_sort(বাম_Range, LargerGain);
  llvm::stable_sort(ডান_Range, LargerGain);

  unsigned অঙ্ক_Moved_Data_Vertices = 0;
  প্রতি (auto [বাম_Pair, ডান_Pair] : llvm::zip(বাম_Range, ডান_Range)) {
    auto &[বাম_Gain, বাম_গ্রন্থি] = বাম_Pair;
    auto &[ডান_Gain, ডান_গ্রন্থি] = ডান_Pair;
    // Stop when the gain is no longer beneficial
    যদি (বাম_Gain + ডান_Gain <= 0.f)
      ভঙ্গ;
    // Try to exchange the nodes between buckets
    যদি (move_Function_গ্রন্থি(*বাম_গ্রন্থি, বাম_Bucket, ডান_Bucket, Signatures, RNG))
      ++অঙ্ক_Moved_Data_Vertices;
    যদি (move_Function_গ্রন্থি(*ডান_গ্রন্থি, বাম_Bucket, ডান_Bucket, Signatures, RNG))
      ++অঙ্ক_Moved_Data_Vertices;
  }
  প্রত্যাগম অঙ্ক_Moved_Data_Vertices;
}

তার্কিক BalancedPartitioning::move_Function_গ্রন্থি(BPFunction_গ্রন্থি &N,
                                            unsigned বাম_Bucket,
                                            unsigned ডান_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ধ্রুব {
  // Sometimes we skip the move. This helps to escape local optima
  যদি (std::uniform_real_distribution<ভিন্ন>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    প্রত্যাগম অসত্য;

  তার্কিক From_বাম_To_ডান = (N.Bucket == বাম_Bucket);
  // Update the current bucket
  N.Bucket = (From_বাম_To_ডান ? ডান_Bucket : বাম_Bucket);

  // Update signatures and invalidate gain cache
  যদি (From_বাম_To_ডান) {
    প্রতি (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.বাম_গুনতি--;
      Signature.ডান_গুনতি++;
      Signature.CachedGainIsValid = অসত্য;
    }
  } অন্যথা {
    প্রতি (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.বাম_গুনতি++;
      Signature.ডান_গুনতি--;
      Signature.CachedGainIsValid = অসত্য;
    }
  }
  প্রত্যাগম সত্য;
}

শূন্য BalancedPartitioning::split(ধ্রুব Function_গ্রন্থি_Range Nodes,
                                 unsigned StartBucket) ধ্রুব {
  unsigned অঙ্ক_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (অঙ্ক_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    প্রত্যাগম L.প্রবেশ_Order_Index < R.প্রবেশ_Order_Index;
  });

  প্রতি (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  প্রতি (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

ভিন্ন BalancedPartitioning::moveGain(ধ্রুব BPFunction_গ্রন্থি &N,
                                     তার্কিক From_বাম_To_ডান,
                                     ধ্রুব SignaturesT &Signatures) {
  ভিন্ন Gain = 0.f;
  প্রতি (auto &UN : N.UtilityNodes)
    Gain += (From_বাম_To_ডান ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  প্রত্যাগম Gain;
}

ভিন্ন BalancedPartitioning::logCost(unsigned X, unsigned Y) ধ্রুব {
  প্রত্যাগম -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

ভিন্ন BalancedPartitioning::log2Cached(unsigned i) ধ্রুব {
  প্রত্যাগম (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
