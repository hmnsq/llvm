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

शून्य BPFunction_गाठ::dump(raw_ostream &OS) स्थिरांक {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.end()), Bucket);
}

ढाँचा <typename Func>
शून्य BalancedPartitioning::BPThreadPool::async(Func &&F) {
#जर LLVM_ENABLE_THREADS
  // This new thread could spawn more threads, so mark it as active
  ++अंक_Active_Threads;
  TheThreadPool.async([हे, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    जर (--अंक_Active_Threads == 0) {
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

शून्य BalancedPartitioning::BPThreadPool::रुको() {
#जर LLVM_ENABLE_THREADS
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_lock<std::mutex> lock(mtx);
    cv.रुको(lock, [&]() { परत IsFinishedSpawning; });
    प्रतिज्ञा(IsFinishedSpawning && अंक_Active_Threads == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.रुको();
#अन्यथा
  llvm_unreachable("threads are disabled");
#endif
}

BalancedPartitioning::BalancedPartitioning(
    स्थिरांक BalancedPartitioningConfig &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  साठी (अचिन्हित I = 1; I < LOG_CACHE_SIZE; I++)
    Log2Cache[I] = std::log2(I);
}

शून्य BalancedPartitioning::run(std::vector<BPFunction_गाठ> &Nodes) स्थिरांक {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.size(), Config.SplitDepth, Config.IterationsPerSplit));
  std::optional<BPThreadPool> TP;
#जर LLVM_ENABLE_THREADS
  DefaultThreadPool TheThreadPool;
  जर (Config.TaskSplitDepth > 1)
    TP.emplace(TheThreadPool);
#endif

  // Record the input order
  साठी (अचिन्हित I = 0; I < Nodes.size(); I++)
    Nodes[I].आदान_Order_Index = I;

  स्वतः NodesRange = llvm::make_range(Nodes.begin(), Nodes.end());
  स्वतः BisectTask = [हे, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  जर (TP) {
    TP->async(std::move(BisectTask));
    TP->रुको();
  } अन्यथा {
    BisectTask();
  }

  llvm::stable_sort(NodesRange, [](स्थिरांक स्वतः &L, स्थिरांक स्वतः &R) {
    परत L.Bucket < R.Bucket;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

शून्य BalancedPartitioning::bisect(स्थिरांक Function_गाठ_Range Nodes,
                                  अचिन्हित RecDepth, अचिन्हित मूळ_Bucket,
                                  अचिन्हित Offset,
                                  std::optional<BPThreadPool> &TP) स्थिरांक {
  अचिन्हित अंक_Nodes = llvm::size(Nodes);
  जर (अंक_Nodes <= 1 || RecDepth >= Config.SplitDepth) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::sort(Nodes, [](स्थिरांक स्वतः &L, स्थिरांक स्वतः &R) {
      परत L.आदान_Order_Index < R.आदान_Order_Index;
    });
    साठी (स्वतः &N : Nodes)
      N.Bucket = Offset++;
    परत;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              अंक_Nodes, मूळ_Bucket));

  std::mt19937 RNG(मूळ_Bucket);

  अचिन्हित डावा_Bucket = 2 * मूळ_Bucket;
  अचिन्हित उजवा_Bucket = 2 * मूळ_Bucket + 1;

  // Split into two and assign to the left and right buckets
  split(Nodes, डावा_Bucket);

  runIterations(Nodes, डावा_Bucket, उजवा_Bucket, RNG);

  // Split nodes wrt the resulting buckets
  स्वतः NodesMid =
      llvm::partition(Nodes, [&](स्वतः &N) { परत N.Bucket == डावा_Bucket; });
  अचिन्हित MidOffset = Offset + std::distance(Nodes.begin(), NodesMid);

  स्वतः डावा_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  स्वतः उजवा_Nodes = llvm::make_range(NodesMid, Nodes.end());

  स्वतः डावा_Rec_Task = [हे, डावा_Nodes, RecDepth, डावा_Bucket, Offset, &TP]() {
    bisect(डावा_Nodes, RecDepth + 1, डावा_Bucket, Offset, TP);
  };
  स्वतः उजवा_Rec_Task = [हे, उजवा_Nodes, RecDepth, उजवा_Bucket, MidOffset,
                       &TP]() {
    bisect(उजवा_Nodes, RecDepth + 1, उजवा_Bucket, MidOffset, TP);
  };

  जर (TP && RecDepth < Config.TaskSplitDepth && अंक_Nodes >= 4) {
    TP->async(std::move(डावा_Rec_Task));
    TP->async(std::move(उजवा_Rec_Task));
  } अन्यथा {
    डावा_Rec_Task();
    उजवा_Rec_Task();
  }
}

शून्य BalancedPartitioning::runIterations(स्थिरांक Function_गाठ_Range Nodes,
                                         अचिन्हित डावा_Bucket,
                                         अचिन्हित उजवा_Bucket,
                                         std::mt19937 &RNG) स्थिरांक {
  अचिन्हित अंक_Nodes = llvm::size(Nodes);
  DenseMap<BPFunction_गाठ::Utility_गाठ_T, अचिन्हित> Utility_गाठ_Index;
  साठी (स्वतः &N : Nodes)
    साठी (स्वतः &UN : N.UtilityNodes)
      ++Utility_गाठ_Index[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  साठी (स्वतः &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](स्वतः &UN) {
      अचिन्हित UNI = Utility_गाठ_Index[UN];
      परत UNI == 1 || UNI == अंक_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_गाठ_Index.clear();
  साठी (स्वतः &N : Nodes)
    साठी (स्वतः &UN : N.UtilityNodes)
      UN = Utility_गाठ_Index.घाला({UN, Utility_गाठ_Index.size()}).first->second;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_गाठ_Index.size());
  साठी (स्वतः &N : Nodes) {
    साठी (स्वतः &UN : N.UtilityNodes) {
      प्रतिज्ञा(UN < Signatures.size());
      जर (N.Bucket == डावा_Bucket) {
        Signatures[UN].डावा_मोजणी++;
      } अन्यथा {
        Signatures[UN].उजवा_मोजणी++;
      }
    }
  }

  साठी (अचिन्हित I = 0; I < Config.IterationsPerSplit; I++) {
    अचिन्हित अंक_Moved_Nodes =
        runIteration(Nodes, डावा_Bucket, उजवा_Bucket, Signatures, RNG);
    जर (अंक_Moved_Nodes == 0)
      थांबा;
  }
}

अचिन्हित BalancedPartitioning::runIteration(स्थिरांक Function_गाठ_Range Nodes,
                                            अचिन्हित डावा_Bucket,
                                            अचिन्हित उजवा_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) स्थिरांक {
  // Init signature cost caches
  साठी (स्वतः &Signature : Signatures) {
    जर (Signature.CachedGainIsValid)
      सुरूठेवा;
    अचिन्हित L = Signature.डावा_मोजणी;
    अचिन्हित R = Signature.उजवा_मोजणी;
    प्रतिज्ञा((L > 0 || R > 0) && "incorrect signature");
    दशांश Cost = logCost(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    जर (L > 0)
      Signature.CachedGainLR = Cost - logCost(L - 1, R + 1);
    जर (R > 0)
      Signature.CachedGainRL = Cost - logCost(L + 1, R - 1);
    Signature.CachedGainIsValid = सत्य;
  }

  // Compute move gains
  using GainPair = std::pair<दशांश, BPFunction_गाठ *>;
  std::vector<GainPair> Gains;
  साठी (स्वतः &N : Nodes) {
    तार्किक From_डावा_To_उजवा = (N.Bucket == डावा_Bucket);
    दशांश Gain = moveGain(N, From_डावा_To_उजवा, Signatures);
    Gains.ढकला_back(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  स्वतः डावा_End = llvm::partition(
      Gains, [&](स्थिरांक स्वतः &GP) { परत GP.second->Bucket == डावा_Bucket; });
  स्वतः डावा_Range = llvm::make_range(Gains.begin(), डावा_End);
  स्वतः उजवा_Range = llvm::make_range(डावा_End, Gains.end());

  // Sort gains in descending order
  स्वतः LargerGain = [](स्थिरांक स्वतः &L, स्थिरांक स्वतः &R) {
    परत L.first > R.first;
  };
  llvm::stable_sort(डावा_Range, LargerGain);
  llvm::stable_sort(उजवा_Range, LargerGain);

  अचिन्हित अंक_Moved_Data_Vertices = 0;
  साठी (स्वतः [डावा_Pair, उजवा_Pair] : llvm::zip(डावा_Range, उजवा_Range)) {
    स्वतः &[डावा_Gain, डावा_गाठ] = डावा_Pair;
    स्वतः &[उजवा_Gain, उजवा_गाठ] = उजवा_Pair;
    // Stop when the gain is no longer beneficial
    जर (डावा_Gain + उजवा_Gain <= 0.f)
      थांबा;
    // Try to exchange the nodes between buckets
    जर (move_Function_गाठ(*डावा_गाठ, डावा_Bucket, उजवा_Bucket, Signatures, RNG))
      ++अंक_Moved_Data_Vertices;
    जर (move_Function_गाठ(*उजवा_गाठ, डावा_Bucket, उजवा_Bucket, Signatures, RNG))
      ++अंक_Moved_Data_Vertices;
  }
  परत अंक_Moved_Data_Vertices;
}

तार्किक BalancedPartitioning::move_Function_गाठ(BPFunction_गाठ &N,
                                            अचिन्हित डावा_Bucket,
                                            अचिन्हित उजवा_Bucket,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) स्थिरांक {
  // Sometimes we skip the move. This helps to escape local optima
  जर (std::uniform_real_distribution<दशांश>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    परत असत्य;

  तार्किक From_डावा_To_उजवा = (N.Bucket == डावा_Bucket);
  // Update the current bucket
  N.Bucket = (From_डावा_To_उजवा ? उजवा_Bucket : डावा_Bucket);

  // Update signatures and invalidate gain cache
  जर (From_डावा_To_उजवा) {
    साठी (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.डावा_मोजणी--;
      Signature.उजवा_मोजणी++;
      Signature.CachedGainIsValid = असत्य;
    }
  } अन्यथा {
    साठी (स्वतः &UN : N.UtilityNodes) {
      स्वतः &Signature = Signatures[UN];
      Signature.डावा_मोजणी++;
      Signature.उजवा_मोजणी--;
      Signature.CachedGainIsValid = असत्य;
    }
  }
  परत सत्य;
}

शून्य BalancedPartitioning::split(स्थिरांक Function_गाठ_Range Nodes,
                                 अचिन्हित StartBucket) स्थिरांक {
  अचिन्हित अंक_Nodes = llvm::size(Nodes);
  स्वतः NodesMid = Nodes.begin() + (अंक_Nodes + 1) / 2;

  llvm::sort(Nodes, [](स्वतः &L, स्वतः &R) {
    परत L.आदान_Order_Index < R.आदान_Order_Index;
  });

  साठी (स्वतः &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.Bucket = StartBucket;
  साठी (स्वतः &N : llvm::make_range(NodesMid, Nodes.end()))
    N.Bucket = StartBucket + 1;
}

दशांश BalancedPartitioning::moveGain(स्थिरांक BPFunction_गाठ &N,
                                     तार्किक From_डावा_To_उजवा,
                                     स्थिरांक SignaturesT &Signatures) {
  दशांश Gain = 0.f;
  साठी (स्वतः &UN : N.UtilityNodes)
    Gain += (From_डावा_To_उजवा ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  परत Gain;
}

दशांश BalancedPartitioning::logCost(अचिन्हित X, अचिन्हित Y) स्थिरांक {
  परत -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

दशांश BalancedPartitioning::log2Cached(अचिन्हित i) स्थिरांक {
  परत (i < LOG_CACHE_SIZE) ? Log2Cache[i] : std::log2(i);
}
