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

#अन्तर्भाव "llvm/Support/BalancedPartitioning.h"
#अन्तर्भाव "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/ThreadPool.h"

#अन्तर्भाव <cmath>

using नामाकाश llvm;
#define DEBUG_TYPE "balanced-partitioning"

शून्य BPFunction_ग्रन्थिः::dump(raw_ostream &OS) ध्रुव {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
शून्य BalancedPartitioning::BPThreadPool::async(Func &&F) {
#जइ LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++अङ्कः_Active_Threads;
  TheThreadPool.async([एतत्, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    जइ (--अङ्कः_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        प्रतिज्ञा(!IsFinishedSpawning);
        IsFinishedSpawning = सच्च;
      }
      cv.notify_one();
    }
  });
#अण्णहा
  llvm_unreachable("threads are disabled");
#endif
}

शून्य BalancedPartitioning::BPThreadPool::wait() {
#जइ LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { प्रत्यागम IsFinishedSpawning; });
    प्रतिज्ञा(IsFinishedSpawning && अङ्कः_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#अण्णहा
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    ध्रुव BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  प्रति (unsigned I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

शून्य BalancedPartitioning::run(std::vector<BPFunction_ग्रन्थिः> &Nodes) ध्रुव {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#जइ LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  जइ (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  प्रति (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].निवेशः_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [एतत्, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  जइ (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } अण्णहा {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ध्रुव auto &L, ध्रुव auto &R) {
    प्रत्यागम L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

शून्य BalancedPartitioning::bisect(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                  unsigned RecDepth, unsigned मूलम्_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ध्रुव {
  unsigned अङ्कः_Nodes = llvm::size(Nodes);
  जइ (अङ्कः_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ध्रुव auto &L, ध्रुव auto &R) {
      प्रत्यागम L.निवेशः_Order_Index < R.निवेशः_Order_Index;
    });
    प्रति (auto &N : Nodes)
      N.Bucket = Offset++;
    प्रत्यागम;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अङ्कः_Nodes, मूलम्_Bucket));

  std::mt19937 RNG(मूलम्_Bucket);

  unsigned वामः_Bucket = 2 * मूलम्_Bucket;
  unsigned दक्षिणः_Bucket = 2 * मूलम्_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, वामः_Bucket);

  runIterations(Nodes, वामः_Bucket, दक्षिणः_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { प्रत्यागम N.Bucket == वामः_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto वामः_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto दक्षिणः_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto वामः_Rec_Task = [एतत्, वामः_Nodes, RecDepth, वामः_Bucket, Offset, &TP]() {
    bisect(वामः_Nodes, RecDepth + 1, वामः_Bucket, Offset, TP);
  };
  auto दक्षिणः_Rec_Task = [एतत्, दक्षिणः_Nodes, RecDepth, दक्षिणः_Bucket, MidOffset,
                       &TP]() {
    bisect(दक्षिणः_Nodes, RecDepth + 1, दक्षिणः_Bucket, MidOffset, TP);
  };

  जइ (TP && RecDepth < Config.TaskSplitDepth && अङ्कः_Nodes >= 4) {
    TP->async(std::move(वामः_Rec_Task));
    TP->async(std::move(दक्षिणः_Rec_Task));
  } अण्णहा {
    वामः_Rec_Task();
    दक्षिणः_Rec_Task();
  }
}

शून्य BalancedPartitioning::runIterations(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                         unsigned वामः_Bucket,
                                         unsigned दक्षिणः_Bucket,
                                         std::mt19937 &RNG) ध्रुव {
  unsigned अङ्कः_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_ग्रन्थिः::Utility_ग्रन्थिः_T, unsigned> Utility_ग्रन्थिः_Index;
  प्रति (auto &N : Nodes)
    प्रति (auto &UN : N.UtilityNodes)
      ++Utility_ग्रन्थिः_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  प्रति (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_ग्रन्थिः_Index[UN];
      प्रत्यागम UNI == 1 || UNI == अङ्कः_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ग्रन्थिः_Index.clear();
  प्रति (auto &N : Nodes)
    प्रति (auto &UN : N.UtilityNodes)
      UN = Utility_ग्रन्थिः_Index.निवेशय({UN, Utility_ग्रन्थिः_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ग्रन्थिः_Index.size());
  प्रति (auto &N : Nodes) {
    प्रति (auto &UN : N.UtilityNodes) {
      प्रतिज्ञा(UN < Signatures.size());
      जइ (N.Bucket == वामः_Bucket) {
        Signatures[UN].वामः_गणनम्++;
      } अण्णहा {
        Signatures[UN].दक्षिणः_गणनम्++;
      }
    }
  }

  प्रति (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned अङ्कः_Moved_Nodes =
        runIteration(Nodes, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG);
    जइ (अङ्कः_Moved_Nodes == 0)
      भङ्ग;
  }
}

unsigned BalancedPartitioning::runIteration(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                            unsigned वामः_Bucket,
                                            unsigned दक्षिणः_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Init signature cost caches
  प्रति (auto &Signature : Signatures) {
    जइ (Signature.CachedGainIsValid)
      अनुवर्त;
    unsigned L = Signature.वामः_गणनम्;
    unsigned R = Signature.दक्षिणः_गणनम्;
    प्रतिज्ञा((L > 0 || R > 0) && "incorrect signature");
    भिन्न Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    जइ (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    जइ (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = सच्च;
  }

  // Compute move gains
  using GainPair = std::pair<भिन्न, BPFunction_ग्रन्थिः *>;
  std::vector<GainPair> Gains;
  प्रति (auto &N : Nodes) {
    तार्किक From_वामः_To_दक्षिणः = (N.Bucket == वामः_Bucket);
    भिन्न Gain = moveGain(N, From_वामः_To_दक्षिणः, Signatures);
    Gains.प्रक्षिप_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto वामः_End = llvm::partition(
      Gains, [&](ध्रुव auto &GP) { प्रत्यागम GP.second->Bucket == वामः_Bucket; });
  auto वामः_Range = llvm::make_range(Gains.begin(), वामः_End);
  auto दक्षिणः_Range = llvm::make_range(वामः_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ध्रुव auto &L, ध्रुव auto &R) {
    प्रत्यागम L.first > R.first;
  };
  llvm::stable_sort(वामः_Range, LargerGain);
  llvm::stable_sort(दक्षिणः_Range, LargerGain);

  unsigned अङ्कः_Moved_Data_Vertices = 0;
  प्रति (auto [वामः_Pair, दक्षिणः_Pair] : llvm::zip(वामः_Range, दक्षिणः_Range)) {
    auto &[वामः_Gain, वामः_ग्रन्थिः] = वामः_Pair;
    auto &[दक्षिणः_Gain, दक्षिणः_ग्रन्थिः] = दक्षिणः_Pair;
    // Stop when the gain is no longer beneficial
    जइ (वामः_Gain + दक्षिणः_Gain <= 0.f)
      भङ्ग;
    // Try to exchange the nodes between buckets
    जइ (move_Function_ग्रन्थिः(*वामः_ग्रन्थिः, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG))
      ++अङ्कः_Moved_Data_Vertices;
    जइ (move_Function_ग्रन्थिः(*दक्षिणः_ग्रन्थिः, वामः_Bucket, दक्षिणः_Bucket, Signatures, RNG))
      ++अङ्कः_Moved_Data_Vertices;
  }
  प्रत्यागम अङ्कः_Moved_Data_Vertices;
}

तार्किक BalancedPartitioning::move_Function_ग्रन्थिः(BPFunction_ग्रन्थिः &N,
                                            unsigned वामः_Bucket,
                                            unsigned दक्षिणः_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Sometimes we skip the move. This helps to escape local optima
  जइ (std::uniform_real_distribution<भिन्न>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    प्रत्यागम मिच्छ;

  तार्किक From_वामः_To_दक्षिणः = (N.Bucket == वामः_Bucket);
  // Update the current bucket
  N.Bucket = (From_वामः_To_दक्षिणः ? दक्षिणः_Bucket : वामः_Bucket);

  // Update signatures and invalidate gain cache
  जइ (From_वामः_To_दक्षिणः) {
    प्रति (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.वामः_गणनम्--;
      Signature.दक्षिणः_गणनम्++;
      Signature.CachedGainIsValid = मिच्छ;
    }
  } अण्णहा {
    प्रति (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.वामः_गणनम्++;
      Signature.दक्षिणः_गणनम्--;
      Signature.CachedGainIsValid = मिच्छ;
    }
  }
  प्रत्यागम सच्च;
}

शून्य BalancedPartitioning::split(ध्रुव Function_ग्रन्थिः_Range Nodes,
                                 unsigned StartBucket) ध्रुव {
  unsigned अङ्कः_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (अङ्कः_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    प्रत्यागम L.निवेशः_Order_Index < R.निवेशः_Order_Index;
  });

  प्रति (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  प्रति (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

भिन्न BalancedPartitioning::moveGain(ध्रुव BPFunction_ग्रन्थिः &N,
                                     तार्किक From_वामः_To_दक्षिणः,
                                     ध्रुव SignaturesT &Signatures) {
  भिन्न Gain = 0.f;
  प्रति (auto &UN : N.UtilityNodes)
    Gain += (From_वामः_To_दक्षिणः ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  प्रत्यागम Gain;
}

भिन्न BalancedPartitioning::logCost(unsigned X, unsigned Y) ध्रुव {
  प्रत्यागम -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

भिन्न BalancedPartitioning::log2Cached(unsigned i) ध्रुव {
  प्रत्यागम (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
