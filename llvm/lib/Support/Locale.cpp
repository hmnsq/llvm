#ضمن "llvm/Support/Locale.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق locale {

صحيح columnWidth(StringRef Text) {
  החזר llvm::sys::unicode::columnWidthUTF8(Text);
}

منطقي isPrint(صحيح UCS) {
  החזר llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
