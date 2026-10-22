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

#شامل "llvm/Support/BalancedPartitioning.h"
#شامل "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/ThreadPool.h"

#شامل <cmath>

using فضاینام llvm;
#define DEBUG_TYPE "balanced-partitioning"

تهی BPFunction_گره::dump(raw_ostream &OS) ثابت {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
تهی BalancedPartitioning::BPThreadPool::async(Func &&F) {
#اگر LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++رقم_Active_Threads;
  TheThreadPool.async([این, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    اگر (--رقم_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        أكد(!IsFinishedSpawning);
        IsFinishedSpawning = درست;
      }
      cv.notify_one();
    }
  });
#وگرنه
  llvm_unreachable("threads are disabled");
#endif
}

تهی BalancedPartitioning::BPThreadPool::wait() {
#اگر LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { بازگشت IsFinishedSpawning; });
    أكد(IsFinishedSpawning && رقم_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#وگرنه
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ثابت BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  برای (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

تهی BalancedPartitioning::run(std::vector<BPFunction_گره> &Nodes) ثابت {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#اگر LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  اگر (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  برای (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].درون_داد_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [این, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  اگر (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } وگرنه {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ثابت auto &L, ثابت auto &R) {
    بازگشت L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

تهی BalancedPartitioning::bisect(ثابت Function_گره_Range Nodes,
                                  unsigned RecDepth, unsigned ریشه_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  اگر (رقم_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ثابت auto &L, ثابت auto &R) {
      بازگشت L.درون_داد_Order_Index < R.درون_داد_Order_Index;
    });
    برای (auto &N : Nodes)
      N.Bucket = Offset++;
    بازگشت;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              رقم_Nodes, ریشه_Bucket));

  std::mt19937 RNG(ریشه_Bucket);

  unsigned چپ_Bucket = 2 * ریشه_Bucket;
  unsigned راست_Bucket = 2 * ریشه_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, چپ_Bucket);

  runIterations(Nodes, چپ_Bucket, راست_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { بازگشت N.Bucket == چپ_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto چپ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto راست_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto چپ_Rec_Task = [این, چپ_Nodes, RecDepth, چپ_Bucket, Offset, &TP]() {
    bisect(چپ_Nodes, RecDepth + 1, چپ_Bucket, Offset, TP);
  };
  auto راست_Rec_Task = [این, راست_Nodes, RecDepth, راست_Bucket, MidOffset,
                       &TP]() {
    bisect(راست_Nodes, RecDepth + 1, راست_Bucket, MidOffset, TP);
  };

  اگر (TP && RecDepth < Config.TaskSplitDepth && رقم_Nodes >= 4) {
    TP->async(std::move(چپ_Rec_Task));
    TP->async(std::move(راست_Rec_Task));
  } وگرنه {
    چپ_Rec_Task();
    راست_Rec_Task();
  }
}

تهی BalancedPartitioning::runIterations(ثابت Function_گره_Range Nodes,
                                         unsigned چپ_Bucket,
                                         unsigned راست_Bucket,
                                         std::mt19937 &RNG) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_گره::Utility_گره_T, unsigned> Utility_گره_Index;
  برای (auto &N : Nodes)
    برای (auto &UN : N.UtilityNodes)
      ++Utility_گره_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  برای (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_گره_Index[UN];
      بازگشت UNI == 1 || UNI == رقم_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_گره_Index.clear();
  برای (auto &N : Nodes)
    برای (auto &UN : N.UtilityNodes)
      UN = Utility_گره_Index.درج({UN, Utility_گره_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_گره_Index.size());
  برای (auto &N : Nodes) {
    برای (auto &UN : N.UtilityNodes) {
      أكد(UN < Signatures.size());
      اگر (N.Bucket == چپ_Bucket) {
        Signatures[UN].چپ_شمار++;
      } وگرنه {
        Signatures[UN].راست_شمار++;
      }
    }
  }

  برای (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned رقم_Moved_Nodes =
        runIteration(Nodes, چپ_Bucket, راست_Bucket, Signatures, RNG);
    اگر (رقم_Moved_Nodes == 0)
      بشکن;
  }
}

unsigned BalancedPartitioning::runIteration(ثابت Function_گره_Range Nodes,
                                            unsigned چپ_Bucket,
                                            unsigned راست_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Init signature cost caches
  برای (auto &Signature : Signatures) {
    اگر (Signature.CachedGainIsValid)
      ادامه;
    unsigned L = Signature.چپ_شمار;
    unsigned R = Signature.راست_شمار;
    أكد((L > 0 || R > 0) && "incorrect signature");
    اعشاری Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    اگر (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    اگر (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = درست;
  }

  // Compute move gains
  using GainPair = std::pair<اعشاری, BPFunction_گره *>;
  std::vector<GainPair> Gains;
  برای (auto &N : Nodes) {
    منطقی From_چپ_To_راست = (N.Bucket == چپ_Bucket);
    اعشاری Gain = moveGain(N, From_چپ_To_راست, Signatures);
    Gains.افزودن_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto چپ_End = llvm::partition(
      Gains, [&](ثابت auto &GP) { بازگشت GP.second->Bucket == چپ_Bucket; });
  auto چپ_Range = llvm::make_range(Gains.begin(), چپ_End);
  auto راست_Range = llvm::make_range(چپ_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ثابت auto &L, ثابت auto &R) {
    بازگشت L.first > R.first;
  };
  llvm::stable_sort(چپ_Range, LargerGain);
  llvm::stable_sort(راست_Range, LargerGain);

  unsigned رقم_Moved_Data_Vertices = 0;
  برای (auto [چپ_Pair, راست_Pair] : llvm::zip(چپ_Range, راست_Range)) {
    auto &[چپ_Gain, چپ_گره] = چپ_Pair;
    auto &[راست_Gain, راست_گره] = راست_Pair;
    // Stop when the gain is no longer beneficial
    اگر (چپ_Gain + راست_Gain <= 0.f)
      بشکن;
    // Try to exchange the nodes between buckets
    اگر (move_Function_گره(*چپ_گره, چپ_Bucket, راست_Bucket, Signatures, RNG))
      ++رقم_Moved_Data_Vertices;
    اگر (move_Function_گره(*راست_گره, چپ_Bucket, راست_Bucket, Signatures, RNG))
      ++رقم_Moved_Data_Vertices;
  }
  بازگشت رقم_Moved_Data_Vertices;
}

منطقی BalancedPartitioning::move_Function_گره(BPFunction_گره &N,
                                            unsigned چپ_Bucket,
                                            unsigned راست_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ثابت {
  // Sometimes we skip the move. This helps to escape local optima
  اگر (std::uniform_real_distribution<اعشاری>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    بازگشت نادرست;

  منطقی From_چپ_To_راست = (N.Bucket == چپ_Bucket);
  // Update the current bucket
  N.Bucket = (From_چپ_To_راست ? راست_Bucket : چپ_Bucket);

  // Update signatures and invalidate gain cache
  اگر (From_چپ_To_راست) {
    برای (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.چپ_شمار--;
      Signature.راست_شمار++;
      Signature.CachedGainIsValid = نادرست;
    }
  } وگرنه {
    برای (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.چپ_شمار++;
      Signature.راست_شمار--;
      Signature.CachedGainIsValid = نادرست;
    }
  }
  بازگشت درست;
}

تهی BalancedPartitioning::split(ثابت Function_گره_Range Nodes,
                                 unsigned StartBucket) ثابت {
  unsigned رقم_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (رقم_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    بازگشت L.درون_داد_Order_Index < R.درون_داد_Order_Index;
  });

  برای (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  برای (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

اعشاری BalancedPartitioning::moveGain(ثابت BPFunction_گره &N,
                                     منطقی From_چپ_To_راست,
                                     ثابت SignaturesT &Signatures) {
  اعشاری Gain = 0.f;
  برای (auto &UN : N.UtilityNodes)
    Gain += (From_چپ_To_راست ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  بازگشت Gain;
}

اعشاری BalancedPartitioning::logCost(unsigned X, unsigned Y) ثابت {
  بازگشت -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

اعشاری BalancedPartitioning::log2Cached(unsigned i) ثابت {
  بازگشت (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
