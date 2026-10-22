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

#ಅನ್ತರ್ಭಾವ "llvm/Support/BalancedPartitioning.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ThreadPool.h"

#ಅನ್ತರ್ಭಾವ <cmath>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "balanced-partitioning"

ಶೂನ್ಯ BPFunction_ಗಂಟು::dump(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.ಕೊನೆ()), ಪಾತ್ರೆ);
}

ಮಾದರಿ <ಬಗೆ_ಹೆಸರು Func>
ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::BPThreadPool::async(Func &&F) {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  // This new thread could spawn more threads, so mark it as active
  ++ಅಂಕೆ_ಸಕ್ರಿಯ_ಎಳೆಗಳು;
  TheThreadPool.async([ಇದು, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    ಆದರೆ (--ಅಂಕೆ_ಸಕ್ರಿಯ_ಎಳೆಗಳು == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_ಬೀಗ<std::mutex> ಬೀಗ_2(mtx);
        ಪ್ರತಿಜ್ಞಾ(!Is_ಪೂರ್ಣಗೊಂಡ_Spawning);
        Is_ಪೂರ್ಣಗೊಂಡ_Spawning = ನಿಜ;
      }
      cv.notify_one();
    }
  });
#ಇಲ್ಲದಿದ್ದರೆ
  llvm_unreachable("threads are disabled");
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::BPThreadPool::wait() {
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_ಬೀಗ<std::mutex> ಬೀಗ_2(mtx);
    cv.wait(ಬೀಗ_2, [&]() { ಹಿಂತಿರುಗಿ Is_ಪೂರ್ಣಗೊಂಡ_Spawning; });
    ಪ್ರತಿಜ್ಞಾ(Is_ಪೂರ್ಣಗೊಂಡ_Spawning && ಅಂಕೆ_ಸಕ್ರಿಯ_ಎಳೆಗಳು == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#ಇಲ್ಲದಿದ್ದರೆ
  llvm_unreachable("threads are disabled");
#ಆದರೆ_ಅಂತ್ಯ
}

ಸಮತೋಲಿತ_Partitioning::ಸಮತೋಲಿತ_Partitioning(
    ಸ್ಥಿರಾಂಕ ಸಮತೋಲಿತ_Partitioning_Config &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 1; I < LOG_CACHE_ಗಾತ್ರ; I++)
    Log2Cache[I] = std::log2(I);
}

ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::ಸತತ(std::ಸದಿಶ<BPFunction_ಗಂಟು> &Nodes) ಸ್ಥಿರಾಂಕ {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.ಗಾತ್ರ_3(), Config.ವಿಭಜಿಸು_ಆಳ, Config.Iterations_ಪ್ರತಿ_ವಿಭಜಿಸು));
  std::optional<BPThreadPool> TP;
#ಆದರೆ LLVM_ENABLE_ಎಳೆಗಳು
  DefaultThreadPool TheThreadPool;
  ಆದರೆ (Config.Task_ವಿಭಜಿಸು_ಆಳ > 1)
    TP.emplace(TheThreadPool);
