#ಅನ್ತರ್ಭಾವ "llvm/Support/Locale.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ locale {

ಪೂರ್ಣಾಂಕ ಸ್ತಂಭ_ಅಗಲ(StringRef ಪಠ್ಯ_2) {
  ಹಿಂತಿರುಗಿ llvm::sys::unicode::ಸ್ತಂಭ_ಅಗಲ_UTF8(ಪಠ್ಯ_2);
}

ತಾರ್ಕಿಕ isPrint(ಪೂರ್ಣಾಂಕ UCS) {
  ಹಿಂತಿರುಗಿ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
