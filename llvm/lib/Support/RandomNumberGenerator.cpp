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

#અન્તર્ભાવ "llvm/Support/RandomNumberGenerator.h"

#અન્તર્ભાવ "DebugOptions.h"

#અન્તર્ભાવ "llvm/Support/CommandLine.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#અન્તર્ભાવ "llvm/Support/Windows/WindowsSupport.h"
#નહીંતર
#અન્તર્ભાવ "Unix/Unix.h"
#endif

using નામાકાશ llvm;

#define DEBUG_TYPE "rng"
નામાકાશ {
સંરચના CreateSeed {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<uint64_t>(
        "rng-seed", cl::કિંમત_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
સ્થિર ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
શૂન્ય llvm::initRandomSeedOptions() { *Seed; }

Random_સંખ્યા_Generator::Random_સંખ્યા_Generator(StringRef Salt) {
  LLVM_DEBUG(જો (*Seed == 0) dbgs()
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

Random_સંખ્યા_Generator::result_type Random_સંખ્યા_Generator::operator()() {
  પાછા Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(શૂન્ય *Buffer, size_t Size) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  જો (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    જો (CryptGenRandom(hProvider, Size, static_cast<BYTE *>(Buffer)))
      પાછા std::error_code();
  }
  પાછા std::error_code(GetLastError(), std::system_category());
#નહીંતર
  પૂર્ણાંક Fd = open("/dev/urandom", O_RDONLY);
  જો (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = પઠ(Fd, Buffer, Size);
    જો (BytesRead == -1)
      Ret = errnoAsErrorCode();
    નહીંતર જો (BytesRead != static_cast<ssize_t>(Size))
      Ret = std::error_code(EIO, std::system_category());
    જો (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    પાછા Ret;
  }
  પાછા errnoAsErrorCode();
#endif
}
