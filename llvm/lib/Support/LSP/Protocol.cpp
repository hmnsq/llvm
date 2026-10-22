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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/LSP/Protocol.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringSet.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/JSON.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
ସ୍ଥିର ତାର୍କିକ mapOptOrNull(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ଅଚଳ llvm::json::Object *O = Params.getAsObject();
  ପ୍ରତିଜ୍ଞା(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  ଯଦି (!V || V->getAsNull())
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

ଅକ୍ଷର LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

ସ୍ଥିର ତାର୍କିକ isWindowsPath(StringRef Path) {
  ଫେରନ୍ତୁ Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

ସ୍ଥିର ତାର୍କିକ isNetworkPath(StringRef Path) {
  ଫେରନ୍ତୁ Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

ସ୍ଥିର ତାର୍କିକ shouldEscapeInURI(unsigned ଅକ୍ଷର C) {
  // Unreserved characters.
  ଯଦି ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ବାଛ (C) {
  ସ୍ଥିତି '-':
  ସ୍ଥିତି '_':
  ସ୍ଥିତି '.':
  ସ୍ଥିତି '~':
  // '/' is only reserved when parsing.
  ସ୍ଥିତି '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  ସ୍ଥିତି ':':
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
ସ୍ଥିର ଶୂନ୍ୟ percentEncode(StringRef Content, std::string &Out) {
  ପାଇଁ (unsigned ଅକ୍ଷର C : Content) {
    ଯଦି (shouldEscapeInURI(C)) {
      Out.ଠେଲ_back('%');
      Out.ଠେଲ_back(llvm::hexdigit(C / 16));
      Out.ଠେଲ_back(llvm::hexdigit(C % 16));
    } ଅନ୍ୟଥା {
      Out.ଠେଲ_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
ସ୍ଥିର std::string percentDecode(StringRef Content) {
  std::string Result;
  ପାଇଁ (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    ଯଦି (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.ଠେଲ_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } ଅନ୍ୟଥା {
      Result.ଠେଲ_back(*I);
    }
  }
  ଫେରନ୍ତୁ Result;
}

/// Return the set containing the supported URI schemes.
ସ୍ଥିର StringSet<> &getSupportedSchemes() {
  ସ୍ଥିର StringSet<> Schemes({"file", "test"});
  ଫେରନ୍ତୁ Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
ସ୍ଥିର ତାର୍କିକ isStructurallyValidScheme(StringRef Scheme) {
  ଯଦି (Scheme.empty())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (!llvm::isAlpha(Scheme[0]))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ llvm::all_of(llvm::drop_begin(Scheme), [](ଅକ୍ଷର C) {
    ଫେରନ୍ତୁ llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

ସ୍ଥିର llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef ମୂଳ_2 = llvm::sys::path::ମୂଳ_name(AbsolutePath);
  ଯଦି (isNetworkPath(ମୂଳ_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = ମୂଳ_2.drop_front(2);
    AbsolutePath.consume_front(ମୂଳ_2);
  } ଅନ୍ୟଥା ଯଦି (isWindowsPath(ମୂଳ_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  ଯଦି (Authority.empty() && Body.empty())
    ଫେରନ୍ତୁ Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ଯଦି (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ଫେରନ୍ତୁ Uri;
}

ସ୍ଥିର llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  ଯଦି (!Body.starts_with("/"))
    ଫେରନ୍ତୁ llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  ଯଦି (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } ଅନ୍ୟଥା ଯଦି (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ଫେରନ୍ତୁ std::string(Path);
}

ସ୍ଥିର llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  ଯଦି (Pos == StringRef::npos)
    ଫେରନ୍ତୁ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ଯଦି (!isStructurallyValidScheme(UriScheme))
    ଫେରନ୍ତୁ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ଯଦି (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ଯଦି (!getSupportedSchemes().contains(UriScheme)) {
    ଫେରନ୍ତୁ llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ଫେରନ୍ତୁ getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  ଯଦି (!FilePath)
    ଫେରନ୍ତୁ FilePath.takeError();
  ଫେରନ୍ତୁ URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  ଯଦି (!Uri)
    ଫେରନ୍ତୁ Uri.takeError();
  ଫେରନ୍ତୁ fromURI(*Uri);
}

StringRef URIForFile::scheme() ଅଚଳ { ଫେରନ୍ତୁ uri().split(':').first; }

ଶୂନ୍ୟ URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ଭର୍ତ୍ତି(Scheme);
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, URIForFile &Result,
                         llvm::json::Path Path) {
  ଯଦି (std::optional<StringRef> Str = ମୂଲ୍ୟ.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ଯଦି (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    Result = std::move(*ExpectedUri);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ URIForFile &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ ମୂଲ୍ୟ.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ଅଚଳ URIForFile &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  ଅଚଳ llvm::json::Object *O = ମୂଲ୍ୟ.getAsObject();
  ଯଦି (!O) {
    Path.report("expected object");
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଯଦି (ଅଚଳ llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    ଯଦି (ଅଚଳ llvm::json::Object *Document_ପ୍ରତୀକ =
            TextDocument->getObject("documentSymbol")) {
      ଯଦି (std::optional<ତାର୍କିକ> HierarchicalSupport =
              Document_ପ୍ରତୀକ->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_ପ୍ରତୀକ = *HierarchicalSupport;
    }
    ଯଦି (auto *CodeAction = TextDocument->getObject("codeAction")) {
      ଯଦି (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = ସତ୍ୟ;
    }
  }
  ଯଦି (auto *Window = O->getObject("window")) {
    ଯଦି (std::optional<ତାର୍କିକ> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଯଦି (!O || !O.map("name", Result.name))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, TraceLevel &Result,
                         llvm::json::Path Path) {
  ଯଦି (std::optional<StringRef> Str = ମୂଲ୍ୟ.getAsString()) {
    ଯଦି (*Str == "off") {
      Result = TraceLevel::Off;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ଯଦି (*Str == "messages") {
      Result = TraceLevel::Messages;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ଯଦି (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଯଦି (!O)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.ମୂଳ_Uri);
  O.map("rootPath", Result.ମୂଳ_Path);
  mapOptOrNull(ମୂଲ୍ୟ, "clientInfo", Result.clientInfo, Path);

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ TextDocumentIdentifier &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{{"uri", ମୂଲ୍ୟ.uri}};
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ
llvm::lsp::toJSON(ଅଚଳ VersionedTextDocumentIdentifier &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"uri", ମୂଲ୍ୟ.uri},
      {"version", ମୂଲ୍ୟ.version},
  };
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("line", Result.ଧାଡ଼ି_2) &&
         O.map("character", Result.character);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Position &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"line", ମୂଲ୍ୟ.ଧାଡ଼ି_2},
      {"character", ମୂଲ୍ୟ.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ଅଚଳ Position &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.ଧାଡ଼ି_2 << ':' << ମୂଲ୍ୟ.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Range &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"start", ମୂଲ୍ୟ.start},
      {"end", ମୂଲ୍ୟ.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ଅଚଳ Range &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.start << '-' << ମୂଲ୍ୟ.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Location &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"uri", ମୂଲ୍ୟ.uri},
      {"range", ମୂଲ୍ୟ.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ଅଚଳ Location &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.range << '@' << ମୂଲ୍ୟ.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ fromJSON(ମୂଲ୍ୟ, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ଫେରନ୍ତୁ O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ଅଚଳ {
  // If there is no range, the full document changed.
  ଯଦି (!range) {
    Contents = text;
    ଫେରନ୍ତୁ success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ଯଦି (!RangeLoc.isValid())
    ଫେରନ୍ତୁ failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ଫେରନ୍ତୁ success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  ପାଇଁ (ଅଚଳ auto &Change : Changes)
    ଯଦି (failed(Change.applyTo(Contents)))
      ଫେରନ୍ତୁ failure();
  ଫେରନ୍ତୁ success();
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

ସ୍ଥିର llvm::StringRef toTextKind(MarkupKind Kind) {
  ବାଛ (Kind) {
  ସ୍ଥିତି MarkupKind::PlainText:
    ଫେରନ୍ତୁ "plaintext";
  ସ୍ଥିତି MarkupKind::Markdown:
    ଫେରନ୍ତୁ "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ଫେରନ୍ତୁ Os << toTextKind(Kind);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ MarkupContent &Mc) {
  ଯଦି (Mc.ମୂଲ୍ୟ_2.empty())
    ଫେରନ୍ତୁ nullptr;

  ଫେରନ୍ତୁ llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.ମୂଲ୍ୟ_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  ଯଦି (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  ଫେରନ୍ତୁ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Document_ପ୍ରତୀକ &ପ୍ରତୀକ) {
  llvm::json::Object Result{{"name", ପ୍ରତୀକ.name},
                            {"kind", static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(ପ୍ରତୀକ.kind)},
                            {"range", ପ୍ରତୀକ.range},
                            {"selectionRange", ପ୍ରତୀକ.selectionRange}};

  ଯଦି (!ପ୍ରତୀକ.detail.empty())
    Result["detail"] = ପ୍ରତୀକ.detail;
  ଯଦି (!ପ୍ରତୀକ.children.empty())
    Result["children"] = ପ୍ରତୀକ.children;
  ଫେରନ୍ତୁ std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         Document_ପ୍ରତୀକ_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ DiagnosticRelatedInformation &Info) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(DiagnosticTag Tag) {
  ଫେରନ୍ତୁ static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(Tag);
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  ଯଦି (std::optional<int64_t> I = ମୂଲ୍ୟ.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)Diag.severity},
      {"message", Diag.message},
  };
  ଯଦି (Diag.category)
    Result["category"] = *Diag.category;
  ଯଦି (!Diag.source.empty())
    Result["source"] = Diag.source;
  ଯଦି (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  ଯଦି (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  ଫେରନ୍ତୁ std::move(Result);
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଯଦି (!O)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Severity = 0;
  ଯଦି (!mapOptOrNull(ମୂଲ୍ୟ, "severity", Severity, Path))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  Result.severity = (DiagnosticSeverity)Severity;

  ଫେରନ୍ତୁ O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(ମୂଲ୍ୟ, "category", Result.category, Path) &&
         mapOptOrNull(ମୂଲ୍ୟ, "source", Result.source, Path) &&
         mapOptOrNull(ମୂଲ୍ୟ, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(ମୂଲ୍ୟ, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ PublishDiagnosticsParams &Params) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ TextEdit &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"range", ମୂଲ୍ୟ.range},
      {"newText", ମୂଲ୍ୟ.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ଅଚଳ TextEdit &ମୂଲ୍ୟ) {
  Os << ମୂଲ୍ୟ.range << " => \"";
  llvm::printEscapedString(ମୂଲ୍ୟ.newText, Os);
  ଫେରନ୍ତୁ Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  ଯଦି (std::optional<int64_t> Int_ମୂଲ୍ୟ = ମୂଲ୍ୟ.getAsInteger()) {
    ଯଦି (*Int_ମୂଲ୍ୟ < static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(CompletionItemKind::Text) ||
        *Int_ମୂଲ୍ୟ > static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(CompletionItemKind::TypeParameter))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    Result = static_cast<CompletionItemKind>(*Int_ମୂଲ୍ୟ);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  ଯଦି (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    ଫେରନ୍ତୁ Kind;

  // Provide some fall backs for common kinds that are close enough.
  ବାଛ (Kind) {
  ସ୍ଥିତି CompletionItemKind::Folder:
    ଫେରନ୍ତୁ CompletionItemKind::File;
  ସ୍ଥିତି CompletionItemKind::EnumMember:
    ଫେରନ୍ତୁ CompletionItemKind::Enum;
  ସ୍ଥିତି CompletionItemKind::Struct:
    ଫେରନ୍ତୁ CompletionItemKind::Class;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ CompletionItemKind::Text;
  }
}

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  ଯଦି (ଅଚଳ llvm::json::Array *Array_ମୂଲ୍ୟ = ମୂଲ୍ୟ.getAsArray()) {
    ପାଇଁ (size_t I = 0, E = Array_ମୂଲ୍ୟ->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      ଯଦି (fromJSON((*Array_ମୂଲ୍ୟ)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ CompletionItem &ମୂଲ୍ୟ) {
  ପ୍ରତିଜ୍ଞା(!ମୂଲ୍ୟ.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", ମୂଲ୍ୟ.label}};
  ଯଦି (ମୂଲ୍ୟ.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(ମୂଲ୍ୟ.kind);
  ଯଦି (!ମୂଲ୍ୟ.detail.empty())
    Result["detail"] = ମୂଲ୍ୟ.detail;
  ଯଦି (ମୂଲ୍ୟ.documentation)
    Result["documentation"] = ମୂଲ୍ୟ.documentation;
  ଯଦି (!ମୂଲ୍ୟ.sortText.empty())
    Result["sortText"] = ମୂଲ୍ୟ.sortText;
  ଯଦି (!ମୂଲ୍ୟ.filterText.empty())
    Result["filterText"] = ମୂଲ୍ୟ.filterText;
  ଯଦି (!ମୂଲ୍ୟ.ଭର୍ତ୍ତି_Text.empty())
    Result["insertText"] = ମୂଲ୍ୟ.ଭର୍ତ୍ତି_Text;
  ଯଦି (ମୂଲ୍ୟ.ଭର୍ତ୍ତି_Text_Format != ଭର୍ତ୍ତି_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(ମୂଲ୍ୟ.ଭର୍ତ୍ତି_Text_Format);
  ଯଦି (ମୂଲ୍ୟ.textEdit)
    Result["textEdit"] = *ମୂଲ୍ୟ.textEdit;
  ଯଦି (!ମୂଲ୍ୟ.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(ମୂଲ୍ୟ.additionalTextEdits);
  }
  ଯଦି (ମୂଲ୍ୟ.deprecated)
    Result["deprecated"] = ମୂଲ୍ୟ.deprecated;
  ଫେରନ୍ତୁ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ଅଚଳ CompletionItem &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.label << " - " << toJSON(ମୂଲ୍ୟ);
}

ତାର୍କିକ llvm::lsp::operator<(ଅଚଳ CompletionItem &Lhs,
                          ଅଚଳ CompletionItem &Rhs) {
  ଫେରନ୍ତୁ (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ CompletionList &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"isIncomplete", ମୂଲ୍ୟ.isIncomplete},
      {"items", llvm::json::Array(ମୂଲ୍ୟ.ବସ୍ତୁଗୁଡ଼ିକ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା TriggerKind;
  ଯଦି (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(ମୂଲ୍ୟ, "triggerCharacter", Result.triggerCharacter, Path))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CompletionParams &Result, llvm::json::Path Path) {
  ଯଦି (!fromJSON(ମୂଲ୍ୟ, static_cast<TextDocumentPositionParams &>(Result), Path))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (ଅଚଳ llvm::json::ମୂଲ୍ୟ *Context = ମୂଲ୍ୟ.getAsObject()->get("context"))
    ଫେରନ୍ତୁ fromJSON(*Context, Result.context, Path.field("context"));
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ ParameterInformation &ମୂଲ୍ୟ) {
  ପ୍ରତିଜ୍ଞା((ମୂଲ୍ୟ.labelOffsets || !ମୂଲ୍ୟ.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  ଯଦି (ମୂଲ୍ୟ.labelOffsets)
    Result["label"] = llvm::json::Array(
        {ମୂଲ୍ୟ.labelOffsets->first, ମୂଲ୍ୟ.labelOffsets->second});
  ଅନ୍ୟଥା
    Result["label"] = ମୂଲ୍ୟ.labelString;
  ଯଦି (!ମୂଲ୍ୟ.documentation.empty())
    Result["documentation"] = ମୂଲ୍ୟ.documentation;
  ଫେରନ୍ତୁ std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ SignatureInformation &ମୂଲ୍ୟ) {
  ପ୍ରତିଜ୍ଞା(!ମୂଲ୍ୟ.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", ମୂଲ୍ୟ.label},
      {"parameters", llvm::json::Array(ମୂଲ୍ୟ.parameters)},
  };
  ଯଦି (!ମୂଲ୍ୟ.documentation.empty())
    Result["documentation"] = ମୂଲ୍ୟ.documentation;
  ଫେରନ୍ତୁ std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ଅଚଳ SignatureInformation &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ Os << ମୂଲ୍ୟ.label << " - " << toJSON(ମୂଲ୍ୟ);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ SignatureHelp &ମୂଲ୍ୟ) {
  ପ୍ରତିଜ୍ଞା(ମୂଲ୍ୟ.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ପ୍ରତିଜ୍ଞା(ମୂଲ୍ୟ.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ଫେରନ୍ତୁ llvm::json::Object{
      {"activeSignature", ମୂଲ୍ୟ.activeSignature},
      {"activeParameter", ମୂଲ୍ୟ.activeParameter},
      {"signatures", llvm::json::Array(ମୂଲ୍ୟ.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ DocumentLink &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{
      {"range", ମୂଲ୍ୟ.range},
      {"target", ମୂଲ୍ୟ.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ InlayHint &ମୂଲ୍ୟ) {
  ଫେରନ୍ତୁ llvm::json::Object{{"position", ମୂଲ୍ୟ.position},
                            {"kind", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ମୂଲ୍ୟ.kind},
                            {"label", ମୂଲ୍ୟ.label},
                            {"paddingLeft", ମୂଲ୍ୟ.padding_ବାମ},
                            {"paddingRight", ମୂଲ୍ୟ.padding_ଡାହାଣ}};
}
ତାର୍କିକ llvm::lsp::operator==(ଅଚଳ InlayHint &Lhs, ଅଚଳ InlayHint &Rhs) {
  ଫେରନ୍ତୁ std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
ତାର୍କିକ llvm::lsp::operator<(ଅଚଳ InlayHint &Lhs, ଅଚଳ InlayHint &Rhs) {
  ଫେରନ୍ତୁ std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind ମୂଲ୍ୟ) {
  ବାଛ (ମୂଲ୍ୟ) {
  ସ୍ଥିତି InlayHintKind::Parameter:
    ଫେରନ୍ତୁ Os << "parameter";
  ସ୍ଥିତି InlayHintKind::Type:
    ଫେରନ୍ତୁ Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଯଦି (!O || !O.map("diagnostics", Result.diagnostics))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  O.map("only", Result.only);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

ତାର୍କିକ llvm::lsp::fromJSON(ଅଚଳ llvm::json::ମୂଲ୍ୟ &ମୂଲ୍ୟ, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(ମୂଲ୍ୟ, Path);
  ଫେରନ୍ତୁ O && O.map("changes", Result.changes);
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ WorkspaceEdit &ମୂଲ୍ୟ) {
  llvm::json::Object FileChanges;
  ପାଇଁ (auto &Change : ମୂଲ୍ୟ.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ଫେରନ୍ତୁ llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ଅଚଳ llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ଅଚଳ llvm::StringLiteral CodeAction::kRefactor = "refactor";
ଅଚଳ llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ CodeAction &ମୂଲ୍ୟ) {
  llvm::json::Object CodeAction{{"title", ମୂଲ୍ୟ.title}};
  ଯଦି (ମୂଲ୍ୟ.kind)
    CodeAction["kind"] = *ମୂଲ୍ୟ.kind;
  ଯଦି (ମୂଲ୍ୟ.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*ମୂଲ୍ୟ.diagnostics);
  ଯଦି (ମୂଲ୍ୟ.isPreferred)
    CodeAction["isPreferred"] = ସତ୍ୟ;
  ଯଦି (ମୂଲ୍ୟ.edit)
    CodeAction["edit"] = *ମୂଲ୍ୟ.edit;
  ଫେରନ୍ତୁ std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<ପୂର୍ଣ୍ଣସଂଖ୍ୟା>(Params.type)},
      {"message", Params.message},
  };
  ଯଦି (Params.actions)
    Out["actions"] = *Params.actions;
  ଫେରନ୍ତୁ Out;
}

llvm::json::ମୂଲ୍ୟ llvm::lsp::toJSON(ଅଚଳ MessageActionItem &Params) {
  ଫେରନ୍ତୁ llvm::json::Object{{"title", Params.title}};
}
