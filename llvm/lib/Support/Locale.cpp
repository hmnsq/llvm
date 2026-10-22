#அந்தர்பாவ "llvm/Support/Locale.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/Unicode.h"

நாமாகாஶ llvm {
நாமாகாஶ sys {
நாமாகாஶ locale {

முழு columnWidth(StringRef Text) {
  திரும்பு llvm::sys::unicode::columnWidthUTF8(Text);
}

தருக்க isPrint(முழு UCS) {
  திரும்பு llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
