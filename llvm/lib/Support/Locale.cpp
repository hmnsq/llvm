#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

ціле columnWidth(StringRef Text) {
  повернути llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(ціле UCS) {
  повернути llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
