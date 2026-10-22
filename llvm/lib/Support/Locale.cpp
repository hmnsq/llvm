#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Locale.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {
ਨਾਮਾਕਾਸ਼ locale {

ਪੂਰਨ ਕਾਲਮ_ਚੌੜਾਈ(StringRef ਪਾਠ_2) {
  ਵਾਪਸ llvm::sys::unicode::ਕਾਲਮ_ਚੌੜਾਈ_UTF8(ਪਾਠ_2);
}

ਤਾਰਕਿਕ isPrint(ਪੂਰਨ UCS) {
  ਵਾਪਸ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
