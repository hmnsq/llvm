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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ "Unix/Unix.h"
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_TYPE "rng"
ನಾಮಾಕಾಶ {
ರಚನೆ Create_ಬೀಜ {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಹಿಂತಿರುಗಿ ಹೊಸ cl::opt<uint64_t>(
        "rng-seed", cl::ಮೌಲ್ಯ_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ಸ್ಥಿರ ManagedStatic<cl::opt<uint64_t>, Create_ಬೀಜ> ಬೀಜ;
ಶೂನ್ಯ llvm::init_Random_ಬೀಜ_Options() { *ಬೀಜ; }

Random_ಸಂಖ್ಯೆ_Generator::Random_ಸಂಖ್ಯೆ_Generator(StringRef Salt) {
  LLVM_DEBUG(ಆದರೆ (*ಬೀಜ == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::ಸದಿಶ<uint32_t> ದತ್ತಾಂಶ_2;
  ದತ್ತಾಂಶ_2.resize(2 + Salt.ಗಾತ್ರ_3());
  ದತ್ತಾಂಶ_2[0] = *ಬೀಜ;
  ದತ್ತಾಂಶ_2[1] = *ಬೀಜ >> 32;

  llvm::copy(Salt, ದತ್ತಾಂಶ_2.begin() + 2);

  std::ಬೀಜ_seq ಬೀಜ_Seq(ದತ್ತಾಂಶ_2.begin(), ದತ್ತಾಂಶ_2.ಕೊನೆ());
  Generator.ಬೀಜ_2(ಬೀಜ_Seq);
}

Random_ಸಂಖ್ಯೆ_Generator::ಫಲಿತಾಂಶ_type Random_ಸಂಖ್ಯೆ_Generator::ಕ್ರಿಯಾಕಾರಕ()() {
  ಹಿಂತಿರುಗಿ Generator();
}

// Get random vector of specified size
std::error_ಸಂಕೇತ llvm::get_Random_ಬೈಟುಗಳು(ಶೂನ್ಯ *ಬಫರ್, size_t ಗಾತ್ರ_2) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  HCRYPTPROV hProvider;
  ಆದರೆ (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_ತುಂಬಿದ,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    ಆದರೆ (CryptGenRandom(hProvider, ಗಾತ್ರ_2, static_cast<BYTE *>(ಬಫರ್)))
      ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
  }
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(Get_ಕೊನೆಯ_Error(), std::system_category());
#ಇಲ್ಲದಿದ್ದರೆ
  ಪೂರ್ಣಾಂಕ Fd = open("/dev/urandom", O_RDONLY);
  ಆದರೆ (Fd != -1) {
    std::error_ಸಂಕೇತ Ret;
    ssize_t ಬೈಟುಗಳು_Read = ಪಠ(Fd, ಬಫರ್, ಗಾತ್ರ_2);
    ಆದರೆ (ಬೈಟುಗಳು_Read == -1)
      Ret = errno_As_Error_ಸಂಕೇತ();
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬೈಟುಗಳು_Read != static_cast<ssize_t>(ಗಾತ್ರ_2))
      Ret = std::error_ಸಂಕೇತ(EIO, std::system_category());
    ಆದರೆ (close(Fd) == -1)
      Ret = errno_As_Error_ಸಂಕೇತ();

    ಹಿಂತಿರುಗಿ Ret;
  }
  ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();
#ಆದರೆ_ಅಂತ್ಯ
}
