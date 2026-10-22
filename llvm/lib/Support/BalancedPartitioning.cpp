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

#समावेश "llvm/Support/BalancedPartitioning.h"
#समावेश "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/FormatVariadic.h"
#समावेश "llvm/Support/ThreadPool.h"

#समावेश <cmath>

using namespace llvm;
#घोषणा DEBUG_TYPE "balanced-partitioning"

व्योम BPFunction_गाँठ::dump(raw_ostream &OS) स्थिर {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

ढाँचा <typename Func>
व्योम BalancedPartitioning::BPThreadPool::async(Func &&F) {
#अगर LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++अंक_Active_Threads;
  TheThreadPool.async([यह, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    अगर (--अंक_Active_Threads == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_lock<std::mutex> lock(mtx);
        निश्चित(!IsFinishedSpawning);
        IsFinishedSpawning = true;
      }
      cv.notify_one();
    }
  });
#अन्यथा
  llvm_unreachable("threads are disabled");
#पूर्ण_अगर
}

व्योम BalancedPartitioning::BPThreadPool::रुको() {
#अगर LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.रुको(lock, [&]() { वापस IsFinishedSpawning; });
    निश्चित(IsFinishedSpawning && अंक_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.रुको();
#अन्यथा
  llvm_unreachable("threads are disabled");
#पूर्ण_अगर
}

BalancedPartitioning::BalancedPartitioning(
    स्थिर BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  क्रम (अचिन्हित I = 1; I < LOG_CACHE_आकार; I++)
    Log2Cache[I] = std::log2(I);
}

व्योम BalancedPartitioning::run(std::vector<BPFunction_गाँठ> &Nodes) स्थिर {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.आकार_2(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#अगर LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  अगर (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#पूर्ण_अगर

  // Record the input order
  क्रम (अचिन्हित I = 0; I < Nodes.आकार_2(); I++)
    Nodes[I].निवेश_Order_सूचकांक = I;

  स्वतः NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  स्वतः BisectTask = [यह, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  अगर (TP) {
    TP->async(std::move(BisectTask));
    TP->रुको();
  } अन्यथा {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](स्थिर स्वतः &L, स्थिर स्वतः &R) {
    वापस L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

व्योम BalancedPartitioning::bisect(स्थिर Function_गाँठ_Range Nodes,
                                  अचिन्हित RecDepth, अचिन्हित मूल_Bucket,
                                  अचिन्हित Offset,
                                  std::optional<BPThreadPool> &TP) स्थिर {
  अचिन्हित अंक_Nodes = llvm::आकार_2(Nodes);
  अगर (अंक_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](स्थिर स्वतः &L, स्थिर स्वतः &R) {
      वापस L.निवेश_Order_सूचकांक < R.निवेश_Order_सूचकांक;
    });
    क्रम (स्वतः &N : Nodes)
      N.Bucket = Offset++;
    वापस;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अंक_Nodes, मूल_Bucket));

  std::mt19937 RNG(मूल_Bucket);

  अचिन्हित बायाँ_Bucket = 2 * मूल_Bucket;
  अचिन्हित दायाँ_Bucket = 2 * मूल_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, बायाँ_Bucket);

  runIterations(Nodes, बायाँ_Bucket, दायाँ_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  स्वतः NodesMid =
      llvm::partition(Nodes, [&](स्वतः &N) { वापस N.Bucket == बायाँ_Bucket; });
  अचिन्हित MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  स्वतः बायाँ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  स्वतः दायाँ_Nodes = llvm::make_range(NodesMid, Nodes.end());

  स्वतः बायाँ_Rec_Task = [यह, बायाँ_Nodes, RecDepth, बायाँ_Bucket, Offset, &TP]() {
    bisect(बायाँ_Nodes, RecDepth + 1, बायाँ_Bucket, Offset, TP);
  };
  स्वतः दायाँ_Rec_Task = [यह, दायाँ_Nodes, RecDepth, दायाँ_Bucket, MidOffset,
                       &TP]() {
    bisect(दायाँ_Nodes, RecDepth + 1, दायाँ_Bucket, MidOffset, TP);
  };

  अगर (TP && RecDepth < Config.TaskSplitDepth && अंक_Nodes >= 4) {
    TP->async(std::move(बायाँ_Rec_Task));
    TP->async(std::move(दायाँ_Rec_Task));
  } अन्यथा {
    बायाँ_Rec_Task();
    दायाँ_Rec_Task();
  }
}

व्योम BalancedPartitioning::runIterations(स्थिर Function_गाँठ_Range Nodes,
                                         अचिन्हित बायाँ_Bucket,
                                         अचिन्हित दायाँ_Bucket,
                                         std::mt19937 &RNG) स्थिर {
  अचिन्हित अंक_Nodes = llvm::आकार_2(Nodes);
  DenseMap<BPFunction_गाँठ::Utility_गाँठ_T, अचिन्हित> Utility_गाँठ_सूचकांक;
  क्रम (स्वतः &N : Nodes)
    क्रम (स्वतः &UN : N.UtilityNodes)
      ++Utility_गाँठ_सूचकांक[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  क्रम (स्वतः &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](स्वतः &UN) {
      अचिन्हित UNI = Utility_गाँठ_सूचकांक[UN];
      वापस UNI == 1 || UNI == अंक_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_गाँठ_सूचकांक.सफा();
  क्रम (स्वतः &N : Nodes)
    क्रम (स्वतः &UN : N.UtilityNodes)
      UN = Utility_गाँठ_सूचकांक.जोड़ो({UN, Utility_गाँठ_सूचकांक.आकार_2()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_गाँठ_सूचकांक.आकार_2());
  क्रम (स्वतः &N : Nodes) {
    क्रम (स्वतः &UN : N.UtilityNodes) {
      निश्चित(UN < Signatures.आकार_2());
      अगर (N.Bucket == बायाँ_Bucket) {
        Signatures[UN].बायाँ_गिनती++;
      } अन्यथा {
        Signatures[UN].दायाँ_गिनती++;
      }
    }
  }

  क्रम (अचिन्हित I = 0; I < Config.IterationsPerSplit; I++) {
    अचिन्हित अंक_Moved_Nodes =
        runIteration(Nodes, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG);
    अगर (अंक_Moved_Nodes == 0)
      अवरोध;
  }
}

अचिन्हित BalancedPartitioning::runIteration(स्थिर Function_गाँठ_Range Nodes,
                                            अचिन्हित बायाँ_Bucket,
                                            अचिन्हित दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) स्थिर {
  // Init signature cost caches
  क्रम (स्वतः &Signature : Signatures) {
    अगर (Signature.CachedGainIsValid)
      जारी;
    अचिन्हित L = Signature.बायाँ_गिनती;
    अचिन्हित R = Signature.दायाँ_गिनती;
    निश्चित((L > 0 || R > 0) && "incorrect signature");
    भग्न Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    अगर (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    अगर (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = true;
  }

  // Compute move gains
  using GainPair = std::pair<भग्न, BPFunction_गाँठ *>;
  std::vector<GainPair> Gains;
  क्रम (स्वतः &N : Nodes) {
    bool From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
    भग्न Gain = moveGain(N, From_बायाँ_To_दायाँ, Signatures);
    Gains.धकेलो_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  स्वतः बायाँ_End = llvm::partition(
      Gains, [&](स्थिर स्वतः &GP) { वापस GP.second->Bucket == बायाँ_Bucket; });
  स्वतः बायाँ_Range = llvm::make_range(Gains.begin(), बायाँ_End);
  स्वतः दायाँ_Range = llvm::make_range(बायाँ_End, Gains.end());

  // Sort gains in descending order
  स्वतः LargerGain = [](स्थिर स्वतः &L, स्थिर स्वतः &R) {
    वापस L.first > R.first;
  };
  llvm::stable_sort(बायाँ_Range, LargerGain);
  llvm::stable_sort(दायाँ_Range, LargerGain);

  अचिन्हित अंक_Moved_तथ्य_Vertices = 0;
  क्रम (स्वतः [बायाँ_Pair, दायाँ_Pair] : llvm::zip(बायाँ_Range, दायाँ_Range)) {
    स्वतः &[बायाँ_Gain, बायाँ_गाँठ] = बायाँ_Pair;
    स्वतः &[दायाँ_Gain, दायाँ_गाँठ] = दायाँ_Pair;
    // Stop when the gain is no longer beneficial
    अगर (बायाँ_Gain + दायाँ_Gain <= 0.f)
      अवरोध;
    // Try to exchange the nodes between buckets
    अगर (move_Function_गाँठ(*बायाँ_गाँठ, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अंक_Moved_तथ्य_Vertices;
    अगर (move_Function_गाँठ(*दायाँ_गाँठ, बायाँ_Bucket, दायाँ_Bucket, Signatures, RNG))
      ++अंक_Moved_तथ्य_Vertices;
  }
  वापस अंक_Moved_तथ्य_Vertices;
}

bool BalancedPartitioning::move_Function_गाँठ(BPFunction_गाँठ &N,
                                            अचिन्हित बायाँ_Bucket,
                                            अचिन्हित दायाँ_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) स्थिर {
  // Sometimes we skip the move. This helps to escape local optima
  अगर (std::uniform_real_distribution<भग्न>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    वापस false;

  bool From_बायाँ_To_दायाँ = (N.Bucket == बायाँ_Bucket);
  // Update the current bucket
  N.Bucket = (From_बायाँ_To_दायाँ ? दायाँ_Bucket : बायाँ_Bucket);

  // Update signatures and invalidate gain cache
  अगर (From_बायाँ_To_दायाँ) {
    क्रम (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.बायाँ_गिनती--;
      Signature.दायाँ_गिनती++;
      Signature.CachedGainIsValid = false;
    }
  } अन्यथा {
    क्रम (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.बायाँ_गिनती++;
      Signature.दायाँ_गिनती--;
      Signature.CachedGainIsValid = false;
    }
  }
  वापस true;
}

व्योम BalancedPartitioning::split(स्थिर Function_गाँठ_Range Nodes,
                                 अचिन्हित StartBucket) स्थिर {
  अचिन्हित अंक_Nodes = llvm::आकार_2(Nodes);
  स्वतः NodesMid = Nodes.begin() + (अंक_Nodes + 1) / 2;

  llvm::sort(Nodes, [](स्वतः &L, स्वतः &R) {
    वापस L.निवेश_Order_सूचकांक < R.निवेश_Order_सूचकांक;
  });

  क्रम (स्वतः &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  क्रम (स्वतः &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

भग्न BalancedPartitioning::moveGain(स्थिर BPFunction_गाँठ &N,
                                     bool From_बायाँ_To_दायाँ,
                                     स्थिर SignaturesT &Signatures) {
  भग्न Gain = 0.f;
  क्रम (स्वतः &UN : N.UtilityNodes)
    Gain += (From_बायाँ_To_दायाँ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  वापस Gain;
}

भग्न BalancedPartitioning::logCost(अचिन्हित X, अचिन्हित Y) स्थिर {
  वापस -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

भग्न BalancedPartitioning::log2Cached(अचिन्हित i) स्थिर {
  वापस (i < LOG_CACHE_आकार) ? Log2Cache[i] : std::log2(i);
}
