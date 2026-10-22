#అన్తర్భావ "llvm/Support/Locale.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/Unicode.h"

నామాకాశ llvm {
నామాకాశ sys {
నామాకాశ locale {

పూర్ణాంకం స్తంభం_వెడల్పు(StringRef పాఠ్యం_2) {
  తిరిగి llvm::sys::unicode::స్తంభం_వెడల్పు_UTF8(పాఠ్యం_2);
}

తార్కిక isPrint(పూర్ణాంకం UCS) {
  తిరిగి llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
