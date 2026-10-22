#ضمن "llvm/Support/Locale.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق locale {

صحيح columnWidth(StringRef Text) {
  ܦܢܐ llvm::sys::unicode::columnWidthUTF8(Text);
}

منطقي isPrint(صحيح UCS) {
  ܦܢܐ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
