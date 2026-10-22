#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

số_nguyên columnWidth(StringRef Text) {
  trả_về llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(số_nguyên UCS) {
  trả_về llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
