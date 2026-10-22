#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

จำนวนเต็ม columnWidth(StringRef Text) {
  คืน llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(จำนวนเต็ม UCS) {
  คืน llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
