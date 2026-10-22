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

#ಅನ್ತರ್ಭಾವ "llvm/Support/RandomNumberGenerator.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ "Unix/Unix.h"
#endif

using ನಾಮಾಕಾಶ llvm;

#define DEBUG_TYPE "rng"
ನಾಮಾಕಾಶ {
ಸಂರಚನಾ CreateSeed {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಪ್ರತ್ಯಾಗಮ ನೂತನ cl::opt<uint64_t>(
        "rng-seed", cl::ಮೌಲ್ಯ_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
ಶೂನ್ಯ llvm::initRandomSeedOptions() { *Seed; }

Random_ಸಂಖ್ಯೆ_Generator::Random_ಸಂಖ್ಯೆ_Generator(StringRef Salt) {
  LLVM_DEBUG(ಯದಿ (*Seed == 0) dbgs()
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

Random_ಸಂಖ್ಯೆ_Generator::result_type Random_ಸಂಖ್ಯೆ_Generator::operator()() {
  ಪ್ರತ್ಯಾಗಮ Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(ಶೂನ್ಯ *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  ಯದಿ (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    ಯದಿ (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      ಪ್ರತ್ಯಾಗಮ std::error_code();
  }
  ಪ್ರತ್ಯಾಗಮ std::error_code(GetLastError(), std::system_category());
#ಅನ್ಯಥಾ
  ಪೂರ್ಣಾಙ್ಕ Fd = open("/dev/urandom", O_RDONLY);
  ಯದಿ (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = ಪಠ(Fd, Buffer, Size);
    ಯದಿ (BytesRead == -1)
      Ret = errnoAsErrorCode();
    ಅನ್ಯಥಾ ಯದಿ (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    ಯದಿ (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    ಪ್ರತ್ಯಾಗಮ Ret;
  }
  ಪ್ರತ್ಯಾಗಮ errnoAsErrorCode();
#endif
}
