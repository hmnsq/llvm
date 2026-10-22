#include "llvm/Support/Locale.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

ຈຳນວນເຕັມ columnWidth(StringRef Text) {
  ສົ່ງຄືນ llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(ຈຳນວນເຕັມ UCS) {
  ສົ່ງຄືນ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
