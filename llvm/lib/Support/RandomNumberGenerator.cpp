//===-- RandomNumberGenerator.cpp - Implement RNG class -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements deterministic random number generation (RNG).
// The current implementation is NOT cryptographically secure as it uses
// the C++11 <random> facilities.
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/RandomNumberGenerator.h"

#അന്തര്ഭാവ "DebugOptions.h"

#അന്തര്ഭാവ "llvm/Support/CommandLine.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#അന്തര്ഭാവ "llvm/Support/Windows/WindowsSupport.h"
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ "Unix/Unix.h"
#endif

using നാമാകാശ llvm;

#define DEBUG_TYPE "rng"
നാമാകാശ {
ഘടന CreateSeed {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<uint64_t>(
        "rng-seed", cl::മൂല്യം_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
സ്ഥിതം ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
ശൂന്യം llvm::initRandomSeedOptions() { *Seed; }

Random_സംഖ്യ_Generator::Random_സംഖ്യ_Generator(StringRef Salt) {
  LLVM_DEBUG(എങ്കിൽ (*Seed == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::vector<uint32_t> Data;
  Data.resize(2 + Salt.size());
  Data[0] = *Seed;
  Data[1] = *Seed >> 32;

  llvm::copy(Salt, Data.begin() + 2);

  std::seed_seq SeedSeq(Data.begin(), Data.end());
  Generator.seed(SeedSeq);
}

Random_സംഖ്യ_Generator::result_type Random_സംഖ്യ_Generator::operator()() {
  തിരികെ Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(ശൂന്യം *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  എങ്കിൽ (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    എങ്കിൽ (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      തിരികെ std::error_code();
  }
  തിരികെ std::error_code(GetLastError(), std::system_category());
#അല്ലെങ്കിൽ
  പൂർണ്ണസംഖ്യ Fd = open("/dev/urandom", O_RDONLY);
  എങ്കിൽ (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = പഠ(Fd, Buffer, Size);
    എങ്കിൽ (BytesRead == -1)
      Ret = errnoAsErrorCode();
    അല്ലെങ്കിൽ എങ്കിൽ (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    എങ്കിൽ (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    തിരികെ Ret;
  }
  തിരികെ errnoAsErrorCode();
#endif
}
