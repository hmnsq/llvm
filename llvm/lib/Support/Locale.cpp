#समावेश "llvm/Support/Locale.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/Unicode.h"

namespace llvm {
namespace sys {
namespace locale {

पूर्णांक columnWidth(StringRef Text) {
  वापस llvm::sys::unicode::columnWidthUTF8(Text);
}

bool isPrint(पूर्णांक UCS) {
  वापस llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
