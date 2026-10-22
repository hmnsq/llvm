#അന്തര്ഭാവ "llvm/Support/Locale.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Support/Unicode.h"

നാമാകാശ llvm {
നാമാകാശ sys {
നാമാകാശ locale {

പൂർണ്ണസംഖ്യ columnWidth(StringRef Text) {
  തിരികെ llvm::sys::unicode::columnWidthUTF8(Text);
}

തർക്കം isPrint(പൂർണ്ണസംഖ്യ UCS) {
  തിരികെ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
