#অন্তর্ভাব "llvm/Support/Locale.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ locale {

পূর্ণাঙ্ক columnWidth(StringRef Text) {
  প্রত্যাগম llvm::sys::unicode::columnWidthUTF8(Text);
}

তার্কিক isPrint(পূর্ণাঙ্ক UCS) {
  প্রত্যাগম llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
