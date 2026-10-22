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

#شامل "llvm/Support/LSP/Protocol.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringSet.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/JSON.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/raw_ostream.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
سانچہ <قسم_نام T>
ساکن منطقی mapOptOrNull(ثابت llvm::json::قدر &Params,
                         llvm::StringLiteral Prop, T &برآمد,
                         llvm::json::راستہ راستہ) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  خودکار *V = O->get(Prop);
  اگر (!V || V->getAsNull())
    واپس سچ;
  واپس fromJSON(*V, برآمد, راستہ.میدان(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساکن منطقی is_Windows_راستہ(StringRef راستہ) {
  واپس راستہ.سائز_2() > 1 && llvm::isAlpha(راستہ[0]) && راستہ[1] == ':';
}

ساکن منطقی is_جال_راستہ(StringRef راستہ) {
  واپس راستہ.سائز_2() > 2 && راستہ[0] == راستہ[1] &&
         llvm::sys::راستہ_2::is_separator(راستہ[0]);
}

ساکن منطقی should_Escape_اندر_URI(غیرعلامتی حرف C) {
  // Unreserved characters.
  اگر ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    واپس جھوٹ;

  انتخاب (C) {
  حالت '-':
  حالت '_':
  حالت '.':
  حالت '~':
  // '/' is only reserved when parsing.
  حالت '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  حالت ':':
    واپس جھوٹ;
  }
  واپس سچ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساکن خالی percentEncode(StringRef Content, std::string &برآمد) {
  برائے (غیرعلامتی حرف C : Content) {
    اگر (should_Escape_اندر_URI(C)) {
      برآمد.ڈالو_عقب('%');
      برآمد.ڈالو_عقب(llvm::hexdigit(C / 16));
      برآمد.ڈالو_عقب(llvm::hexdigit(C % 16));
    } ورنہ {
      برآمد.ڈالو_عقب(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساکن std::string percentDecode(StringRef Content) {
  std::string نتیجہ;
  برائے (خودکار I = Content.begin(), E = Content.انتہا(); I != E; ++I) {
    اگر (*I == '%' && I + 2 < Content.انتہا() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      نتیجہ.ڈالو_عقب(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ورنہ {
      نتیجہ.ڈالو_عقب(*I);
    }
  }
  واپس نتیجہ;
}

/// Return the set containing the supported URI schemes.
ساکن String_سیٹ<> &getSupportedSchemes() {
  ساکن String_سیٹ<> Schemes({"file", "test"});
  واپس Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساکن منطقی isStructurallyValidScheme(StringRef Scheme) {
  اگر (Scheme.خالی_خانے())
    واپس جھوٹ;
  اگر (!llvm::isAlpha(Scheme[0]))
    واپس جھوٹ;
  واپس llvm::all_کا(llvm::drop_begin(Scheme), [](حرف C) {
    واپس llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساکن llvm::Expected<std::string> uri_From_Absolute_راستہ(StringRef Absolute_راستہ,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef جڑ_2 = llvm::sys::راستہ_2::جڑ_name(Absolute_راستہ);
  اگر (is_جال_راستہ(جڑ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = جڑ_2.drop_front(2);
    Absolute_راستہ.consume_front(جڑ_2);
  } ورنہ اگر (is_Windows_راستہ(جڑ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::راستہ_2::convert_مقصد_slash(Absolute_راستہ);

  std::string Uri = Scheme.str() + ":";
  اگر (Authority.خالی_خانے() && Body.خالی_خانے())
    واپس Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  اگر (!Authority.خالی_خانے() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  واپس Uri;
}

ساکن llvm::Expected<std::string> get_Absolute_راستہ(StringRef Authority,
                                                   StringRef Body) {
  اگر (!Body.starts_with("/"))
    واپس llvm::createStringError(
        llvm::inconvertible_Error_رمز(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> راستہ;
  اگر (!Authority.خالی_خانے()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).مقصد_سمتیہ(راستہ);
  } ورنہ اگر (is_Windows_راستہ(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  راستہ.append(Body);
  llvm::sys::راستہ_2::native(راستہ);
  واپس std::string(راستہ);
}

ساکن llvm::Expected<std::string> parse_File_راستہ_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.ڈھونڈو(':');
  اگر (Pos == StringRef::npos)
    واپس llvm::createStringError(llvm::inconvertible_Error_رمز(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  اگر (!isStructurallyValidScheme(UriScheme))
    واپس llvm::createStringError(llvm::inconvertible_Error_رمز(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  اگر (Uri.consume_front("//")) {
    Pos = Uri.ڈھونڈو('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  اگر (!getSupportedSchemes().شامل_ہے(UriScheme)) {
    واپس llvm::createStringError(llvm::inconvertible_Error_رمز(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  واپس get_Absolute_راستہ(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_راستہ = parse_File_راستہ_From_URI(Uri);
  اگر (!File_راستہ)
    واپس File_راستہ.takeError();
  واپس URIForFile(std::move(*File_راستہ), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_راستہ(AbsoluteFilepath, Scheme);
  اگر (!Uri)
    واپس Uri.takeError();
  واپس fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { واپس uri().بانٹو(':').پہلا_2; }

خالی URIForFile::رجسٹر_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().داخل_کرو(Scheme);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, URIForFile &نتیجہ,
                         llvm::json::راستہ راستہ) {
  اگر (std::optional<StringRef> Str = قدر.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    اگر (!ExpectedUri) {
      راستہ.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      واپس جھوٹ;
    }
    نتیجہ = std::move(*ExpectedUri);
    واپس سچ;
  }
  واپس جھوٹ;
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت URIForFile &قدر) {
  واپس قدر.uri();
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت URIForFile &قدر) {
  واپس Os << قدر.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ClientCapabilities &نتیجہ, llvm::json::راستہ راستہ) {
  ثابت llvm::json::Object *O = قدر.getAsObject();
  اگر (!O) {
    راستہ.report("expected object");
    واپس جھوٹ;
  }
  اگر (ثابت llvm::json::Object *متن_Document = O->getObject("textDocument")) {
    اگر (ثابت llvm::json::Object *Document_علامت =
            متن_Document->getObject("documentSymbol")) {
      اگر (std::optional<منطقی> درجہ_وار_Support =
              Document_علامت->getBoolean("hierarchicalDocumentSymbolSupport"))
        نتیجہ.درجہ_وار_Document_علامت = *درجہ_وار_Support;
    }
    اگر (خودکار *رمز_Action = متن_Document->getObject("codeAction")) {
      اگر (رمز_Action->getObject("codeActionLiteralSupport"))
        نتیجہ.رمز_Action_Structure = سچ;
    }
  }
  اگر (خودکار *کھڑکی = O->getObject("window")) {
    اگر (std::optional<منطقی> Work_مکمل_پیش_رفت_Support =
            کھڑکی->getBoolean("workDoneProgress"))
      نتیجہ.work_مکمل_پیش_رفت = *Work_مکمل_پیش_رفت_Support;
  }
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, ClientInfo &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  اگر (!O || !O.map("name", نتیجہ.name))
    واپس جھوٹ;

  // Don't fail if we can't parse version.
  O.map("version", نتیجہ.نسخہ_2);
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, TraceLevel &نتیجہ,
                         llvm::json::راستہ راستہ) {
  اگر (std::optional<StringRef> Str = قدر.getAsString()) {
    اگر (*Str == "off") {
      نتیجہ = TraceLevel::Off;
      واپس سچ;
    }
    اگر (*Str == "messages") {
      نتیجہ = TraceLevel::Messages;
      واپس سچ;
    }
    اگر (*Str == "verbose") {
      نتیجہ = TraceLevel::Verbose;
      واپس سچ;
    }
  }
  واپس جھوٹ;
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InitializeParams &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  اگر (!O)
    واپس جھوٹ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", نتیجہ.capabilities);
  O.map("trace", نتیجہ.trace);
  O.map("rootUri", نتیجہ.جڑ_Uri);
  O.map("rootPath", نتیجہ.جڑ_راستہ);
  mapOptOrNull(قدر, "clientInfo", نتیجہ.clientInfo, راستہ);

  واپس سچ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_شے &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("uri", نتیجہ.uri) &&
         O.map("languageId", نتیجہ.languageId) && O.map("text", نتیجہ.متن) &&
         O.map("version", نتیجہ.نسخہ_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت متن_Document_Identifier &قدر) {
  واپس llvm::json::Object{{"uri", قدر.uri}};
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Identifier &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("uri", نتیجہ.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر
llvm::lsp::مقصد_JSON(ثابت Versioned_متن_Document_Identifier &قدر) {
  واپس llvm::json::Object{
      {"uri", قدر.uri},
      {"version", قدر.نسخہ_2},
  };
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Versioned_متن_Document_Identifier &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("uri", نتیجہ.uri) && O.map("version", نتیجہ.نسخہ_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Position &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("line", نتیجہ.سطر_2) &&
         O.map("character", نتیجہ.character);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Position &قدر) {
  واپس llvm::json::Object{
      {"line", قدر.سطر_2},
      {"character", قدر.character},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Position &قدر) {
  واپس Os << قدر.سطر_2 << ':' << قدر.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Range &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("start", نتیجہ.آغاز_2) && O.map("end", نتیجہ.انتہا);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Range &قدر) {
  واپس llvm::json::Object{
      {"start", قدر.آغاز_2},
      {"end", قدر.انتہا},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Range &قدر) {
  واپس Os << قدر.آغاز_2 << '-' << قدر.انتہا;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Location &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("uri", نتیجہ.uri) && O.map("range", نتیجہ.range);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Location &قدر) {
  واپس llvm::json::Object{
      {"uri", قدر.uri},
      {"range", قدر.range},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت Location &قدر) {
  واپس Os << قدر.range << '@' << قدر.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Position_Params &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("position", نتیجہ.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceContext &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.mapOptional("includeDeclaration", نتیجہ.includeDeclaration);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceParams &نتیجہ, llvm::json::راستہ راستہ) {
  متن_Document_Position_Params &بنیاد = نتیجہ;
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس fromJSON(قدر, بنیاد, راستہ) && O &&
         O.mapOptional("context", نتیجہ.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Open_متن_Document_Params &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Close_متن_Document_Params &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &Params,
                         Did_Save_متن_Document_Params &R, llvm::json::راستہ P) {
  llvm::json::ObjectMapper O(Params, P);
  واپس O && O.map("textDocument", R.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_نتیجہ
متن_Document_Content_Change_Event::apply_مقصد(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  اگر (!range) {
    Contents = متن;
    واپس success();
  }

  // Try to map the replacement range to the content.
  llvm::ماخذ_Mgr TmpScrMgr;
  TmpScrMgr.جوڑو_New_ماخذ_بفر(llvm::حافظہ_بفر::get_Mem_بفر(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  اگر (!RangeLoc.isValid())
    واپس failure();

  Contents.replace(RangeLoc.آغاز.get_اشارہ_گر() - Contents.ڈیٹا(),
                   RangeLoc.انتہا_2.get_اشارہ_گر() - RangeLoc.آغاز.get_اشارہ_گر(),
                   متن);
  واپس success();
}

Logical_نتیجہ متن_Document_Content_Change_Event::apply_مقصد(
    ArrayRef<متن_Document_Content_Change_Event> Changes, std::string &Contents) {
  برائے (ثابت خودکار &Change : Changes)
    اگر (failed(Change.apply_مقصد(Contents)))
      واپس failure();
  واپس success();
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         متن_Document_Content_Change_Event &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("range", نتیجہ.range) &&
         O.map("rangeLength", نتیجہ.range_لمبائی) && O.map("text", نتیجہ.متن);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Did_Change_متن_Document_Params &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("contentChanges", نتیجہ.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساکن llvm::StringRef مقصد_متن_قسم(Markup_قسم قسم_3) {
  انتخاب (قسم_3) {
  حالت Markup_قسم::Plain_متن:
    واپس "plaintext";
  حالت Markup_قسم::Markdown:
    واپس "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, Markup_قسم قسم_3) {
  واپس Os << مقصد_متن_قسم(قسم_3);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت MarkupContent &Mc) {
  اگر (Mc.قدر_2.خالی_خانے())
    واپس خالی_اشاریہ;

  واپس llvm::json::Object{
      {"kind", مقصد_متن_قسم(Mc.قسم_2)},
      {"value", Mc.قدر_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Hover &Hover) {
  llvm::json::Object نتیجہ{{"contents", مقصد_JSON(Hover.contents)}};
  اگر (Hover.range)
    نتیجہ["range"] = مقصد_JSON(*Hover.range);
  واپس std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Document_علامت &علامت) {
  llvm::json::Object نتیجہ{{"name", علامت.name},
                            {"kind", static_cast<صحیح>(علامت.قسم_2)},
                            {"range", علامت.range},
                            {"selectionRange", علامت.selectionRange}};

  اگر (!علامت.detail.خالی_خانے())
    نتیجہ["detail"] = علامت.detail;
  اگر (!علامت.children.خالی_خانے())
    نتیجہ["children"] = علامت.children;
  واپس std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Document_علامت_Params &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DiagnosticRelatedInformation &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("location", نتیجہ.location) &&
         O.map("message", نتیجہ.پیغام_2);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت DiagnosticRelatedInformation &Info) {
  واپس llvm::json::Object{
      {"location", Info.location},
      {"message", Info.پیغام_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(Diagnostic_لیبل لیبل) {
  واپس static_cast<صحیح>(لیبل);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic_لیبل &نتیجہ,
                         llvm::json::راستہ راستہ) {
  اگر (std::optional<int64_t> I = قدر.getAsInteger()) {
    نتیجہ = (Diagnostic_لیبل)*I;
    واپس سچ;
  }

  واپس جھوٹ;
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Diagnostic &Diag) {
  llvm::json::Object نتیجہ{
      {"range", Diag.range},
      {"severity", (صحیح)Diag.severity},
      {"message", Diag.پیغام_2},
  };
  اگر (Diag.category)
    نتیجہ["category"] = *Diag.category;
  اگر (!Diag.ماخذ_2.خالی_خانے())
    نتیجہ["source"] = Diag.ماخذ_2;
  اگر (Diag.relatedInformation)
    نتیجہ["relatedInformation"] = *Diag.relatedInformation;
  اگر (!Diag.لیبل_فہرست.خالی_خانے())
    نتیجہ["tags"] = Diag.لیبل_فہرست;
  واپس std::move(نتیجہ);
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  اگر (!O)
    واپس جھوٹ;
  صحیح Severity = 0;
  اگر (!mapOptOrNull(قدر, "severity", Severity, راستہ))
    واپس جھوٹ;
  نتیجہ.severity = (DiagnosticSeverity)Severity;

  واپس O.map("range", نتیجہ.range) && O.map("message", نتیجہ.پیغام_2) &&
         mapOptOrNull(قدر, "category", نتیجہ.category, راستہ) &&
         mapOptOrNull(قدر, "source", نتیجہ.ماخذ_2, راستہ) &&
         mapOptOrNull(قدر, "relatedInformation", نتیجہ.relatedInformation,
                      راستہ) &&
         mapOptOrNull(قدر, "tags", نتیجہ.لیبل_فہرست, راستہ);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت PublishDiagnosticsParams &Params) {
  واپس llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.نسخہ_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, متن_Edit &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("range", نتیجہ.range) && O.map("newText", نتیجہ.new_متن);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت متن_Edit &قدر) {
  واپس llvm::json::Object{
      {"range", قدر.range},
      {"newText", قدر.new_متن},
  };
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os, ثابت متن_Edit &قدر) {
  Os << قدر.range << " => \"";
  llvm::printEscapedString(قدر.new_متن, Os);
  واپس Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شے_قسم &نتیجہ, llvm::json::راستہ راستہ) {
  اگر (std::optional<int64_t> Int_قدر = قدر.getAsInteger()) {
    اگر (*Int_قدر < static_cast<صحیح>(Completion_شے_قسم::متن_2) ||
        *Int_قدر > static_cast<صحیح>(Completion_شے_قسم::TypeParameter))
      واپس جھوٹ;
    نتیجہ = static_cast<Completion_شے_قسم>(*Int_قدر);
    واپس سچ;
  }
  واپس جھوٹ;
}

Completion_شے_قسم llvm::lsp::adjust_قسم_مقصد_Capability(
    Completion_شے_قسم قسم_3,
    Completion_شے_قسم_Bitset &Supported_Completion_شے_Kinds) {
  size_t قسم_Val = static_cast<size_t>(قسم_3);
  اگر (قسم_Val >= k_Completion_شے_قسم_Min &&
      قسم_Val <= Supported_Completion_شے_Kinds.سائز_2() &&
      Supported_Completion_شے_Kinds[قسم_Val])
    واپس قسم_3;

  // Provide some fall backs for common kinds that are close enough.
  انتخاب (قسم_3) {
  حالت Completion_شے_قسم::Folder:
    واپس Completion_شے_قسم::File;
  حالت Completion_شے_قسم::EnumMember:
    واپس Completion_شے_قسم::Enum;
  حالت Completion_شے_قسم::Struct:
    واپس Completion_شے_قسم::Class;
  طےشدہ:
    واپس Completion_شے_قسم::متن_2;
  }
}

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Completion_شے_قسم_Bitset &نتیجہ,
                         llvm::json::راستہ راستہ) {
  اگر (ثابت llvm::json::Array *Array_قدر = قدر.getAsArray()) {
    برائے (size_t I = 0, E = Array_قدر->سائز_2(); I < E; ++I) {
      Completion_شے_قسم قسم_برآمد;
      اگر (fromJSON((*Array_قدر)[I], قسم_برآمد, راستہ.اشاریہ(I)))
        نتیجہ.سیٹ(size_t(قسم_برآمد));
    }
    واپس سچ;
  }
  واپس جھوٹ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Completion_شے &قدر) {
  أكد(!قدر.label.خالی_خانے() && "completion item label is required");
  llvm::json::Object نتیجہ{{"label", قدر.label}};
  اگر (قدر.قسم_2 != Completion_شے_قسم::Missing)
    نتیجہ["kind"] = static_cast<صحیح>(قدر.قسم_2);
  اگر (!قدر.detail.خالی_خانے())
    نتیجہ["detail"] = قدر.detail;
  اگر (قدر.documentation)
    نتیجہ["documentation"] = قدر.documentation;
  اگر (!قدر.ترتیب_دو_متن.خالی_خانے())
    نتیجہ["sortText"] = قدر.ترتیب_دو_متن;
  اگر (!قدر.filter_متن.خالی_خانے())
    نتیجہ["filterText"] = قدر.filter_متن;
  اگر (!قدر.داخل_کرو_متن.خالی_خانے())
    نتیجہ["insertText"] = قدر.داخل_کرو_متن;
  اگر (قدر.داخل_کرو_متن_Format != داخل_کرو_متن_Format_2::Missing)
    نتیجہ["insertTextFormat"] = static_cast<صحیح>(قدر.داخل_کرو_متن_Format);
  اگر (قدر.متن_Edit_2)
    نتیجہ["textEdit"] = *قدر.متن_Edit_2;
  اگر (!قدر.additional_متن_Edits.خالی_خانے()) {
    نتیجہ["additionalTextEdits"] =
        llvm::json::Array(قدر.additional_متن_Edits);
  }
  اگر (قدر.deprecated)
    نتیجہ["deprecated"] = قدر.deprecated;
  واپس std::move(نتیجہ);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت Completion_شے &قدر) {
  واپس Os << قدر.label << " - " << مقصد_JSON(قدر);
}

منطقی llvm::lsp::عامل<(ثابت Completion_شے &Lhs,
                          ثابت Completion_شے &Rhs) {
  واپس (Lhs.ترتیب_دو_متن.خالی_خانے() ? Lhs.label : Lhs.ترتیب_دو_متن) <
         (Rhs.ترتیب_دو_متن.خالی_خانے() ? Rhs.label : Rhs.ترتیب_دو_متن);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت CompletionList &قدر) {
  واپس llvm::json::Object{
      {"isIncomplete", قدر.isIncomplete},
      {"items", llvm::json::Array(قدر.اشیاء)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionContext &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  صحیح Trigger_قسم;
  اگر (!O || !O.map("triggerKind", Trigger_قسم) ||
      !mapOptOrNull(قدر, "triggerCharacter", نتیجہ.triggerCharacter, راستہ))
    واپس جھوٹ;
  نتیجہ.trigger_قسم = static_cast<Completion_Trigger_قسم>(Trigger_قسم);
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionParams &نتیجہ, llvm::json::راستہ راستہ) {
  اگر (!fromJSON(قدر, static_cast<متن_Document_Position_Params &>(نتیجہ), راستہ))
    واپس جھوٹ;
  اگر (ثابت llvm::json::قدر *Context = قدر.getAsObject()->get("context"))
    واپس fromJSON(*Context, نتیجہ.context, راستہ.میدان("context"));
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت ParameterInformation &قدر) {
  أكد((قدر.labelOffsets || !قدر.labelString.خالی_خانے()) &&
         "parameter information label is required");
  llvm::json::Object نتیجہ;
  اگر (قدر.labelOffsets)
    نتیجہ["label"] = llvm::json::Array(
        {قدر.labelOffsets->پہلا_2, قدر.labelOffsets->دوسرا_2});
  ورنہ
    نتیجہ["label"] = قدر.labelString;
  اگر (!قدر.documentation.خالی_خانے())
    نتیجہ["documentation"] = قدر.documentation;
  واپس std::move(نتیجہ);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت SignatureInformation &قدر) {
  أكد(!قدر.label.خالی_خانے() && "signature information label is required");
  llvm::json::Object نتیجہ{
      {"label", قدر.label},
      {"parameters", llvm::json::Array(قدر.parameters)},
  };
  اگر (!قدر.documentation.خالی_خانے())
    نتیجہ["documentation"] = قدر.documentation;
  واپس std::move(نتیجہ);
}

raw_ostream &llvm::lsp::عامل<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قدر) {
  واپس Os << قدر.label << " - " << مقصد_JSON(قدر);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت SignatureHelp &قدر) {
  أكد(قدر.فعال_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قدر.فعال_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  واپس llvm::json::Object{
      {"activeSignature", قدر.فعال_Signature},
      {"activeParameter", قدر.فعال_Parameter},
      {"signatures", llvm::json::Array(قدر.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DocumentLinkParams &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت DocumentLink &قدر) {
  واپس llvm::json::Object{
      {"range", قدر.range},
      {"target", قدر.ہدف},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InlayHintsParams &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("range", نتیجہ.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت InlayHint &قدر) {
  واپس llvm::json::Object{{"position", قدر.position},
                            {"kind", (صحیح)قدر.قسم_2},
                            {"label", قدر.label},
                            {"paddingLeft", قدر.padding_بایاں},
                            {"paddingRight", قدر.padding_دایاں}};
}
منطقی llvm::lsp::عامل==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  واپس std::tie(Lhs.position, Lhs.قسم_2, Lhs.label) ==
         std::tie(Rhs.position, Rhs.قسم_2, Rhs.label);
}
منطقی llvm::lsp::عامل<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  واپس std::tie(Lhs.position, Lhs.قسم_2, Lhs.label) <
         std::tie(Rhs.position, Rhs.قسم_2, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::عامل<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_قسم قدر) {
  انتخاب (قدر) {
  حالت Inlay_Hint_قسم::Parameter:
    واپس Os << "parameter";
  حالت Inlay_Hint_قسم::Type:
    واپس Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         رمز_Action_Context &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  اگر (!O || !O.map("diagnostics", نتیجہ.diagnostics))
    واپس جھوٹ;
  O.map("only", نتیجہ.only);
  واپس سچ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         رمز_Action_Params &نتیجہ, llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("textDocument", نتیجہ.متن_Document_2) &&
         O.map("range", نتیجہ.range) && O.map("context", نتیجہ.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقی llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, WorkspaceEdit &نتیجہ,
                         llvm::json::راستہ راستہ) {
  llvm::json::ObjectMapper O(قدر, راستہ);
  واپس O && O.map("changes", نتیجہ.changes);
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت WorkspaceEdit &قدر) {
  llvm::json::Object FileChanges;
  برائے (خودکار &Change : قدر.changes)
    FileChanges[Change.پہلا_2] = llvm::json::Array(Change.دوسرا_2);
  واپس llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral رمز_Action::k_تیز_Fix = "quickfix";
ثابت llvm::StringLiteral رمز_Action::kRefactor = "refactor";
ثابت llvm::StringLiteral رمز_Action::kInfo = "info";

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت رمز_Action &قدر) {
  llvm::json::Object رمز_Action{{"title", قدر.title}};
  اگر (قدر.قسم_2)
    رمز_Action["kind"] = *قدر.قسم_2;
  اگر (قدر.diagnostics)
    رمز_Action["diagnostics"] = llvm::json::Array(*قدر.diagnostics);
  اگر (قدر.isPreferred)
    رمز_Action["isPreferred"] = سچ;
  اگر (قدر.edit)
    رمز_Action["edit"] = *قدر.edit;
  واپس std::move(رمز_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت Show_پیغام_Params &Params) {
  خودکار برآمد = llvm::json::Object{
      {"type", static_cast<صحیح>(Params.type)},
      {"message", Params.پیغام_2},
  };
  اگر (Params.actions)
    برآمد["actions"] = *Params.actions;
  واپس برآمد;
}

llvm::json::قدر llvm::lsp::مقصد_JSON(ثابت پیغام_Action_شے &Params) {
  واپس llvm::json::Object{{"title", Params.title}};
}
