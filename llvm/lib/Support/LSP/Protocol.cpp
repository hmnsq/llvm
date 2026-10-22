//===--- Protocol.cpp - Language Server Protocol Implementation -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the serialization code for the LSP structs.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/LSP/Protocol.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringSet.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/JSON.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/raw_ostream.h"

کارول نطاق llvm;
کارول نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
کينډۍ <ډول_نوم T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::ارزښت &Params,
                         llvm::StringLiteral Prop, T &بهر,
                         llvm::json::لار لار) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  خپلکاری *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع ریښتیا;
  أرجع fromJSON(*V, بهر, لار.ډګر(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي is_Windows_لار(StringRef لار) {
  أرجع لار.اندازه_2() > 1 && llvm::isAlpha(لار[0]) && لار[1] == ':';
}

ساكن منطقي is_شبکه_لار(StringRef لار) {
  أرجع لار.اندازه_2() > 2 && لار[0] == لار[1] &&
         llvm::sys::لار_2::is_separator(لار[0]);
}

ساكن منطقي should_Escape_دننه_URI(بینښه حرف C) {
  // Unreserved characters.
  إذا ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    أرجع خطأ;

  بدل (C) {
  حالة '-':
  حالة '_':
  حالة '.':
  حالة '~':
  // '/' is only reserved when parsing.
  حالة '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  حالة ':':
    أرجع خطأ;
  }
  أرجع ریښتیا;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &بهر) {
  لكل (بینښه حرف C : Content) {
    إذا (should_Escape_دننه_URI(C)) {
      بهر.ټېل_شا('%');
      بهر.ټېل_شا(llvm::hexdigit(C / 16));
      بهر.ټېل_شا(llvm::hexdigit(C % 16));
    } وإلا {
      بهر.ټېل_شا(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string پایله;
  لكل (خپلکاری I = Content.begin(), E = Content.پای_ټکی(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.پای_ټکی() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      پایله.ټېل_شا(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      پایله.ټېل_شا(*I);
    }
  }
  أرجع پایله;
}

/// Return the set containing the supported URI schemes.
ساكن String_سېټ<> &getSupportedSchemes() {
  ساكن String_سېټ<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.تشه())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_د(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uri_From_Absolute_لار(StringRef Absolute_لار,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ریښه_2 = llvm::sys::لار_2::ریښه_name(Absolute_لار);
  إذا (is_شبکه_لار(ریښه_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ریښه_2.drop_front(2);
    Absolute_لار.consume_front(ریښه_2);
  } وإلا إذا (is_Windows_لار(ریښه_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::لار_2::convert_منزل_slash(Absolute_لار);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.تشه() && Body.تشه())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.تشه() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> get_Absolute_لار(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertible_Error_کوډ(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> لار;
  إذا (!Authority.تشه()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).منزل_ویکټور(لار);
  } وإلا إذا (is_Windows_لار(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  لار.append(Body);
  llvm::sys::لار_2::native(لار);
  أرجع std::string(لار);
}

ساكن llvm::Expected<std::string> parse_File_لار_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.پیدا_کړه(':');
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوډ(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوډ(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.پیدا_کړه('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().لري(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوډ(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع get_Absolute_لار(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_لار = parse_File_لار_From_URI(Uri);
  إذا (!File_لار)
    أرجع File_لار.takeError();
  أرجع URIForFile(std::move(*File_لار), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_لار(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().وېشل(':').لومړی_2; }

فراغ URIForFile::ثبتوونکی_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().ورننباسه(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, URIForFile &پایله,
                         llvm::json::لار لار) {
  إذا (std::optional<StringRef> Str = ارزښت.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      لار.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    پایله = std::move(*ExpectedUri);
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت URIForFile &ارزښت) {
  أرجع ارزښت.uri();
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, ثابت URIForFile &ارزښت) {
  أرجع Os << ارزښت.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         ClientCapabilities &پایله, llvm::json::لار لار) {
  ثابت llvm::json::Object *O = ارزښت.getAsObject();
  إذا (!O) {
    لار.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *متن_Document = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_نښه =
            متن_Document->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> پوړیز_Support =
              Document_نښه->getBoolean("hierarchicalDocumentSymbolSupport"))
        پایله.پوړیز_Document_نښه = *پوړیز_Support;
    }
    إذا (خپلکاری *کوډ_Action = متن_Document->getObject("codeAction")) {
      إذا (کوډ_Action->getObject("codeActionLiteralSupport"))
        پایله.کوډ_Action_Structure = ریښتیا;
    }
  }
  إذا (خپلکاری *کړکۍ = O->getObject("window")) {
    إذا (std::optional<منطقي> Work_بشپړ_پرمختګ_Support =
            کړکۍ->getBoolean("workDoneProgress"))
      پایله.work_بشپړ_پرمختګ = *Work_بشپړ_پرمختګ_Support;
  }
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, ClientInfo &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  إذا (!O || !O.map("name", پایله.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", پایله.نسخه_2);
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, TraceLevel &پایله,
                         llvm::json::لار لار) {
  إذا (std::optional<StringRef> Str = ارزښت.getAsString()) {
    إذا (*Str == "off") {
      پایله = TraceLevel::Off;
      أرجع ریښتیا;
    }
    إذا (*Str == "messages") {
      پایله = TraceLevel::Messages;
      أرجع ریښتیا;
    }
    إذا (*Str == "verbose") {
      پایله = TraceLevel::Verbose;
      أرجع ریښتیا;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         InitializeParams &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", پایله.capabilities);
  O.map("trace", پایله.trace);
  O.map("rootUri", پایله.ریښه_Uri);
  O.map("rootPath", پایله.ریښه_لار);
  mapOptOrNull(ارزښت, "clientInfo", پایله.clientInfo, لار);

  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         متن_Document_توکی &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("uri", پایله.uri) &&
         O.map("languageId", پایله.languageId) && O.map("text", پایله.متن) &&
         O.map("version", پایله.نسخه_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت متن_Document_Identifier &ارزښت) {
  أرجع llvm::json::Object{{"uri", ارزښت.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         متن_Document_Identifier &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("uri", پایله.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ارزښت
llvm::lsp::منزل_JSON(ثابت Versioned_متن_Document_Identifier &ارزښت) {
  أرجع llvm::json::Object{
      {"uri", ارزښت.uri},
      {"version", ارزښت.نسخه_2},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Versioned_متن_Document_Identifier &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("uri", پایله.uri) && O.map("version", پایله.نسخه_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, Position &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("line", پایله.کرښه_2) &&
         O.map("character", پایله.character);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Position &ارزښت) {
  أرجع llvm::json::Object{
      {"line", ارزښت.کرښه_2},
      {"character", ارزښت.character},
  };
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, ثابت Position &ارزښت) {
  أرجع Os << ارزښت.کرښه_2 << ':' << ارزښت.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, Range &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("start", پایله.پیل_2) && O.map("end", پایله.پای_ټکی);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Range &ارزښت) {
  أرجع llvm::json::Object{
      {"start", ارزښت.پیل_2},
      {"end", ارزښت.پای_ټکی},
  };
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, ثابت Range &ارزښت) {
  أرجع Os << ارزښت.پیل_2 << '-' << ارزښت.پای_ټکی;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, Location &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("uri", پایله.uri) && O.map("range", پایله.range);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Location &ارزښت) {
  أرجع llvm::json::Object{
      {"uri", ارزښت.uri},
      {"range", ارزښت.range},
  };
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, ثابت Location &ارزښت) {
  أرجع Os << ارزښت.range << '@' << ارزښت.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         متن_Document_Position_Params &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2) &&
         O.map("position", پایله.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         ReferenceContext &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.mapOptional("includeDeclaration", پایله.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         ReferenceParams &پایله, llvm::json::لار لار) {
  متن_Document_Position_Params &بنسټ = پایله;
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع fromJSON(ارزښت, بنسټ, لار) && O &&
         O.mapOptional("context", پایله.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Did_Open_متن_Document_Params &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Did_Close_متن_Document_Params &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &Params,
                         Did_Save_متن_Document_Params &R, llvm::json::لار P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_پایله
متن_Document_Content_Change_Event::apply_منزل(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = متن;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::سرچینه_Mgr TmpScrMgr;
  TmpScrMgr.جمع_کړه_New_سرچینه_بفر(llvm::حافظه_بفر::get_Mem_بفر(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.پیل.get_اشاره_ګر() - Contents.معلومات(),
                   RangeLoc.پای_ټکی_2.get_اشاره_ګر() - RangeLoc.پیل.get_اشاره_ګر(),
                   متن);
  أرجع success();
}

Logical_پایله متن_Document_Content_Change_Event::apply_منزل(
    ArrayRef<متن_Document_Content_Change_Event> Changes, std::string &Contents) {
  لكل (ثابت خپلکاری &Change : Changes)
    إذا (failed(Change.apply_منزل(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         متن_Document_Content_Change_Event &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("range", پایله.range) &&
         O.map("rangeLength", پایله.range_اوږدتیا) && O.map("text", پایله.متن);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Did_Change_متن_Document_Params &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2) &&
         O.map("contentChanges", پایله.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef منزل_متن_ډول(Markup_ډول ډول_2) {
  بدل (ډول_2) {
  حالة Markup_ډول::Plain_متن:
    أرجع "plaintext";
  حالة Markup_ډول::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, Markup_ډول ډول_2) {
  أرجع Os << منزل_متن_ډول(ډول_2);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت MarkupContent &Mc) {
  إذا (Mc.ارزښت_2.تشه())
    أرجع تش_اشاره;

  أرجع llvm::json::Object{
      {"kind", منزل_متن_ډول(Mc.ډول)},
      {"value", Mc.ارزښت_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Hover &Hover) {
  llvm::json::Object پایله{{"contents", منزل_JSON(Hover.contents)}};
  إذا (Hover.range)
    پایله["range"] = منزل_JSON(*Hover.range);
  أرجع std::move(پایله);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Document_نښه &نښه) {
  llvm::json::Object پایله{{"name", نښه.name},
                            {"kind", static_cast<صحيح>(نښه.ډول)},
                            {"range", نښه.range},
                            {"selectionRange", نښه.selectionRange}};

  إذا (!نښه.detail.تشه())
    پایله["detail"] = نښه.detail;
  إذا (!نښه.children.تشه())
    پایله["children"] = نښه.children;
  أرجع std::move(پایله);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Document_نښه_Params &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         DiagnosticRelatedInformation &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("location", پایله.location) &&
         O.map("message", پایله.پیغام_2);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.پیغام_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(Diagnostic_نښان نښان) {
  أرجع static_cast<صحيح>(نښان);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, Diagnostic_نښان &پایله,
                         llvm::json::لار لار) {
  إذا (std::optional<int64_t> I = ارزښت.getAsInteger()) {
    پایله = (Diagnostic_نښان)*I;
    أرجع ریښتیا;
  }

  أرجع خطأ;
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Diagnostic &Diag) {
  llvm::json::Object پایله{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.پیغام_2},
  };
  إذا (Diag.category)
    پایله["category"] = *Diag.category;
  إذا (!Diag.سرچینه_2.تشه())
    پایله["source"] = Diag.سرچینه_2;
  إذا (Diag.relatedInformation)
    پایله["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.نښانونه.تشه())
    پایله["tags"] = Diag.نښانونه;
  أرجع std::move(پایله);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, Diagnostic &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(ارزښت, "severity", Severity, لار))
    أرجع خطأ;
  پایله.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", پایله.range) && O.map("message", پایله.پیغام_2) &&
         mapOptOrNull(ارزښت, "category", پایله.category, لار) &&
         mapOptOrNull(ارزښت, "source", پایله.سرچینه_2, لار) &&
         mapOptOrNull(ارزښت, "relatedInformation", پایله.relatedInformation,
                      لار) &&
         mapOptOrNull(ارزښت, "tags", پایله.نښانونه, لار);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.نسخه_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, متن_Edit &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("range", پایله.range) && O.map("newText", پایله.new_متن);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت متن_Edit &ارزښت) {
  أرجع llvm::json::Object{
      {"range", ارزښت.range},
      {"newText", ارزښت.new_متن},
  };
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os, ثابت متن_Edit &ارزښت) {
  Os << ارزښت.range << " => \"";
  llvm::printEscapedString(ارزښت.new_متن, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Completion_توکی_ډول &پایله, llvm::json::لار لار) {
  إذا (std::optional<int64_t> Int_ارزښت = ارزښت.getAsInteger()) {
    إذا (*Int_ارزښت < static_cast<صحيح>(Completion_توکی_ډول::متن_2) ||
        *Int_ارزښت > static_cast<صحيح>(Completion_توکی_ډول::TypeParameter))
      أرجع خطأ;
    پایله = static_cast<Completion_توکی_ډول>(*Int_ارزښت);
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

Completion_توکی_ډول llvm::lsp::adjust_ډول_منزل_Capability(
    Completion_توکی_ډول ډول_2,
    Completion_توکی_ډول_Bitset &Supported_Completion_توکی_Kinds) {
  size_t ډول_Val = static_cast<size_t>(ډول_2);
  إذا (ډول_Val >= k_Completion_توکی_ډول_Min &&
      ډول_Val <= Supported_Completion_توکی_Kinds.اندازه_2() &&
      Supported_Completion_توکی_Kinds[ډول_Val])
    أرجع ډول_2;

  // Provide some fall backs for common kinds that are close enough.
  بدل (ډول_2) {
  حالة Completion_توکی_ډول::Folder:
    أرجع Completion_توکی_ډول::File;
  حالة Completion_توکی_ډول::EnumMember:
    أرجع Completion_توکی_ډول::Enum;
  حالة Completion_توکی_ډول::Struct:
    أرجع Completion_توکی_ډول::Class;
  افتراضي:
    أرجع Completion_توکی_ډول::متن_2;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         Completion_توکی_ډول_Bitset &پایله,
                         llvm::json::لار لار) {
  إذا (ثابت llvm::json::Array *Array_ارزښت = ارزښت.getAsArray()) {
    لكل (size_t I = 0, E = Array_ارزښت->اندازه_2(); I < E; ++I) {
      Completion_توکی_ډول ډول_بهر;
      إذا (fromJSON((*Array_ارزښت)[I], ډول_بهر, لار.شاخص(I)))
        پایله.سېټ(size_t(ډول_بهر));
    }
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Completion_توکی &ارزښت) {
  أكد(!ارزښت.label.تشه() && "completion item label is required");
  llvm::json::Object پایله{{"label", ارزښت.label}};
  إذا (ارزښت.ډول != Completion_توکی_ډول::Missing)
    پایله["kind"] = static_cast<صحيح>(ارزښت.ډول);
  إذا (!ارزښت.detail.تشه())
    پایله["detail"] = ارزښت.detail;
  إذا (ارزښت.documentation)
    پایله["documentation"] = ارزښت.documentation;
  إذا (!ارزښت.ترتیبول_متن.تشه())
    پایله["sortText"] = ارزښت.ترتیبول_متن;
  إذا (!ارزښت.filter_متن.تشه())
    پایله["filterText"] = ارزښت.filter_متن;
  إذا (!ارزښت.ورننباسه_متن.تشه())
    پایله["insertText"] = ارزښت.ورننباسه_متن;
  إذا (ارزښت.ورننباسه_متن_Format != ورننباسه_متن_Format_2::Missing)
    پایله["insertTextFormat"] = static_cast<صحيح>(ارزښت.ورننباسه_متن_Format);
  إذا (ارزښت.متن_Edit_2)
    پایله["textEdit"] = *ارزښت.متن_Edit_2;
  إذا (!ارزښت.additional_متن_Edits.تشه()) {
    پایله["additionalTextEdits"] =
        llvm::json::Array(ارزښت.additional_متن_Edits);
  }
  إذا (ارزښت.deprecated)
    پایله["deprecated"] = ارزښت.deprecated;
  أرجع std::move(پایله);
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os,
                                   ثابت Completion_توکی &ارزښت) {
  أرجع Os << ارزښت.label << " - " << منزل_JSON(ارزښت);
}

منطقي llvm::lsp::چلوونکی<(ثابت Completion_توکی &Lhs,
                          ثابت Completion_توکی &Rhs) {
  أرجع (Lhs.ترتیبول_متن.تشه() ? Lhs.label : Lhs.ترتیبول_متن) <
         (Rhs.ترتیبول_متن.تشه() ? Rhs.label : Rhs.ترتیبول_متن);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت CompletionList &ارزښت) {
  أرجع llvm::json::Object{
      {"isIncomplete", ارزښت.isIncomplete},
      {"items", llvm::json::Array(ارزښت.توکي)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         CompletionContext &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  صحيح Trigger_ډول;
  إذا (!O || !O.map("triggerKind", Trigger_ډول) ||
      !mapOptOrNull(ارزښت, "triggerCharacter", پایله.triggerCharacter, لار))
    أرجع خطأ;
  پایله.trigger_ډول = static_cast<Completion_Trigger_ډول>(Trigger_ډول);
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         CompletionParams &پایله, llvm::json::لار لار) {
  إذا (!fromJSON(ارزښت, static_cast<متن_Document_Position_Params &>(پایله), لار))
    أرجع خطأ;
  إذا (ثابت llvm::json::ارزښت *Context = ارزښت.getAsObject()->get("context"))
    أرجع fromJSON(*Context, پایله.context, لار.ډګر("context"));
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت ParameterInformation &ارزښت) {
  أكد((ارزښت.labelOffsets || !ارزښت.labelString.تشه()) &&
         "parameter information label is required");
  llvm::json::Object پایله;
  إذا (ارزښت.labelOffsets)
    پایله["label"] = llvm::json::Array(
        {ارزښت.labelOffsets->لومړی_2, ارزښت.labelOffsets->دوهم_2});
  وإلا
    پایله["label"] = ارزښت.labelString;
  إذا (!ارزښت.documentation.تشه())
    پایله["documentation"] = ارزښت.documentation;
  أرجع std::move(پایله);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت SignatureInformation &ارزښت) {
  أكد(!ارزښت.label.تشه() && "signature information label is required");
  llvm::json::Object پایله{
      {"label", ارزښت.label},
      {"parameters", llvm::json::Array(ارزښت.parameters)},
  };
  إذا (!ارزښت.documentation.تشه())
    پایله["documentation"] = ارزښت.documentation;
  أرجع std::move(پایله);
}

raw_ostream &llvm::lsp::چلوونکی<<(raw_ostream &Os,
                                   ثابت SignatureInformation &ارزښت) {
  أرجع Os << ارزښت.label << " - " << منزل_JSON(ارزښت);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت SignatureHelp &ارزښت) {
  أكد(ارزښت.فعال_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(ارزښت.فعال_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", ارزښت.فعال_Signature},
      {"activeParameter", ارزښت.فعال_Parameter},
      {"signatures", llvm::json::Array(ارزښت.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         DocumentLinkParams &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت DocumentLink &ارزښت) {
  أرجع llvm::json::Object{
      {"range", ارزښت.range},
      {"target", ارزښت.موخه},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         InlayHintsParams &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2) &&
         O.map("range", پایله.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت InlayHint &ارزښت) {
  أرجع llvm::json::Object{{"position", ارزښت.position},
                            {"kind", (صحيح)ارزښت.ډول},
                            {"label", ارزښت.label},
                            {"paddingLeft", ارزښت.padding_کیڼ},
                            {"paddingRight", ارزښت.padding_ښی}};
}
منطقي llvm::lsp::چلوونکی==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.ډول, Lhs.label) ==
         std::tie(Rhs.position, Rhs.ډول, Rhs.label);
}
منطقي llvm::lsp::چلوونکی<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.ډول, Lhs.label) <
         std::tie(Rhs.position, Rhs.ډول, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::چلوونکی<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_ډول ارزښت) {
  بدل (ارزښت) {
  حالة Inlay_Hint_ډول::Parameter:
    أرجع Os << "parameter";
  حالة Inlay_Hint_ډول::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         کوډ_Action_Context &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  إذا (!O || !O.map("diagnostics", پایله.diagnostics))
    أرجع خطأ;
  O.map("only", پایله.only);
  أرجع ریښتیا;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت,
                         کوډ_Action_Params &پایله, llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("textDocument", پایله.متن_Document_2) &&
         O.map("range", پایله.range) && O.map("context", پایله.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::ارزښت &ارزښت, WorkspaceEdit &پایله,
                         llvm::json::لار لار) {
  llvm::json::ObjectMapper O(ارزښت, لار);
  أرجع O && O.map("changes", پایله.changes);
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت WorkspaceEdit &ارزښت) {
  llvm::json::Object FileChanges;
  لكل (خپلکاری &Change : ارزښت.changes)
    FileChanges[Change.لومړی_2] = llvm::json::Array(Change.دوهم_2);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral کوډ_Action::k_چټک_Fix = "quickfix";
ثابت llvm::StringLiteral کوډ_Action::kRefactor = "refactor";
ثابت llvm::StringLiteral کوډ_Action::kInfo = "info";

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت کوډ_Action &ارزښت) {
  llvm::json::Object کوډ_Action{{"title", ارزښت.title}};
  إذا (ارزښت.ډول)
    کوډ_Action["kind"] = *ارزښت.ډول;
  إذا (ارزښت.diagnostics)
    کوډ_Action["diagnostics"] = llvm::json::Array(*ارزښت.diagnostics);
  إذا (ارزښت.isPreferred)
    کوډ_Action["isPreferred"] = ریښتیا;
  إذا (ارزښت.edit)
    کوډ_Action["edit"] = *ارزښت.edit;
  أرجع std::move(کوډ_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت Show_پیغام_Params &Params) {
  خپلکاری بهر = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.پیغام_2},
  };
  إذا (Params.actions)
    بهر["actions"] = *Params.actions;
  أرجع بهر;
}

llvm::json::ارزښت llvm::lsp::منزل_JSON(ثابت پیغام_Action_توکی &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
