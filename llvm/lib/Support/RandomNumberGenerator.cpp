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
#که_تعریف_شوی _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#وإلا
#ضمن "Unix/Unix.h"
#د_که_پای

کارول نطاق llvm;

#تعریف_کړه DEBUG_TYPE "rng"
نطاق {
بنية Create_تخم {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<uint64_t>(
        "rng-seed", cl::ارزښت_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<uint64_t>, Create_تخم> تخم;
فراغ llvm::init_Random_تخم_Options() { *تخم; }

Random_شمېره_Generator::Random_شمېره_Generator(StringRef Salt) {
  LLVM_DEBUG(إذا (*تخم == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::ویکټور<uint32_t> معلومات_2;
  معلومات_2.resize(2 + Salt.اندازه_2());
  معلومات_2[0] = *تخم;
  معلومات_2[1] = *تخم >> 32;

  llvm::copy(Salt, معلومات_2.begin() + 2);

  std::تخم_seq تخم_Seq(معلومات_2.begin(), معلومات_2.پای_ټکی());
  Generator.تخم_2(تخم_Seq);
}

Random_شمېره_Generator::پایله_type Random_شمېره_Generator::چلوونکی()() {
  أرجع Generator();
}

// Get random vector of specified size
std::error_کوډ llvm::get_Random_بایټونه(فراغ *بفر, size_t اندازه) {
#که_تعریف_شوی _WIN32
  HCRYPTPROV hProvider;
  إذا (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_ډک,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    إذا (CryptGenRandom(hProvider, اندازه, static_cast<BYTE *>(بفر)))
      أرجع std::error_کوډ();
  }
  أرجع std::error_کوډ(Get_وروستی_Error(), std::system_category());
#وإلا
  صحيح Fd = open("/dev/urandom", O_RDONLY);
  إذا (Fd != -1) {
    std::error_کوډ Ret;
    ssize_t بایټونه_Read = اقرأ(Fd, بفر, اندازه);
    إذا (بایټونه_Read == -1)
      Ret = errno_As_Error_کوډ();
    وإلا إذا (بایټونه_Read != static_cast<ssize_t>(اندازه))
      Ret = std::error_کوډ(EIO, std::system_category());
    إذا (close(Fd) == -1)
      Ret = errno_As_Error_کوډ();

    أرجع Ret;
  }
  أرجع errno_As_Error_کوډ();
#د_که_پای
}
