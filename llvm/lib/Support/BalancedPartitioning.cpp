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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/BalancedPartitioning.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h" // for LLVM_ENABLE_THREADS
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormatVariadic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ThreadPool.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "balanced-partitioning"

ਖਾਲੀ BPFunction_ਗੰਢ::dump(raw_ostream &OS) ਅਚਲ {
  OS << formatv("{{ID={0} Utilities={{{1:$[,]}} Bucket={2}}", Id,
                make_range(UtilityNodes.begin(), UtilityNodes.ਸਿਰਾ()), ਬਾਲਟੀ);
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ Func>
ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::BPThreadPool::async(Func &&F) {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  // This new thread could spawn more threads, so mark it as active
  ++ਅੰਕ_ਸਰਗਰਮ_ਧਾਗੇ;
  TheThreadPool.async([ਇਹ, F]() {
    // Run the task
    F();

    // This thread will no longer spawn new threads, so mark it as inactive
    ਜੇ (--ਅੰਕ_ਸਰਗਰਮ_ਧਾਗੇ == 0) {
      // There are no more active threads, so mark as finished and notify
      {
        std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ(mtx);
        ਪ੍ਰਤਿਜ੍ਞਾ(!Is_ਮੁਕੰਮਲ_Spawning);
        Is_ਮੁਕੰਮਲ_Spawning = ਸੱਚ;
      }
      cv.notify_one();
    }
  });
#ਨਹੀਂਤਾਂ
  llvm_unreachable("threads are disabled");
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::BPThreadPool::wait() {
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  // TODO: We could remove the mutex and condition variable and use
  // std::atomic::wait() instead, but that isn't available until C++20
  {
    std::unique_ਤਾਲਾ<std::mutex> ਤਾਲਾ(mtx);
    cv.wait(ਤਾਲਾ, [&]() { ਵਾਪਸ Is_ਮੁਕੰਮਲ_Spawning; });
    ਪ੍ਰਤਿਜ੍ਞਾ(Is_ਮੁਕੰਮਲ_Spawning && ਅੰਕ_ਸਰਗਰਮ_ਧਾਗੇ == 0);
  }
  // Now we can call ThreadPool::wait() since all tasks have been submitted
  TheThreadPool.wait();
#ਨਹੀਂਤਾਂ
  llvm_unreachable("threads are disabled");
#ਜੇ_ਅੰਤ
}

ਸੰਤੁਲਿਤ_Partitioning::ਸੰਤੁਲਿਤ_Partitioning(
    ਅਚਲ ਸੰਤੁਲਿਤ_Partitioning_Config &Config)
    : Config(Config) {
  // Pre-computing log2 values
  Log2Cache[0] = 0.0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 1; I < LOG_CACHE_ਮਾਪ; I++)
    Log2Cache[I] = std::log2(I);
}

ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::ਲੜੀਵਾਰ(std::ਸਦਿਸ਼<BPFunction_ਗੰਢ> &Nodes) ਅਚਲ {
  LLVM_DEBUG(
      dbgs() << format(
          "Partitioning %d nodes using depth %d and %d iterations per split\n",
          Nodes.ਮਾਪ_2(), Config.ਵੰਡੋ_ਡੂੰਘਾਈ, Config.Iterations_ਪ੍ਰਤੀ_ਵੰਡੋ));
  std::optional<BPThreadPool> TP;
#ਜੇ LLVM_ENABLE_ਧਾਗੇ
  DefaultThreadPool TheThreadPool;
  ਜੇ (Config.Task_ਵੰਡੋ_ਡੂੰਘਾਈ > 1)
    TP.emplace(TheThreadPool);
#ਜੇ_ਅੰਤ

  // Record the input order
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Nodes.ਮਾਪ_2(); I++)
    Nodes[I].ਨਿਵੇਸ਼_ਕ੍ਰਮ_ਸੂਚਕਾਂਕ = I;

  ਸਵੈਚਾਲਿਤ NodesRange = llvm::make_range(Nodes.begin(), Nodes.ਸਿਰਾ());
  ਸਵੈਚਾਲਿਤ BisectTask = [ਇਹ, NodesRange, &TP]() {
    bisect(NodesRange, /*RecDepth=*/0, /*RootBucket=*/1, /*Offset=*/0, TP);
  };
  ਜੇ (TP) {
    TP->async(std::move(BisectTask));
    TP->wait();
  } ਨਹੀਂਤਾਂ {
    BisectTask();
  }

  llvm::stable_ਛਾਂਟੋ(NodesRange, [](ਅਚਲ ਸਵੈਚਾਲਿਤ &L, ਅਚਲ ਸਵੈਚਾਲਿਤ &R) {
    ਵਾਪਸ L.ਬਾਲਟੀ < R.ਬਾਲਟੀ;
  });

  LLVM_DEBUG(dbgs() << "Balanced partitioning completed\n");
}

ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::bisect(ਅਚਲ Function_ਗੰਢ_Range Nodes,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ Rec_ਡੂੰਘਾਈ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਜੜ੍ਹ_ਬਾਲਟੀ,
                                  ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਥ,
                                  std::optional<BPThreadPool> &TP) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Nodes = llvm::ਮਾਪ_2(Nodes);
  ਜੇ (ਅੰਕ_Nodes <= 1 || Rec_ਡੂੰਘਾਈ >= Config.ਵੰਡੋ_ਡੂੰਘਾਈ) {
    // We've reach the lowest level of the recursion tree. Fall back to the
    // original order and assign to buckets.
    llvm::ਛਾਂਟੋ(Nodes, [](ਅਚਲ ਸਵੈਚਾਲਿਤ &L, ਅਚਲ ਸਵੈਚਾਲਿਤ &R) {
      ਵਾਪਸ L.ਨਿਵੇਸ਼_ਕ੍ਰਮ_ਸੂਚਕਾਂਕ < R.ਨਿਵੇਸ਼_ਕ੍ਰਮ_ਸੂਚਕਾਂਕ;
    });
    ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes)
      N.ਬਾਲਟੀ = ਵਿੱਥ++;
    ਵਾਪਸ;
  }

  LLVM_DEBUG(dbgs() << format("Bisect with %d nodes and root bucket %d\n",
                              ਅੰਕ_Nodes, ਜੜ੍ਹ_ਬਾਲਟੀ));

  std::mt19937 RNG(ਜੜ੍ਹ_ਬਾਲਟੀ);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ_ਬਾਲਟੀ = 2 * ਜੜ੍ਹ_ਬਾਲਟੀ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੱਜਾ_ਬਾਲਟੀ = 2 * ਜੜ੍ਹ_ਬਾਲਟੀ + 1;

  // Split into two and assign to the left and right buckets
  ਵੰਡੋ(Nodes, ਖੱਬਾ_ਬਾਲਟੀ);

  ਲੜੀਵਾਰ_Iterations(Nodes, ਖੱਬਾ_ਬਾਲਟੀ, ਸੱਜਾ_ਬਾਲਟੀ, RNG);

  // Split nodes wrt the resulting buckets
  ਸਵੈਚਾਲਿਤ NodesMid =
      llvm::ਹਿੱਸੇ_ਕਰੋ(Nodes, [&](ਸਵੈਚਾਲਿਤ &N) { ਵਾਪਸ N.ਬਾਲਟੀ == ਖੱਬਾ_ਬਾਲਟੀ; });
  ਬਿਨਾਨਿਸ਼ਾਨੀ Mid_ਵਿੱਥ = ਵਿੱਥ + std::ਦੂਰੀ(Nodes.begin(), NodesMid);

  ਸਵੈਚਾਲਿਤ ਖੱਬਾ_Nodes = llvm::make_range(Nodes.begin(), NodesMid);
  ਸਵੈਚਾਲਿਤ ਸੱਜਾ_Nodes = llvm::make_range(NodesMid, Nodes.ਸਿਰਾ());

  ਸਵੈਚਾਲਿਤ ਖੱਬਾ_Rec_Task = [ਇਹ, ਖੱਬਾ_Nodes, Rec_ਡੂੰਘਾਈ, ਖੱਬਾ_ਬਾਲਟੀ, ਵਿੱਥ, &TP]() {
    bisect(ਖੱਬਾ_Nodes, Rec_ਡੂੰਘਾਈ + 1, ਖੱਬਾ_ਬਾਲਟੀ, ਵਿੱਥ, TP);
  };
  ਸਵੈਚਾਲਿਤ ਸੱਜਾ_Rec_Task = [ਇਹ, ਸੱਜਾ_Nodes, Rec_ਡੂੰਘਾਈ, ਸੱਜਾ_ਬਾਲਟੀ, Mid_ਵਿੱਥ,
                       &TP]() {
    bisect(ਸੱਜਾ_Nodes, Rec_ਡੂੰਘਾਈ + 1, ਸੱਜਾ_ਬਾਲਟੀ, Mid_ਵਿੱਥ, TP);
  };

  ਜੇ (TP && Rec_ਡੂੰਘਾਈ < Config.Task_ਵੰਡੋ_ਡੂੰਘਾਈ && ਅੰਕ_Nodes >= 4) {
    TP->async(std::move(ਖੱਬਾ_Rec_Task));
    TP->async(std::move(ਸੱਜਾ_Rec_Task));
  } ਨਹੀਂਤਾਂ {
    ਖੱਬਾ_Rec_Task();
    ਸੱਜਾ_Rec_Task();
  }
}

ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::ਲੜੀਵਾਰ_Iterations(ਅਚਲ Function_ਗੰਢ_Range Nodes,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ_ਬਾਲਟੀ,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੱਜਾ_ਬਾਲਟੀ,
                                         std::mt19937 &RNG) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Nodes = llvm::ਮਾਪ_2(Nodes);
  DenseMap<BPFunction_ਗੰਢ::Utility_ਗੰਢ_T, ਬਿਨਾਨਿਸ਼ਾਨੀ> Utility_ਗੰਢ_ਸੂਚਕਾਂਕ;
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes)
    ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes)
      ++Utility_ਗੰਢ_ਸੂਚਕਾਂਕ[UN];
  // Remove utility nodes if they have just one edge or are connected to all
  // functions
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes)
    llvm::erase_if(N.UtilityNodes, [&](ਸਵੈਚਾਲਿਤ &UN) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ UNI = Utility_ਗੰਢ_ਸੂਚਕਾਂਕ[UN];
      ਵਾਪਸ UNI == 1 || UNI == ਅੰਕ_Nodes;
    });

  // Renumber utility nodes so they can be used to index into Signatures
  Utility_ਗੰਢ_ਸੂਚਕਾਂਕ.clear();
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes)
    ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes)
      UN = Utility_ਗੰਢ_ਸੂਚਕਾਂਕ.ਪਾਓ({UN, Utility_ਗੰਢ_ਸੂਚਕਾਂਕ.ਮਾਪ_2()}).ਪਹਿਲਾ_2->ਦੂਜਾ_2;

  // Initialize signatures
  SignaturesT Signatures(/*Size=*/Utility_ਗੰਢ_ਸੂਚਕਾਂਕ.ਮਾਪ_2());
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes) {
    ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes) {
      ਪ੍ਰਤਿਜ੍ਞਾ(UN < Signatures.ਮਾਪ_2());
      ਜੇ (N.ਬਾਲਟੀ == ਖੱਬਾ_ਬਾਲਟੀ) {
        Signatures[UN].ਖੱਬਾ_ਗਿਣਤੀ++;
      } ਨਹੀਂਤਾਂ {
        Signatures[UN].ਸੱਜਾ_ਗਿਣਤੀ++;
      }
    }
  }

  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < Config.Iterations_ਪ੍ਰਤੀ_ਵੰਡੋ; I++) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Moved_Nodes =
        ਲੜੀਵਾਰ_Iteration(Nodes, ਖੱਬਾ_ਬਾਲਟੀ, ਸੱਜਾ_ਬਾਲਟੀ, Signatures, RNG);
    ਜੇ (ਅੰਕ_Moved_Nodes == 0)
      ਰੋਕੋ;
  }
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੰਤੁਲਿਤ_Partitioning::ਲੜੀਵਾਰ_Iteration(ਅਚਲ Function_ਗੰਢ_Range Nodes,
                                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ_ਬਾਲਟੀ,
                                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੱਜਾ_ਬਾਲਟੀ,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ਅਚਲ {
  // Init signature cost caches
  ਲਈ (ਸਵੈਚਾਲਿਤ &Signature : Signatures) {
    ਜੇ (Signature.CachedGainIsValid)
      ਜਾਰੀ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ L = Signature.ਖੱਬਾ_ਗਿਣਤੀ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ R = Signature.ਸੱਜਾ_ਗਿਣਤੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ((L > 0 || R > 0) && "incorrect signature");
    ਦਸ਼ਮਲਵ ਲਾਗਤ = log_ਲਾਗਤ(L, R);
    Signature.CachedGainLR = 0.f;
    Signature.CachedGainRL = 0.f;
    ਜੇ (L > 0)
      Signature.CachedGainLR = ਲਾਗਤ - log_ਲਾਗਤ(L - 1, R + 1);
    ਜੇ (R > 0)
      Signature.CachedGainRL = ਲਾਗਤ - log_ਲਾਗਤ(L + 1, R - 1);
    Signature.CachedGainIsValid = ਸੱਚ;
  }

  // Compute move gains
  ਵਰਤੋਂ GainPair = std::pair<ਦਸ਼ਮਲਵ, BPFunction_ਗੰਢ *>;
  std::ਸਦਿਸ਼<GainPair> Gains;
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : Nodes) {
    ਤਾਰਕਿਕ From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ = (N.ਬਾਲਟੀ == ਖੱਬਾ_ਬਾਲਟੀ);
    ਦਸ਼ਮਲਵ Gain = moveGain(N, From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ, Signatures);
    Gains.ਧੱਕੋ_ਪਿੱਠ(std::make_pair(Gain, &N));
  }

  // Collect left and right gains
  ਸਵੈਚਾਲਿਤ ਖੱਬਾ_ਸਿਰਾ = llvm::ਹਿੱਸੇ_ਕਰੋ(
      Gains, [&](ਅਚਲ ਸਵੈਚਾਲਿਤ &GP) { ਵਾਪਸ GP.ਦੂਜਾ_2->ਬਾਲਟੀ == ਖੱਬਾ_ਬਾਲਟੀ; });
  ਸਵੈਚਾਲਿਤ ਖੱਬਾ_Range = llvm::make_range(Gains.begin(), ਖੱਬਾ_ਸਿਰਾ);
  ਸਵੈਚਾਲਿਤ ਸੱਜਾ_Range = llvm::make_range(ਖੱਬਾ_ਸਿਰਾ, Gains.ਸਿਰਾ());

  // Sort gains in descending order
  ਸਵੈਚਾਲਿਤ LargerGain = [](ਅਚਲ ਸਵੈਚਾਲਿਤ &L, ਅਚਲ ਸਵੈਚਾਲਿਤ &R) {
    ਵਾਪਸ L.ਪਹਿਲਾ_2 > R.ਪਹਿਲਾ_2;
  };
  llvm::stable_ਛਾਂਟੋ(ਖੱਬਾ_Range, LargerGain);
  llvm::stable_ਛਾਂਟੋ(ਸੱਜਾ_Range, LargerGain);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Moved_ਅੰਕੜੇ_Vertices = 0;
  ਲਈ (ਸਵੈਚਾਲਿਤ [ਖੱਬਾ_Pair, ਸੱਜਾ_Pair] : llvm::zip(ਖੱਬਾ_Range, ਸੱਜਾ_Range)) {
    ਸਵੈਚਾਲਿਤ &[ਖੱਬਾ_Gain, ਖੱਬਾ_ਗੰਢ] = ਖੱਬਾ_Pair;
    ਸਵੈਚਾਲਿਤ &[ਸੱਜਾ_Gain, ਸੱਜਾ_ਗੰਢ] = ਸੱਜਾ_Pair;
    // Stop when the gain is no longer beneficial
    ਜੇ (ਖੱਬਾ_Gain + ਸੱਜਾ_Gain <= 0.f)
      ਰੋਕੋ;
    // Try to exchange the nodes between buckets
    ਜੇ (move_Function_ਗੰਢ(*ਖੱਬਾ_ਗੰਢ, ਖੱਬਾ_ਬਾਲਟੀ, ਸੱਜਾ_ਬਾਲਟੀ, Signatures, RNG))
      ++ਅੰਕ_Moved_ਅੰਕੜੇ_Vertices;
    ਜੇ (move_Function_ਗੰਢ(*ਸੱਜਾ_ਗੰਢ, ਖੱਬਾ_ਬਾਲਟੀ, ਸੱਜਾ_ਬਾਲਟੀ, Signatures, RNG))
      ++ਅੰਕ_Moved_ਅੰਕੜੇ_Vertices;
  }
  ਵਾਪਸ ਅੰਕ_Moved_ਅੰਕੜੇ_Vertices;
}

