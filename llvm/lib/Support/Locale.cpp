#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

ཧྲིལ columnWidth(StringRef Text) {
  ལོག llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(ཧྲིལ UCS) {
  ལོག llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
