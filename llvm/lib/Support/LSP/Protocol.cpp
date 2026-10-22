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
ساكن منطقي mapOptOrNull(ثابت llvm::json::نيلاي &Params,
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
      Out.تولق_back('%');
      Out.تولق_back(llvm::hexdigit(C / 16));
      Out.تولق_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.تولق_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.تولق_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.تولق_back(*I);
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
  StringRef اکر_2 = llvm::sys::path::اکر_name(AbsolutePath);
  إذا (isNetworkPath(اکر_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = اکر_2.drop_front(2);
    AbsolutePath.consume_front(اکر_2);
  } وإلا إذا (isWindowsPath(اکر_2)) {
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
  getSupportedSchemes().سيسيڤ(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, URIForFile &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = نيلاي.getAsString()) {
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

llvm::json::نيلاي llvm::lsp::toJSON(ثابت URIForFile &نيلاي) {
  أرجع نيلاي.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &نيلاي) {
  أرجع Os << نيلاي.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = نيلاي.getAsObject();
  إذا (!O) {
    Path.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_سيمبول =
            TextDocument->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> HierarchicalSupport =
              Document_سيمبول->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_سيمبول = *HierarchicalSupport;
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  إذا (!O || !O.map("name", Result.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, TraceLevel &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = نيلاي.getAsString()) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.اکر_Uri);
  O.map("rootPath", Result.اکر_Path);
  mapOptOrNull(نيلاي, "clientInfo", Result.clientInfo, Path);

  أرجع true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت TextDocumentIdentifier &نيلاي) {
  أرجع llvm::json::Object{{"uri", نيلاي.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::نيلاي
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &نيلاي) {
  أرجع llvm::json::Object{
      {"uri", نيلاي.uri},
      {"version", نيلاي.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("line", Result.باريس_2) &&
         O.map("character", Result.character);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Position &نيلاي) {
  أرجع llvm::json::Object{
      {"line", نيلاي.باريس_2},
      {"character", نيلاي.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &نيلاي) {
  أرجع Os << نيلاي.باريس_2 << ':' << نيلاي.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Range &نيلاي) {
  أرجع llvm::json::Object{
      {"start", نيلاي.start},
      {"end", نيلاي.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &نيلاي) {
  أرجع Os << نيلاي.start << '-' << نيلاي.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Location &نيلاي) {
  أرجع llvm::json::Object{
      {"uri", نيلاي.uri},
      {"range", نيلاي.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &نيلاي) {
  أرجع Os << نيلاي.range << '@' << نيلاي.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع fromJSON(نيلاي, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &Params,
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
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

llvm::json::نيلاي llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  إذا (Mc.نيلاي_2.empty())
    أرجع nullptr;

  أرجع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.نيلاي_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  إذا (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Document_سيمبول &سيمبول) {
  llvm::json::Object Result{{"name", سيمبول.name},
                            {"kind", static_cast<صحيح>(سيمبول.kind)},
                            {"range", سيمبول.range},
                            {"selectionRange", سيمبول.selectionRange}};

  إذا (!سيمبول.detail.empty())
    Result["detail"] = سيمبول.detail;
  إذا (!سيمبول.children.empty())
    Result["children"] = سيمبول.children;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         Document_سيمبول_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(DiagnosticTag Tag) {
  أرجع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<int64_t> I = نيلاي.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    أرجع true;
  }

  أرجع خطأ;
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(نيلاي, "severity", Severity, Path))
    أرجع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(نيلاي, "category", Result.category, Path) &&
         mapOptOrNull(نيلاي, "source", Result.source, Path) &&
         mapOptOrNull(نيلاي, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(نيلاي, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت TextEdit &نيلاي) {
  أرجع llvm::json::Object{
      {"range", نيلاي.range},
      {"newText", نيلاي.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &نيلاي) {
  Os << نيلاي.range << " => \"";
  llvm::printEscapedString(نيلاي.newText, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  إذا (std::optional<int64_t> Int_نيلاي = نيلاي.getAsInteger()) {
    إذا (*Int_نيلاي < static_cast<صحيح>(CompletionItemKind::Text) ||
        *Int_نيلاي > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      أرجع خطأ;
    Result = static_cast<CompletionItemKind>(*Int_نيلاي);
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  إذا (ثابت llvm::json::Array *Array_نيلاي = نيلاي.getAsArray()) {
    لكل (size_t I = 0, E = Array_نيلاي->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      إذا (fromJSON((*Array_نيلاي)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    أرجع true;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت CompletionItem &نيلاي) {
  أكد(!نيلاي.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", نيلاي.label}};
  إذا (نيلاي.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(نيلاي.kind);
  إذا (!نيلاي.detail.empty())
    Result["detail"] = نيلاي.detail;
  إذا (نيلاي.documentation)
    Result["documentation"] = نيلاي.documentation;
  إذا (!نيلاي.sortText.empty())
    Result["sortText"] = نيلاي.sortText;
  إذا (!نيلاي.filterText.empty())
    Result["filterText"] = نيلاي.filterText;
  إذا (!نيلاي.سيسيڤ_Text.empty())
    Result["insertText"] = نيلاي.سيسيڤ_Text;
  إذا (نيلاي.سيسيڤ_Text_Format != سيسيڤ_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(نيلاي.سيسيڤ_Text_Format);
  إذا (نيلاي.textEdit)
    Result["textEdit"] = *نيلاي.textEdit;
  إذا (!نيلاي.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(نيلاي.additionalTextEdits);
  }
  إذا (نيلاي.deprecated)
    Result["deprecated"] = نيلاي.deprecated;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &نيلاي) {
  أرجع Os << نيلاي.label << " - " << toJSON(نيلاي);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  أرجع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت CompletionList &نيلاي) {
  أرجع llvm::json::Object{
      {"isIncomplete", نيلاي.isIncomplete},
      {"items", llvm::json::Array(نيلاي.بارڠن)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  صحيح TriggerKind;
  إذا (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(نيلاي, "triggerCharacter", Result.triggerCharacter, Path))
    أرجع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CompletionParams &Result, llvm::json::Path Path) {
  إذا (!fromJSON(نيلاي, static_cast<TextDocumentPositionParams &>(Result), Path))
    أرجع خطأ;
  إذا (ثابت llvm::json::نيلاي *Context = نيلاي.getAsObject()->get("context"))
    أرجع fromJSON(*Context, Result.context, Path.field("context"));
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت ParameterInformation &نيلاي) {
  أكد((نيلاي.labelOffsets || !نيلاي.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  إذا (نيلاي.labelOffsets)
    Result["label"] = llvm::json::Array(
        {نيلاي.labelOffsets->first, نيلاي.labelOffsets->second});
  وإلا
    Result["label"] = نيلاي.labelString;
  إذا (!نيلاي.documentation.empty())
    Result["documentation"] = نيلاي.documentation;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت SignatureInformation &نيلاي) {
  أكد(!نيلاي.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", نيلاي.label},
      {"parameters", llvm::json::Array(نيلاي.parameters)},
  };
  إذا (!نيلاي.documentation.empty())
    Result["documentation"] = نيلاي.documentation;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &نيلاي) {
  أرجع Os << نيلاي.label << " - " << toJSON(نيلاي);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت SignatureHelp &نيلاي) {
  أكد(نيلاي.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(نيلاي.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", نيلاي.activeSignature},
      {"activeParameter", نيلاي.activeParameter},
      {"signatures", llvm::json::Array(نيلاي.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت DocumentLink &نيلاي) {
  أرجع llvm::json::Object{
      {"range", نيلاي.range},
      {"target", نيلاي.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت InlayHint &نيلاي) {
  أرجع llvm::json::Object{{"position", نيلاي.position},
                            {"kind", (صحيح)نيلاي.kind},
                            {"label", نيلاي.label},
                            {"paddingLeft", نيلاي.padding_کيري},
                            {"paddingRight", نيلاي.padding_کانن}};
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
                                         InlayHintKind نيلاي) {
  بدل (نيلاي) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  إذا (!O || !O.map("diagnostics", Result.diagnostics))
    أرجع خطأ;
  O.map("only", Result.only);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::نيلاي &نيلاي, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(نيلاي, Path);
  أرجع O && O.map("changes", Result.changes);
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت WorkspaceEdit &نيلاي) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : نيلاي.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::نيلاي llvm::lsp::toJSON(ثابت CodeAction &نيلاي) {
  llvm::json::Object CodeAction{{"title", نيلاي.title}};
  إذا (نيلاي.kind)
    CodeAction["kind"] = *نيلاي.kind;
  إذا (نيلاي.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*نيلاي.diagnostics);
  إذا (نيلاي.isPreferred)
    CodeAction["isPreferred"] = true;
  إذا (نيلاي.edit)
    CodeAction["edit"] = *نيلاي.edit;
  أرجع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::نيلاي llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  إذا (Params.actions)
    Out["actions"] = *Params.actions;
  أرجع Out;
}

llvm::json::نيلاي llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