#ಆದರೆ_ಅಂತ್ಯ

  // Record the input order
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Nodes.ಗಾತ್ರ_3(); I++)
    Nodes[I].ಒಳಹರಿವು_ಕ್ರಮ_ಸೂಚ್ಯಂಕ = I;

  ಸ್ವಯಂಚಾಲಿತ NodesRange = llvm::make_range(Nodes.begin(), Nodes.ಕೊನೆ());
  ಸ್ವಯಂಚಾಲಿತ BisectTask = [ಇದು, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  ಆದರೆ (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } ಇಲ್ಲದಿದ್ದರೆ {
    BisectTask();
  }

  llvm::stable_ವಿಂಗಡಿಸು(NodesRange, [](ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &L, ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R) {
    ಹಿಂತಿರುಗಿ L.ಪಾತ್ರೆ < R.ಪಾತ್ರೆ;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::bisect(ಸ್ಥಿರಾಂಕ Function_ಗಂಟು_Range Nodes,
                                  ಚಿಹ್ನೆರಹಿತ Rec_ಆಳ, ಚಿಹ್ನೆರಹಿತ ಬೇರು_ಪಾತ್ರೆ,
                                  ಚಿಹ್ನೆರಹಿತ ಸ್ಥಾನಾಂತರ,
                                  std::optional<BPThreadPool> &TP) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Nodes = llvm::ಗಾತ್ರ_3(Nodes);
  ಆದರೆ (ಅಂಕೆ_Nodes <= 1 || Rec_ಆಳ >= Config.ವಿಭಜಿಸು_ಆಳ) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::ವಿಂಗಡಿಸು(Nodes, [](ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &L, ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R) {
      ಹಿಂತಿರುಗಿ L.ಒಳಹರಿವು_ಕ್ರಮ_ಸೂಚ್ಯಂಕ < R.ಒಳಹರಿವು_ಕ್ರಮ_ಸೂಚ್ಯಂಕ;
    });
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes)
      N.ಪಾತ್ರೆ = ಸ್ಥಾನಾಂತರ++;
    ಹಿಂತಿರುಗಿ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              ಅಂಕೆ_Nodes, ಬೇರು_ಪಾತ್ರೆ));

  std::mt19937 RNG(ಬೇರು_ಪಾತ್ರೆ);

  ಚಿಹ್ನೆರಹಿತ ಎಡ_ಪಾತ್ರೆ = 2 * ಬೇರು_ಪಾತ್ರೆ;
  ಚಿಹ್ನೆರಹಿತ ಬಲ_ಪಾತ್ರೆ = 2 * ಬೇರು_ಪಾತ್ರೆ + 1;

  // Split into two and assign to the left and right buckets
  ವಿಭಜಿಸು(Nodes, ಎಡ_ಪಾತ್ರೆ);

  ಸತತ_Iterations(Nodes, ಎಡ_ಪಾತ್ರೆ, ಬಲ_ಪಾತ್ರೆ, RNG);

  // Split nodes wrt the resulting buckets
  ಸ್ವಯಂಚಾಲಿತ NodesMid =
      llvm::ವಿಭಜನೆ(Nodes, [&](ಸ್ವಯಂಚಾಲಿತ &N) { ಹಿಂತಿರುಗಿ N.ಪಾತ್ರೆ == ಎಡ_ಪಾತ್ರೆ; });
  ಚಿಹ್ನೆರಹಿತ Mid_ಸ್ಥಾನಾಂತರ = ಸ್ಥಾನಾಂತರ + std::ದೂರ(Nodes.begin(), NodesMid);

  ಸ್ವಯಂಚಾಲಿತ ಎಡ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  ಸ್ವಯಂಚಾಲಿತ ಬಲ_Nodes = llvm::make_range(NodesMid, Nodes.ಕೊನೆ());

  ಸ್ವಯಂಚಾಲಿತ ಎಡ_Rec_Task = [ಇದು, ಎಡ_Nodes, Rec_ಆಳ, ಎಡ_ಪಾತ್ರೆ, ಸ್ಥಾನಾಂತರ, &TP]() {
    bisect(ಎಡ_Nodes, Rec_ಆಳ + 1, ಎಡ_ಪಾತ್ರೆ, ಸ್ಥಾನಾಂತರ, TP);
  };
  ಸ್ವಯಂಚಾಲಿತ ಬಲ_Rec_Task = [ಇದು, ಬಲ_Nodes, Rec_ಆಳ, ಬಲ_ಪಾತ್ರೆ, Mid_ಸ್ಥಾನಾಂತರ,
                       &TP]() {
    bisect(ಬಲ_Nodes, Rec_ಆಳ + 1, ಬಲ_ಪಾತ್ರೆ, Mid_ಸ್ಥಾನಾಂತರ, TP);
  };

  ಆದರೆ (TP && Rec_ಆಳ < Config.Task_ವಿಭಜಿಸು_ಆಳ && ಅಂಕೆ_Nodes >= 4) {
    TP->async(std::move(ಎಡ_Rec_Task));
    TP->async(std::move(ಬಲ_Rec_Task));
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಎಡ_Rec_Task();
    ಬಲ_Rec_Task();
  }
}

ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::ಸತತ_Iterations(ಸ್ಥಿರಾಂಕ Function_ಗಂಟು_Range Nodes,
                                         ಚಿಹ್ನೆರಹಿತ ಎಡ_ಪಾತ್ರೆ,
                                         ಚಿಹ್ನೆರಹಿತ ಬಲ_ಪಾತ್ರೆ,
                                         std::mt19937 &RNG) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Nodes = llvm::ಗಾತ್ರ_3(Nodes);
  DenseMap<BPFunction_ಗಂಟು::Utility_ಗಂಟು_T, ಚಿಹ್ನೆರಹಿತ> Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes)
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes)
      ++Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](ಸ್ವಯಂಚಾಲಿತ &UN) {
      ಚಿಹ್ನೆರಹಿತ UNI = Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ[UN];
      ಹಿಂತಿರುಗಿ UNI == 1 || UNI == ಅಂಕೆ_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ.clear();
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes)
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes)
      UN = Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ.ಸೇರಿಸು({UN, Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ.ಗಾತ್ರ_3()}).ಮೊದಲ_2->ಎರಡನೇ_2;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ಗಂಟು_ಸೂಚ್ಯಂಕ.ಗಾತ್ರ_3());
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes) {
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes) {
      ಪ್ರತಿಜ್ಞಾ(UN < Signatures.ಗಾತ್ರ_3());
      ಆದರೆ (N.ಪಾತ್ರೆ == ಎಡ_ಪಾತ್ರೆ) {
        Signatures[UN].ಎಡ_ಎಣಿಕೆ++;
      } ಇಲ್ಲದಿದ್ದರೆ {
        Signatures[UN].ಬಲ_ಎಣಿಕೆ++;
      }
    }
  }

  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < Config.Iterations_ಪ್ರತಿ_ವಿಭಜಿಸು; I++) {
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Moved_Nodes =
        ಸತತ_Iteration(Nodes, ಎಡ_ಪಾತ್ರೆ, ಬಲ_ಪಾತ್ರೆ, Signatures, RNG);
    ಆದರೆ (ಅಂಕೆ_Moved_Nodes == 0)
      ನಿಲ್ಲಿಸು;
  }
}

