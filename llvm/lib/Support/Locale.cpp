#ଅନ୍ତର୍ଭାଵ "llvm/Support/Locale.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"

ନାମାକାଶ llvm {
ନାମାକାଶ sys {
ନାମାକାଶ locale {

ପୂର୍ଣ୍ଣସଂଖ୍ୟା columnWidth(StringRef Text) {
  ଫେରନ୍ତୁ llvm::sys::unicode::columnWidthUTF8(Text);
}

ତାର୍କିକ isPrint(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ଫେରନ୍ତୁ llvm::sys::unicode::isPrintable(UCS);
}

} // namespace locale
} // namespace sys
} // namespace llvm
