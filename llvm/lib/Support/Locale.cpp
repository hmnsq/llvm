#अन्तर्भाव "llvm/Support/Locale.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/Unicode.h"

नामाकाश llvm {
नामाकाश sys {
नामाकाश locale {

पूर्णाङ्क columnWidth(StringRef Text) {
  पटिनिवत्त llvm::sys::unicode::columnWidthUTF8(Text);
}

तार्किक isPrint(पूर्णाङ्क UCS) {
  पटिनिवत्त llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
