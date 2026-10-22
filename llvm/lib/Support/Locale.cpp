#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

整数 columnWidth(StringRef Text) {
  返回 llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(整数 UCS) {
  返回 llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