ਤਾਰਕਿਕ ਸੰਤੁਲਿਤ_Partitioning::move_Function_ਗੰਢ(BPFunction_ਗੰਢ &N,
                                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ_ਬਾਲਟੀ,
                                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੱਜਾ_ਬਾਲਟੀ,
                                            SignaturesT &Signatures,
                                            std::mt19937 &RNG) ਅਚਲ {
  // Sometimes we skip the move. This helps to escape local optima
  ਜੇ (std::ਇਕਸਾਰ_real_distribution<ਦਸ਼ਮਲਵ>(0.f, 1.f)(RNG) <=
      Config.SkipProbability)
    ਵਾਪਸ ਝੂਠ;

  ਤਾਰਕਿਕ From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ = (N.ਬਾਲਟੀ == ਖੱਬਾ_ਬਾਲਟੀ);
  // Update the current bucket
  N.ਬਾਲਟੀ = (From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ ? ਸੱਜਾ_ਬਾਲਟੀ : ਖੱਬਾ_ਬਾਲਟੀ);

  // Update signatures and invalidate gain cache
  ਜੇ (From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ) {
    ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes) {
      ਸਵੈਚਾਲਿਤ &Signature = Signatures[UN];
      Signature.ਖੱਬਾ_ਗਿਣਤੀ--;
      Signature.ਸੱਜਾ_ਗਿਣਤੀ++;
      Signature.CachedGainIsValid = ਝੂਠ;
    }
  } ਨਹੀਂਤਾਂ {
    ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes) {
      ਸਵੈਚਾਲਿਤ &Signature = Signatures[UN];
      Signature.ਖੱਬਾ_ਗਿਣਤੀ++;
      Signature.ਸੱਜਾ_ਗਿਣਤੀ--;
      Signature.CachedGainIsValid = ਝੂਠ;
    }
  }
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ ਸੰਤੁਲਿਤ_Partitioning::ਵੰਡੋ(ਅਚਲ Function_ਗੰਢ_Range Nodes,
                                 ਬਿਨਾਨਿਸ਼ਾਨੀ ਸ਼ੁਰੂ_ਬਾਲਟੀ) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Nodes = llvm::ਮਾਪ_2(Nodes);
  ਸਵੈਚਾਲਿਤ NodesMid = Nodes.begin() + (ਅੰਕ_Nodes + 1) / 2;

  llvm::ਛਾਂਟੋ(Nodes, [](ਸਵੈਚਾਲਿਤ &L, ਸਵੈਚਾਲਿਤ &R) {
    ਵਾਪਸ L.ਨਿਵੇਸ਼_ਕ੍ਰਮ_ਸੂਚਕਾਂਕ < R.ਨਿਵੇਸ਼_ਕ੍ਰਮ_ਸੂਚਕਾਂਕ;
  });

  ਲਈ (ਸਵੈਚਾਲਿਤ &N : llvm::make_range(Nodes.begin(), NodesMid))
    N.ਬਾਲਟੀ = ਸ਼ੁਰੂ_ਬਾਲਟੀ;
  ਲਈ (ਸਵੈਚਾਲਿਤ &N : llvm::make_range(NodesMid, Nodes.ਸਿਰਾ()))
    N.ਬਾਲਟੀ = ਸ਼ੁਰੂ_ਬਾਲਟੀ + 1;
}

