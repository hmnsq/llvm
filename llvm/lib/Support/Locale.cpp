#شامل "llvm/Support/Locale.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/Unicode.h"

نامخلا llvm {
نامخلا sys {
نامخلا locale {

صحیح عمود_چوڑائی(StringRef متن_2) {
  واپس llvm::sys::unicode::عمود_چوڑائی_UTF8(متن_2);
}

منطقی isPrint(صحیح UCS) {
  واپس llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
