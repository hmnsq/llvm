#شامل "llvm/Support/Locale.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Unicode.h"

فضاینام llvm {
فضاینام sys {
فضاینام locale {

صحیح columnWidth(StringRef Text) {
  بازگشت llvm::sys::unicode::columnWidthUTF8(Text);
}

منطقی isPrint(صحیح UCS) {
  بازگشت llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
