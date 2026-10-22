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

#অন্তর্ভাব "llvm/Support/RandomNumberGenerator.h"

#অন্তর্ভাব "DebugOptions.h"

#অন্তর্ভাব "llvm/Support/CommandLine.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/ManagedStatic.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#ifdef _WIN32
#অন্তর্ভাব "llvm/Support/Windows/WindowsSupport.h"
#অন্যথা
#অন্তর্ভাব "Unix/Unix.h"
#endif

using নামাকাশ llvm;

#define DEBUG_TYPE "rng"
নামাকাশ {
কাঠামো CreateSeed {
  স্থির শূন্য *call() {
    ফেরত নতুন cl::opt<uint64_t>(
        "rng-seed", cl::মান_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
স্থির ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
শূন্য llvm::initRandomSeedOptions() { *Seed; }

Random_সংখ্যা_Generator::Random_সংখ্যা_Generator(StringRef Salt) {
  LLVM_DEBUG(যদি (*Seed == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::vector<uint32_t> তথ্য_2;
  তথ্য_2.resize(2 + Salt.আকার_3());
  তথ্য_2[0] = *Seed;
  তথ্য_2[1] = *Seed >> 32;

  llvm::copy(Salt, তথ্য_2.begin() + 2);

  std::seed_seq SeedSeq(তথ্য_2.begin(), তথ্য_2.end());
  Generator.seed(SeedSeq);
}

Random_সংখ্যা_Generator::ফলাফল_type Random_সংখ্যা_Generator::operator()() {
  ফেরত Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(শূন্য *বাফার, size_t আকার_2) {
#ifdef _WIN32
  HCRYPTPROV hProvider;
  যদি (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    যদি (CryptGenRandom(hProvider, আকার_2, static_cast<BYTE *>(বাফার)))
      ফেরত std::error_code();
  }
  ফেরত std::error_code(GetLastError(), std::system_category());
#অন্যথা
  পূর্ণসংখ্যা Fd = open("/dev/urandom", O_RDONLY);
  যদি (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = পঠ(Fd, বাফার, আকার_2);
    যদি (BytesRead == -1)
      Ret = errnoAsErrorCode();
    অন্যথা যদি (BytesRead != static_cast<ssize_t>(আকার_2))
      Ret = std::error_code(EIO, std::system_category());
    যদি (close(Fd) == -1)
      Ret = errnoAsErrorCode();

    ফেরত Ret;
  }
  ফেরত errnoAsErrorCode();
#endif
}
