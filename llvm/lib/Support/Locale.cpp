#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

පූර්ණ_සංඛ්‍යා columnWidth(StringRef Text) {
  ආපසු llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(පූර්ණ_සංඛ්‍යා UCS) {
  ආපසු llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
