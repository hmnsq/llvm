#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

целое columnWidth(StringRef Text) {
  вернуть llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(целое UCS) {
  вернуть llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
