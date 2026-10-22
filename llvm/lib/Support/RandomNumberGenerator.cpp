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

#ضمن "llvm/Support/RandomNumberGenerator.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/raw_ostream.h"
#جے_تعریف_شدہ _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#وإلا
#ضمن "Unix/Unix.h"
#جے_انت

ورتوں نطاق llvm;

#تعریف_کرو DEBUG_TYPE "rng"
نطاق {
بنية Create_بیج {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<uint64_t>(
        "rng-seed", cl::قدر_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<uint64_t>, Create_بیج> بیج;
فراغ llvm::init_Random_بیج_Options() { *بیج; }

Random_نمبر_Generator::Random_نمبر_Generator(StringRef Salt) {
  LLVM_DEBUG(إذا (*بیج == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::سمتیہ<uint32_t> انکڑے_2;
  انکڑے_2.resize(2 + Salt.ناپ_2());
  انکڑے_2[0] = *بیج;
  انکڑے_2[1] = *بیج >> 32;

  llvm::copy(Salt, انکڑے_2.begin() + 2);

  std::بیج_seq بیج_Seq(انکڑے_2.begin(), انکڑے_2.سرا());
  Generator.بیج_2(بیج_Seq);
}

Random_نمبر_Generator::نتیجہ_type Random_نمبر_Generator::عامل()() {
  أرجع Generator();
}

// Get random vector of specified size
std::error_کوڈ llvm::get_Random_بائٹاں(فراغ *بفر, size_t ناپ) {
#جے_تعریف_شدہ _WIN32
  HCRYPTPROV hProvider;
  إذا (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_بھریا,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    إذا (CryptGenRandom(hProvider, ناپ, static_cast<BYTE *>(بفر)))
      أرجع std::error_کوڈ();
  }
  أرجع std::error_کوڈ(Get_آخری_Error(), std::system_category());
#وإلا
  صحيح Fd = open("/dev/urandom", O_RDONLY);
  إذا (Fd != -1) {
    std::error_کوڈ Ret;
    ssize_t بائٹاں_Read = اقرأ(Fd, بفر, ناپ);
    إذا (بائٹاں_Read == -1)
      Ret = errno_As_Error_کوڈ();
    وإلا إذا (بائٹاں_Read != static_cast<ssize_t>(ناپ))
      Ret = std::error_کوڈ(EIO, std::system_category());
    إذا (close(Fd) == -1)
      Ret = errno_As_Error_کوڈ();

    أرجع Ret;
  }
  أرجع errno_As_Error_کوڈ();
#جے_انت
}
