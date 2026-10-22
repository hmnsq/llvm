#অন্তর্ভাৱ "llvm/Support/Locale.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Support/Unicode.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ locale {

পূর্ণাঙ্ক columnWidth(StringRef Text) {
  ঘূৰাই llvm::sys::unicode::columnWidthUTF8(Text);
}

তার্কিক isPrint(পূর্ণাঙ্ক UCS) {
  ঘূৰাই llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
