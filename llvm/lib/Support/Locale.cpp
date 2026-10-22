#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

puur_naa_nka columnWidth(StringRef Text) {
  ᰀᰧᰌ llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(puur_naa_nka UCS) {
  ᰀᰧᰌ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
