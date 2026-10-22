#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

ចំនួនគត់ columnWidth(StringRef Text) {
  ត្រឡប់ llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(ចំនួនគត់ UCS) {
  ត្រឡប់ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
