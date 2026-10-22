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

#include "llvm/Support/LSP/Protocol.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/JSON.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
static bool mapOptOrNull(const llvm::json::Value &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  const llvm::json::Object *O = Params.getAsObject();
  assert(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  ᨀᨘ (!V || V->getAsNull())
    ᨀᨘᨒ true;
  ᨀᨘᨒ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

var_na LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

static bool isWindowsPath(StringRef Path) {
  ᨀᨘᨒ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

static bool isNetworkPath(StringRef Path) {
  ᨀᨘᨒ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

static bool shouldEscapeInURI(unsigned var_na C) {
  // Unreserved characters.
  ᨀᨘ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ᨀᨘᨒ false;

  switch (C) {
  pak_sa '-':
  pak_sa '_':
  pak_sa '.':
  pak_sa '~':
  // '/' is only reserved when parsing.
  pak_sa '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  pak_sa ':':
    ᨀᨘᨒ false;
  }
  ᨀᨘᨒ true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
static void percentEncode(StringRef Content, std::string &Out) {
  for (unsigned var_na C : Content) {
    ᨀᨘ (shouldEscapeInURI(C)) {
      Out.push_back('%');
      Out.push_back(llvm::hexdigit(C / 16));
      Out.push_back(llvm::hexdigit(C % 16));
    } ᨀᨘᨁ {
      Out.push_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
static std::string percentDecode(StringRef Content) {
  std::string Result;
  for (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    ᨀᨘ (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.push_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ᨀᨘᨁ {
      Result.push_back(*I);
    }
  }
  ᨀᨘᨒ Result;
}

/// Return the set containing the supported URI schemes.
static StringSet<> &getSupportedSchemes() {
  static StringSet<> Schemes({"file", "test"});
  ᨀᨘᨒ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
static bool isStructurallyValidScheme(StringRef Scheme) {
  ᨀᨘ (Scheme.empty())
    ᨀᨘᨒ false;
  ᨀᨘ (!llvm::isAlpha(Scheme[0]))
    ᨀᨘᨒ false;
  ᨀᨘᨒ llvm::all_of(llvm::drop_begin(Scheme), [](var_na C) {
    ᨀᨘᨒ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

static llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef Root = llvm::sys::path::root_name(AbsolutePath);
  ᨀᨘ (isNetworkPath(Root)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = Root.drop_front(2);
    AbsolutePath.consume_front(Root);
  } ᨀᨘᨁ ᨀᨘ (isWindowsPath(Root)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  ᨀᨘ (Authority.empty() && Body.empty())
    ᨀᨘᨒ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ᨀᨘ (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ᨀᨘᨒ Uri;
}

static llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  ᨀᨘ (!Body.starts_with("/"))
    ᨀᨘᨒ llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  ᨀᨘ (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } ᨀᨘᨁ ᨀᨘ (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ᨀᨘᨒ std::string(Path);
}

static llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  ᨀᨘ (Pos == StringRef::npos)
    ᨀᨘᨒ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ᨀᨘ (!isStructurallyValidScheme(UriScheme))
    ᨀᨘᨒ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ᨀᨘ (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ᨀᨘ (!getSupportedSchemes().contains(UriScheme)) {
    ᨀᨘᨒ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ᨀᨘᨒ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  ᨀᨘ (!FilePath)
    ᨀᨘᨒ FilePath.takeError();
  ᨀᨘᨒ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  ᨀᨘ (!Uri)
    ᨀᨘᨒ Uri.takeError();
  ᨀᨘᨒ fromURI(*Uri);
}

StringRef URIForFile::scheme() const { ᨀᨘᨒ uri().split(':').first; }

void URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().insert(Scheme);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, URIForFile &Result,
                         llvm::json::Path Path) {
  ᨀᨘ (std::optional<StringRef> Str = Value.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ᨀᨘ (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ᨀᨘᨒ false;
    }
    Result = std::move(*ExpectedUri);
    ᨀᨘᨒ true;
  }
  ᨀᨘᨒ false;
}

llvm::json::Value llvm::lsp::toJSON(const URIForFile &Value) {
  ᨀᨘᨒ Value.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const URIForFile &Value) {
  ᨀᨘᨒ Os << Value.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  const llvm::json::Object *O = Value.getAsObject();
  ᨀᨘ (!O) {
    Path.report("expected object");
    ᨀᨘᨒ false;
  }
  ᨀᨘ (const llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    ᨀᨘ (const llvm::json::Object *DocumentSymbol =
            TextDocument->getObject("documentSymbol")) {
      ᨀᨘ (std::optional<bool> HierarchicalSupport =
              DocumentSymbol->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchicalDocumentSymbol = *HierarchicalSupport;
    }
    ᨀᨘ (auto *CodeAction = TextDocument->getObject("codeAction")) {
      ᨀᨘ (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  ᨀᨘ (auto *Window = O->getObject("window")) {
    ᨀᨘ (std::optional<bool> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘ (!O || !O.map("name", Result.name))
    ᨀᨘᨒ false;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, TraceLevel &Result,
                         llvm::json::Path Path) {
  ᨀᨘ (std::optional<StringRef> Str = Value.getAsString()) {
    ᨀᨘ (*Str == "off") {
      Result = TraceLevel::Off;
      ᨀᨘᨒ true;
    }
    ᨀᨘ (*Str == "messages") {
      Result = TraceLevel::Messages;
      ᨀᨘᨒ true;
    }
    ᨀᨘ (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      ᨀᨘᨒ true;
    }
  }
  ᨀᨘᨒ false;
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘ (!O)
    ᨀᨘᨒ false;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.rootUri);
  O.map("rootPath", Result.rootPath);
  mapOptOrNull(Value, "clientInfo", Result.clientInfo, Path);

  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const TextDocumentIdentifier &Value) {
  ᨀᨘᨒ llvm::json::Object{{"uri", Value.uri}};
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value
llvm::lsp::toJSON(const VersionedTextDocumentIdentifier &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"uri", Value.uri},
      {"version", Value.version},
  };
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("line", Result.line) &&
         O.map("character", Result.character);
}

llvm::json::Value llvm::lsp::toJSON(const Position &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"line", Value.line},
      {"character", Value.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Position &Value) {
  ᨀᨘᨒ Os << Value.line << ':' << Value.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::Value llvm::lsp::toJSON(const Range &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"start", Value.start},
      {"end", Value.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Range &Value) {
  ᨀᨘᨒ Os << Value.start << '-' << Value.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::Value llvm::lsp::toJSON(const Location &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"uri", Value.uri},
      {"range", Value.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Location &Value) {
  ᨀᨘᨒ Os << Value.range << '@' << Value.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ fromJSON(Value, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ᨀᨘᨒ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) const {
  // If there is no range, the full document changed.
  ᨀᨘ (!range) {
    Contents = text;
    ᨀᨘᨒ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ᨀᨘ (!RangeLoc.isValid())
    ᨀᨘᨒ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ᨀᨘᨒ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  for (const auto &Change : Changes)
    ᨀᨘ (failed(Change.applyTo(Contents)))
      ᨀᨘᨒ failure();
  ᨀᨘᨒ success();
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

static llvm::StringRef toTextKind(MarkupKind Kind) {
  switch (Kind) {
  pak_sa MarkupKind::PlainText:
    ᨀᨘᨒ "plaintext";
  pak_sa MarkupKind::Markdown:
    ᨀᨘᨒ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ᨀᨘᨒ Os << toTextKind(Kind);
}

llvm::json::Value llvm::lsp::toJSON(const MarkupContent &Mc) {
  ᨀᨘ (Mc.value.empty())
    ᨀᨘᨒ nullptr;

  ᨀᨘᨒ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.value},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  ᨀᨘ (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  ᨀᨘᨒ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentSymbol &Symbol) {
  llvm::json::Object Result{{"name", Symbol.name},
                            {"kind", static_cast<puur_naa_nka>(Symbol.kind)},
                            {"range", Symbol.range},
                            {"selectionRange", Symbol.selectionRange}};

  ᨀᨘ (!Symbol.detail.empty())
    Result["detail"] = Symbol.detail;
  ᨀᨘ (!Symbol.children.empty())
    Result["children"] = Symbol.children;
  ᨀᨘᨒ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DocumentSymbolParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::Value llvm::lsp::toJSON(const DiagnosticRelatedInformation &Info) {
  ᨀᨘᨒ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(DiagnosticTag Tag) {
  ᨀᨘᨒ static_cast<puur_naa_nka>(Tag);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  ᨀᨘ (std::optional<int64_t> I = Value.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    ᨀᨘᨒ true;
  }

  ᨀᨘᨒ false;
}

llvm::json::Value llvm::lsp::toJSON(const Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (puur_naa_nka)Diag.severity},
      {"message", Diag.message},
  };
  ᨀᨘ (Diag.category)
    Result["category"] = *Diag.category;
  ᨀᨘ (!Diag.source.empty())
    Result["source"] = Diag.source;
  ᨀᨘ (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  ᨀᨘ (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  ᨀᨘᨒ std::move(Result);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘ (!O)
    ᨀᨘᨒ false;
  puur_naa_nka Severity = 0;
  ᨀᨘ (!mapOptOrNull(Value, "severity", Severity, Path))
    ᨀᨘᨒ false;
  Result.severity = (DiagnosticSeverity)Severity;

  ᨀᨘᨒ O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(Value, "category", Result.category, Path) &&
         mapOptOrNull(Value, "source", Result.source, Path) &&
         mapOptOrNull(Value, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(Value, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const PublishDiagnosticsParams &Params) {
  ᨀᨘᨒ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::Value llvm::lsp::toJSON(const TextEdit &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"range", Value.range},
      {"newText", Value.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const TextEdit &Value) {
  Os << Value.range << " => \"";
  llvm::printEscapedString(Value.newText, Os);
  ᨀᨘᨒ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  ᨀᨘ (std::optional<int64_t> IntValue = Value.getAsInteger()) {
    ᨀᨘ (*IntValue < static_cast<puur_naa_nka>(CompletionItemKind::Text) ||
        *IntValue > static_cast<puur_naa_nka>(CompletionItemKind::TypeParameter))
      ᨀᨘᨒ false;
    Result = static_cast<CompletionItemKind>(*IntValue);
    ᨀᨘᨒ true;
  }
  ᨀᨘᨒ false;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  ᨀᨘ (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    ᨀᨘᨒ Kind;

  // Provide some fall backs for common kinds that are close enough.
  switch (Kind) {
  pak_sa CompletionItemKind::Folder:
    ᨀᨘᨒ CompletionItemKind::File;
  pak_sa CompletionItemKind::EnumMember:
    ᨀᨘᨒ CompletionItemKind::Enum;
  pak_sa CompletionItemKind::Struct:
    ᨀᨘᨒ CompletionItemKind::Class;
  default:
    ᨀᨘᨒ CompletionItemKind::Text;
  }
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  ᨀᨘ (const llvm::json::Array *ArrayValue = Value.getAsArray()) {
    for (size_t I = 0, E = ArrayValue->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      ᨀᨘ (fromJSON((*ArrayValue)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    ᨀᨘᨒ true;
  }
  ᨀᨘᨒ false;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionItem &Value) {
  assert(!Value.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", Value.label}};
  ᨀᨘ (Value.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<puur_naa_nka>(Value.kind);
  ᨀᨘ (!Value.detail.empty())
    Result["detail"] = Value.detail;
  ᨀᨘ (Value.documentation)
    Result["documentation"] = Value.documentation;
  ᨀᨘ (!Value.sortText.empty())
    Result["sortText"] = Value.sortText;
  ᨀᨘ (!Value.filterText.empty())
    Result["filterText"] = Value.filterText;
  ᨀᨘ (!Value.insertText.empty())
    Result["insertText"] = Value.insertText;
  ᨀᨘ (Value.insertTextFormat != InsertTextFormat::Missing)
    Result["insertTextFormat"] = static_cast<puur_naa_nka>(Value.insertTextFormat);
  ᨀᨘ (Value.textEdit)
    Result["textEdit"] = *Value.textEdit;
  ᨀᨘ (!Value.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(Value.additionalTextEdits);
  }
  ᨀᨘ (Value.deprecated)
    Result["deprecated"] = Value.deprecated;
  ᨀᨘᨒ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const CompletionItem &Value) {
  ᨀᨘᨒ Os << Value.label << " - " << toJSON(Value);
}

bool llvm::lsp::operator<(const CompletionItem &Lhs,
                          const CompletionItem &Rhs) {
  ᨀᨘᨒ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionList &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"isIncomplete", Value.isIncomplete},
      {"items", llvm::json::Array(Value.items)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  puur_naa_nka TriggerKind;
  ᨀᨘ (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(Value, "triggerCharacter", Result.triggerCharacter, Path))
    ᨀᨘᨒ false;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionParams &Result, llvm::json::Path Path) {
  ᨀᨘ (!fromJSON(Value, static_cast<TextDocumentPositionParams &>(Result), Path))
    ᨀᨘᨒ false;
  ᨀᨘ (const llvm::json::Value *Context = Value.getAsObject()->get("context"))
    ᨀᨘᨒ fromJSON(*Context, Result.context, Path.field("context"));
  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ParameterInformation &Value) {
  assert((Value.labelOffsets || !Value.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  ᨀᨘ (Value.labelOffsets)
    Result["label"] = llvm::json::Array(
        {Value.labelOffsets->first, Value.labelOffsets->second});
  ᨀᨘᨁ
    Result["label"] = Value.labelString;
  ᨀᨘ (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  ᨀᨘᨒ std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const SignatureInformation &Value) {
  assert(!Value.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", Value.label},
      {"parameters", llvm::json::Array(Value.parameters)},
  };
  ᨀᨘ (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  ᨀᨘᨒ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const SignatureInformation &Value) {
  ᨀᨘᨒ Os << Value.label << " - " << toJSON(Value);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const SignatureHelp &Value) {
  assert(Value.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  assert(Value.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ᨀᨘᨒ llvm::json::Object{
      {"activeSignature", Value.activeSignature},
      {"activeParameter", Value.activeParameter},
      {"signatures", llvm::json::Array(Value.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentLink &Value) {
  ᨀᨘᨒ llvm::json::Object{
      {"range", Value.range},
      {"target", Value.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const InlayHint &Value) {
  ᨀᨘᨒ llvm::json::Object{{"position", Value.position},
                            {"kind", (puur_naa_nka)Value.kind},
                            {"label", Value.label},
                            {"paddingLeft", Value.paddingLeft},
                            {"paddingRight", Value.paddingRight}};
}
bool llvm::lsp::operator==(const InlayHint &Lhs, const InlayHint &Rhs) {
  ᨀᨘᨒ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
bool llvm::lsp::operator<(const InlayHint &Lhs, const InlayHint &Rhs) {
  ᨀᨘᨒ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind Value) {
  switch (Value) {
  pak_sa InlayHintKind::Parameter:
    ᨀᨘᨒ Os << "parameter";
  pak_sa InlayHintKind::Type:
    ᨀᨘᨒ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘ (!O || !O.map("diagnostics", Result.diagnostics))
    ᨀᨘᨒ false;
  O.map("only", Result.only);
  ᨀᨘᨒ true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᨀᨘᨒ O && O.map("changes", Result.changes);
}

llvm::json::Value llvm::lsp::toJSON(const WorkspaceEdit &Value) {
  llvm::json::Object FileChanges;
  for (auto &Change : Value.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ᨀᨘᨒ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

const llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
const llvm::StringLiteral CodeAction::kRefactor = "refactor";
const llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::Value llvm::lsp::toJSON(const CodeAction &Value) {
  llvm::json::Object CodeAction{{"title", Value.title}};
  ᨀᨘ (Value.kind)
    CodeAction["kind"] = *Value.kind;
  ᨀᨘ (Value.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*Value.diagnostics);
  ᨀᨘ (Value.isPreferred)
    CodeAction["isPreferred"] = true;
  ᨀᨘ (Value.edit)
    CodeAction["edit"] = *Value.edit;
  ᨀᨘᨒ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<puur_naa_nka>(Params.type)},
      {"message", Params.message},
  };
  ᨀᨘ (Params.actions)
    Out["actions"] = *Params.actions;
  ᨀᨘᨒ Out;
}

llvm::json::Value llvm::lsp::toJSON(const MessageActionItem &Params) {
  ᨀᨘᨒ llvm::json::Object{{"title", Params.title}};
}
