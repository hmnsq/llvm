#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

цео columnWidth(StringRef Text) {
  врати llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(цео UCS) {
  врати llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
