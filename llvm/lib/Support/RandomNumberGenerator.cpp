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

#समावेश "llvm/Support/RandomNumberGenerator.h"

#समावेश "DebugOptions.h"

#समावेश "llvm/Support/CommandLine.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/raw_ostream.h"
#अगर_घोषित _WIN32
#समावेश "llvm/Support/Windows/WindowsSupport.h"
#अन्यथा
#समावेश "Unix/Unix.h"
#पूर्ण_अगर

using namespace llvm;

#घोषणा DEBUG_TYPE "rng"
namespace {
काष्ठा CreateSeed {
  अटल व्योम *call() {
    वापस नया cl::opt<uint64_t>(
        "rng-seed", cl::मूल्य_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
अटल ManagedStatic<cl::opt<uint64_t>, CreateSeed> Seed;
व्योम llvm::initRandomSeedOptions() { *Seed; }

Random_संख्या_Generator::Random_संख्या_Generator(StringRef Salt) {
  LLVM_DEBUG(अगर (*Seed == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::vector<uint32_t> तथ्य_2;
  तथ्य_2.resize(2 + Salt.आकार_2());
  तथ्य_2[0] = *Seed;
  तथ्य_2[1] = *Seed >> 32;

  llvm::copy(Salt, तथ्य_2.begin() + 2);

  std::seed_seq SeedSeq(तथ्य_2.begin(), तथ्य_2.end());
  Generator.seed(SeedSeq);
}

Random_संख्या_Generator::परिणाम_type Random_संख्या_Generator::चालक()() {
  वापस Generator();
}

// Get random vector of specified size
std::error_code llvm::getRandomBytes(व्योम *बफर, माप_प्रकार आकार) {
#अगर_घोषित _WIN32
  HCRYPTPROV hProvider;
  अगर (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    अगर (CryptGenRandom(hProvider, आकार, static_cast<BYTE *>(बफर)))
      वापस std::error_code();
  }
  वापस std::error_code(GetLastError(), std::system_category());
#अन्यथा
  पूर्णांक Fd = खोलो("/dev/urandom", O_RDONLY);
  अगर (Fd != -1) {
    std::error_code Ret;
    ssize_t BytesRead = पढ़ो(Fd, बफर, आकार);
    अगर (BytesRead == -1)
      Ret = errnoAsErrorCode();
    अन्यथा अगर (BytesRead != static_cast<ssize_t>(आकार))
      Ret = std::error_code(EIO, std::system_category());
    अगर (बंद(Fd) == -1)
      Ret = errnoAsErrorCode();

    वापस Ret;
  }
  वापस errnoAsErrorCode();
#पूर्ण_अगर
}
