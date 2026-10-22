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
  ᱡᱩᱫᱤ (!V || V->getAsNull())
    ᱨᱩᱣᱟᱹᱲ true;
  ᱨᱩᱣᱟᱹᱲ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

var_na LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

static bool isWindowsPath(StringRef Path) {
  ᱨᱩᱣᱟᱹᱲ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

static bool isNetworkPath(StringRef Path) {
  ᱨᱩᱣᱟᱹᱲ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

static bool shouldEscapeInURI(unsigned var_na C) {
  // Unreserved characters.
  ᱡᱩᱫᱤ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ᱨᱩᱣᱟᱹᱲ false;

  switch (C) {
  pak_sa '-':
  pak_sa '_':
  pak_sa '.':
  pak_sa '~':
  // '/' is only reserved when parsing.
  pak_sa '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  pak_sa ':':
    ᱨᱩᱣᱟᱹᱲ false;
  }
  ᱨᱩᱣᱟᱹᱲ true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
static void percentEncode(StringRef Content, std::string &Out) {
  for (unsigned var_na C : Content) {
    ᱡᱩᱫᱤ (shouldEscapeInURI(C)) {
      Out.push_back('%');
      Out.push_back(llvm::hexdigit(C / 16));
      Out.push_back(llvm::hexdigit(C % 16));
    } ᱵᱟᱝ {
      Out.push_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
static std::string percentDecode(StringRef Content) {
  std::string Result;
  for (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    ᱡᱩᱫᱤ (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.push_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ᱵᱟᱝ {
      Result.push_back(*I);
    }
  }
  ᱨᱩᱣᱟᱹᱲ Result;
}

/// Return the set containing the supported URI schemes.
static StringSet<> &getSupportedSchemes() {
  static StringSet<> Schemes({"file", "test"});
  ᱨᱩᱣᱟᱹᱲ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
static bool isStructurallyValidScheme(StringRef Scheme) {
  ᱡᱩᱫᱤ (Scheme.empty())
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (!llvm::isAlpha(Scheme[0]))
    ᱨᱩᱣᱟᱹᱲ false;
  ᱨᱩᱣᱟᱹᱲ llvm::all_of(llvm::drop_begin(Scheme), [](var_na C) {
    ᱨᱩᱣᱟᱹᱲ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

static llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef Root = llvm::sys::path::root_name(AbsolutePath);
  ᱡᱩᱫᱤ (isNetworkPath(Root)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = Root.drop_front(2);
    AbsolutePath.consume_front(Root);
  } ᱵᱟᱝ ᱡᱩᱫᱤ (isWindowsPath(Root)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  ᱡᱩᱫᱤ (Authority.empty() && Body.empty())
    ᱨᱩᱣᱟᱹᱲ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ᱡᱩᱫᱤ (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ᱨᱩᱣᱟᱹᱲ Uri;
}

static llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  ᱡᱩᱫᱤ (!Body.starts_with("/"))
    ᱨᱩᱣᱟᱹᱲ llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  ᱡᱩᱫᱤ (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } ᱵᱟᱝ ᱡᱩᱫᱤ (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ᱨᱩᱣᱟᱹᱲ std::string(Path);
}

static llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  ᱡᱩᱫᱤ (Pos == StringRef::npos)
    ᱨᱩᱣᱟᱹᱲ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ᱡᱩᱫᱤ (!isStructurallyValidScheme(UriScheme))
    ᱨᱩᱣᱟᱹᱲ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ᱡᱩᱫᱤ (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ᱡᱩᱫᱤ (!getSupportedSchemes().contains(UriScheme)) {
    ᱨᱩᱣᱟᱹᱲ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ᱨᱩᱣᱟᱹᱲ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  ᱡᱩᱫᱤ (!FilePath)
    ᱨᱩᱣᱟᱹᱲ FilePath.takeError();
  ᱨᱩᱣᱟᱹᱲ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  ᱡᱩᱫᱤ (!Uri)
    ᱨᱩᱣᱟᱹᱲ Uri.takeError();
  ᱨᱩᱣᱟᱹᱲ fromURI(*Uri);
}

StringRef URIForFile::scheme() const { ᱨᱩᱣᱟᱹᱲ uri().split(':').first; }

void URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().insert(Scheme);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, URIForFile &Result,
                         llvm::json::Path Path) {
  ᱡᱩᱫᱤ (std::optional<StringRef> Str = Value.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ᱡᱩᱫᱤ (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ᱨᱩᱣᱟᱹᱲ false;
    }
    Result = std::move(*ExpectedUri);
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

llvm::json::Value llvm::lsp::toJSON(const URIForFile &Value) {
  ᱨᱩᱣᱟᱹᱲ Value.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const URIForFile &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  const llvm::json::Object *O = Value.getAsObject();
  ᱡᱩᱫᱤ (!O) {
    Path.report("expected object");
    ᱨᱩᱣᱟᱹᱲ false;
  }
  ᱡᱩᱫᱤ (const llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    ᱡᱩᱫᱤ (const llvm::json::Object *DocumentSymbol =
            TextDocument->getObject("documentSymbol")) {
      ᱡᱩᱫᱤ (std::optional<bool> HierarchicalSupport =
              DocumentSymbol->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchicalDocumentSymbol = *HierarchicalSupport;
    }
    ᱡᱩᱫᱤ (auto *CodeAction = TextDocument->getObject("codeAction")) {
      ᱡᱩᱫᱤ (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  ᱡᱩᱫᱤ (auto *Window = O->getObject("window")) {
    ᱡᱩᱫᱤ (std::optional<bool> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱡᱩᱫᱤ (!O || !O.map("name", Result.name))
    ᱨᱩᱣᱟᱹᱲ false;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, TraceLevel &Result,
                         llvm::json::Path Path) {
  ᱡᱩᱫᱤ (std::optional<StringRef> Str = Value.getAsString()) {
    ᱡᱩᱫᱤ (*Str == "off") {
      Result = TraceLevel::Off;
      ᱨᱩᱣᱟᱹᱲ true;
    }
    ᱡᱩᱫᱤ (*Str == "messages") {
      Result = TraceLevel::Messages;
      ᱨᱩᱣᱟᱹᱲ true;
    }
    ᱡᱩᱫᱤ (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      ᱨᱩᱣᱟᱹᱲ true;
    }
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱡᱩᱫᱤ (!O)
    ᱨᱩᱣᱟᱹᱲ false;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.rootUri);
  O.map("rootPath", Result.rootPath);
  mapOptOrNull(Value, "clientInfo", Result.clientInfo, Path);

  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const TextDocumentIdentifier &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{{"uri", Value.uri}};
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::Value
llvm::lsp::toJSON(const VersionedTextDocumentIdentifier &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"uri", Value.uri},
      {"version", Value.version},
  };
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("line", Result.line) &&
         O.map("character", Result.character);
}

llvm::json::Value llvm::lsp::toJSON(const Position &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"line", Value.line},
      {"character", Value.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Position &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.line << ':' << Value.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::Value llvm::lsp::toJSON(const Range &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"start", Value.start},
      {"end", Value.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Range &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.start << '-' << Value.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::Value llvm::lsp::toJSON(const Location &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"uri", Value.uri},
      {"range", Value.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const Location &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.range << '@' << Value.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ fromJSON(Value, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) const {
  // If there is no range, the full document changed.
  ᱡᱩᱫᱤ (!range) {
    Contents = text;
    ᱨᱩᱣᱟᱹᱲ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ᱡᱩᱫᱤ (!RangeLoc.isValid())
    ᱨᱩᱣᱟᱹᱲ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ᱨᱩᱣᱟᱹᱲ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  for (const auto &Change : Changes)
    ᱡᱩᱫᱤ (failed(Change.applyTo(Contents)))
      ᱨᱩᱣᱟᱹᱲ failure();
  ᱨᱩᱣᱟᱹᱲ success();
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

static llvm::StringRef toTextKind(MarkupKind Kind) {
  switch (Kind) {
  pak_sa MarkupKind::PlainText:
    ᱨᱩᱣᱟᱹᱲ "plaintext";
  pak_sa MarkupKind::Markdown:
    ᱨᱩᱣᱟᱹᱲ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ᱨᱩᱣᱟᱹᱲ Os << toTextKind(Kind);
}

llvm::json::Value llvm::lsp::toJSON(const MarkupContent &Mc) {
  ᱡᱩᱫᱤ (Mc.value.empty())
    ᱨᱩᱣᱟᱹᱲ nullptr;

  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.value},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  ᱡᱩᱫᱤ (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentSymbol &Symbol) {
  llvm::json::Object Result{{"name", Symbol.name},
                            {"kind", static_cast<puur_naa_nka>(Symbol.kind)},
                            {"range", Symbol.range},
                            {"selectionRange", Symbol.selectionRange}};

  ᱡᱩᱫᱤ (!Symbol.detail.empty())
    Result["detail"] = Symbol.detail;
  ᱡᱩᱫᱤ (!Symbol.children.empty())
    Result["children"] = Symbol.children;
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DocumentSymbolParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::Value llvm::lsp::toJSON(const DiagnosticRelatedInformation &Info) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(DiagnosticTag Tag) {
  ᱨᱩᱣᱟᱹᱲ static_cast<puur_naa_nka>(Tag);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  ᱡᱩᱫᱤ (std::optional<int64_t> I = Value.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    ᱨᱩᱣᱟᱹᱲ true;
  }

  ᱨᱩᱣᱟᱹᱲ false;
}

llvm::json::Value llvm::lsp::toJSON(const Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (puur_naa_nka)Diag.severity},
      {"message", Diag.message},
  };
  ᱡᱩᱫᱤ (Diag.category)
    Result["category"] = *Diag.category;
  ᱡᱩᱫᱤ (!Diag.source.empty())
    Result["source"] = Diag.source;
  ᱡᱩᱫᱤ (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  ᱡᱩᱫᱤ (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱡᱩᱫᱤ (!O)
    ᱨᱩᱣᱟᱹᱲ false;
  puur_naa_nka Severity = 0;
  ᱡᱩᱫᱤ (!mapOptOrNull(Value, "severity", Severity, Path))
    ᱨᱩᱣᱟᱹᱲ false;
  Result.severity = (DiagnosticSeverity)Severity;

  ᱨᱩᱣᱟᱹᱲ O.map("range", Result.range) && O.map("message", Result.message) &&
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
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
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
  ᱨᱩᱣᱟᱹᱲ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::Value llvm::lsp::toJSON(const TextEdit &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
      {"range", Value.range},
      {"newText", Value.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, const TextEdit &Value) {
  Os << Value.range << " => \"";
  llvm::printEscapedString(Value.newText, Os);
  ᱨᱩᱣᱟᱹᱲ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  ᱡᱩᱫᱤ (std::optional<int64_t> IntValue = Value.getAsInteger()) {
    ᱡᱩᱫᱤ (*IntValue < static_cast<puur_naa_nka>(CompletionItemKind::Text) ||
        *IntValue > static_cast<puur_naa_nka>(CompletionItemKind::TypeParameter))
      ᱨᱩᱣᱟᱹᱲ false;
    Result = static_cast<CompletionItemKind>(*IntValue);
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  ᱡᱩᱫᱤ (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    ᱨᱩᱣᱟᱹᱲ Kind;

  // Provide some fall backs for common kinds that are close enough.
  switch (Kind) {
  pak_sa CompletionItemKind::Folder:
    ᱨᱩᱣᱟᱹᱲ CompletionItemKind::File;
  pak_sa CompletionItemKind::EnumMember:
    ᱨᱩᱣᱟᱹᱲ CompletionItemKind::Enum;
  pak_sa CompletionItemKind::Struct:
    ᱨᱩᱣᱟᱹᱲ CompletionItemKind::Class;
  default:
    ᱨᱩᱣᱟᱹᱲ CompletionItemKind::Text;
  }
}

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  ᱡᱩᱫᱤ (const llvm::json::Array *ArrayValue = Value.getAsArray()) {
    for (size_t I = 0, E = ArrayValue->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      ᱡᱩᱫᱤ (fromJSON((*ArrayValue)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionItem &Value) {
  assert(!Value.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", Value.label}};
  ᱡᱩᱫᱤ (Value.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<puur_naa_nka>(Value.kind);
  ᱡᱩᱫᱤ (!Value.detail.empty())
    Result["detail"] = Value.detail;
  ᱡᱩᱫᱤ (Value.documentation)
    Result["documentation"] = Value.documentation;
  ᱡᱩᱫᱤ (!Value.sortText.empty())
    Result["sortText"] = Value.sortText;
  ᱡᱩᱫᱤ (!Value.filterText.empty())
    Result["filterText"] = Value.filterText;
  ᱡᱩᱫᱤ (!Value.insertText.empty())
    Result["insertText"] = Value.insertText;
  ᱡᱩᱫᱤ (Value.insertTextFormat != InsertTextFormat::Missing)
    Result["insertTextFormat"] = static_cast<puur_naa_nka>(Value.insertTextFormat);
  ᱡᱩᱫᱤ (Value.textEdit)
    Result["textEdit"] = *Value.textEdit;
  ᱡᱩᱫᱤ (!Value.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(Value.additionalTextEdits);
  }
  ᱡᱩᱫᱤ (Value.deprecated)
    Result["deprecated"] = Value.deprecated;
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const CompletionItem &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.label << " - " << toJSON(Value);
}

bool llvm::lsp::operator<(const CompletionItem &Lhs,
                          const CompletionItem &Rhs) {
  ᱨᱩᱣᱟᱹᱲ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const CompletionList &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
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
  ᱡᱩᱫᱤ (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(Value, "triggerCharacter", Result.triggerCharacter, Path))
    ᱨᱩᱣᱟᱹᱲ false;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CompletionParams &Result, llvm::json::Path Path) {
  ᱡᱩᱫᱤ (!fromJSON(Value, static_cast<TextDocumentPositionParams &>(Result), Path))
    ᱨᱩᱣᱟᱹᱲ false;
  ᱡᱩᱫᱤ (const llvm::json::Value *Context = Value.getAsObject()->get("context"))
    ᱨᱩᱣᱟᱹᱲ fromJSON(*Context, Result.context, Path.field("context"));
  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ParameterInformation &Value) {
  assert((Value.labelOffsets || !Value.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  ᱡᱩᱫᱤ (Value.labelOffsets)
    Result["label"] = llvm::json::Array(
        {Value.labelOffsets->first, Value.labelOffsets->second});
  ᱵᱟᱝ
    Result["label"] = Value.labelString;
  ᱡᱩᱫᱤ (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
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
  ᱡᱩᱫᱤ (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  ᱨᱩᱣᱟᱹᱲ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   const SignatureInformation &Value) {
  ᱨᱩᱣᱟᱹᱲ Os << Value.label << " - " << toJSON(Value);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const SignatureHelp &Value) {
  assert(Value.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  assert(Value.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
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
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const DocumentLink &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{
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
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const InlayHint &Value) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{{"position", Value.position},
                            {"kind", (puur_naa_nka)Value.kind},
                            {"label", Value.label},
                            {"paddingLeft", Value.paddingLeft},
                            {"paddingRight", Value.paddingRight}};
}
bool llvm::lsp::operator==(const InlayHint &Lhs, const InlayHint &Rhs) {
  ᱨᱩᱣᱟᱹᱲ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
bool llvm::lsp::operator<(const InlayHint &Lhs, const InlayHint &Rhs) {
  ᱨᱩᱣᱟᱹᱲ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind Value) {
  switch (Value) {
  pak_sa InlayHintKind::Parameter:
    ᱨᱩᱣᱟᱹᱲ Os << "parameter";
  pak_sa InlayHintKind::Type:
    ᱨᱩᱣᱟᱹᱲ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱡᱩᱫᱤ (!O || !O.map("diagnostics", Result.diagnostics))
    ᱨᱩᱣᱟᱹᱲ false;
  O.map("only", Result.only);
  ᱨᱩᱣᱟᱹᱲ true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

bool llvm::lsp::fromJSON(const llvm::json::Value &Value, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(Value, Path);
  ᱨᱩᱣᱟᱹᱲ O && O.map("changes", Result.changes);
}

llvm::json::Value llvm::lsp::toJSON(const WorkspaceEdit &Value) {
  llvm::json::Object FileChanges;
  for (auto &Change : Value.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

const llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
const llvm::StringLiteral CodeAction::kRefactor = "refactor";
const llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::Value llvm::lsp::toJSON(const CodeAction &Value) {
  llvm::json::Object CodeAction{{"title", Value.title}};
  ᱡᱩᱫᱤ (Value.kind)
    CodeAction["kind"] = *Value.kind;
  ᱡᱩᱫᱤ (Value.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*Value.diagnostics);
  ᱡᱩᱫᱤ (Value.isPreferred)
    CodeAction["isPreferred"] = true;
  ᱡᱩᱫᱤ (Value.edit)
    CodeAction["edit"] = *Value.edit;
  ᱨᱩᱣᱟᱹᱲ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::Value llvm::lsp::toJSON(const ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<puur_naa_nka>(Params.type)},
      {"message", Params.message},
  };
  ᱡᱩᱫᱤ (Params.actions)
    Out["actions"] = *Params.actions;
  ᱨᱩᱣᱟᱹᱲ Out;
}

llvm::json::Value llvm::lsp::toJSON(const MessageActionItem &Params) {
  ᱨᱩᱣᱟᱹᱲ llvm::json::Object{{"title", Params.title}};
}
