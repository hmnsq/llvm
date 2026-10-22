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

शून्य BPFunction_गाँठ::dump(raw_ostream &OS) ध्रुव {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

template <typename Func>
शून्य BalancedPartitioning::BPThreadPool::async(Func &&F) {
#यदि LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++अंक_Active_Threads;
  TheThreadPool.async([एतत्, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    यदि (--अंक_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        प्रतिज्ञा(!IsFinishedSpawning);
        IsFinishedSpawning = सत्य;
      }
      cv.notify_one();
    }
  });
#अन्यथा
  llvm_unreachable("threads are disabled");
#endif
}

शून्य BalancedPartitioning::BPThreadPool::wait() {
#यदि LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&]() { प्रत्यागम IsFinishedSpawning; });
    प्रतिज्ञा(IsFinishedSpawning && अंक_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#अन्यथा
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

शून्य BalancedPartitioning::run(std::vector<BPFunction_गाँठ> &Nodes) ध्रुव {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#यदि LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  यदि (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  प्रति (unsigned I = 0; I < Nodes.size(); I++)
    Nodes[I].निवेश_Order_Index = I;

  auto NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  auto BisectTask = [एतत्, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  यदि (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } अन्यथा {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](ध्रुव auto &L, ध्रुव auto &R) {
    प्रत्यागम L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

शून्य BalancedPartitioning::bisect(ध्रुव Function_गाँठ_Range Nodes,
                                  unsigned RecDepth, unsigned मूल_Bucket,
                                  unsigned Offset,
                                  std::optional<BPThreadPool> &TP) ध्रुव {
  unsigned अंक_Nodes = llvm::size(Nodes);
  यदि (अंक_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](ध्रुव auto &L, ध्रुव auto &R) {
      प्रत्यागम L.निवेश_Order_Index < R.निवेश_Order_Index;
    });
    प्रति (auto &N : Nodes)
      N.Bucket = Offset++;
    प्रत्यागम;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अंक_Nodes, मूल_Bucket));

  std::mt19937 RNG(मूल_Bucket);

  unsigned बायाँ_Bucket = 2 * मूल_Bucket;
  unsigned दायाँ_Bucket = 2 * मूल_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, बायाँ_Bucket);

  runIterations(Nodes, बायाँ_Bucket, दायाँ_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  auto NodesMid =
      llvm::partition(Nodes, [&](auto &N) { प्रत्यागम N.Bucket == बायाँ_Bucket; });
  unsigned MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  auto बायाँ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  auto दायाँ_Nodes = llvm::make_range(NodesMid, Nodes.end());

  auto बायाँ_Rec_Task = [एतत्, बायाँ_Nodes, RecDepth, बायाँ_Bucket, Offset, &TP]() {
    bisect(बायाँ_Nodes, RecDepth + 1, बायाँ_Bucket, Offset, TP);
  };
  auto दायाँ_Rec_Task = [एतत्, दायाँ_Nodes, RecDepth, दायाँ_Bucket, MidOffset,
                       &TP]() {
    bisect(दायाँ_Nodes, RecDepth + 1, दायाँ_Bucket, MidOffset, TP);
  };

  यदि (TP && RecDepth < Config.TaskSplitDepth && अंक_Nodes >= 4) {
    TP->async(std::move(बायाँ_Rec_Task));
    TP->async(std::move(दायाँ_Rec_Task));
  } अन्यथा {
    बायाँ_Rec_Task();
    दायाँ_Rec_Task();
  }
}

शून्य BalancedPartitioning::runIterations(ध्रुव Function_गाँठ_Range Nodes,
                                         unsigned बायाँ_Bucket,
                                         unsigned दायाँ_Bucket,
                                         std::mt19937 &RNG) ध्रुव {
  unsigned अंक_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_गाँठ::Utility_गाँठ_T, unsigned> Utility_गाँठ_Index;
  प्रति (auto &N : Nodes)
    प्रति (auto &UN : N.UtilityNodes)
      ++Utility_गाँठ_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  प्रति (auto &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](auto &UN) {
      unsigned UNI = Utility_गाँठ_Index[UN];
      प्रत्यागम UNI == 1 || UNI == अंक_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_गाँठ_Index.clear();
  प्रति (auto &N : Nodes)
    प्रति (auto &UN : N.UtilityNodes)
      UN = Utility_गाँठ_Index.जोड़ो({UN, Utility_गाँठ_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_गाँठ_Index.size());
  प्रति (auto &N : Nodes) {
    प्रति (auto &UN : N.UtilityNodes) {
      प्रतिज्ञा(UN < Signatures.size());
      यदि (N.Bucket == बायाँ_Bucket) {
        Signatures[UN].बायाँ_गिनती++;
      } अन्यथा {
        Signatures[UN].दायाँ_गिनती++;
      }
    }
  }

  प्रति (unsigned I = 0; I < Config.IterationsPerSplit; I++) {
    unsigned अंक_Moved_Nodes =
        runIteration(Nodes, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG);
    यदि (अंक_Moved_Nodes == 0)
      भङ्ग;
  }
}

unsigned BalancedPartitioning::runIteration(ध्रुव Function_गाँठ_Range Nodes,
                                            unsigned बायाँ_Bucket,
                                            unsigned दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Init signature cost caches
  प्रति (auto &Signature : Signatures) {
    यदि (Signature.CachedGainIsValid)
      अनुवर्त;
    unsigned L = Signature.बायाँ_गिनती;
    unsigned R = Signature.दायाँ_गिनती;
    प्रतिज्ञा((L > 0 || R > 0) && "incorrect signature");
    भिन्न Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    यदि (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    यदि (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = सत्य;
  }

  // Compute move gains
  using GainPair = std::pair<भिन्न, BPFunction_गाँठ *>;
  std::vector<GainPair> Gains;
  प्रति (auto &N : Nodes) {
    तार्किक From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
    भिन्न Gain = moveGain(N, From_बायाँ_To_दायाँ, Signatures);
    Gains.धकेलो_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  auto बायाँ_End = llvm::partition(
      Gains, [&](ध्रुव auto &GP) { प्रत्यागम GP.second->Bucket == बायाँ_Bucket; });
  auto बायाँ_Range = llvm::make_range(Gains.begin(), बायाँ_End);
  auto दायाँ_Range = llvm::make_range(बायाँ_End, Gains.end());

  // Sort gains in descending order
  auto LargerGain = [](ध्रुव auto &L, ध्रुव auto &R) {
    प्रत्यागम L.first > R.first;
  };
  llvm::stable_sort(बायाँ_Range, LargerGain);
  llvm::stable_sort(दायाँ_Range, LargerGain);

  unsigned अंक_Moved_Data_Vertices = 0;
  प्रति (auto [बायाँ_Pair, दायाँ_Pair] : llvm::zip(बायाँ_Range, दायाँ_Range)) {
    auto &[बायाँ_Gain, बायाँ_गाँठ] = बायाँ_Pair;
    auto &[दायाँ_Gain, दायाँ_गाँठ] = दायाँ_Pair;
    // Stop when the gain is no longer beneficial
    यदि (बायाँ_Gain + दायाँ_Gain <= 0.f)
      भङ्ग;
    // Try to exchange the nodes between buckets
    यदि (move_Function_गाँठ(*बायाँ_गाँठ, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अंक_Moved_Data_Vertices;
    यदि (move_Function_गाँठ(*दायाँ_गाँठ, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अंक_Moved_Data_Vertices;
  }
  प्रत्यागम अंक_Moved_Data_Vertices;
}

तार्किक BalancedPartitioning::move_Function_गाँठ(BPFunction_गाँठ &N,
                                            unsigned बायाँ_Bucket,
                                            unsigned दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ध्रुव {
  // Sometimes we skip the move. This helps to escape local optima
  यदि (std::uniform_real_distribution<भिन्न>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    प्रत्यागम असत्य;

  तार्किक From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
  // Update the current bucket
  N.Bucket = (From_बायाँ_To_दायाँ ? दायाँ_Bucket : बायाँ_Bucket);

  // Update signatures and invalidate gain cache
  यदि (From_बायाँ_To_दायाँ) {
    प्रति (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.बायाँ_गिनती--;
      Signature.दायाँ_गिनती++;
      Signature.CachedGainIsValid = असत्य;
    }
  } अन्यथा {
    प्रति (auto &UN : N.UtilityNodes) {
      auto &Signature = Signatures[UN];
      Signature.बायाँ_गिनती++;
      Signature.दायाँ_गिनती--;
      Signature.CachedGainIsValid = असत्य;
    }
  }
  प्रत्यागम सत्य;
}

शून्य BalancedPartitioning::split(ध्रुव Function_गाँठ_Range Nodes,
                                 unsigned StartBucket) ध्रुव {
  unsigned अंक_Nodes = llvm::size(Nodes);
  auto NodesMid = Nodes.begin() + (अंक_Nodes + 1) / 2;

  llvm::sort(Nodes, [](auto &L, auto &R) {
    प्रत्यागम L.निवेश_Order_Index < R.निवेश_Order_Index;
  });

  प्रति (auto &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  प्रति (auto &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

भिन्न BalancedPartitioning::moveGain(ध्रुव BPFunction_गाँठ &N,
                                     तार्किक From_बायाँ_To_दायाँ,
                                     ध्रुव SignaturesT &Signatures) {
  भिन्न Gain = 0.f;
  प्रति (auto &UN : N.UtilityNodes)
    Gain += (From_बायाँ_To_दायाँ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  प्रत्यागम Gain;
}

भिन्न BalancedPartitioning::logCost(unsigned X, unsigned Y) ध्रुव {
  प्रत्यागम -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

भिन्न BalancedPartitioning::log2Cached(unsigned i) ध्रुव {
  प्रत्यागम (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
