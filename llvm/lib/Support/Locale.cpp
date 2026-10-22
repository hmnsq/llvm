#ضمن "llvm/Support/Locale.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق locale {

صحيح columnWidth(StringRef Text) {
  أرجع llvm::sys::unicode::columnWidthUTF8(Text);
}

منطقي is_بغ(صحيح UCS) {
  أرجع llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
