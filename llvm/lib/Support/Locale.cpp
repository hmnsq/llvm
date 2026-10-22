#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

정수 columnWidth(StringRef Text) {
  반환 llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(정수 UCS) {
  반환 llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
