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

#അന്തര്ഭാവ "llvm/Support/LSP/Protocol.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringSet.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/JSON.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
സ്ഥിതം തർക്കം mapOptOrNull(സ്ഥിരം llvm::json::മൂല്യം &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  സ്ഥിരം llvm::json::Object *O = Params.getAsObject();
  പ്രതിജ്ഞാ(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  എങ്കിൽ (!V || V->getAsNull())
    തിരികെ സത്യം;
  തിരികെ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

അക്ഷരം LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

സ്ഥിതം തർക്കം isWindowsPath(StringRef Path) {
  തിരികെ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

സ്ഥിതം തർക്കം isNetworkPath(StringRef Path) {
  തിരികെ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

സ്ഥിതം തർക്കം shouldEscapeInURI(unsigned അക്ഷരം C) {
  // Unreserved characters.
  എങ്കിൽ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    തിരികെ അസത്യം;

  തിരഞ്ഞെടുപ്പ് (C) {
  അവസ്ഥ '-':
  അവസ്ഥ '_':
  അവസ്ഥ '.':
  അവസ്ഥ '~':
  // '/' is only reserved when parsing.
  അവസ്ഥ '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  അവസ്ഥ ':':
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
സ്ഥിതം ശൂന്യം percentEncode(StringRef Content, std::string &Out) {
  വേണ്ടി (unsigned അക്ഷരം C : Content) {
    എങ്കിൽ (shouldEscapeInURI(C)) {
      Out.തള്ളുക_back('%');
      Out.തള്ളുക_back(llvm::hexdigit(C / 16));
      Out.തള്ളുക_back(llvm::hexdigit(C % 16));
    } അല്ലെങ്കിൽ {
      Out.തള്ളുക_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
സ്ഥിതം std::string percentDecode(StringRef Content) {
  std::string Result;
  വേണ്ടി (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    എങ്കിൽ (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.തള്ളുക_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } അല്ലെങ്കിൽ {
      Result.തള്ളുക_back(*I);
    }
  }
  തിരികെ Result;
}

/// Return the set containing the supported URI schemes.
സ്ഥിതം StringSet<> &getSupportedSchemes() {
  സ്ഥിതം StringSet<> Schemes({"file", "test"});
  തിരികെ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
സ്ഥിതം തർക്കം isStructurallyValidScheme(StringRef Scheme) {
  എങ്കിൽ (Scheme.empty())
    തിരികെ അസത്യം;
  എങ്കിൽ (!llvm::isAlpha(Scheme[0]))
    തിരികെ അസത്യം;
  തിരികെ llvm::all_of(llvm::drop_begin(Scheme), [](അക്ഷരം C) {
    തിരികെ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

സ്ഥിതം llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef വേര്_2 = llvm::sys::path::വേര്_name(AbsolutePath);
  എങ്കിൽ (isNetworkPath(വേര്_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = വേര്_2.drop_front(2);
    AbsolutePath.consume_front(വേര്_2);
  } അല്ലെങ്കിൽ എങ്കിൽ (isWindowsPath(വേര്_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  എങ്കിൽ (Authority.empty() && Body.empty())
    തിരികെ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  എങ്കിൽ (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  തിരികെ Uri;
}

സ്ഥിതം llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  എങ്കിൽ (!Body.starts_with("/"))
    തിരികെ llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  എങ്കിൽ (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } അല്ലെങ്കിൽ എങ്കിൽ (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  തിരികെ std::string(Path);
}

സ്ഥിതം llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  എങ്കിൽ (Pos == StringRef::npos)
    തിരികെ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  എങ്കിൽ (!isStructurallyValidScheme(UriScheme))
    തിരികെ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  എങ്കിൽ (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  എങ്കിൽ (!getSupportedSchemes().contains(UriScheme)) {
    തിരികെ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  തിരികെ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  എങ്കിൽ (!FilePath)
    തിരികെ FilePath.takeError();
  തിരികെ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  എങ്കിൽ (!Uri)
    തിരികെ Uri.takeError();
  തിരികെ fromURI(*Uri);
}

StringRef URIForFile::scheme() സ്ഥിരം { തിരികെ uri().split(':').first; }

ശൂന്യം URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().തിരുകുക(Scheme);
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, URIForFile &Result,
                         llvm::json::Path Path) {
  എങ്കിൽ (std::optional<StringRef> Str = മൂല്യം.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    എങ്കിൽ (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      തിരികെ അസത്യം;
    }
    Result = std::move(*ExpectedUri);
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം URIForFile &മൂല്യം) {
  തിരികെ മൂല്യം.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, സ്ഥിരം URIForFile &മൂല്യം) {
  തിരികെ Os << മൂല്യം.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  സ്ഥിരം llvm::json::Object *O = മൂല്യം.getAsObject();
  എങ്കിൽ (!O) {
    Path.report("expected object");
    തിരികെ അസത്യം;
  }
  എങ്കിൽ (സ്ഥിരം llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    എങ്കിൽ (സ്ഥിരം llvm::json::Object *Document_ചിഹ്നം =
            TextDocument->getObject("documentSymbol")) {
      എങ്കിൽ (std::optional<തർക്കം> HierarchicalSupport =
              Document_ചിഹ്നം->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_ചിഹ്നം = *HierarchicalSupport;
    }
    എങ്കിൽ (auto *CodeAction = TextDocument->getObject("codeAction")) {
      എങ്കിൽ (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = സത്യം;
    }
  }
  എങ്കിൽ (auto *Window = O->getObject("window")) {
    എങ്കിൽ (std::optional<തർക്കം> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  എങ്കിൽ (!O || !O.map("name", Result.name))
    തിരികെ അസത്യം;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, TraceLevel &Result,
                         llvm::json::Path Path) {
  എങ്കിൽ (std::optional<StringRef> Str = മൂല്യം.getAsString()) {
    എങ്കിൽ (*Str == "off") {
      Result = TraceLevel::Off;
      തിരികെ സത്യം;
    }
    എങ്കിൽ (*Str == "messages") {
      Result = TraceLevel::Messages;
      തിരികെ സത്യം;
    }
    എങ്കിൽ (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      തിരികെ സത്യം;
    }
  }
  തിരികെ അസത്യം;
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  എങ്കിൽ (!O)
    തിരികെ അസത്യം;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.വേര്_Uri);
  O.map("rootPath", Result.വേര്_Path);
  mapOptOrNull(മൂല്യം, "clientInfo", Result.clientInfo, Path);

  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം TextDocumentIdentifier &മൂല്യം) {
  തിരികെ llvm::json::Object{{"uri", മൂല്യം.uri}};
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം
llvm::lsp::toJSON(സ്ഥിരം VersionedTextDocumentIdentifier &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"uri", മൂല്യം.uri},
      {"version", മൂല്യം.version},
  };
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("line", Result.വരി_2) &&
         O.map("character", Result.character);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Position &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"line", മൂല്യം.വരി_2},
      {"character", മൂല്യം.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, സ്ഥിരം Position &മൂല്യം) {
  തിരികെ Os << മൂല്യം.വരി_2 << ':' << മൂല്യം.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Range &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"start", മൂല്യം.start},
      {"end", മൂല്യം.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, സ്ഥിരം Range &മൂല്യം) {
  തിരികെ Os << മൂല്യം.start << '-' << മൂല്യം.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Location &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"uri", മൂല്യം.uri},
      {"range", മൂല്യം.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, സ്ഥിരം Location &മൂല്യം) {
  തിരികെ Os << മൂല്യം.range << '@' << മൂല്യം.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ fromJSON(മൂല്യം, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  തിരികെ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) സ്ഥിരം {
  // If there is no range, the full document changed.
  എങ്കിൽ (!range) {
    Contents = text;
    തിരികെ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  എങ്കിൽ (!RangeLoc.isValid())
    തിരികെ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  തിരികെ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  വേണ്ടി (സ്ഥിരം auto &Change : Changes)
    എങ്കിൽ (failed(Change.applyTo(Contents)))
      തിരികെ failure();
  തിരികെ success();
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

സ്ഥിതം llvm::StringRef toTextKind(MarkupKind Kind) {
  തിരഞ്ഞെടുപ്പ് (Kind) {
  അവസ്ഥ MarkupKind::PlainText:
    തിരികെ "plaintext";
  അവസ്ഥ MarkupKind::Markdown:
    തിരികെ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  തിരികെ Os << toTextKind(Kind);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം MarkupContent &Mc) {
  എങ്കിൽ (Mc.മൂല്യം_2.empty())
    തിരികെ nullptr;

  തിരികെ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.മൂല്യം_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  എങ്കിൽ (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  തിരികെ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Document_ചിഹ്നം &ചിഹ്നം) {
  llvm::json::Object Result{{"name", ചിഹ്നം.name},
                            {"kind", static_cast<പൂർണ്ണസംഖ്യ>(ചിഹ്നം.kind)},
                            {"range", ചിഹ്നം.range},
                            {"selectionRange", ചിഹ്നം.selectionRange}};

  എങ്കിൽ (!ചിഹ്നം.detail.empty())
    Result["detail"] = ചിഹ്നം.detail;
  എങ്കിൽ (!ചിഹ്നം.children.empty())
    Result["children"] = ചിഹ്നം.children;
  തിരികെ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         Document_ചിഹ്നം_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം DiagnosticRelatedInformation &Info) {
  തിരികെ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(DiagnosticTag Tag) {
  തിരികെ static_cast<പൂർണ്ണസംഖ്യ>(Tag);
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  എങ്കിൽ (std::optional<int64_t> I = മൂല്യം.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    തിരികെ സത്യം;
  }

  തിരികെ അസത്യം;
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (പൂർണ്ണസംഖ്യ)Diag.severity},
      {"message", Diag.message},
  };
  എങ്കിൽ (Diag.category)
    Result["category"] = *Diag.category;
  എങ്കിൽ (!Diag.source.empty())
    Result["source"] = Diag.source;
  എങ്കിൽ (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  എങ്കിൽ (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  തിരികെ std::move(Result);
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  എങ്കിൽ (!O)
    തിരികെ അസത്യം;
  പൂർണ്ണസംഖ്യ Severity = 0;
  എങ്കിൽ (!mapOptOrNull(മൂല്യം, "severity", Severity, Path))
    തിരികെ അസത്യം;
  Result.severity = (DiagnosticSeverity)Severity;

  തിരികെ O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(മൂല്യം, "category", Result.category, Path) &&
         mapOptOrNull(മൂല്യം, "source", Result.source, Path) &&
         mapOptOrNull(മൂല്യം, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(മൂല്യം, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം PublishDiagnosticsParams &Params) {
  തിരികെ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം TextEdit &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"range", മൂല്യം.range},
      {"newText", മൂല്യം.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, സ്ഥിരം TextEdit &മൂല്യം) {
  Os << മൂല്യം.range << " => \"";
  llvm::printEscapedString(മൂല്യം.newText, Os);
  തിരികെ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  എങ്കിൽ (std::optional<int64_t> Int_മൂല്യം = മൂല്യം.getAsInteger()) {
    എങ്കിൽ (*Int_മൂല്യം < static_cast<പൂർണ്ണസംഖ്യ>(CompletionItemKind::Text) ||
        *Int_മൂല്യം > static_cast<പൂർണ്ണസംഖ്യ>(CompletionItemKind::TypeParameter))
      തിരികെ അസത്യം;
    Result = static_cast<CompletionItemKind>(*Int_മൂല്യം);
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  എങ്കിൽ (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    തിരികെ Kind;

  // Provide some fall backs for common kinds that are close enough.
  തിരഞ്ഞെടുപ്പ് (Kind) {
  അവസ്ഥ CompletionItemKind::Folder:
    തിരികെ CompletionItemKind::File;
  അവസ്ഥ CompletionItemKind::EnumMember:
    തിരികെ CompletionItemKind::Enum;
  അവസ്ഥ CompletionItemKind::Struct:
    തിരികെ CompletionItemKind::Class;
  സ്ഥിരസ്ഥിതി:
    തിരികെ CompletionItemKind::Text;
  }
}

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  എങ്കിൽ (സ്ഥിരം llvm::json::Array *Array_മൂല്യം = മൂല്യം.getAsArray()) {
    വേണ്ടി (size_t I = 0, E = Array_മൂല്യം->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      എങ്കിൽ (fromJSON((*Array_മൂല്യം)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം CompletionItem &മൂല്യം) {
  പ്രതിജ്ഞാ(!മൂല്യം.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", മൂല്യം.label}};
  എങ്കിൽ (മൂല്യം.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<പൂർണ്ണസംഖ്യ>(മൂല്യം.kind);
  എങ്കിൽ (!മൂല്യം.detail.empty())
    Result["detail"] = മൂല്യം.detail;
  എങ്കിൽ (മൂല്യം.documentation)
    Result["documentation"] = മൂല്യം.documentation;
  എങ്കിൽ (!മൂല്യം.sortText.empty())
    Result["sortText"] = മൂല്യം.sortText;
  എങ്കിൽ (!മൂല്യം.filterText.empty())
    Result["filterText"] = മൂല്യം.filterText;
  എങ്കിൽ (!മൂല്യം.തിരുകുക_Text.empty())
    Result["insertText"] = മൂല്യം.തിരുകുക_Text;
  എങ്കിൽ (മൂല്യം.തിരുകുക_Text_Format != തിരുകുക_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<പൂർണ്ണസംഖ്യ>(മൂല്യം.തിരുകുക_Text_Format);
  എങ്കിൽ (മൂല്യം.textEdit)
    Result["textEdit"] = *മൂല്യം.textEdit;
  എങ്കിൽ (!മൂല്യം.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(മൂല്യം.additionalTextEdits);
  }
  എങ്കിൽ (മൂല്യം.deprecated)
    Result["deprecated"] = മൂല്യം.deprecated;
  തിരികെ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   സ്ഥിരം CompletionItem &മൂല്യം) {
  തിരികെ Os << മൂല്യം.label << " - " << toJSON(മൂല്യം);
}

തർക്കം llvm::lsp::operator<(സ്ഥിരം CompletionItem &Lhs,
                          സ്ഥിരം CompletionItem &Rhs) {
  തിരികെ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം CompletionList &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"isIncomplete", മൂല്യം.isIncomplete},
      {"items", llvm::json::Array(മൂല്യം.ഇനങ്ങൾ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  പൂർണ്ണസംഖ്യ TriggerKind;
  എങ്കിൽ (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(മൂല്യം, "triggerCharacter", Result.triggerCharacter, Path))
    തിരികെ അസത്യം;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CompletionParams &Result, llvm::json::Path Path) {
  എങ്കിൽ (!fromJSON(മൂല്യം, static_cast<TextDocumentPositionParams &>(Result), Path))
    തിരികെ അസത്യം;
  എങ്കിൽ (സ്ഥിരം llvm::json::മൂല്യം *Context = മൂല്യം.getAsObject()->get("context"))
    തിരികെ fromJSON(*Context, Result.context, Path.field("context"));
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം ParameterInformation &മൂല്യം) {
  പ്രതിജ്ഞാ((മൂല്യം.labelOffsets || !മൂല്യം.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  എങ്കിൽ (മൂല്യം.labelOffsets)
    Result["label"] = llvm::json::Array(
        {മൂല്യം.labelOffsets->first, മൂല്യം.labelOffsets->second});
  അല്ലെങ്കിൽ
    Result["label"] = മൂല്യം.labelString;
  എങ്കിൽ (!മൂല്യം.documentation.empty())
    Result["documentation"] = മൂല്യം.documentation;
  തിരികെ std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം SignatureInformation &മൂല്യം) {
  പ്രതിജ്ഞാ(!മൂല്യം.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", മൂല്യം.label},
      {"parameters", llvm::json::Array(മൂല്യം.parameters)},
  };
  എങ്കിൽ (!മൂല്യം.documentation.empty())
    Result["documentation"] = മൂല്യം.documentation;
  തിരികെ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   സ്ഥിരം SignatureInformation &മൂല്യം) {
  തിരികെ Os << മൂല്യം.label << " - " << toJSON(മൂല്യം);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം SignatureHelp &മൂല്യം) {
  പ്രതിജ്ഞാ(മൂല്യം.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  പ്രതിജ്ഞാ(മൂല്യം.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  തിരികെ llvm::json::Object{
      {"activeSignature", മൂല്യം.activeSignature},
      {"activeParameter", മൂല്യം.activeParameter},
      {"signatures", llvm::json::Array(മൂല്യം.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം DocumentLink &മൂല്യം) {
  തിരികെ llvm::json::Object{
      {"range", മൂല്യം.range},
      {"target", മൂല്യം.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം InlayHint &മൂല്യം) {
  തിരികെ llvm::json::Object{{"position", മൂല്യം.position},
                            {"kind", (പൂർണ്ണസംഖ്യ)മൂല്യം.kind},
                            {"label", മൂല്യം.label},
                            {"paddingLeft", മൂല്യം.padding_ഇടത്},
                            {"paddingRight", മൂല്യം.padding_വലത്}};
}
തർക്കം llvm::lsp::operator==(സ്ഥിരം InlayHint &Lhs, സ്ഥിരം InlayHint &Rhs) {
  തിരികെ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
തർക്കം llvm::lsp::operator<(സ്ഥിരം InlayHint &Lhs, സ്ഥിരം InlayHint &Rhs) {
  തിരികെ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind മൂല്യം) {
  തിരഞ്ഞെടുപ്പ് (മൂല്യം) {
  അവസ്ഥ InlayHintKind::Parameter:
    തിരികെ Os << "parameter";
  അവസ്ഥ InlayHintKind::Type:
    തിരികെ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  എങ്കിൽ (!O || !O.map("diagnostics", Result.diagnostics))
    തിരികെ അസത്യം;
  O.map("only", Result.only);
  തിരികെ സത്യം;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

തർക്കം llvm::lsp::fromJSON(സ്ഥിരം llvm::json::മൂല്യം &മൂല്യം, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(മൂല്യം, Path);
  തിരികെ O && O.map("changes", Result.changes);
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം WorkspaceEdit &മൂല്യം) {
  llvm::json::Object FileChanges;
  വേണ്ടി (auto &Change : മൂല്യം.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  തിരികെ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

സ്ഥിരം llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
സ്ഥിരം llvm::StringLiteral CodeAction::kRefactor = "refactor";
സ്ഥിരം llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം CodeAction &മൂല്യം) {
  llvm::json::Object CodeAction{{"title", മൂല്യം.title}};
  എങ്കിൽ (മൂല്യം.kind)
    CodeAction["kind"] = *മൂല്യം.kind;
  എങ്കിൽ (മൂല്യം.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*മൂല്യം.diagnostics);
  എങ്കിൽ (മൂല്യം.isPreferred)
    CodeAction["isPreferred"] = സത്യം;
  എങ്കിൽ (മൂല്യം.edit)
    CodeAction["edit"] = *മൂല്യം.edit;
  തിരികെ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<പൂർണ്ണസംഖ്യ>(Params.type)},
      {"message", Params.message},
  };
  എങ്കിൽ (Params.actions)
    Out["actions"] = *Params.actions;
  തിരികെ Out;
}

llvm::json::മൂല്യം llvm::lsp::toJSON(സ്ഥിരം MessageActionItem &Params) {
  തിരികെ llvm::json::Object{{"title", Params.title}};
}
