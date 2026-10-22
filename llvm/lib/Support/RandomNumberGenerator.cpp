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
#اگر_تعریف_شده _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#وإلا
#ضمن "Unix/Unix.h"
#پایان_اگر

استفاده نطاق llvm;

#تعریف_کن DEBUG_TYPE "rng"
نطاق {
بنية Create_دانه {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<uint64_t>(
        "rng-seed", cl::مقدار_desc("seed"), cl::Hidden,
        cl::desc("Seed for the random number generator"), cl::init(0));
  }
};
} // namespace
ساكن ManagedStatic<cl::opt<uint64_t>, Create_دانه> دانه;
فراغ llvm::init_Random_دانه_Options() { *دانه; }

Random_عدد_Generator::Random_عدد_Generator(StringRef Salt) {
  LLVM_DEBUG(إذا (*دانه == 0) dbgs()
             << "Warning! Using unseeded random number generator.\n");

  // Combine seed and salts using std::seed_seq.
  // Data: Seed-low, Seed-high, Salt
  // Note: std::seed_seq can only store 32-bit values, even though we
  // are using a 64-bit RNG. This isn't a problem since the Mersenne
  // twister constructor copies these correctly into its initial state.
  std::وکتور<uint32_t> داده_2;
  داده_2.resize(2 + Salt.اندازه_2());
  داده_2[0] = *دانه;
  داده_2[1] = *دانه >> 32;

  llvm::copy(Salt, داده_2.begin() + 2);

  std::دانه_seq دانه_Seq(داده_2.begin(), داده_2.انتها());
  Generator.دانه_2(دانه_Seq);
}

Random_عدد_Generator::نتیجه_type Random_عدد_Generator::عملگر()() {
  أرجع Generator();
}

// Get random vector of specified size
std::error_کد llvm::get_Random_بایتها(فراغ *میانگیر, size_t اندازه) {
#اگر_تعریف_شده _WIN32
  HCRYPTPROV hProvider;
  إذا (CryptAcquireContext(&hProvider, 0, 0, PROV_RSA_پر,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
    ScopedCryptContext ScopedHandle(hProvider);
    إذا (CryptGenRandom(hProvider, اندازه, static_cast<BYTE *>(میانگیر)))
      أرجع std::error_کد();
  }
  أرجع std::error_کد(Get_آخرین_Error(), std::system_category());
#وإلا
  صحيح Fd = open("/dev/urandom", O_RDONLY);
  إذا (Fd != -1) {
    std::error_کد Ret;
    ssize_t بایتها_Read = اقرأ(Fd, میانگیر, اندازه);
    إذا (بایتها_Read == -1)
      Ret = errno_As_Error_کد();
    وإلا إذا (بایتها_Read != static_cast<ssize_t>(اندازه))
      Ret = std::error_کد(EIO, std::system_category());
    إذا (close(Fd) == -1)
      Ret = errno_As_Error_کد();

    أرجع Ret;
  }
  أرجع errno_As_Error_کد();
#پایان_اگر
}
