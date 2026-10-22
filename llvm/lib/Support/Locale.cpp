#અન્તર્ભાવ "llvm/Support/Locale.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/Unicode.h"

નામાકાશ llvm {
નામાકાશ sys {
નામાકાશ locale {

પૂર્ણાંક columnWidth(StringRef Text) {
  પાછા llvm::sys::unicode::columnWidthUTF8(Text);
}

તાર્કિક isPrint(પૂર્ણાંક UCS) {
  પાછા llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
