#ಅನ್ತರ್ಭಾವ "llvm/Support/Locale.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ locale {

ಪೂರ್ಣಾಙ್ಕ columnWidth(StringRef Text) {
  ಪ್ರತ್ಯಾಗಮ llvm::sys::unicode::columnWidthUTF8(Text);
}

ತಾರ್ಕಿಕ isPrint(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಪ್ರತ್ಯಾಗಮ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
