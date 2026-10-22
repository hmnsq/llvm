#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

ကိန်းပြည့် columnWidth(StringRef Text) {
  ပြန်ပေး llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(ကိန်းပြည့် UCS) {
  ပြန်ပေး llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
