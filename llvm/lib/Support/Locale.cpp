#ضمن "llvm/Support/Locale.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/Unicode.h"

نطاق llvm {
نطاق sys {
نطاق locale {

صحيح کالم_چوڑائی(StringRef متن_2) {
  أرجع llvm::sys::unicode::کالم_چوڑائی_UTF8(متن_2);
}

منطقي isPrint(صحيح UCS) {
  أرجع llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