ಚಿಹ್ನೆರಹಿತ ಸಮತೋಲಿತ_Partitioning::ಸತತ_Iteration(ಸ್ಥಿರಾಂಕ Function_ಗಂಟು_Range Nodes,
                                            ಚಿಹ್ನೆರಹಿತ ಎಡ_ಪಾತ್ರೆ,
                                            ಚಿಹ್ನೆರಹಿತ ಬಲ_ಪಾತ್ರೆ,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ಸ್ಥಿರಾಂಕ {
  // Init signature cost caches
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &Signature : Signatures) {
    ಆದರೆ (Signature.CachedGainIsValid)
      ಮುಂದುವರಿ;
    ಚಿಹ್ನೆರಹಿತ L = Signature.ಎಡ_ಎಣಿಕೆ;
    ಚಿಹ್ನೆರಹಿತ R = Signature.ಬಲ_ಎಣಿಕೆ;
    ಪ್ರತಿಜ್ಞಾ((L > 0 || R > 0) && "incorrect signature");
    ದಶಮಾಂಶ ವೆಚ್ಚ = log_ವೆಚ್ಚ(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    ಆದರೆ (L > 0)
      Signature.CachedGainLR = ವೆಚ್ಚ - log_ವೆಚ್ಚ(L - 1, R + 1);
    ಆದರೆ (R > 0)
      Signature.CachedGainRL = ವೆಚ್ಚ - log_ವೆಚ್ಚ(L + 1, R - 1);
    Signature.CachedGainIsValid = ನಿಜ;
  }

  // Compute move gains
  ಬಳಸಿ GainPair = std::pair<ದಶಮಾಂಶ, BPFunction_ಗಂಟು *>;
  std::ಸದಿಶ<GainPair> Gains;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : Nodes) {
    ತಾರ್ಕಿಕ From_ಎಡ_ಗಮ್ಯ_ಬಲ = (N.ಪಾತ್ರೆ == ಎಡ_ಪಾತ್ರೆ);
    ದಶಮಾಂಶ Gain = moveGain(N, From_ಎಡ_ಗಮ್ಯ_ಬಲ, Signatures);
    Gains.ತಳ್ಳು_ಹಿಂಭಾಗ(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  ಸ್ವಯಂಚಾಲಿತ ಎಡ_ಕೊನೆ = llvm::ವಿಭಜನೆ(
      Gains, [&](ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &GP) { ಹಿಂತಿರುಗಿ GP.ಎರಡನೇ_2->ಪಾತ್ರೆ == ಎಡ_ಪಾತ್ರೆ; });
  ಸ್ವಯಂಚಾಲಿತ ಎಡ_Range = llvm::make_range(Gains.begin(), ಎಡ_ಕೊನೆ);
  ಸ್ವಯಂಚಾಲಿತ ಬಲ_Range = llvm::make_range(ಎಡ_ಕೊನೆ, Gains.ಕೊನೆ());

  // Sort gains in descending order
  ಸ್ವಯಂಚಾಲಿತ LargerGain = [](ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &L, ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R) {
    ಹಿಂತಿರುಗಿ L.ಮೊದಲ_2 > R.ಮೊದಲ_2;
  };
  llvm::stable_ವಿಂಗಡಿಸು(ಎಡ_Range, LargerGain);
  llvm::stable_ವಿಂಗಡಿಸು(ಬಲ_Range, LargerGain);

  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Moved_ದತ್ತಾಂಶ_Vertices = 0;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ [ಎಡ_Pair, ಬಲ_Pair] : llvm::zip(ಎಡ_Range, ಬಲ_Range)) {
    ಸ್ವಯಂಚಾಲಿತ &[ಎಡ_Gain, ಎಡ_ಗಂಟು] = ಎಡ_Pair;
    ಸ್ವಯಂಚಾಲಿತ &[ಬಲ_Gain, ಬಲ_ಗಂಟು] = ಬಲ_Pair;
    // Stop when the gain is no longer beneficial
    ಆದರೆ (ಎಡ_Gain + ಬಲ_Gain <= 0.f)
      ನಿಲ್ಲಿಸು;
    // Try to exchange the nodes between buckets
    ಆದರೆ (move_Function_ಗಂಟು(*ಎಡ_ಗಂಟು, ಎಡ_ಪಾತ್ರೆ, ಬಲ_ಪಾತ್ರೆ, Signatures, RNG))
      ++ಅಂಕೆ_Moved_ದತ್ತಾಂಶ_Vertices;
    ಆದರೆ (move_Function_ಗಂಟು(*ಬಲ_ಗಂಟು, ಎಡ_ಪಾತ್ರೆ, ಬಲ_ಪಾತ್ರೆ, Signatures, RNG))
      ++ಅಂಕೆ_Moved_ದತ್ತಾಂಶ_Vertices;
  }
  ಹಿಂತಿರುಗಿ ಅಂಕೆ_Moved_ದತ್ತಾಂಶ_Vertices;
}

ತಾರ್ಕಿಕ ಸಮತೋಲಿತ_Partitioning::move_Function_ಗಂಟು(BPFunction_ಗಂಟು &N,
                                            ಚಿಹ್ನೆರಹಿತ ಎಡ_ಪಾತ್ರೆ,
                                            ಚಿಹ್ನೆರಹಿತ ಬಲ_ಪಾತ್ರೆ,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ಸ್ಥಿರಾಂಕ {
  // Sometimes we skip the move. This helps to escape local optima
  ಆದರೆ (std::ಏಕರೂಪ_real_distribution<ದಶಮಾಂಶ>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ತಾರ್ಕಿಕ From_ಎಡ_ಗಮ್ಯ_ಬಲ = (N.ಪಾತ್ರೆ == ಎಡ_ಪಾತ್ರೆ);
  // Update the current bucket
  N.ಪಾತ್ರೆ = (From_ಎಡ_ಗಮ್ಯ_ಬಲ ? ಬಲ_ಪಾತ್ರೆ : ಎಡ_ಪಾತ್ರೆ);

  // Update signatures and invalidate gain cache
  ಆದರೆ (From_ಎಡ_ಗಮ್ಯ_ಬಲ) {
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes) {
      ಸ್ವಯಂಚಾಲಿತ &Signature = Signatures[UN];
      Signature.ಎಡ_ಎಣಿಕೆ--;
      Signature.ಬಲ_ಎಣಿಕೆ++;
      Signature.CachedGainIsValid = ಸುಳ್ಳು;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes) {
      ಸ್ವಯಂಚಾಲಿತ &Signature = Signatures[UN];
      Signature.ಎಡ_ಎಣಿಕೆ++;
      Signature.ಬಲ_ಎಣಿಕೆ--;
      Signature.CachedGainIsValid = ಸುಳ್ಳು;
    }
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ ಸಮತೋಲಿತ_Partitioning::ವಿಭಜಿಸು(ಸ್ಥಿರಾಂಕ Function_ಗಂಟು_Range Nodes,
                                 ಚಿಹ್ನೆರಹಿತ ಆರಂಭ_ಪಾತ್ರೆ) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Nodes = llvm::ಗಾತ್ರ_3(Nodes);
  ಸ್ವಯಂಚಾಲಿತ NodesMid = Nodes.begin() + (ಅಂಕೆ_Nodes + 1) / 2;

  llvm::ವಿಂಗಡಿಸು(Nodes, [](ಸ್ವಯಂಚಾಲಿತ &L, ಸ್ವಯಂಚಾಲಿತ &R) {
    ಹಿಂತಿರುಗಿ L.ಒಳಹರಿವು_ಕ್ರಮ_ಸೂಚ್ಯಂಕ < R.ಒಳಹರಿವು_ಕ್ರಮ_ಸೂಚ್ಯಂಕ;
  });

  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.ಪಾತ್ರೆ = ಆರಂಭ_ಪಾತ್ರೆ;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &N : llvm::make_range(NodesMid, Nodes.ಕೊನೆ()))
    N.ಪಾತ್ರೆ = ಆರಂಭ_ಪಾತ್ರೆ + 1;
}

ದಶಮಾಂಶ ಸಮತೋಲಿತ_Partitioning::moveGain(ಸ್ಥಿರಾಂಕ BPFunction_ಗಂಟು &N,
                                     ತಾರ್ಕಿಕ From_ಎಡ_ಗಮ್ಯ_ಬಲ,
                                     ಸ್ಥಿರಾಂಕ SignaturesT &Signatures) {
  ದಶಮಾಂಶ Gain = 0.f;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ &UN : N.UtilityNodes)
    Gain += (From_ಎಡ_ಗಮ್ಯ_ಬಲ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  ಹಿಂತಿರುಗಿ Gain;
}

ದಶಮಾಂಶ ಸಮತೋಲಿತ_Partitioning::log_ವೆಚ್ಚ(ಚಿಹ್ನೆರಹಿತ X, ಚಿಹ್ನೆರಹಿತ Y) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

ದಶಮಾಂಶ ಸಮತೋಲಿತ_Partitioning::log2Cached(ಚಿಹ್ನೆರಹಿತ i) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ (i < LOG_CACHE_ಗಾತ್ರ) ? Log2Cache[i] : std::log2(i);
}
