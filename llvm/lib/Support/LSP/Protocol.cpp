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
ساكن منطقي mapOptOrNull(ثابت llvm::json::قدر &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ثابت llvm::json::Object *O = Params.getAsObject();
  أكد(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  إذا (!V || V->getAsNull())
    أرجع true;
  أرجع fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

حرف LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ساكن منطقي isWindowsPath(StringRef Path) {
  أرجع Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ساكن منطقي isNetworkPath(StringRef Path) {
  أرجع Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ساكن منطقي shouldEscapeInURI(unsigned حرف C) {
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
  أرجع true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ساكن فراغ percentEncode(StringRef Content, std::string &Out) {
  لكل (unsigned حرف C : Content) {
    إذا (shouldEscapeInURI(C)) {
      Out.ڈالو_back('%');
      Out.ڈالو_back(llvm::hexdigit(C / 16));
      Out.ڈالو_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.ڈالو_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ڈالو_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.ڈالو_back(*I);
    }
  }
  أرجع Result;
}

/// Return the set containing the supported URI schemes.
ساكن StringSet<> &getSupportedSchemes() {
  ساكن StringSet<> Schemes({"file", "test"});
  أرجع Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ساكن منطقي isStructurallyValidScheme(StringRef Scheme) {
  إذا (Scheme.empty())
    أرجع خطأ;
  إذا (!llvm::isAlpha(Scheme[0]))
    أرجع خطأ;
  أرجع llvm::all_of(llvm::drop_begin(Scheme), [](حرف C) {
    أرجع llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ساكن llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef جڑ_2 = llvm::sys::path::جڑ_name(AbsolutePath);
  إذا (isNetworkPath(جڑ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = جڑ_2.drop_front(2);
    AbsolutePath.consume_front(جڑ_2);
  } وإلا إذا (isWindowsPath(جڑ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  إذا (Authority.empty() && Body.empty())
    أرجع Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  إذا (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  أرجع Uri;
}

ساكن llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  إذا (!Body.starts_with("/"))
    أرجع llvm::createStringError(
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
  أرجع std::string(Path);
}

ساكن llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  إذا (Pos == StringRef::npos)
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  إذا (!isStructurallyValidScheme(UriScheme))
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
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
    أرجع llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  أرجع getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  إذا (!FilePath)
    أرجع FilePath.takeError();
  أرجع URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  إذا (!Uri)
    أرجع Uri.takeError();
  أرجع fromURI(*Uri);
}

StringRef URIForFile::scheme() ثابت { أرجع uri().split(':').first; }

فراغ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().داخل_کرو(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, URIForFile &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    إذا (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      أرجع خطأ;
    }
    Result = std::move(*ExpectedUri);
    أرجع true;
  }
  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::toJSON(ثابت URIForFile &قدر) {
  أرجع قدر.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &قدر) {
  أرجع Os << قدر.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = قدر.getAsObject();
  إذا (!O) {
    Path.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_علامت =
            TextDocument->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> HierarchicalSupport =
              Document_علامت->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_علامت = *HierarchicalSupport;
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
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  إذا (!O || !O.map("name", Result.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, TraceLevel &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قدر.getAsString()) {
    إذا (*Str == "off") {
      Result = TraceLevel::Off;
      أرجع true;
    }
    إذا (*Str == "messages") {
      Result = TraceLevel::Messages;
      أرجع true;
    }
    إذا (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      أرجع true;
    }
  }
  أرجع خطأ;
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.جڑ_Uri);
  O.map("rootPath", Result.جڑ_Path);
  mapOptOrNull(قدر, "clientInfo", Result.clientInfo, Path);

  أرجع true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت TextDocumentIdentifier &قدر) {
  أرجع llvm::json::Object{{"uri", قدر.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قدر
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"version", قدر.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("line", Result.سطر_2) &&
         O.map("character", Result.character);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت Position &قدر) {
  أرجع llvm::json::Object{
      {"line", قدر.سطر_2},
      {"character", قدر.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &قدر) {
  أرجع Os << قدر.سطر_2 << ':' << قدر.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت Range &قدر) {
  أرجع llvm::json::Object{
      {"start", قدر.start},
      {"end", قدر.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &قدر) {
  أرجع Os << قدر.start << '-' << قدر.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت Location &قدر) {
  أرجع llvm::json::Object{
      {"uri", قدر.uri},
      {"range", قدر.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &قدر) {
  أرجع Os << قدر.range << '@' << قدر.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع fromJSON(قدر, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  أرجع O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ثابت {
  // If there is no range, the full document changed.
  إذا (!range) {
    Contents = text;
    أرجع success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  إذا (!RangeLoc.isValid())
    أرجع failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  أرجع success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  لكل (ثابت auto &Change : Changes)
    إذا (failed(Change.applyTo(Contents)))
      أرجع failure();
  أرجع success();
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ساكن llvm::StringRef toTextKind(MarkupKind Kind) {
  بدل (Kind) {
  حالة MarkupKind::PlainText:
    أرجع "plaintext";
  حالة MarkupKind::Markdown:
    أرجع "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  أرجع Os << toTextKind(Kind);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  إذا (Mc.قدر_2.empty())
    أرجع nullptr;

  أرجع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.قدر_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  إذا (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت Document_علامت &علامت) {
  llvm::json::Object Result{{"name", علامت.name},
                            {"kind", static_cast<صحيح>(علامت.kind)},
                            {"range", علامت.range},
                            {"selectionRange", علامت.selectionRange}};

  إذا (!علامت.detail.empty())
    Result["detail"] = علامت.detail;
  إذا (!علامت.children.empty())
    Result["children"] = علامت.children;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         Document_علامت_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(DiagnosticTag Tag) {
  أرجع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<int64_t> I = قدر.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    أرجع true;
  }

  أرجع خطأ;
}

llvm::json::قدر llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
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
  أرجع std::move(Result);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(قدر, "severity", Severity, Path))
    أرجع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(قدر, "category", Result.category, Path) &&
         mapOptOrNull(قدر, "source", Result.source, Path) &&
         mapOptOrNull(قدر, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(قدر, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت TextEdit &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"newText", قدر.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &قدر) {
  Os << قدر.range << " => \"";
  llvm::printEscapedString(قدر.newText, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  إذا (std::optional<int64_t> Int_قدر = قدر.getAsInteger()) {
    إذا (*Int_قدر < static_cast<صحيح>(CompletionItemKind::Text) ||
        *Int_قدر > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      أرجع خطأ;
    Result = static_cast<CompletionItemKind>(*Int_قدر);
    أرجع true;
  }
  أرجع خطأ;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  إذا (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    أرجع Kind;

  // Provide some fall backs for common kinds that are close enough.
  بدل (Kind) {
  حالة CompletionItemKind::Folder:
    أرجع CompletionItemKind::File;
  حالة CompletionItemKind::EnumMember:
    أرجع CompletionItemKind::Enum;
  حالة CompletionItemKind::Struct:
    أرجع CompletionItemKind::Class;
  افتراضي:
    أرجع CompletionItemKind::Text;
  }
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  إذا (ثابت llvm::json::Array *Array_قدر = قدر.getAsArray()) {
    لكل (size_t I = 0, E = Array_قدر->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      إذا (fromJSON((*Array_قدر)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    أرجع true;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت CompletionItem &قدر) {
  أكد(!قدر.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", قدر.label}};
  إذا (قدر.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(قدر.kind);
  إذا (!قدر.detail.empty())
    Result["detail"] = قدر.detail;
  إذا (قدر.documentation)
    Result["documentation"] = قدر.documentation;
  إذا (!قدر.sortText.empty())
    Result["sortText"] = قدر.sortText;
  إذا (!قدر.filterText.empty())
    Result["filterText"] = قدر.filterText;
  إذا (!قدر.داخل_کرو_Text.empty())
    Result["insertText"] = قدر.داخل_کرو_Text;
  إذا (قدر.داخل_کرو_Text_Format != داخل_کرو_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(قدر.داخل_کرو_Text_Format);
  إذا (قدر.textEdit)
    Result["textEdit"] = *قدر.textEdit;
  إذا (!قدر.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(قدر.additionalTextEdits);
  }
  إذا (قدر.deprecated)
    Result["deprecated"] = قدر.deprecated;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &قدر) {
  أرجع Os << قدر.label << " - " << toJSON(قدر);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  أرجع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت CompletionList &قدر) {
  أرجع llvm::json::Object{
      {"isIncomplete", قدر.isIncomplete},
      {"items", llvm::json::Array(قدر.اشیاء)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  صحيح TriggerKind;
  إذا (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(قدر, "triggerCharacter", Result.triggerCharacter, Path))
    أرجع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CompletionParams &Result, llvm::json::Path Path) {
  إذا (!fromJSON(قدر, static_cast<TextDocumentPositionParams &>(Result), Path))
    أرجع خطأ;
  إذا (ثابت llvm::json::قدر *Context = قدر.getAsObject()->get("context"))
    أرجع fromJSON(*Context, Result.context, Path.field("context"));
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت ParameterInformation &قدر) {
  أكد((قدر.labelOffsets || !قدر.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  إذا (قدر.labelOffsets)
    Result["label"] = llvm::json::Array(
        {قدر.labelOffsets->first, قدر.labelOffsets->second});
  وإلا
    Result["label"] = قدر.labelString;
  إذا (!قدر.documentation.empty())
    Result["documentation"] = قدر.documentation;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت SignatureInformation &قدر) {
  أكد(!قدر.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", قدر.label},
      {"parameters", llvm::json::Array(قدر.parameters)},
  };
  إذا (!قدر.documentation.empty())
    Result["documentation"] = قدر.documentation;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قدر) {
  أرجع Os << قدر.label << " - " << toJSON(قدر);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت SignatureHelp &قدر) {
  أكد(قدر.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قدر.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", قدر.activeSignature},
      {"activeParameter", قدر.activeParameter},
      {"signatures", llvm::json::Array(قدر.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت DocumentLink &قدر) {
  أرجع llvm::json::Object{
      {"range", قدر.range},
      {"target", قدر.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت InlayHint &قدر) {
  أرجع llvm::json::Object{{"position", قدر.position},
                            {"kind", (صحيح)قدر.kind},
                            {"label", قدر.label},
                            {"paddingLeft", قدر.padding_بایاں},
                            {"paddingRight", قدر.padding_دایاں}};
}
منطقي llvm::lsp::operator==(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
منطقي llvm::lsp::operator<(ثابت InlayHint &Lhs, ثابت InlayHint &Rhs) {
  أرجع std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind قدر) {
  بدل (قدر) {
  حالة InlayHintKind::Parameter:
    أرجع Os << "parameter";
  حالة InlayHintKind::Type:
    أرجع Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  إذا (!O || !O.map("diagnostics", Result.diagnostics))
    أرجع خطأ;
  O.map("only", Result.only);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قدر &قدر, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قدر, Path);
  أرجع O && O.map("changes", Result.changes);
}

llvm::json::قدر llvm::lsp::toJSON(ثابت WorkspaceEdit &قدر) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : قدر.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::قدر llvm::lsp::toJSON(ثابت CodeAction &قدر) {
  llvm::json::Object CodeAction{{"title", قدر.title}};
  إذا (قدر.kind)
    CodeAction["kind"] = *قدر.kind;
  إذا (قدر.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*قدر.diagnostics);
  إذا (قدر.isPreferred)
    CodeAction["isPreferred"] = true;
  إذا (قدر.edit)
    CodeAction["edit"] = *قدر.edit;
  أرجع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قدر llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  إذا (Params.actions)
    Out["actions"] = *Params.actions;
  أرجع Out;
}

llvm::json::قدر llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
