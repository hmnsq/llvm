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

using نطاق llvm;
using نطاق llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
ساكن منطقي mapOptOrNull(ثابت llvm::json::قيمة &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    إرجاع true;
  إرجاع fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي isWindowsPath(StringRef Path) {
  إرجاع Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ساكن منطقي isNetworkPath(StringRef Path) {
  إرجاع Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ساكن منطقي shouldEscapeInURI(unsigned حرف C) {
  // Unreserved characters.
  إذا ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    إرجاع خطأ;

  بدل (C) {
  حالة '-':
  حالة '_':
  حالة '.':
  حالة '~':
  // '/' is only reserved when parsing.
  حالة '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  حالة ':':
    إرجاع خطأ;
  }
  إرجاع true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &Out) {
  لكل (unsigned حرف C : Content) {
    إذا (shouldEscapeInURI(C)) {
      Out.ادفع_back('%');
      Out.ادفع_back(llvm::hexdigit(C / 16));
      Out.ادفع_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.ادفع_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ادفع_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.ادفع_back(*I);
    }
  }
  إرجاع Result;
}

/// Return the set containing the supported URI schemes.
ساكن StringSet<> &getSupportedSchemes() {
  ساكن StringSet<> Schemes({"file", "test"});
  إرجاع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.empty())
    إرجاع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    إرجاع خطأ;
  إرجاع llvm::all_of(llvm::drop_begin(Scheme), [](حرف C) {
    إرجاع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef جذر_2 = llvm::sys::path::جذر_name(AbsolutePath);
  إذا (isNetworkPath(جذر_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = جذر_2.drop_front(2);
    AbsolutePath.consume_front(جذر_2);
  } وإلا إذا (isWindowsPath(جذر_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.empty() && Body.empty())
    إرجاع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  إرجاع Uri;
}

ساكن llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    إرجاع llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  إذا (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } وإلا إذا (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  إرجاع std::string(Path);
}

ساكن llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  إذا (Pos == StringRef::npos)
    إرجاع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    إرجاع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  إذا (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  إذا (!getSupportedSchemes().contains(UriScheme)) {
    إرجاع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  إرجاع getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  إذا (!FilePath)
    إرجاع FilePath.takeError();
  إرجاع URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    إرجاع Uri.takeError();
  إرجاع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { إرجاع uri().split(':').first; }

فراغ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().أدرج(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, URIForFile &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قيمة.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      إرجاع خطأ;
    }
    Result = std::move(*ExpectedUri);
    إرجاع true;
  }
  إرجاع خطأ;
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت URIForFile &قيمة) {
  إرجاع قيمة.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &قيمة) {
  إرجاع Os << قيمة.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = قيمة.getAsObject();
  إذا (!O) {
    Path.report("expected object");
    إرجاع خطأ;
  }
  إذا (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_رمز =
            TextDocument->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> HierarchicalSupport =
              Document_رمز->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_رمز = *HierarchicalSupport;
    }
    إذا (auto *CodeAction = TextDocument->getObject("codeAction")) {
      إذا (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  إذا (auto *Window = O->getObject("window")) {
    إذا (std::optional<منطقي> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  إرجاع true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إذا (!O || !O.map("name", Result.name))
    إرجاع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  إرجاع true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, TraceLevel &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قيمة.getAsString()) {
    إذا (*Str == "off") {
      Result = TraceLevel::Off;
      إرجاع true;
    }
    إذا (*Str == "messages") {
      Result = TraceLevel::Messages;
      إرجاع true;
    }
    إذا (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      إرجاع true;
    }
  }
  إرجاع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إذا (!O)
    إرجاع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.جذر_Uri);
  O.map("rootPath", Result.جذر_Path);
  mapOptOrNull(قيمة, "clientInfo", Result.clientInfo, Path);

  إرجاع true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت TextDocumentIdentifier &قيمة) {
  إرجاع llvm::json::Object{{"uri", قيمة.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قيمة
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &قيمة) {
  إرجاع llvm::json::Object{
      {"uri", قيمة.uri},
      {"version", قيمة.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("line", Result.سطر_2) &&
         O.map("character", Result.character);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت Position &قيمة) {
  إرجاع llvm::json::Object{
      {"line", قيمة.سطر_2},
      {"character", قيمة.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &قيمة) {
  إرجاع Os << قيمة.سطر_2 << ':' << قيمة.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت Range &قيمة) {
  إرجاع llvm::json::Object{
      {"start", قيمة.start},
      {"end", قيمة.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &قيمة) {
  إرجاع Os << قيمة.start << '-' << قيمة.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت Location &قيمة) {
  إرجاع llvm::json::Object{
      {"uri", قيمة.uri},
      {"range", قيمة.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &قيمة) {
  إرجاع Os << قيمة.range << '@' << قيمة.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع fromJSON(قيمة, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  إرجاع O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = text;
    إرجاع success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    إرجاع failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  إرجاع success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  لكل (ثابت auto &Change : Changes)
    إذا (failed(Change.applyTo(Contents)))
      إرجاع failure();
  إرجاع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef toTextKind(MarkupKind Kind) {
  بدل (Kind) {
  حالة MarkupKind::PlainText:
    إرجاع "plaintext";
  حالة MarkupKind::Markdown:
    إرجاع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  إرجاع Os << toTextKind(Kind);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  إذا (Mc.قيمة_2.empty())
    إرجاع nullptr;

  إرجاع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.قيمة_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  إذا (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  إرجاع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت Document_رمز &رمز) {
  llvm::json::Object Result{{"name", رمز.name},
                            {"kind", static_cast<صحيح>(رمز.kind)},
                            {"range", رمز.range},
                            {"selectionRange", رمز.selectionRange}};

  إذا (!رمز.detail.empty())
    Result["detail"] = رمز.detail;
  إذا (!رمز.children.empty())
    Result["children"] = رمز.children;
  إرجاع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         Document_رمز_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  إرجاع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(DiagnosticTag Tag) {
  إرجاع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<int64_t> I = قيمة.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    إرجاع true;
  }

  إرجاع خطأ;
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (صحيح)Diag.severity},
      {"message", Diag.message},
  };
  إذا (Diag.category)
    Result["category"] = *Diag.category;
  إذا (!Diag.source.empty())
    Result["source"] = Diag.source;
  إذا (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  إذا (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  إرجاع std::move(Result);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إذا (!O)
    إرجاع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(قيمة, "severity", Severity, Path))
    إرجاع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  إرجاع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(قيمة, "category", Result.category, Path) &&
         mapOptOrNull(قيمة, "source", Result.source, Path) &&
         mapOptOrNull(قيمة, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(قيمة, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  إرجاع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت TextEdit &قيمة) {
  إرجاع llvm::json::Object{
      {"range", قيمة.range},
      {"newText", قيمة.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &قيمة) {
  Os << قيمة.range << " => \"";
  llvm::printEscapedString(قيمة.newText, Os);
  إرجاع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  إذا (std::optional<int64_t> Int_قيمة = قيمة.getAsInteger()) {
    إذا (*Int_قيمة < static_cast<صحيح>(CompletionItemKind::Text) ||
        *Int_قيمة > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      إرجاع خطأ;
    Result = static_cast<CompletionItemKind>(*Int_قيمة);
    إرجاع true;
  }
  إرجاع خطأ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  إذا (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    إرجاع Kind;

  // Provide some fall backs for common kinds that are close enough.
  بدل (Kind) {
  حالة CompletionItemKind::Folder:
    إرجاع CompletionItemKind::File;
  حالة CompletionItemKind::EnumMember:
    إرجاع CompletionItemKind::Enum;
  حالة CompletionItemKind::Struct:
    إرجاع CompletionItemKind::Class;
  افتراضي:
    إرجاع CompletionItemKind::Text;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  إذا (ثابت llvm::json::Array *Array_قيمة = قيمة.getAsArray()) {
    لكل (size_t I = 0, E = Array_قيمة->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      إذا (fromJSON((*Array_قيمة)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    إرجاع true;
  }
  إرجاع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت CompletionItem &قيمة) {
  أكد(!قيمة.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", قيمة.label}};
  إذا (قيمة.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(قيمة.kind);
  إذا (!قيمة.detail.empty())
    Result["detail"] = قيمة.detail;
  إذا (قيمة.documentation)
    Result["documentation"] = قيمة.documentation;
  إذا (!قيمة.sortText.empty())
    Result["sortText"] = قيمة.sortText;
  إذا (!قيمة.filterText.empty())
    Result["filterText"] = قيمة.filterText;
  إذا (!قيمة.أدرج_Text.empty())
    Result["insertText"] = قيمة.أدرج_Text;
  إذا (قيمة.أدرج_Text_Format != أدرج_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(قيمة.أدرج_Text_Format);
  إذا (قيمة.textEdit)
    Result["textEdit"] = *قيمة.textEdit;
  إذا (!قيمة.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(قيمة.additionalTextEdits);
  }
  إذا (قيمة.deprecated)
    Result["deprecated"] = قيمة.deprecated;
  إرجاع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &قيمة) {
  إرجاع Os << قيمة.label << " - " << toJSON(قيمة);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  إرجاع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت CompletionList &قيمة) {
  إرجاع llvm::json::Object{
      {"isIncomplete", قيمة.isIncomplete},
      {"items", llvm::json::Array(قيمة.عناصر)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  صحيح TriggerKind;
  إذا (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(قيمة, "triggerCharacter", Result.triggerCharacter, Path))
    إرجاع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  إرجاع true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CompletionParams &Result, llvm::json::Path Path) {
  إذا (!fromJSON(قيمة, static_cast<TextDocumentPositionParams &>(Result), Path))
    إرجاع خطأ;
  إذا (ثابت llvm::json::قيمة *Context = قيمة.getAsObject()->get("context"))
    إرجاع fromJSON(*Context, Result.context, Path.field("context"));
  إرجاع true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت ParameterInformation &قيمة) {
  أكد((قيمة.labelOffsets || !قيمة.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  إذا (قيمة.labelOffsets)
    Result["label"] = llvm::json::Array(
        {قيمة.labelOffsets->first, قيمة.labelOffsets->second});
  وإلا
    Result["label"] = قيمة.labelString;
  إذا (!قيمة.documentation.empty())
    Result["documentation"] = قيمة.documentation;
  إرجاع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت SignatureInformation &قيمة) {
  أكد(!قيمة.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", قيمة.label},
      {"parameters", llvm::json::Array(قيمة.parameters)},
  };
  إذا (!قيمة.documentation.empty())
    Result["documentation"] = قيمة.documentation;
  إرجاع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قيمة) {
  إرجاع Os << قيمة.label << " - " << toJSON(قيمة);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت SignatureHelp &قيمة) {
  أكد(قيمة.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قيمة.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  إرجاع llvm::json::Object{
      {"activeSignature", قيمة.activeSignature},
      {"activeParameter", قيمة.activeParameter},
      {"signatures", llvm::json::Array(قيمة.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت DocumentLink &قيمة) {
  إرجاع llvm::json::Object{
      {"range", قيمة.range},
      {"target", قيمة.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت InlayHint &قيمة) {
  إرجاع llvm::json::Object{{"position", قيمة.position},
                            {"kind", (صحيح)قيمة.kind},
                            {"label", قيمة.label},
                            {"paddingLeft", قيمة.padding_يسار},
                            {"paddingRight", قيمة.padding_يمين}};
}
منطقي llvm::lsp::operator==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  إرجاع std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
منطقي llvm::lsp::operator<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  إرجاع std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind قيمة) {
  بدل (قيمة) {
  حالة InlayHintKind::Parameter:
    إرجاع Os << "parameter";
  حالة InlayHintKind::Type:
    إرجاع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إذا (!O || !O.map("diagnostics", Result.diagnostics))
    إرجاع خطأ;
  O.map("only", Result.only);
  إرجاع true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قيمة &قيمة, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قيمة, Path);
  إرجاع O && O.map("changes", Result.changes);
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت WorkspaceEdit &قيمة) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : قيمة.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  إرجاع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::قيمة llvm::lsp::toJSON(ثابت CodeAction &قيمة) {
  llvm::json::Object CodeAction{{"title", قيمة.title}};
  إذا (قيمة.kind)
    CodeAction["kind"] = *قيمة.kind;
  إذا (قيمة.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*قيمة.diagnostics);
  إذا (قيمة.isPreferred)
    CodeAction["isPreferred"] = true;
  إذا (قيمة.edit)
    CodeAction["edit"] = *قيمة.edit;
  إرجاع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قيمة llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  إذا (Params.actions)
    Out["actions"] = *Params.actions;
  إرجاع Out;
}

llvm::json::قيمة llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  إرجاع llvm::json::Object{{"title", Params.title}};
}
