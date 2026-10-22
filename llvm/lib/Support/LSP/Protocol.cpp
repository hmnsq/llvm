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
ساكن منطقي mapOptOrNull(ثابت llvm::json::قىممەت &Params,
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
      Out.ئىتتىرىش_back('%');
      Out.ئىتتىرىش_back(llvm::hexdigit(C / 16));
      Out.ئىتتىرىش_back(llvm::hexdigit(C % 16));
    } وإلا {
      Out.ئىتتىرىش_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ساكن std::string percentDecode(StringRef Content) {
  std::string Result;
  لكل (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    إذا (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ئىتتىرىش_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } وإلا {
      Result.ئىتتىرىش_back(*I);
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
  StringRef يىلتىز_2 = llvm::sys::path::يىلتىز_name(AbsolutePath);
  إذا (isNetworkPath(يىلتىز_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = يىلتىز_2.drop_front(2);
    AbsolutePath.consume_front(يىلتىز_2);
  } وإلا إذا (isWindowsPath(يىلتىز_2)) {
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
  getSupportedSchemes().قىستۇرۇش(Scheme);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, URIForFile &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قىممەت.getAsString()) {
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

llvm::json::قىممەت llvm::lsp::toJSON(ثابت URIForFile &قىممەت) {
  أرجع قىممەت.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت URIForFile &قىممەت) {
  أرجع Os << قىممەت.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ثابت llvm::json::Object *O = قىممەت.getAsObject();
  إذا (!O) {
    Path.report("expected object");
    أرجع خطأ;
  }
  إذا (ثابت llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    إذا (ثابت llvm::json::Object *Document_بەلگە =
            TextDocument->getObject("documentSymbol")) {
      إذا (std::optional<منطقي> HierarchicalSupport =
              Document_بەلگە->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_بەلگە = *HierarchicalSupport;
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  إذا (!O || !O.map("name", Result.name))
    أرجع خطأ;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, TraceLevel &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<StringRef> Str = قىممەت.getAsString()) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  إذا (!O)
    أرجع خطأ;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.يىلتىز_Uri);
  O.map("rootPath", Result.يىلتىز_Path);
  mapOptOrNull(قىممەت, "clientInfo", Result.clientInfo, Path);

  أرجع true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت TextDocumentIdentifier &قىممەت) {
  أرجع llvm::json::Object{{"uri", قىممەت.uri}};
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::قىممەت
llvm::lsp::toJSON(ثابت VersionedTextDocumentIdentifier &قىممەت) {
  أرجع llvm::json::Object{
      {"uri", قىممەت.uri},
      {"version", قىممەت.version},
  };
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("line", Result.قۇر_2) &&
         O.map("character", Result.character);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Position &قىممەت) {
  أرجع llvm::json::Object{
      {"line", قىممەت.قۇر_2},
      {"character", قىممەت.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Position &قىممەت) {
  أرجع Os << قىممەت.قۇر_2 << ':' << قىممەت.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Range &قىممەت) {
  أرجع llvm::json::Object{
      {"start", قىممەت.start},
      {"end", قىممەت.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Range &قىممەت) {
  أرجع Os << قىممەت.start << '-' << قىممەت.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Location &قىممەت) {
  أرجع llvm::json::Object{
      {"uri", قىممەت.uri},
      {"range", قىممەت.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت Location &قىممەت) {
  أرجع Os << قىممەت.range << '@' << قىممەت.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع fromJSON(قىممەت, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &Params,
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
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

llvm::json::قىممەت llvm::lsp::toJSON(ثابت MarkupContent &Mc) {
  إذا (Mc.قىممەت_2.empty())
    أرجع nullptr;

  أرجع llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.قىممەت_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  إذا (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Document_بەلگە &بەلگە) {
  llvm::json::Object Result{{"name", بەلگە.name},
                            {"kind", static_cast<صحيح>(بەلگە.kind)},
                            {"range", بەلگە.range},
                            {"selectionRange", بەلگە.selectionRange}};

  إذا (!بەلگە.detail.empty())
    Result["detail"] = بەلگە.detail;
  إذا (!بەلگە.children.empty())
    Result["children"] = بەلگە.children;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         Document_بەلگە_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت DiagnosticRelatedInformation &Info) {
  أرجع llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(DiagnosticTag Tag) {
  أرجع static_cast<صحيح>(Tag);
}

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  إذا (std::optional<int64_t> I = قىممەت.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    أرجع true;
  }

  أرجع خطأ;
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت Diagnostic &Diag) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  إذا (!O)
    أرجع خطأ;
  صحيح Severity = 0;
  إذا (!mapOptOrNull(قىممەت, "severity", Severity, Path))
    أرجع خطأ;
  Result.severity = (DiagnosticSeverity)Severity;

  أرجع O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(قىممەت, "category", Result.category, Path) &&
         mapOptOrNull(قىممەت, "source", Result.source, Path) &&
         mapOptOrNull(قىممەت, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(قىممەت, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت PublishDiagnosticsParams &Params) {
  أرجع llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت TextEdit &قىممەت) {
  أرجع llvm::json::Object{
      {"range", قىممەت.range},
      {"newText", قىممەت.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ثابت TextEdit &قىممەت) {
  Os << قىممەت.range << " => \"";
  llvm::printEscapedString(قىممەت.newText, Os);
  أرجع Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  إذا (std::optional<int64_t> Int_قىممەت = قىممەت.getAsInteger()) {
    إذا (*Int_قىممەت < static_cast<صحيح>(CompletionItemKind::Text) ||
        *Int_قىممەت > static_cast<صحيح>(CompletionItemKind::TypeParameter))
      أرجع خطأ;
    Result = static_cast<CompletionItemKind>(*Int_قىممەت);
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  إذا (ثابت llvm::json::Array *Array_قىممەت = قىممەت.getAsArray()) {
    لكل (size_t I = 0, E = Array_قىممەت->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      إذا (fromJSON((*Array_قىممەت)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    أرجع true;
  }
  أرجع خطأ;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت CompletionItem &قىممەت) {
  أكد(!قىممەت.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", قىممەت.label}};
  إذا (قىممەت.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<صحيح>(قىممەت.kind);
  إذا (!قىممەت.detail.empty())
    Result["detail"] = قىممەت.detail;
  إذا (قىممەت.documentation)
    Result["documentation"] = قىممەت.documentation;
  إذا (!قىممەت.sortText.empty())
    Result["sortText"] = قىممەت.sortText;
  إذا (!قىممەت.filterText.empty())
    Result["filterText"] = قىممەت.filterText;
  إذا (!قىممەت.قىستۇرۇش_Text.empty())
    Result["insertText"] = قىممەت.قىستۇرۇش_Text;
  إذا (قىممەت.قىستۇرۇش_Text_Format != قىستۇرۇش_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<صحيح>(قىممەت.قىستۇرۇش_Text_Format);
  إذا (قىممەت.textEdit)
    Result["textEdit"] = *قىممەت.textEdit;
  إذا (!قىممەت.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(قىممەت.additionalTextEdits);
  }
  إذا (قىممەت.deprecated)
    Result["deprecated"] = قىممەت.deprecated;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت CompletionItem &قىممەت) {
  أرجع Os << قىممەت.label << " - " << toJSON(قىممەت);
}

منطقي llvm::lsp::operator<(ثابت CompletionItem &Lhs,
                          ثابت CompletionItem &Rhs) {
  أرجع (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت CompletionList &قىممەت) {
  أرجع llvm::json::Object{
      {"isIncomplete", قىممەت.isIncomplete},
      {"items", llvm::json::Array(قىممەت.بۇيۇملار)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  صحيح TriggerKind;
  إذا (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(قىممەت, "triggerCharacter", Result.triggerCharacter, Path))
    أرجع خطأ;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CompletionParams &Result, llvm::json::Path Path) {
  إذا (!fromJSON(قىممەت, static_cast<TextDocumentPositionParams &>(Result), Path))
    أرجع خطأ;
  إذا (ثابت llvm::json::قىممەت *Context = قىممەت.getAsObject()->get("context"))
    أرجع fromJSON(*Context, Result.context, Path.field("context"));
  أرجع true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت ParameterInformation &قىممەت) {
  أكد((قىممەت.labelOffsets || !قىممەت.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  إذا (قىممەت.labelOffsets)
    Result["label"] = llvm::json::Array(
        {قىممەت.labelOffsets->first, قىممەت.labelOffsets->second});
  وإلا
    Result["label"] = قىممەت.labelString;
  إذا (!قىممەت.documentation.empty())
    Result["documentation"] = قىممەت.documentation;
  أرجع std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت SignatureInformation &قىممەت) {
  أكد(!قىممەت.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", قىممەت.label},
      {"parameters", llvm::json::Array(قىممەت.parameters)},
  };
  إذا (!قىممەت.documentation.empty())
    Result["documentation"] = قىممەت.documentation;
  أرجع std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ثابت SignatureInformation &قىممەت) {
  أرجع Os << قىممەت.label << " - " << toJSON(قىممەت);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت SignatureHelp &قىممەت) {
  أكد(قىممەت.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  أكد(قىممەت.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  أرجع llvm::json::Object{
      {"activeSignature", قىممەت.activeSignature},
      {"activeParameter", قىممەت.activeParameter},
      {"signatures", llvm::json::Array(قىممەت.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت DocumentLink &قىممەت) {
  أرجع llvm::json::Object{
      {"range", قىممەت.range},
      {"target", قىممەت.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت InlayHint &قىممەت) {
  أرجع llvm::json::Object{{"position", قىممەت.position},
                            {"kind", (صحيح)قىممەت.kind},
                            {"label", قىممەت.label},
                            {"paddingLeft", قىممەت.padding_سول},
                            {"paddingRight", قىممەت.padding_ئوڭ}};
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
                                         InlayHintKind قىممەت) {
  بدل (قىممەت) {
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

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  إذا (!O || !O.map("diagnostics", Result.diagnostics))
    أرجع خطأ;
  O.map("only", Result.only);
  أرجع true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

منطقي llvm::lsp::fromJSON(ثابت llvm::json::قىممەت &قىممەت, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(قىممەت, Path);
  أرجع O && O.map("changes", Result.changes);
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت WorkspaceEdit &قىممەت) {
  llvm::json::Object FileChanges;
  لكل (auto &Change : قىممەت.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  أرجع llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ثابت llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ثابت llvm::StringLiteral CodeAction::kRefactor = "refactor";
ثابت llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::قىممەت llvm::lsp::toJSON(ثابت CodeAction &قىممەت) {
  llvm::json::Object CodeAction{{"title", قىممەت.title}};
  إذا (قىممەت.kind)
    CodeAction["kind"] = *قىممەت.kind;
  إذا (قىممەت.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*قىممەت.diagnostics);
  إذا (قىممەت.isPreferred)
    CodeAction["isPreferred"] = true;
  إذا (قىممەت.edit)
    CodeAction["edit"] = *قىممەت.edit;
  أرجع std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::قىممەت llvm::lsp::toJSON(ثابت ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<صحيح>(Params.type)},
      {"message", Params.message},
  };
  إذا (Params.actions)
    Out["actions"] = *Params.actions;
  أرجع Out;
}

llvm::json::قىممەت llvm::lsp::toJSON(ثابت MessageActionItem &Params) {
  أرجع llvm::json::Object{{"title", Params.title}};
}
