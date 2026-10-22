#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

цяло columnWidth(StringRef Text) {
  върни llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(цяло UCS) {
  върни llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
