#অন্তর্ভাব "llvm/Support/Locale.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ locale {

পূর্ণসংখ্যা columnWidth(StringRef Text) {
  ফেরত llvm::sys::unicode::columnWidthUTF8(Text);
}

যৌক্তিক isPrint(পূর্ণসংখ্যা UCS) {
  ফেরত llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