ਦਸ਼ਮਲਵ ਸੰਤੁਲਿਤ_Partitioning::moveGain(ਅਚਲ BPFunction_ਗੰਢ &N,
                                     ਤਾਰਕਿਕ From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ,
                                     ਅਚਲ SignaturesT &Signatures) {
  ਦਸ਼ਮਲਵ Gain = 0.f;
  ਲਈ (ਸਵੈਚਾਲਿਤ &UN : N.UtilityNodes)
    Gain += (From_ਖੱਬਾ_ਮੰਜ਼ਿਲ_ਸੱਜਾ ? Signatures[UN].CachedGainLR
                             : Signatures[UN].CachedGainRL);
  ਵਾਪਸ Gain;
}

ਦਸ਼ਮਲਵ ਸੰਤੁਲਿਤ_Partitioning::log_ਲਾਗਤ(ਬਿਨਾਨਿਸ਼ਾਨੀ X, ਬਿਨਾਨਿਸ਼ਾਨੀ Y) ਅਚਲ {
  ਵਾਪਸ -(X * log2Cached(X + 1) + Y * log2Cached(Y + 1));
}

ਦਸ਼ਮਲਵ ਸੰਤੁਲਿਤ_Partitioning::log2Cached(ਬਿਨਾਨਿਸ਼ਾਨੀ i) ਅਚਲ {
  ਵਾਪਸ (i < LOG_CACHE_ਮਾਪ) ? Log2Cache[i] : std::log2(i);
}
