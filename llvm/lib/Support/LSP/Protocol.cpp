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

#அந்தர்பாவ "llvm/Support/LSP/Protocol.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringSet.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/JSON.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
நிலையான தருக்க mapOptOrNull(மாறிலி llvm::json::மதிப்பு &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  மாறிலி llvm::json::Object *O = Params.getAsObject();
  ப்ரதிஜ்ஞா(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  எனில் (!V || V->getAsNull())
    திரும்பு உண்மை;
  திரும்பு fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

எழுத்து LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

நிலையான தருக்க isWindowsPath(StringRef Path) {
  திரும்பு Path.size() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

நிலையான தருக்க isNetworkPath(StringRef Path) {
  திரும்பு Path.size() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

நிலையான தருக்க shouldEscapeInURI(unsigned எழுத்து C) {
  // Unreserved characters.
  எனில் ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    திரும்பு பொய்;

  தேர்வு (C) {
  நிலை '-':
  நிலை '_':
  நிலை '.':
  நிலை '~':
  // '/' is only reserved when parsing.
  நிலை '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  நிலை ':':
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
நிலையான வெறுமை percentEncode(StringRef Content, std::string &Out) {
  க்கு (unsigned எழுத்து C : Content) {
    எனில் (shouldEscapeInURI(C)) {
      Out.தள்ளு_back('%');
      Out.தள்ளு_back(llvm::hexdigit(C / 16));
      Out.தள்ளு_back(llvm::hexdigit(C % 16));
    } இல்லையெனில் {
      Out.தள்ளு_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
நிலையான std::string percentDecode(StringRef Content) {
  std::string Result;
  க்கு (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    எனில் (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      Result.தள்ளு_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } இல்லையெனில் {
      Result.தள்ளு_back(*I);
    }
  }
  திரும்பு Result;
}

/// Return the set containing the supported URI schemes.
நிலையான StringSet<> &getSupportedSchemes() {
  நிலையான StringSet<> Schemes({"file", "test"});
  திரும்பு Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
நிலையான தருக்க isStructurallyValidScheme(StringRef Scheme) {
  எனில் (Scheme.empty())
    திரும்பு பொய்;
  எனில் (!llvm::isAlpha(Scheme[0]))
    திரும்பு பொய்;
  திரும்பு llvm::all_of(llvm::drop_begin(Scheme), [](எழுத்து C) {
    திரும்பு llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

நிலையான llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef வேர்_2 = llvm::sys::path::வேர்_name(AbsolutePath);
  எனில் (isNetworkPath(வேர்_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = வேர்_2.drop_front(2);
    AbsolutePath.consume_front(வேர்_2);
  } இல்லையெனில் எனில் (isWindowsPath(வேர்_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  எனில் (Authority.empty() && Body.empty())
    திரும்பு Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  எனில் (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  திரும்பு Uri;
}

நிலையான llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  எனில் (!Body.starts_with("/"))
    திரும்பு llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  எனில் (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } இல்லையெனில் எனில் (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  திரும்பு std::string(Path);
}

நிலையான llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  எனில் (Pos == StringRef::npos)
    திரும்பு llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  எனில் (!isStructurallyValidScheme(UriScheme))
    திரும்பு llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  எனில் (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  எனில் (!getSupportedSchemes().contains(UriScheme)) {
    திரும்பு llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  திரும்பு getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  எனில் (!FilePath)
    திரும்பு FilePath.takeError();
  திரும்பு URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  எனில் (!Uri)
    திரும்பு Uri.takeError();
  திரும்பு fromURI(*Uri);
}

StringRef URIForFile::scheme() மாறிலி { திரும்பு uri().split(':').first; }

வெறுமை URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().செருகு(Scheme);
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, URIForFile &Result,
                         llvm::json::Path Path) {
  எனில் (std::optional<StringRef> Str = மதிப்பு.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    எனில் (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      திரும்பு பொய்;
    }
    Result = std::move(*ExpectedUri);
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி URIForFile &மதிப்பு) {
  திரும்பு மதிப்பு.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, மாறிலி URIForFile &மதிப்பு) {
  திரும்பு Os << மதிப்பு.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         ClientCapabilities &Result, llvm::json::Path Path) {
  மாறிலி llvm::json::Object *O = மதிப்பு.getAsObject();
  எனில் (!O) {
    Path.report("expected object");
    திரும்பு பொய்;
  }
  எனில் (மாறிலி llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    எனில் (மாறிலி llvm::json::Object *Document_குறியீடு =
            TextDocument->getObject("documentSymbol")) {
      எனில் (std::optional<தருக்க> HierarchicalSupport =
              Document_குறியீடு->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchical_Document_குறியீடு = *HierarchicalSupport;
    }
    எனில் (auto *CodeAction = TextDocument->getObject("codeAction")) {
      எனில் (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = உண்மை;
    }
  }
  எனில் (auto *Window = O->getObject("window")) {
    எனில் (std::optional<தருக்க> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, ClientInfo &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  எனில் (!O || !O.map("name", Result.name))
    திரும்பு பொய்;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, TraceLevel &Result,
                         llvm::json::Path Path) {
  எனில் (std::optional<StringRef> Str = மதிப்பு.getAsString()) {
    எனில் (*Str == "off") {
      Result = TraceLevel::Off;
      திரும்பு உண்மை;
    }
    எனில் (*Str == "messages") {
      Result = TraceLevel::Messages;
      திரும்பு உண்மை;
    }
    எனில் (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      திரும்பு உண்மை;
    }
  }
  திரும்பு பொய்;
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         InitializeParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  எனில் (!O)
    திரும்பு பொய்;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  O.map("rootUri", Result.வேர்_Uri);
  O.map("rootPath", Result.வேர்_Path);
  mapOptOrNull(மதிப்பு, "clientInfo", Result.clientInfo, Path);

  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         TextDocumentItem &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி TextDocumentIdentifier &மதிப்பு) {
  திரும்பு llvm::json::Object{{"uri", மதிப்பு.uri}};
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         TextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு
llvm::lsp::toJSON(மாறிலி VersionedTextDocumentIdentifier &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"uri", மதிப்பு.uri},
      {"version", மதிப்பு.version},
  };
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         VersionedTextDocumentIdentifier &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, Position &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("line", Result.வரி_2) &&
         O.map("character", Result.character);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Position &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"line", மதிப்பு.வரி_2},
      {"character", மதிப்பு.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, மாறிலி Position &மதிப்பு) {
  திரும்பு Os << மதிப்பு.வரி_2 << ':' << மதிப்பு.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, Range &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("start", Result.start) && O.map("end", Result.end);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Range &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"start", மதிப்பு.start},
      {"end", மதிப்பு.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, மாறிலி Range &மதிப்பு) {
  திரும்பு Os << மதிப்பு.start << '-' << மதிப்பு.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, Location &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Location &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"uri", மதிப்பு.uri},
      {"range", மதிப்பு.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, மாறிலி Location &மதிப்பு) {
  திரும்பு Os << மதிப்பு.range << '@' << மதிப்பு.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         TextDocumentPositionParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         ReferenceContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         ReferenceParams &Result, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு fromJSON(மதிப்பு, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         DidOpenTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         DidCloseTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  திரும்பு O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) மாறிலி {
  // If there is no range, the full document changed.
  எனில் (!range) {
    Contents = text;
    திரும்பு success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  எனில் (!RangeLoc.isValid())
    திரும்பு failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  திரும்பு success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  க்கு (மாறிலி auto &Change : Changes)
    எனில் (failed(Change.applyTo(Contents)))
      திரும்பு failure();
  திரும்பு success();
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         TextDocumentContentChangeEvent &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         DidChangeTextDocumentParams &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

நிலையான llvm::StringRef toTextKind(MarkupKind Kind) {
  தேர்வு (Kind) {
  நிலை MarkupKind::PlainText:
    திரும்பு "plaintext";
  நிலை MarkupKind::Markdown:
    திரும்பு "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  திரும்பு Os << toTextKind(Kind);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி MarkupContent &Mc) {
  எனில் (Mc.மதிப்பு_2.empty())
    திரும்பு nullptr;

  திரும்பு llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.மதிப்பு_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Hover &Hover) {
  llvm::json::Object Result{{"contents", toJSON(Hover.contents)}};
  எனில் (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  திரும்பு std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Document_குறியீடு &குறியீடு) {
  llvm::json::Object Result{{"name", குறியீடு.name},
                            {"kind", static_cast<முழு>(குறியீடு.kind)},
                            {"range", குறியீடு.range},
                            {"selectionRange", குறியீடு.selectionRange}};

  எனில் (!குறியீடு.detail.empty())
    Result["detail"] = குறியீடு.detail;
  எனில் (!குறியீடு.children.empty())
    Result["children"] = குறியீடு.children;
  திரும்பு std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         Document_குறியீடு_Params &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         DiagnosticRelatedInformation &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி DiagnosticRelatedInformation &Info) {
  திரும்பு llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(DiagnosticTag Tag) {
  திரும்பு static_cast<முழு>(Tag);
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, DiagnosticTag &Result,
                         llvm::json::Path Path) {
  எனில் (std::optional<int64_t> I = மதிப்பு.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    திரும்பு உண்மை;
  }

  திரும்பு பொய்;
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி Diagnostic &Diag) {
  llvm::json::Object Result{
      {"range", Diag.range},
      {"severity", (முழு)Diag.severity},
      {"message", Diag.message},
  };
  எனில் (Diag.category)
    Result["category"] = *Diag.category;
  எனில் (!Diag.source.empty())
    Result["source"] = Diag.source;
  எனில் (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  எனில் (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  திரும்பு std::move(Result);
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, Diagnostic &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  எனில் (!O)
    திரும்பு பொய்;
  முழு Severity = 0;
  எனில் (!mapOptOrNull(மதிப்பு, "severity", Severity, Path))
    திரும்பு பொய்;
  Result.severity = (DiagnosticSeverity)Severity;

  திரும்பு O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(மதிப்பு, "category", Result.category, Path) &&
         mapOptOrNull(மதிப்பு, "source", Result.source, Path) &&
         mapOptOrNull(மதிப்பு, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(மதிப்பு, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி PublishDiagnosticsParams &Params) {
  திரும்பு llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, TextEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி TextEdit &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"range", மதிப்பு.range},
      {"newText", மதிப்பு.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, மாறிலி TextEdit &மதிப்பு) {
  Os << மதிப்பு.range << " => \"";
  llvm::printEscapedString(மதிப்பு.newText, Os);
  திரும்பு Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CompletionItemKind &Result, llvm::json::Path Path) {
  எனில் (std::optional<int64_t> Int_மதிப்பு = மதிப்பு.getAsInteger()) {
    எனில் (*Int_மதிப்பு < static_cast<முழு>(CompletionItemKind::Text) ||
        *Int_மதிப்பு > static_cast<முழு>(CompletionItemKind::TypeParameter))
      திரும்பு பொய்;
    Result = static_cast<CompletionItemKind>(*Int_மதிப்பு);
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  எனில் (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    திரும்பு Kind;

  // Provide some fall backs for common kinds that are close enough.
  தேர்வு (Kind) {
  நிலை CompletionItemKind::Folder:
    திரும்பு CompletionItemKind::File;
  நிலை CompletionItemKind::EnumMember:
    திரும்பு CompletionItemKind::Enum;
  நிலை CompletionItemKind::Struct:
    திரும்பு CompletionItemKind::Class;
  இயல்பு:
    திரும்பு CompletionItemKind::Text;
  }
}

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CompletionItemKindBitset &Result,
                         llvm::json::Path Path) {
  எனில் (மாறிலி llvm::json::Array *Array_மதிப்பு = மதிப்பு.getAsArray()) {
    க்கு (size_t I = 0, E = Array_மதிப்பு->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      எனில் (fromJSON((*Array_மதிப்பு)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி CompletionItem &மதிப்பு) {
  ப்ரதிஜ்ஞா(!மதிப்பு.label.empty() && "completion item label is required");
  llvm::json::Object Result{{"label", மதிப்பு.label}};
  எனில் (மதிப்பு.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<முழு>(மதிப்பு.kind);
  எனில் (!மதிப்பு.detail.empty())
    Result["detail"] = மதிப்பு.detail;
  எனில் (மதிப்பு.documentation)
    Result["documentation"] = மதிப்பு.documentation;
  எனில் (!மதிப்பு.sortText.empty())
    Result["sortText"] = மதிப்பு.sortText;
  எனில் (!மதிப்பு.filterText.empty())
    Result["filterText"] = மதிப்பு.filterText;
  எனில் (!மதிப்பு.செருகு_Text.empty())
    Result["insertText"] = மதிப்பு.செருகு_Text;
  எனில் (மதிப்பு.செருகு_Text_Format != செருகு_Text_Format_2::Missing)
    Result["insertTextFormat"] = static_cast<முழு>(மதிப்பு.செருகு_Text_Format);
  எனில் (மதிப்பு.textEdit)
    Result["textEdit"] = *மதிப்பு.textEdit;
  எனில் (!மதிப்பு.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        llvm::json::Array(மதிப்பு.additionalTextEdits);
  }
  எனில் (மதிப்பு.deprecated)
    Result["deprecated"] = மதிப்பு.deprecated;
  திரும்பு std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   மாறிலி CompletionItem &மதிப்பு) {
  திரும்பு Os << மதிப்பு.label << " - " << toJSON(மதிப்பு);
}

தருக்க llvm::lsp::operator<(மாறிலி CompletionItem &Lhs,
                          மாறிலி CompletionItem &Rhs) {
  திரும்பு (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி CompletionList &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"isIncomplete", மதிப்பு.isIncomplete},
      {"items", llvm::json::Array(மதிப்பு.பொருட்கள்)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CompletionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  முழு TriggerKind;
  எனில் (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(மதிப்பு, "triggerCharacter", Result.triggerCharacter, Path))
    திரும்பு பொய்;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CompletionParams &Result, llvm::json::Path Path) {
  எனில் (!fromJSON(மதிப்பு, static_cast<TextDocumentPositionParams &>(Result), Path))
    திரும்பு பொய்;
  எனில் (மாறிலி llvm::json::மதிப்பு *Context = மதிப்பு.getAsObject()->get("context"))
    திரும்பு fromJSON(*Context, Result.context, Path.field("context"));
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி ParameterInformation &மதிப்பு) {
  ப்ரதிஜ்ஞா((மதிப்பு.labelOffsets || !மதிப்பு.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object Result;
  எனில் (மதிப்பு.labelOffsets)
    Result["label"] = llvm::json::Array(
        {மதிப்பு.labelOffsets->first, மதிப்பு.labelOffsets->second});
  இல்லையெனில்
    Result["label"] = மதிப்பு.labelString;
  எனில் (!மதிப்பு.documentation.empty())
    Result["documentation"] = மதிப்பு.documentation;
  திரும்பு std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி SignatureInformation &மதிப்பு) {
  ப்ரதிஜ்ஞா(!மதிப்பு.label.empty() && "signature information label is required");
  llvm::json::Object Result{
      {"label", மதிப்பு.label},
      {"parameters", llvm::json::Array(மதிப்பு.parameters)},
  };
  எனில் (!மதிப்பு.documentation.empty())
    Result["documentation"] = மதிப்பு.documentation;
  திரும்பு std::move(Result);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   மாறிலி SignatureInformation &மதிப்பு) {
  திரும்பு Os << மதிப்பு.label << " - " << toJSON(மதிப்பு);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி SignatureHelp &மதிப்பு) {
  ப்ரதிஜ்ஞா(மதிப்பு.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ப்ரதிஜ்ஞா(மதிப்பு.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  திரும்பு llvm::json::Object{
      {"activeSignature", மதிப்பு.activeSignature},
      {"activeParameter", மதிப்பு.activeParameter},
      {"signatures", llvm::json::Array(மதிப்பு.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         DocumentLinkParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி DocumentLink &மதிப்பு) {
  திரும்பு llvm::json::Object{
      {"range", மதிப்பு.range},
      {"target", மதிப்பு.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         InlayHintsParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி InlayHint &மதிப்பு) {
  திரும்பு llvm::json::Object{{"position", மதிப்பு.position},
                            {"kind", (முழு)மதிப்பு.kind},
                            {"label", மதிப்பு.label},
                            {"paddingLeft", மதிப்பு.padding_இடது},
                            {"paddingRight", மதிப்பு.padding_வலது}};
}
தருக்க llvm::lsp::operator==(மாறிலி InlayHint &Lhs, மாறிலி InlayHint &Rhs) {
  திரும்பு std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
தருக்க llvm::lsp::operator<(மாறிலி InlayHint &Lhs, மாறிலி InlayHint &Rhs) {
  திரும்பு std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind மதிப்பு) {
  தேர்வு (மதிப்பு) {
  நிலை InlayHintKind::Parameter:
    திரும்பு Os << "parameter";
  நிலை InlayHintKind::Type:
    திரும்பு Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CodeActionContext &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  எனில் (!O || !O.map("diagnostics", Result.diagnostics))
    திரும்பு பொய்;
  O.map("only", Result.only);
  திரும்பு உண்மை;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு,
                         CodeActionParams &Result, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

தருக்க llvm::lsp::fromJSON(மாறிலி llvm::json::மதிப்பு &மதிப்பு, WorkspaceEdit &Result,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(மதிப்பு, Path);
  திரும்பு O && O.map("changes", Result.changes);
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி WorkspaceEdit &மதிப்பு) {
  llvm::json::Object FileChanges;
  க்கு (auto &Change : மதிப்பு.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  திரும்பு llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

மாறிலி llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
மாறிலி llvm::StringLiteral CodeAction::kRefactor = "refactor";
மாறிலி llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி CodeAction &மதிப்பு) {
  llvm::json::Object CodeAction{{"title", மதிப்பு.title}};
  எனில் (மதிப்பு.kind)
    CodeAction["kind"] = *மதிப்பு.kind;
  எனில் (மதிப்பு.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*மதிப்பு.diagnostics);
  எனில் (மதிப்பு.isPreferred)
    CodeAction["isPreferred"] = உண்மை;
  எனில் (மதிப்பு.edit)
    CodeAction["edit"] = *மதிப்பு.edit;
  திரும்பு std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<முழு>(Params.type)},
      {"message", Params.message},
  };
  எனில் (Params.actions)
    Out["actions"] = *Params.actions;
  திரும்பு Out;
}

llvm::json::மதிப்பு llvm::lsp::toJSON(மாறிலி MessageActionItem &Params) {
  திரும்பு llvm::json::Object{{"title", Params.title}};
}
