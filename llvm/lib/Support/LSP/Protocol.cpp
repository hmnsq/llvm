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

#অন্তর্ভাব "llvm/Support/LSP/Protocol.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringSet.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/JSON.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/Path.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
স্থির যৌক্তিক mapOptOrNull(ধ্রুবক llvm::json::মান &Params,
                         llvm::StringLiteral Prop, T &Out,
                         llvm::json::Path Path) {
  ধ্রুবক llvm::json::Object *O = Params.getAsObject();
  প্রতিজ্ঞা(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  যদি (!V || V->getAsNull())
    ফেরত সত্য;
  ফেরত fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

অক্ষর LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

স্থির যৌক্তিক isWindowsPath(StringRef Path) {
  ফেরত Path.আকার_3() > 1 && llvm::isAlpha(Path[0]) && Path[1] == ':';
}

স্থির যৌক্তিক isNetworkPath(StringRef Path) {
  ফেরত Path.আকার_3() > 2 && Path[0] == Path[1] &&
         llvm::sys::path::is_separator(Path[0]);
}

স্থির যৌক্তিক shouldEscapeInURI(unsigned অক্ষর C) {
  // Unreserved characters.
  যদি ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    ফেরত মিথ্যা;

  নির্বাচন (C) {
  ক্ষেত্র '-':
  ক্ষেত্র '_':
  ক্ষেত্র '.':
  ক্ষেত্র '~':
  // '/' is only reserved when parsing.
  ক্ষেত্র '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  ক্ষেত্র ':':
    ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
স্থির শূন্য percentEncode(StringRef Content, std::string &Out) {
  জন্য (unsigned অক্ষর C : Content) {
    যদি (shouldEscapeInURI(C)) {
      Out.ঠেলো_back('%');
      Out.ঠেলো_back(llvm::hexdigit(C / 16));
      Out.ঠেলো_back(llvm::hexdigit(C % 16));
    } অন্যথা {
      Out.ঠেলো_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
স্থির std::string percentDecode(StringRef Content) {
  std::string ফলাফল;
  জন্য (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    যদি (*I == '%' && I + 2 < Content.end() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      ফলাফল.ঠেলো_back(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } অন্যথা {
      ফলাফল.ঠেলো_back(*I);
    }
  }
  ফেরত ফলাফল;
}

/// Return the set containing the supported URI schemes.
স্থির StringSet<> &getSupportedSchemes() {
  স্থির StringSet<> Schemes({"file", "test"});
  ফেরত Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
স্থির যৌক্তিক isStructurallyValidScheme(StringRef Scheme) {
  যদি (Scheme.empty())
    ফেরত মিথ্যা;
  যদি (!llvm::isAlpha(Scheme[0]))
    ফেরত মিথ্যা;
  ফেরত llvm::all_of(llvm::drop_begin(Scheme), [](অক্ষর C) {
    ফেরত llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

স্থির llvm::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef মূল_2 = llvm::sys::path::মূল_name(AbsolutePath);
  যদি (isNetworkPath(মূল_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = মূল_2.drop_front(2);
    AbsolutePath.consume_front(মূল_2);
  } অন্যথা যদি (isWindowsPath(মূল_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  যদি (Authority.empty() && Body.empty())
    ফেরত Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  যদি (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  ফেরত Uri;
}

স্থির llvm::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  যদি (!Body.starts_with("/"))
    ফেরত llvm::createStringError(
        llvm::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  যদি (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } অন্যথা যদি (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  llvm::sys::path::native(Path);
  ফেরত std::string(Path);
}

স্থির llvm::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  যদি (Pos == StringRef::npos)
    ফেরত llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  যদি (!isStructurallyValidScheme(UriScheme))
    ফেরত llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  যদি (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  যদি (!getSupportedSchemes().contains(UriScheme)) {
    ফেরত llvm::createStringError(llvm::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  ফেরত getAbsolutePath(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  যদি (!FilePath)
    ফেরত FilePath.takeError();
  ফেরত URIForFile(std::move(*FilePath), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  যদি (!Uri)
    ফেরত Uri.takeError();
  ফেরত fromURI(*Uri);
}

StringRef URIForFile::scheme() ধ্রুবক { ফেরত uri().split(':').first; }

শূন্য URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().ঢোকাও(Scheme);
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, URIForFile &ফলাফল,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    যদি (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      ফেরত মিথ্যা;
    }
    ফলাফল = std::move(*ExpectedUri);
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক URIForFile &মান) {
  ফেরত মান.uri();
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুবক URIForFile &মান) {
  ফেরত Os << মান.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         ClientCapabilities &ফলাফল, llvm::json::Path Path) {
  ধ্রুবক llvm::json::Object *O = মান.getAsObject();
  যদি (!O) {
    Path.report("expected object");
    ফেরত মিথ্যা;
  }
  যদি (ধ্রুবক llvm::json::Object *TextDocument = O->getObject("textDocument")) {
    যদি (ধ্রুবক llvm::json::Object *Document_প্রতীক =
            TextDocument->getObject("documentSymbol")) {
      যদি (std::optional<যৌক্তিক> HierarchicalSupport =
              Document_প্রতীক->getBoolean("hierarchicalDocumentSymbolSupport"))
        ফলাফল.hierarchical_Document_প্রতীক = *HierarchicalSupport;
    }
    যদি (auto *CodeAction = TextDocument->getObject("codeAction")) {
      যদি (CodeAction->getObject("codeActionLiteralSupport"))
        ফলাফল.codeActionStructure = সত্য;
    }
  }
  যদি (auto *Window = O->getObject("window")) {
    যদি (std::optional<যৌক্তিক> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      ফলাফল.workDoneProgress = *WorkDoneProgressSupport;
  }
  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, ClientInfo &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("name", ফলাফল.name))
    ফেরত মিথ্যা;

  // Don't fail if we can't parse version.
  O.map("version", ফলাফল.version);
  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, TraceLevel &ফলাফল,
                         llvm::json::Path Path) {
  যদি (std::optional<StringRef> Str = মান.getAsString()) {
    যদি (*Str == "off") {
      ফলাফল = TraceLevel::Off;
      ফেরত সত্য;
    }
    যদি (*Str == "messages") {
      ফলাফল = TraceLevel::Messages;
      ফেরত সত্য;
    }
    যদি (*Str == "verbose") {
      ফলাফল = TraceLevel::Verbose;
      ফেরত সত্য;
    }
  }
  ফেরত মিথ্যা;
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         InitializeParams &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    ফেরত মিথ্যা;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", ফলাফল.capabilities);
  O.map("trace", ফলাফল.trace);
  O.map("rootUri", ফলাফল.মূল_Uri);
  O.map("rootPath", ফলাফল.মূল_Path);
  mapOptOrNull(মান, "clientInfo", ফলাফল.clientInfo, Path);

  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         TextDocumentItem &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("uri", ফলাফল.uri) &&
         O.map("languageId", ফলাফল.languageId) && O.map("text", ফলাফল.text) &&
         O.map("version", ফলাফল.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক TextDocumentIdentifier &মান) {
  ফেরত llvm::json::Object{{"uri", মান.uri}};
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         TextDocumentIdentifier &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("uri", ফলাফল.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::মান
llvm::lsp::toJSON(ধ্রুবক VersionedTextDocumentIdentifier &মান) {
  ফেরত llvm::json::Object{
      {"uri", মান.uri},
      {"version", মান.version},
  };
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         VersionedTextDocumentIdentifier &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("uri", ফলাফল.uri) && O.map("version", ফলাফল.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, Position &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("line", ফলাফল.সারি_2) &&
         O.map("character", ফলাফল.character);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Position &মান) {
  ফেরত llvm::json::Object{
      {"line", মান.সারি_2},
      {"character", মান.character},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুবক Position &মান) {
  ফেরত Os << মান.সারি_2 << ':' << মান.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, Range &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("start", ফলাফল.start) && O.map("end", ফলাফল.end);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Range &মান) {
  ফেরত llvm::json::Object{
      {"start", মান.start},
      {"end", মান.end},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুবক Range &মান) {
  ফেরত Os << মান.start << '-' << মান.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, Location &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("uri", ফলাফল.uri) && O.map("range", ফলাফল.range);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Location &মান) {
  ফেরত llvm::json::Object{
      {"uri", মান.uri},
      {"range", মান.range},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুবক Location &মান) {
  ফেরত Os << মান.range << '@' << মান.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         TextDocumentPositionParams &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument) &&
         O.map("position", ফলাফল.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         ReferenceContext &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.mapOptional("includeDeclaration", ফলাফল.includeDeclaration);
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         ReferenceParams &ফলাফল, llvm::json::Path Path) {
  TextDocumentPositionParams &Base = ফলাফল;
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত fromJSON(মান, Base, Path) && O &&
         O.mapOptional("context", ফলাফল.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         DidOpenTextDocumentParams &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         DidCloseTextDocumentParams &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &Params,
                         DidSaveTextDocumentParams &R, llvm::json::Path P) {
  llvm::json::ObjectMapper O(Params, P);
  ফেরত O && O.map("textDocument", R.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_ফলাফল
TextDocumentContentChangeEvent::applyTo(std::string &Contents) ধ্রুবক {
  // If there is no range, the full document changed.
  যদি (!range) {
    Contents = text;
    ফেরত success();
  }

  // Try to map the replacement range to the content.
  llvm::SourceMgr TmpScrMgr;
  TmpScrMgr.Add_New_Source_বাফার(llvm::Memory_বাফার::get_Mem_বাফার(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  যদি (!RangeLoc.isValid())
    ফেরত failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.তথ্য(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  ফেরত success();
}

Logical_ফলাফল TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  জন্য (ধ্রুবক auto &Change : Changes)
    যদি (failed(Change.applyTo(Contents)))
      ফেরত failure();
  ফেরত success();
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         TextDocumentContentChangeEvent &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("range", ফলাফল.range) &&
         O.map("rangeLength", ফলাফল.range_দৈর্ঘ্য) && O.map("text", ফলাফল.text);
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         DidChangeTextDocumentParams &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument) &&
         O.map("contentChanges", ফলাফল.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

স্থির llvm::StringRef toTextKind(MarkupKind Kind) {
  নির্বাচন (Kind) {
  ক্ষেত্র MarkupKind::PlainText:
    ফেরত "plaintext";
  ক্ষেত্র MarkupKind::Markdown:
    ফেরত "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  ফেরত Os << toTextKind(Kind);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক MarkupContent &Mc) {
  যদি (Mc.মান_2.empty())
    ফেরত nullptr;

  ফেরত llvm::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.মান_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Hover &Hover) {
  llvm::json::Object ফলাফল{{"contents", toJSON(Hover.contents)}};
  যদি (Hover.range)
    ফলাফল["range"] = toJSON(*Hover.range);
  ফেরত std::move(ফলাফল);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Document_প্রতীক &প্রতীক) {
  llvm::json::Object ফলাফল{{"name", প্রতীক.name},
                            {"kind", static_cast<পূর্ণসংখ্যা>(প্রতীক.kind)},
                            {"range", প্রতীক.range},
                            {"selectionRange", প্রতীক.selectionRange}};

  যদি (!প্রতীক.detail.empty())
    ফলাফল["detail"] = প্রতীক.detail;
  যদি (!প্রতীক.children.empty())
    ফলাফল["children"] = প্রতীক.children;
  ফেরত std::move(ফলাফল);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         Document_প্রতীক_Params &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         DiagnosticRelatedInformation &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("location", ফলাফল.location) &&
         O.map("message", ফলাফল.message);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক DiagnosticRelatedInformation &Info) {
  ফেরত llvm::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(DiagnosticTag Tag) {
  ফেরত static_cast<পূর্ণসংখ্যা>(Tag);
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, DiagnosticTag &ফলাফল,
                         llvm::json::Path Path) {
  যদি (std::optional<int64_t> I = মান.getAsInteger()) {
    ফলাফল = (DiagnosticTag)*I;
    ফেরত সত্য;
  }

  ফেরত মিথ্যা;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক Diagnostic &Diag) {
  llvm::json::Object ফলাফল{
      {"range", Diag.range},
      {"severity", (পূর্ণসংখ্যা)Diag.severity},
      {"message", Diag.message},
  };
  যদি (Diag.category)
    ফলাফল["category"] = *Diag.category;
  যদি (!Diag.source.empty())
    ফলাফল["source"] = Diag.source;
  যদি (Diag.relatedInformation)
    ফলাফল["relatedInformation"] = *Diag.relatedInformation;
  যদি (!Diag.tags.empty())
    ফলাফল["tags"] = Diag.tags;
  ফেরত std::move(ফলাফল);
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, Diagnostic &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O)
    ফেরত মিথ্যা;
  পূর্ণসংখ্যা Severity = 0;
  যদি (!mapOptOrNull(মান, "severity", Severity, Path))
    ফেরত মিথ্যা;
  ফলাফল.severity = (DiagnosticSeverity)Severity;

  ফেরত O.map("range", ফলাফল.range) && O.map("message", ফলাফল.message) &&
         mapOptOrNull(মান, "category", ফলাফল.category, Path) &&
         mapOptOrNull(মান, "source", ফলাফল.source, Path) &&
         mapOptOrNull(মান, "relatedInformation", ফলাফল.relatedInformation,
                      Path) &&
         mapOptOrNull(মান, "tags", ফলাফল.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক PublishDiagnosticsParams &Params) {
  ফেরত llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, TextEdit &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("range", ফলাফল.range) && O.map("newText", ফলাফল.newText);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক TextEdit &মান) {
  ফেরত llvm::json::Object{
      {"range", মান.range},
      {"newText", মান.newText},
  };
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os, ধ্রুবক TextEdit &মান) {
  Os << মান.range << " => \"";
  llvm::printEscapedString(মান.newText, Os);
  ফেরত Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CompletionItemKind &ফলাফল, llvm::json::Path Path) {
  যদি (std::optional<int64_t> Int_মান = মান.getAsInteger()) {
    যদি (*Int_মান < static_cast<পূর্ণসংখ্যা>(CompletionItemKind::Text) ||
        *Int_মান > static_cast<পূর্ণসংখ্যা>(CompletionItemKind::TypeParameter))
      ফেরত মিথ্যা;
    ফলাফল = static_cast<CompletionItemKind>(*Int_মান);
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

CompletionItemKind llvm::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  যদি (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.আকার_3() &&
      SupportedCompletionItemKinds[KindVal])
    ফেরত Kind;

  // Provide some fall backs for common kinds that are close enough.
  নির্বাচন (Kind) {
  ক্ষেত্র CompletionItemKind::Folder:
    ফেরত CompletionItemKind::File;
  ক্ষেত্র CompletionItemKind::EnumMember:
    ফেরত CompletionItemKind::Enum;
  ক্ষেত্র CompletionItemKind::Struct:
    ফেরত CompletionItemKind::Class;
  পূর্বনির্ধারিত:
    ফেরত CompletionItemKind::Text;
  }
}

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CompletionItemKindBitset &ফলাফল,
                         llvm::json::Path Path) {
  যদি (ধ্রুবক llvm::json::Array *Array_মান = মান.getAsArray()) {
    জন্য (size_t I = 0, E = Array_মান->আকার_3(); I < E; ++I) {
      CompletionItemKind KindOut;
      যদি (fromJSON((*Array_মান)[I], KindOut, Path.সূচক(I)))
        ফলাফল.set(size_t(KindOut));
    }
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক CompletionItem &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "completion item label is required");
  llvm::json::Object ফলাফল{{"label", মান.label}};
  যদি (মান.kind != CompletionItemKind::Missing)
    ফলাফল["kind"] = static_cast<পূর্ণসংখ্যা>(মান.kind);
  যদি (!মান.detail.empty())
    ফলাফল["detail"] = মান.detail;
  যদি (মান.documentation)
    ফলাফল["documentation"] = মান.documentation;
  যদি (!মান.sortText.empty())
    ফলাফল["sortText"] = মান.sortText;
  যদি (!মান.filterText.empty())
    ফলাফল["filterText"] = মান.filterText;
  যদি (!মান.ঢোকাও_Text.empty())
    ফলাফল["insertText"] = মান.ঢোকাও_Text;
  যদি (মান.ঢোকাও_Text_Format != ঢোকাও_Text_Format_2::Missing)
    ফলাফল["insertTextFormat"] = static_cast<পূর্ণসংখ্যা>(মান.ঢোকাও_Text_Format);
  যদি (মান.textEdit)
    ফলাফল["textEdit"] = *মান.textEdit;
  যদি (!মান.additionalTextEdits.empty()) {
    ফলাফল["additionalTextEdits"] =
        llvm::json::Array(মান.additionalTextEdits);
  }
  যদি (মান.deprecated)
    ফলাফল["deprecated"] = মান.deprecated;
  ফেরত std::move(ফলাফল);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুবক CompletionItem &মান) {
  ফেরত Os << মান.label << " - " << toJSON(মান);
}

যৌক্তিক llvm::lsp::operator<(ধ্রুবক CompletionItem &Lhs,
                          ধ্রুবক CompletionItem &Rhs) {
  ফেরত (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক CompletionList &মান) {
  ফেরত llvm::json::Object{
      {"isIncomplete", মান.isIncomplete},
      {"items", llvm::json::Array(মান.বস্তুসমূহ)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CompletionContext &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  পূর্ণসংখ্যা TriggerKind;
  যদি (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(মান, "triggerCharacter", ফলাফল.triggerCharacter, Path))
    ফেরত মিথ্যা;
  ফলাফল.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CompletionParams &ফলাফল, llvm::json::Path Path) {
  যদি (!fromJSON(মান, static_cast<TextDocumentPositionParams &>(ফলাফল), Path))
    ফেরত মিথ্যা;
  যদি (ধ্রুবক llvm::json::মান *Context = মান.getAsObject()->get("context"))
    ফেরত fromJSON(*Context, ফলাফল.context, Path.field("context"));
  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক ParameterInformation &মান) {
  প্রতিজ্ঞা((মান.labelOffsets || !মান.labelString.empty()) &&
         "parameter information label is required");
  llvm::json::Object ফলাফল;
  যদি (মান.labelOffsets)
    ফলাফল["label"] = llvm::json::Array(
        {মান.labelOffsets->first, মান.labelOffsets->second});
  অন্যথা
    ফলাফল["label"] = মান.labelString;
  যদি (!মান.documentation.empty())
    ফলাফল["documentation"] = মান.documentation;
  ফেরত std::move(ফলাফল);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক SignatureInformation &মান) {
  প্রতিজ্ঞা(!মান.label.empty() && "signature information label is required");
  llvm::json::Object ফলাফল{
      {"label", মান.label},
      {"parameters", llvm::json::Array(মান.parameters)},
  };
  যদি (!মান.documentation.empty())
    ফলাফল["documentation"] = মান.documentation;
  ফেরত std::move(ফলাফল);
}

raw_ostream &llvm::lsp::operator<<(raw_ostream &Os,
                                   ধ্রুবক SignatureInformation &মান) {
  ফেরত Os << মান.label << " - " << toJSON(মান);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক SignatureHelp &মান) {
  প্রতিজ্ঞা(মান.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  প্রতিজ্ঞা(মান.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  ফেরত llvm::json::Object{
      {"activeSignature", মান.activeSignature},
      {"activeParameter", মান.activeParameter},
      {"signatures", llvm::json::Array(মান.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         DocumentLinkParams &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক DocumentLink &মান) {
  ফেরত llvm::json::Object{
      {"range", মান.range},
      {"target", মান.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         InlayHintsParams &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument) &&
         O.map("range", ফলাফল.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক InlayHint &মান) {
  ফেরত llvm::json::Object{{"position", মান.position},
                            {"kind", (পূর্ণসংখ্যা)মান.kind},
                            {"label", মান.label},
                            {"paddingLeft", মান.padding_বাম},
                            {"paddingRight", মান.padding_ডান}};
}
যৌক্তিক llvm::lsp::operator==(ধ্রুবক InlayHint &Lhs, ধ্রুবক InlayHint &Rhs) {
  ফেরত std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
যৌক্তিক llvm::lsp::operator<(ধ্রুবক InlayHint &Lhs, ধ্রুবক InlayHint &Rhs) {
  ফেরত std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::operator<<(llvm::raw_ostream &Os,
                                         InlayHintKind মান) {
  নির্বাচন (মান) {
  ক্ষেত্র InlayHintKind::Parameter:
    ফেরত Os << "parameter";
  ক্ষেত্র InlayHintKind::Type:
    ফেরত Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CodeActionContext &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  যদি (!O || !O.map("diagnostics", ফলাফল.diagnostics))
    ফেরত মিথ্যা;
  O.map("only", ফলাফল.only);
  ফেরত সত্য;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান,
                         CodeActionParams &ফলাফল, llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("textDocument", ফলাফল.textDocument) &&
         O.map("range", ফলাফল.range) && O.map("context", ফলাফল.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

যৌক্তিক llvm::lsp::fromJSON(ধ্রুবক llvm::json::মান &মান, WorkspaceEdit &ফলাফল,
                         llvm::json::Path Path) {
  llvm::json::ObjectMapper O(মান, Path);
  ফেরত O && O.map("changes", ফলাফল.changes);
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক WorkspaceEdit &মান) {
  llvm::json::Object FileChanges;
  জন্য (auto &Change : মান.changes)
    FileChanges[Change.first] = llvm::json::Array(Change.second);
  ফেরত llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

ধ্রুবক llvm::StringLiteral CodeAction::kQuickFix = "quickfix";
ধ্রুবক llvm::StringLiteral CodeAction::kRefactor = "refactor";
ধ্রুবক llvm::StringLiteral CodeAction::kInfo = "info";

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক CodeAction &মান) {
  llvm::json::Object CodeAction{{"title", মান.title}};
  যদি (মান.kind)
    CodeAction["kind"] = *মান.kind;
  যদি (মান.diagnostics)
    CodeAction["diagnostics"] = llvm::json::Array(*মান.diagnostics);
  যদি (মান.isPreferred)
    CodeAction["isPreferred"] = সত্য;
  যদি (মান.edit)
    CodeAction["edit"] = *মান.edit;
  ফেরত std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক ShowMessageParams &Params) {
  auto Out = llvm::json::Object{
      {"type", static_cast<পূর্ণসংখ্যা>(Params.type)},
      {"message", Params.message},
  };
  যদি (Params.actions)
    Out["actions"] = *Params.actions;
  ফেরত Out;
}

llvm::json::মান llvm::lsp::toJSON(ধ্রুবক MessageActionItem &Params) {
  ফেরত llvm::json::Object{{"title", Params.title}};
}
