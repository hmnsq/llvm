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

ورتوں نطاق llvm;
ورتوں نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
سانچا <قسم_ناں T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::قدر &Params,
                         llvm::StringLiteral Prop, T &باہرلا,
                         llvm::json::رستہ رستہ) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  خودکار *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع سچ;
  أرجع fromJSON(*V, باہرلا, رستہ.میدان(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي is_Windows_رستہ(StringRef رستہ) {
  أرجع رستہ.ناپ_2() > 1 && llvm::isAlpha(رستہ[0]) && رستہ[1] == ':';
}

ساكن منطقي is_جال_رستہ(StringRef رستہ) {
  أرجع رستہ.ناپ_2() > 2 && رستہ[0] == رستہ[1] &&
         llvm::sys::رستہ_2::is_separator(رستہ[0]);
}

ساكن منطقي should_Escape_وچ_URI(بنانشانی حرف C) {
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
  أرجع سچ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &باہرلا) {
  لكل (بنانشانی حرف C : Content) {
    إذا (should_Escape_وچ_URI(C)) {
      باہرلا.ڈالو_پٹھ('%');
      باہرلا.ڈالو_پٹھ(llvm::hexdigit(C / 16));
      باہرلا.ڈالو_پٹھ(llvm::hexdigit(C % 16));
    } وإلا {
      باہرلا.ڈالو_پٹھ(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string نتیجہ;
  لكل (خودکار I = Content.begin(), E = Content.سرا(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.سرا() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      نتیجہ.ڈالو_پٹھ(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      نتیجہ.ڈالو_پٹھ(*I);
    }
  }
  أرجع نتیجہ;
}

/// Return the set containing the supported URI schemes.
ساكن String_سیٹ<> &getSupportedSchemes() {
  ساكن String_سیٹ<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.خالی_تھاواں())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_دا(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uri_From_Absolute_رستہ(StringRef Absolute_رستہ,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef جڑ_2 = llvm::sys::رستہ_2::جڑ_name(Absolute_رستہ);
  إذا (is_جال_رستہ(جڑ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = جڑ_2.drop_front(2);
    Absolute_رستہ.consume_front(جڑ_2);
  } وإلا إذا (is_Windows_رستہ(جڑ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::رستہ_2::convert_منزل_slash(Absolute_رستہ);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.خالی_تھاواں() && Body.خالی_تھاواں())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.خالی_تھاواں() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> get_Absolute_رستہ(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
        llvm::inconvertible_Error_کوڈ(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> رستہ;
  إذا (!Authority.خالی_تھاواں()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).منزل_سمتیہ(رستہ);
  } وإلا إذا (is_Windows_رستہ(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  رستہ.append(Body);
  llvm::sys::رستہ_2::native(رستہ);
  أرجع std::string(رستہ);
}

ساكن llvm::Expected<std::string> parse_File_رستہ_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.لبھو(':');
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوڈ(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوڈ(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.لبھو('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().شامل_اے(UriScheme)) {
    أرجع llvm::createStringError(llvm::inconvertible_Error_کوڈ(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع get_Absolute_رستہ(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_رستہ = parse_File_رستہ_From_URI(Uri);
  إذا (!File_رستہ)
    أرجع File_رستہ.takeError();
  أرجع URIForFile(std::move(*File_رستہ), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_رستہ(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().ونڈو(':').پہلا_2; }

فراغ URIForFile::رجسٹر_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().داخل_کرو(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, URIForFile &نتیجہ,
                         llvm::json::رستہ رستہ) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      رستہ.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    نتیجہ = std::move(*ExpectedUri);
    أرجع سچ;
  }
  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت URIForFile &قدر) {
  أرجع قدر.uri();
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت URIForFile &قدر) {
  أرجع Os << قدر.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ClientCapabilities &نتیجہ, llvm::json::رستہ رستہ) {
  ثابت llvm::json::Object *O = قدر.getAsObject();
  إذا (!O) {
    رستہ.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *متن_Document = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_علامت =
            متن_Document->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> درجہ_وار_Support =
              Document_علامت->getBoolean("hierarchicalDocumentSymbolSupport"))
        نتیجہ.درجہ_وار_Document_علامت = *درجہ_وار_Support;
    }
    إذا (خودکار *کوڈ_Action = متن_Document->getObject("codeAction")) {
      إذا (کوڈ_Action->getObject("codeActionLiteralSupport"))
        نتیجہ.کوڈ_Action_Structure = سچ;
    }
  }
  إذا (خودکار *کھڑکی = O->getObject("window")) {
    إذا (std::optional<منطقي> Work_پورا_ترقی_Support =
            کھڑکی->getBoolean("workDoneProgress"))
      نتیجہ.work_پورا_ترقی = *Work_پورا_ترقی_Support;
  }
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, ClientInfo &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  إذا (!O || !O.map("name", نتیجہ.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", نتیجہ.نسخہ_2);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, TraceLevel &نتیجہ,
                         llvm::json::رستہ رستہ) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    إذا (*Str == "off") {
      نتیجہ = TraceLevel::Off;
      أرجع سچ;
    }
    إذا (*Str == "messages") {
      نتیجہ = TraceLevel::Messages;
      أرجع سچ;
    }
    إذا (*Str == "verbose") {
      نتیجہ = TraceLevel::Verbose;
      أرجع سچ;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InitializeParams &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", نتیجہ.capabilities);
  O.map("trace", نتیجہ.trace);
  O.map("rootUri", نتیجہ.جڑ_Uri);
  O.map("rootPath", نتیجہ.جڑ_رستہ);
  mapOptOrNull(قدر, "clientInfo", نتیجہ.clientInfo, رستہ);

  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_شے &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("uri", نتیجہ.uri) &&
         O.map("languageId", نتیجہ.languageId) && O.map("text", نتیجہ.متن) &&
         O.map("version", نتیجہ.نسخہ_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت متن_Document_Identifier &قدر) {
  أرجع llvm::json::Object{{"uri", قدر.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Identifier &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("uri", نتیجہ.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر
llvm::lsp::منزل_JSON(ثابت Versioned_متن_Document_Identifier &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"version", قدر.نسخہ_2},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Versioned_متن_Document_Identifier &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("uri", نتیجہ.uri) && O.map("version", نتیجہ.نسخہ_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Position &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("line", نتیجہ.سطر_2) &&
         O.map("character", نتیجہ.character);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Position &قدر) {
  أرجع llvm::json::Object{
      {"line", قدر.سطر_2},
      {"character", قدر.character},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Position &قدر) {
  أرجع Os << قدر.سطر_2 << ':' << قدر.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Range &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("start", نتیجہ.شروع_2) && O.map("end", نتیجہ.سرا);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Range &قدر) {
  أرجع llvm::json::Object{
      {"start", قدر.شروع_2},
      {"end", قدر.سرا},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Range &قدر) {
  أرجع Os << قدر.شروع_2 << '-' << قدر.سرا;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Location &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("uri", نتیجہ.uri) && O.map("range", نتیجہ.range);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Location &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"range", قدر.range},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Location &قدر) {
  أرجع Os << قدر.range << '@' << قدر.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Position_Params &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("position", نتیجہ.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceContext &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.mapOptional("includeDeclaration", نتیجہ.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceParams &نتیجہ, llvm::json::رستہ رستہ) {
  متن_Document_Position_Params &ادھار = نتیجہ;
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع fromJSON(قدر, ادھار, رستہ) && O &&
         O.mapOptional("context", نتیجہ.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Open_متن_Document_Params &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Close_متن_Document_Params &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &Params,
                         Did_Save_متن_Document_Params &R, llvm::json::رستہ P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_نتیجہ
متن_Document_Content_Change_Event::apply_منزل(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = متن;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::سروت_Mgr TmpScrMgr;
  TmpScrMgr.جوڑو_New_سروت_بفر(llvm::حافظہ_بفر::get_Mem_بفر(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.شروع.get_اشارہ_گر() - Contents.انکڑے(),
                   RangeLoc.سرا_2.get_اشارہ_گر() - RangeLoc.شروع.get_اشارہ_گر(),
                   متن);
  أرجع success();
}

Logical_نتیجہ متن_Document_Content_Change_Event::apply_منزل(
    ArrayRef<متن_Document_Content_Change_Event> Changes, std::string &Contents) {
  لكل (ثابت خودکار &Change : Changes)
    إذا (failed(Change.apply_منزل(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Content_Change_Event &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("range", نتیجہ.range) &&
         O.map("rangeLength", نتیجہ.range_لمائی) && O.map("text", نتیجہ.متن);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Change_متن_Document_Params &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("contentChanges", نتیجہ.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef منزل_متن_قسم(Markup_قسم قسم_2) {
  بدل (قسم_2) {
  حالة Markup_قسم::Plain_متن:
    أرجع "plaintext";
  حالة Markup_قسم::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, Markup_قسم قسم_2) {
  أرجع Os << منزل_متن_قسم(قسم_2);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت MarkupContent &Mc) {
  إذا (Mc.قدر_2.خالی_تھاواں())
    أرجع خالی_سوچک;

  أرجع llvm::json::Object{
      {"kind", منزل_متن_قسم(Mc.قسم)},
      {"value", Mc.قدر_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Hover &Hover) {
  llvm::json::Object نتیجہ{{"contents", منزل_JSON(Hover.contents)}};
  إذا (Hover.range)
    نتیجہ["range"] = منزل_JSON(*Hover.range);
  أرجع std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Document_علامت &علامت) {
  llvm::json::Object نتیجہ{{"name", علامت.name},
                            {"kind", static_cast<صحيح>(علامت.قسم)},
                            {"range", علامت.range},
                            {"selectionRange", علامت.selectionRange}};

  إذا (!علامت.detail.خالی_تھاواں())
    نتیجہ["detail"] = علامت.detail;
  إذا (!علامت.children.خالی_تھاواں())
    نتیجہ["children"] = علامت.children;
  أرجع std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Document_علامت_Params &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DiagnosticRelatedInformation &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("location", نتیجہ.location) &&
         O.map("message", نتیجہ.سنیہا);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.سنیہا},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(Diagnostic_ٹیگ ٹیگ) {
  أرجع static_cast<صحيح>(ٹیگ);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic_ٹیگ &نتیجہ,
                         llvm::json::رستہ رستہ) {
  إذا (std::optional<int64_t> I = قدر.getAsInteger()) {
    نتیجہ = (Diagnostic_ٹیگ)*I;
    أرجع سچ;
  }

  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Diagnostic &Diag) {
  llvm::json::Object نتیجہ{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.سنیہا},
  };
  إذا (Diag.category)
    نتیجہ["category"] = *Diag.category;
  إذا (!Diag.سروت_2.خالی_تھاواں())
    نتیجہ["source"] = Diag.سروت_2;
  إذا (Diag.relatedInformation)
    نتیجہ["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.ٹیگ_سوچی.خالی_تھاواں())
    نتیجہ["tags"] = Diag.ٹیگ_سوچی;
  أرجع std::move(نتیجہ);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(قدر, "severity", Severity, رستہ))
    أرجع خطأ;
  نتیجہ.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", نتیجہ.range) && O.map("message", نتیجہ.سنیہا) &&
         mapOptOrNull(قدر, "category", نتیجہ.category, رستہ) &&
         mapOptOrNull(قدر, "source", نتیجہ.سروت_2, رستہ) &&
         mapOptOrNull(قدر, "relatedInformation", نتیجہ.relatedInformation,
                      رستہ) &&
         mapOptOrNull(قدر, "tags", نتیجہ.ٹیگ_سوچی, رستہ);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.نسخہ_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, متن_Edit &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("range", نتیجہ.range) && O.map("newText", نتیجہ.new_متن);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت متن_Edit &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"newText", قدر.new_متن},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت متن_Edit &قدر) {
  Os << قدر.range << " => \"";
  llvm::printEscapedString(قدر.new_متن, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شے_قسم &نتیجہ, llvm::json::رستہ رستہ) {
  إذا (std::optional<int64_t> Int_قدر = قدر.getAsInteger()) {
    إذا (*Int_قدر < static_cast<صحيح>(Completion_شے_قسم::متن_2) ||
        *Int_قدر > static_cast<صحيح>(Completion_شے_قسم::TypeParameter))
      أرجع خطأ;
    نتیجہ = static_cast<Completion_شے_قسم>(*Int_قدر);
    أرجع سچ;
  }
  أرجع خطأ;
}

Completion_شے_قسم llvm::lsp::adjust_قسم_منزل_Capability(
    Completion_شے_قسم قسم_2,
    Completion_شے_قسم_Bitset &Supported_Completion_شے_Kinds) {
  size_t قسم_Val = static_cast<size_t>(قسم_2);
  إذا (قسم_Val >= k_Completion_شے_قسم_Min &&
      قسم_Val <= Supported_Completion_شے_Kinds.ناپ_2() &&
      Supported_Completion_شے_Kinds[قسم_Val])
    أرجع قسم_2;

  // Provide some fall backs for common kinds that are close enough.
  بدل (قسم_2) {
  حالة Completion_شے_قسم::Folder:
    أرجع Completion_شے_قسم::File;
  حالة Completion_شے_قسم::EnumMember:
    أرجع Completion_شے_قسم::Enum;
  حالة Completion_شے_قسم::Struct:
    أرجع Completion_شے_قسم::Class;
  افتراضي:
    أرجع Completion_شے_قسم::متن_2;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شے_قسم_Bitset &نتیجہ,
                         llvm::json::رستہ رستہ) {
  إذا (ثابت llvm::json::Array *Array_قدر = قدر.getAsArray()) {
    لكل (size_t I = 0, E = Array_قدر->ناپ_2(); I < E; ++I) {
      Completion_شے_قسم قسم_باہرلا;
      إذا (fromJSON((*Array_قدر)[I], قسم_باہرلا, رستہ.اشاریہ(I)))
        نتیجہ.سیٹ(size_t(قسم_باہرلا));
    }
    أرجع سچ;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Completion_شے &قدر) {
  أكد(!قدر.label.خالی_تھاواں() && "completion item label is required");
  llvm::json::Object نتیجہ{{"label", قدر.label}};
  إذا (قدر.قسم != Completion_شے_قسم::Missing)
    نتیجہ["kind"] = static_cast<صحيح>(قدر.قسم);
  إذا (!قدر.detail.خالی_تھاواں())
    نتیجہ["detail"] = قدر.detail;
  إذا (قدر.documentation)
    نتیجہ["documentation"] = قدر.documentation;
  إذا (!قدر.ترتیب_دیو_متن.خالی_تھاواں())
    نتیجہ["sortText"] = قدر.ترتیب_دیو_متن;
  إذا (!قدر.filter_متن.خالی_تھاواں())
    نتیجہ["filterText"] = قدر.filter_متن;
  إذا (!قدر.داخل_کرو_متن.خالی_تھاواں())
    نتیجہ["insertText"] = قدر.داخل_کرو_متن;
  إذا (قدر.داخل_کرو_متن_Format != داخل_کرو_متن_Format_2::Missing)
    نتیجہ["insertTextFormat"] = static_cast<صحيح>(قدر.داخل_کرو_متن_Format);
  إذا (قدر.متن_Edit_2)
    نتیجہ["textEdit"] = *قدر.متن_Edit_2;
  إذا (!قدر.additional_متن_Edits.خالی_تھاواں()) {
    نتیجہ["additionalTextEdits"] =
        llvm::json::Array(قدر.additional_متن_Edits);
  }
  إذا (قدر.deprecated)
    نتیجہ["deprecated"] = قدر.deprecated;
  أرجع std::move(نتیجہ);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت Completion_شے &قدر) {
  أرجع Os << قدر.label << " - " << منزل_JSON(قدر);
}

منطقي llvm::lsp::عامل<(ثابت Completion_شے &Lhs,
                          ثابت Completion_شے &Rhs) {
  أرجع (Lhs.ترتیب_دیو_متن.خالی_تھاواں() ? Lhs.label : Lhs.ترتیب_دیو_متن) <
         (Rhs.ترتیب_دیو_متن.خالی_تھاواں() ? Rhs.label : Rhs.ترتیب_دیو_متن);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت CompletionList &قدر) {
  أرجع llvm::json::Object{
      {"isIncomplete", قدر.isIncomplete},
      {"items", llvm::json::Array(قدر.اشیاء)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionContext &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  صحيح Trigger_قسم;
  إذا (!O || !O.map("triggerKind", Trigger_قسم) ||
      !mapOptOrNull(قدر, "triggerCharacter", نتیجہ.triggerCharacter, رستہ))
    أرجع خطأ;
  نتیجہ.trigger_قسم = static_cast<Completion_Trigger_قسم>(Trigger_قسم);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionParams &نتیجہ, llvm::json::رستہ رستہ) {
  إذا (!fromJSON(قدر, static_cast<متن_Document_Position_Params &>(نتیجہ), رستہ))
    أرجع خطأ;
  إذا (ثابت llvm::json::قدر *Context = قدر.getAsObject()->get("context"))
    أرجع fromJSON(*Context, نتیجہ.context, رستہ.میدان("context"));
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت ParameterInformation &قدر) {
  أكد((قدر.labelOffsets || !قدر.labelString.خالی_تھاواں()) &&
         "parameter information label is required");
  llvm::json::Object نتیجہ;
  إذا (قدر.labelOffsets)
    نتیجہ["label"] = llvm::json::Array(
        {قدر.labelOffsets->پہلا_2, قدر.labelOffsets->دوجا_2});
  وإلا
    نتیجہ["label"] = قدر.labelString;
  إذا (!قدر.documentation.خالی_تھاواں())
    نتیجہ["documentation"] = قدر.documentation;
  أرجع std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت SignatureInformation &قدر) {
  أكد(!قدر.label.خالی_تھاواں() && "signature information label is required");
  llvm::json::Object نتیجہ{
      {"label", قدر.label},
      {"parameters", llvm::json::Array(قدر.parameters)},
  };
  إذا (!قدر.documentation.خالی_تھاواں())
    نتیجہ["documentation"] = قدر.documentation;
  أرجع std::move(نتیجہ);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قدر) {
  أرجع Os << قدر.label << " - " << منزل_JSON(قدر);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت SignatureHelp &قدر) {
  أكد(قدر.سرگرم_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قدر.سرگرم_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", قدر.سرگرم_Signature},
      {"activeParameter", قدر.سرگرم_Parameter},
      {"signatures", llvm::json::Array(قدر.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DocumentLinkParams &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت DocumentLink &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"target", قدر.نشانہ},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InlayHintsParams &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("range", نتیجہ.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت InlayHint &قدر) {
  أرجع llvm::json::Object{{"position", قدر.position},
                            {"kind", (صحيح)قدر.قسم},
                            {"label", قدر.label},
                            {"paddingLeft", قدر.padding_بایاں},
                            {"paddingRight", قدر.padding_دایاں}};
}
منطقي llvm::lsp::عامل==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.قسم, Lhs.label) ==
         std::tie(Rhs.position, Rhs.قسم, Rhs.label);
}
منطقي llvm::lsp::عامل<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.قسم, Lhs.label) <
         std::tie(Rhs.position, Rhs.قسم, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::عامل<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_قسم قدر) {
  بدل (قدر) {
  حالة Inlay_Hint_قسم::Parameter:
    أرجع Os << "parameter";
  حالة Inlay_Hint_قسم::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         کوڈ_Action_Context &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  إذا (!O || !O.map("diagnostics", نتیجہ.diagnostics))
    أرجع خطأ;
  O.map("only", نتیجہ.only);
  أرجع سچ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         کوڈ_Action_Params &نتیجہ, llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("range", نتیجہ.range) && O.map("context", نتیجہ.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, WorkspaceEdit &نتیجہ,
                         llvm::json::رستہ رستہ) {
  llvm::json::ObjectMapper O(قدر, رستہ);
  أرجع O && O.map("changes", نتیجہ.changes);
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت WorkspaceEdit &قدر) {
  llvm::json::Object FileChanges;
  لكل (خودکار &Change : قدر.changes)
    FileChanges[Change.پہلا_2] = llvm::json::Array(Change.دوجا_2);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral کوڈ_Action::k_تیز_Fix = "quickfix";
ثابت llvm::StringLiteral کوڈ_Action::kRefactor = "refactor";
ثابت llvm::StringLiteral کوڈ_Action::kInfo = "info";

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت کوڈ_Action &قدر) {
  llvm::json::Object کوڈ_Action{{"title", قدر.title}};
  إذا (قدر.قسم)
    کوڈ_Action["kind"] = *قدر.قسم;
  إذا (قدر.diagnostics)
    کوڈ_Action["diagnostics"] = llvm::json::Array(*قدر.diagnostics);
  إذا (قدر.isPreferred)
    کوڈ_Action["isPreferred"] = سچ;
  إذا (قدر.edit)
    کوڈ_Action["edit"] = *قدر.edit;
  أرجع std::move(کوڈ_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت Show_سنیہا_Params &Params) {
  خودکار باہرلا = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.سنیہا},
  };
  إذا (Params.actions)
    باہرلا["actions"] = *Params.actions;
  أرجع باہرلا;
}

llvm::json::قدر llvm::lsp::منزل_JSON(ثابت سنیہا_Action_شے &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
