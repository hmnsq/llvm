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

#అన్తర్భావ "llvm/Support/LSP/Protocol.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringSet.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/JSON.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
మూస <రకం_పేరు T>
స్థిర తార్కిక mapOptOrNull(స్థిరరాశి llvm::json::విలువ &Params,
                         llvm::StringLiteral Prop, T &బహిర్గతం,
                         llvm::json::మార్గం మార్గం) {
  స్థిరరాశి llvm::json::Object *O = Params.getAsObject();
  ప్రతిజ్ఞా(O);

  // Field is missing or null.
  స్వయంచాలక *V = O->get(Prop);
  ఒకవేళ (!V || V->getAsNull())
    తిరిగి నిజం;
  తిరిగి fromJSON(*V, బహిర్గతం, మార్గం.క్షేత్రం(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

అక్షరం LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

స్థిర తార్కిక is_Windows_మార్గం(StringRef మార్గం) {
  తిరిగి మార్గం.కొలత_2() > 1 && llvm::isAlpha(మార్గం[0]) && మార్గం[1] == ':';
}

స్థిర తార్కిక is_జాలం_మార్గం(StringRef మార్గం) {
  తిరిగి మార్గం.కొలత_2() > 2 && మార్గం[0] == మార్గం[1] &&
         llvm::sys::మార్గం_2::is_separator(మార్గం[0]);
}

స్థిర తార్కిక should_Escape_లోపల_URI(గుర్తులేని అక్షరం C) {
  // Unreserved characters.
  ఒకవేళ ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    తిరిగి అబద్ధం;

  ఎంపిక (C) {
  స్థితి '-':
  స్థితి '_':
  స్థితి '.':
  స్థితి '~':
  // '/' is only reserved when parsing.
  స్థితి '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  స్థితి ':':
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
స్థిర శూన్య percentEncode(StringRef Content, std::string &బహిర్గతం) {
  కొరకు (గుర్తులేని అక్షరం C : Content) {
    ఒకవేళ (should_Escape_లోపల_URI(C)) {
      బహిర్గతం.నెట్టు_వెనుక('%');
      బహిర్గతం.నెట్టు_వెనుక(llvm::hexdigit(C / 16));
      బహిర్గతం.నెట్టు_వెనుక(llvm::hexdigit(C % 16));
    } లేకపోతే {
      బహిర్గతం.నెట్టు_వెనుక(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
స్థిర std::string percentDecode(StringRef Content) {
  std::string ఫలితం;
  కొరకు (స్వయంచాలక I = Content.begin(), E = Content.చివర(); I != E; ++I) {
    ఒకవేళ (*I == '%' && I + 2 < Content.చివర() && llvm::isHexDigit(*(I + 1)) &&
        llvm::isHexDigit(*(I + 2))) {
      ఫలితం.నెట్టు_వెనుక(llvm::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } లేకపోతే {
      ఫలితం.నెట్టు_వెనుక(*I);
    }
  }
  తిరిగి ఫలితం;
}

/// Return the set containing the supported URI schemes.
స్థిర String_సమితి<> &getSupportedSchemes() {
  స్థిర String_సమితి<> Schemes({"file", "test"});
  తిరిగి Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
స్థిర తార్కిక isStructurallyValidScheme(StringRef Scheme) {
  ఒకవేళ (Scheme.ఖాళీలు())
    తిరిగి అబద్ధం;
  ఒకవేళ (!llvm::isAlpha(Scheme[0]))
    తిరిగి అబద్ధం;
  తిరిగి llvm::all_యొక్క(llvm::drop_begin(Scheme), [](అక్షరం C) {
    తిరిగి llvm::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

స్థిర llvm::Expected<std::string> uri_From_Absolute_మార్గం(StringRef Absolute_మార్గం,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef మూలం_2 = llvm::sys::మార్గం_2::మూలం_name(Absolute_మార్గం);
  ఒకవేళ (is_జాలం_మార్గం(మూలం_2)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = మూలం_2.drop_front(2);
    Absolute_మార్గం.consume_front(మూలం_2);
  } లేకపోతే ఒకవేళ (is_Windows_మార్గం(మూలం_2)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += llvm::sys::మార్గం_2::convert_గమ్యం_slash(Absolute_మార్గం);

  std::string Uri = Scheme.str() + ":";
  ఒకవేళ (Authority.ఖాళీలు() && Body.ఖాళీలు())
    తిరిగి Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  ఒకవేళ (!Authority.ఖాళీలు() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  తిరిగి Uri;
}

స్థిర llvm::Expected<std::string> get_Absolute_మార్గం(StringRef Authority,
                                                   StringRef Body) {
  ఒకవేళ (!Body.starts_with("/"))
    తిరిగి llvm::createStringError(
        llvm::inconvertible_Error_సంకేతం(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> మార్గం;
  ఒకవేళ (!Authority.ఖాళీలు()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).గమ్యం_సదిశ(మార్గం);
  } లేకపోతే ఒకవేళ (is_Windows_మార్గం(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  మార్గం.append(Body);
  llvm::sys::మార్గం_2::native(మార్గం);
  తిరిగి std::string(మార్గం);
}

స్థిర llvm::Expected<std::string> parse_File_మార్గం_From_URI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.కనుగొను(':');
  ఒకవేళ (Pos == StringRef::npos)
    తిరిగి llvm::createStringError(llvm::inconvertible_Error_సంకేతం(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  ఒకవేళ (!isStructurallyValidScheme(UriScheme))
    తిరిగి llvm::createStringError(llvm::inconvertible_Error_సంకేతం(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  ఒకవేళ (Uri.consume_front("//")) {
    Pos = Uri.కనుగొను('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  ఒకవేళ (!getSupportedSchemes().కలిగియుంది(UriScheme)) {
    తిరిగి llvm::createStringError(llvm::inconvertible_Error_సంకేతం(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  తిరిగి get_Absolute_మార్గం(UriAuthority, UriBody);
}

llvm::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  llvm::Expected<std::string> File_మార్గం = parse_File_మార్గం_From_URI(Uri);
  ఒకవేళ (!File_మార్గం)
    తిరిగి File_మార్గం.takeError();
  తిరిగి URIForFile(std::move(*File_మార్గం), Uri.str());
}

llvm::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  llvm::Expected<std::string> Uri =
      uri_From_Absolute_మార్గం(AbsoluteFilepath, Scheme);
  ఒకవేళ (!Uri)
    తిరిగి Uri.takeError();
  తిరిగి fromURI(*Uri);
}

StringRef URIForFile::scheme() స్థిరరాశి { తిరిగి uri().విభజించు(':').మొదటి_2; }

శూన్య URIForFile::రిజిస్టరు_Supported_Scheme(StringRef Scheme) {
  getSupportedSchemes().చొప్పించు(Scheme);
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, URIForFile &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  ఒకవేళ (std::optional<StringRef> Str = విలువ.getAsString()) {
    llvm::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    ఒకవేళ (!ExpectedUri) {
      మార్గం.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      తిరిగి అబద్ధం;
    }
    ఫలితం = std::move(*ExpectedUri);
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి URIForFile &విలువ) {
  తిరిగి విలువ.uri();
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, స్థిరరాశి URIForFile &విలువ) {
  తిరిగి Os << విలువ.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         ClientCapabilities &ఫలితం, llvm::json::మార్గం మార్గం) {
  స్థిరరాశి llvm::json::Object *O = విలువ.getAsObject();
  ఒకవేళ (!O) {
    మార్గం.report("expected object");
    తిరిగి అబద్ధం;
  }
  ఒకవేళ (స్థిరరాశి llvm::json::Object *పాఠ్యం_Document = O->getObject("textDocument")) {
    ఒకవేళ (స్థిరరాశి llvm::json::Object *Document_చిహ్నం =
            పాఠ్యం_Document->getObject("documentSymbol")) {
      ఒకవేళ (std::optional<తార్కిక> క్రమానుగత_Support =
              Document_చిహ్నం->getBoolean("hierarchicalDocumentSymbolSupport"))
        ఫలితం.క్రమానుగత_Document_చిహ్నం = *క్రమానుగత_Support;
    }
    ఒకవేళ (స్వయంచాలక *సంకేతం_Action = పాఠ్యం_Document->getObject("codeAction")) {
      ఒకవేళ (సంకేతం_Action->getObject("codeActionLiteralSupport"))
        ఫలితం.సంకేతం_Action_Structure = నిజం;
    }
  }
  ఒకవేళ (స్వయంచాలక *కిటికీ = O->getObject("window")) {
    ఒకవేళ (std::optional<తార్కిక> Work_పూర్తి_పురోగతి_Support =
            కిటికీ->getBoolean("workDoneProgress"))
      ఫలితం.work_పూర్తి_పురోగతి = *Work_పూర్తి_పురోగతి_Support;
  }
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, ClientInfo &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  ఒకవేళ (!O || !O.map("name", ఫలితం.name))
    తిరిగి అబద్ధం;

  // Don't fail if we can't parse version.
  O.map("version", ఫలితం.సంస్కరణ_2);
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, TraceLevel &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  ఒకవేళ (std::optional<StringRef> Str = విలువ.getAsString()) {
    ఒకవేళ (*Str == "off") {
      ఫలితం = TraceLevel::Off;
      తిరిగి నిజం;
    }
    ఒకవేళ (*Str == "messages") {
      ఫలితం = TraceLevel::Messages;
      తిరిగి నిజం;
    }
    ఒకవేళ (*Str == "verbose") {
      ఫలితం = TraceLevel::Verbose;
      తిరిగి నిజం;
    }
  }
  తిరిగి అబద్ధం;
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         InitializeParams &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  ఒకవేళ (!O)
    తిరిగి అబద్ధం;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", ఫలితం.capabilities);
  O.map("trace", ఫలితం.trace);
  O.map("rootUri", ఫలితం.మూలం_Uri);
  O.map("rootPath", ఫలితం.మూలం_మార్గం);
  mapOptOrNull(విలువ, "clientInfo", ఫలితం.clientInfo, మార్గం);

  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         పాఠ్యం_Document_అంశం &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("uri", ఫలితం.uri) &&
         O.map("languageId", ఫలితం.languageId) && O.map("text", ఫలితం.పాఠ్యం) &&
         O.map("version", ఫలితం.సంస్కరణ_2);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి పాఠ్యం_Document_Identifier &విలువ) {
  తిరిగి llvm::json::Object{{"uri", విలువ.uri}};
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         పాఠ్యం_Document_Identifier &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("uri", ఫలితం.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

llvm::json::విలువ
llvm::lsp::గమ్యం_JSON(స్థిరరాశి Versioned_పాఠ్యం_Document_Identifier &విలువ) {
  తిరిగి llvm::json::Object{
      {"uri", విలువ.uri},
      {"version", విలువ.సంస్కరణ_2},
  };
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Versioned_పాఠ్యం_Document_Identifier &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("uri", ఫలితం.uri) && O.map("version", ఫలితం.సంస్కరణ_2);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, Position &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("line", ఫలితం.పంక్తి_2) &&
         O.map("character", ఫలితం.character);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Position &విలువ) {
  తిరిగి llvm::json::Object{
      {"line", విలువ.పంక్తి_2},
      {"character", విలువ.character},
  };
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, స్థిరరాశి Position &విలువ) {
  తిరిగి Os << విలువ.పంక్తి_2 << ':' << విలువ.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, Range &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("start", ఫలితం.ప్రారంభం_2) && O.map("end", ఫలితం.చివర);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Range &విలువ) {
  తిరిగి llvm::json::Object{
      {"start", విలువ.ప్రారంభం_2},
      {"end", విలువ.చివర},
  };
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, స్థిరరాశి Range &విలువ) {
  తిరిగి Os << విలువ.ప్రారంభం_2 << '-' << విలువ.చివర;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, Location &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("uri", ఫలితం.uri) && O.map("range", ఫలితం.range);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Location &విలువ) {
  తిరిగి llvm::json::Object{
      {"uri", విలువ.uri},
      {"range", విలువ.range},
  };
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, స్థిరరాశి Location &విలువ) {
  తిరిగి Os << విలువ.range << '@' << విలువ.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         పాఠ్యం_Document_Position_Params &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2) &&
         O.map("position", ఫలితం.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         ReferenceContext &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.mapOptional("includeDeclaration", ఫలితం.includeDeclaration);
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         ReferenceParams &ఫలితం, llvm::json::మార్గం మార్గం) {
  పాఠ్యం_Document_Position_Params &పునాది = ఫలితం;
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి fromJSON(విలువ, పునాది, మార్గం) && O &&
         O.mapOptional("context", ఫలితం.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Did_Open_పాఠ్యం_Document_Params &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Did_Close_పాఠ్యం_Document_Params &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2);
}

//===----------------------------------------------------------------------===//
// DidSaveTextDocumentParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &Params,
                         Did_Save_పాఠ్యం_Document_Params &R, llvm::json::మార్గం P) {
  llvm::json::ObjectMapper O(Params, P);
  తిరిగి O && O.map("textDocument", R.పాఠ్యం_Document_2);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

Logical_ఫలితం
పాఠ్యం_Document_Content_Change_Event::apply_గమ్యం(std::string &Contents) స్థిరరాశి {
  // If there is no range, the full document changed.
  ఒకవేళ (!range) {
    Contents = పాఠ్యం;
    తిరిగి success();
  }

  // Try to map the replacement range to the content.
  llvm::ఆధారం_Mgr TmpScrMgr;
  TmpScrMgr.కూడు_New_ఆధారం_బఫరు(llvm::జ్ఞాపకం_బఫరు::get_Mem_బఫరు(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  ఒకవేళ (!RangeLoc.isValid())
    తిరిగి failure();

  Contents.replace(RangeLoc.ప్రారంభం.get_సూచిక() - Contents.దత్తాంశం(),
                   RangeLoc.చివర_2.get_సూచిక() - RangeLoc.ప్రారంభం.get_సూచిక(),
                   పాఠ్యం);
  తిరిగి success();
}

Logical_ఫలితం పాఠ్యం_Document_Content_Change_Event::apply_గమ్యం(
    ArrayRef<పాఠ్యం_Document_Content_Change_Event> Changes, std::string &Contents) {
  కొరకు (స్థిరరాశి స్వయంచాలక &Change : Changes)
    ఒకవేళ (failed(Change.apply_గమ్యం(Contents)))
      తిరిగి failure();
  తిరిగి success();
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         పాఠ్యం_Document_Content_Change_Event &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("range", ఫలితం.range) &&
         O.map("rangeLength", ఫలితం.range_నిడివి) && O.map("text", ఫలితం.పాఠ్యం);
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Did_Change_పాఠ్యం_Document_Params &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2) &&
         O.map("contentChanges", ఫలితం.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

స్థిర llvm::StringRef గమ్యం_పాఠ్యం_రకం(Markup_రకం రకం_3) {
  ఎంపిక (రకం_3) {
  స్థితి Markup_రకం::Plain_పాఠ్యం:
    తిరిగి "plaintext";
  స్థితి Markup_రకం::Markdown:
    తిరిగి "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, Markup_రకం రకం_3) {
  తిరిగి Os << గమ్యం_పాఠ్యం_రకం(రకం_3);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి MarkupContent &Mc) {
  ఒకవేళ (Mc.విలువ_2.ఖాళీలు())
    తిరిగి శూన్యసూచిక;

  తిరిగి llvm::json::Object{
      {"kind", గమ్యం_పాఠ్యం_రకం(Mc.రకం_2)},
      {"value", Mc.విలువ_2},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Hover &Hover) {
  llvm::json::Object ఫలితం{{"contents", గమ్యం_JSON(Hover.contents)}};
  ఒకవేళ (Hover.range)
    ఫలితం["range"] = గమ్యం_JSON(*Hover.range);
  తిరిగి std::move(ఫలితం);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Document_చిహ్నం &చిహ్నం) {
  llvm::json::Object ఫలితం{{"name", చిహ్నం.name},
                            {"kind", static_cast<పూర్ణాంకం>(చిహ్నం.రకం_2)},
                            {"range", చిహ్నం.range},
                            {"selectionRange", చిహ్నం.selectionRange}};

  ఒకవేళ (!చిహ్నం.detail.ఖాళీలు())
    ఫలితం["detail"] = చిహ్నం.detail;
  ఒకవేళ (!చిహ్నం.children.ఖాళీలు())
    ఫలితం["children"] = చిహ్నం.children;
  తిరిగి std::move(ఫలితం);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Document_చిహ్నం_Params &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         DiagnosticRelatedInformation &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("location", ఫలితం.location) &&
         O.map("message", ఫలితం.సందేశం_2);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి DiagnosticRelatedInformation &Info) {
  తిరిగి llvm::json::Object{
      {"location", Info.location},
      {"message", Info.సందేశం_2},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(Diagnostic_ట్యాగ్ ట్యాగ్) {
  తిరిగి static_cast<పూర్ణాంకం>(ట్యాగ్);
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, Diagnostic_ట్యాగ్ &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  ఒకవేళ (std::optional<int64_t> I = విలువ.getAsInteger()) {
    ఫలితం = (Diagnostic_ట్యాగ్)*I;
    తిరిగి నిజం;
  }

  తిరిగి అబద్ధం;
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Diagnostic &Diag) {
  llvm::json::Object ఫలితం{
      {"range", Diag.range},
      {"severity", (పూర్ణాంకం)Diag.severity},
      {"message", Diag.సందేశం_2},
  };
  ఒకవేళ (Diag.category)
    ఫలితం["category"] = *Diag.category;
  ఒకవేళ (!Diag.ఆధారం_2.ఖాళీలు())
    ఫలితం["source"] = Diag.ఆధారం_2;
  ఒకవేళ (Diag.relatedInformation)
    ఫలితం["relatedInformation"] = *Diag.relatedInformation;
  ఒకవేళ (!Diag.ట్యాగులు.ఖాళీలు())
    ఫలితం["tags"] = Diag.ట్యాగులు;
  తిరిగి std::move(ఫలితం);
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, Diagnostic &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  ఒకవేళ (!O)
    తిరిగి అబద్ధం;
  పూర్ణాంకం Severity = 0;
  ఒకవేళ (!mapOptOrNull(విలువ, "severity", Severity, మార్గం))
    తిరిగి అబద్ధం;
  ఫలితం.severity = (DiagnosticSeverity)Severity;

  తిరిగి O.map("range", ఫలితం.range) && O.map("message", ఫలితం.సందేశం_2) &&
         mapOptOrNull(విలువ, "category", ఫలితం.category, మార్గం) &&
         mapOptOrNull(విలువ, "source", ఫలితం.ఆధారం_2, మార్గం) &&
         mapOptOrNull(విలువ, "relatedInformation", ఫలితం.relatedInformation,
                      మార్గం) &&
         mapOptOrNull(విలువ, "tags", ఫలితం.ట్యాగులు, మార్గం);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి PublishDiagnosticsParams &Params) {
  తిరిగి llvm::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.సంస్కరణ_2},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, పాఠ్యం_Edit &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("range", ఫలితం.range) && O.map("newText", ఫలితం.new_పాఠ్యం);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి పాఠ్యం_Edit &విలువ) {
  తిరిగి llvm::json::Object{
      {"range", విలువ.range},
      {"newText", విలువ.new_పాఠ్యం},
  };
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os, స్థిరరాశి పాఠ్యం_Edit &విలువ) {
  Os << విలువ.range << " => \"";
  llvm::printEscapedString(విలువ.new_పాఠ్యం, Os);
  తిరిగి Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Completion_అంశం_రకం &ఫలితం, llvm::json::మార్గం మార్గం) {
  ఒకవేళ (std::optional<int64_t> Int_విలువ = విలువ.getAsInteger()) {
    ఒకవేళ (*Int_విలువ < static_cast<పూర్ణాంకం>(Completion_అంశం_రకం::పాఠ్యం_2) ||
        *Int_విలువ > static_cast<పూర్ణాంకం>(Completion_అంశం_రకం::TypeParameter))
      తిరిగి అబద్ధం;
    ఫలితం = static_cast<Completion_అంశం_రకం>(*Int_విలువ);
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

Completion_అంశం_రకం llvm::lsp::adjust_రకం_గమ్యం_Capability(
    Completion_అంశం_రకం రకం_3,
    Completion_అంశం_రకం_Bitset &Supported_Completion_అంశం_Kinds) {
  size_t రకం_Val = static_cast<size_t>(రకం_3);
  ఒకవేళ (రకం_Val >= k_Completion_అంశం_రకం_Min &&
      రకం_Val <= Supported_Completion_అంశం_Kinds.కొలత_2() &&
      Supported_Completion_అంశం_Kinds[రకం_Val])
    తిరిగి రకం_3;

  // Provide some fall backs for common kinds that are close enough.
  ఎంపిక (రకం_3) {
  స్థితి Completion_అంశం_రకం::Folder:
    తిరిగి Completion_అంశం_రకం::File;
  స్థితి Completion_అంశం_రకం::EnumMember:
    తిరిగి Completion_అంశం_రకం::Enum;
  స్థితి Completion_అంశం_రకం::Struct:
    తిరిగి Completion_అంశం_రకం::Class;
  అప్రమేయ:
    తిరిగి Completion_అంశం_రకం::పాఠ్యం_2;
  }
}

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         Completion_అంశం_రకం_Bitset &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  ఒకవేళ (స్థిరరాశి llvm::json::Array *Array_విలువ = విలువ.getAsArray()) {
    కొరకు (size_t I = 0, E = Array_విలువ->కొలత_2(); I < E; ++I) {
      Completion_అంశం_రకం రకం_బహిర్గతం;
      ఒకవేళ (fromJSON((*Array_విలువ)[I], రకం_బహిర్గతం, మార్గం.సూచీ(I)))
        ఫలితం.సమితి(size_t(రకం_బహిర్గతం));
    }
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Completion_అంశం &విలువ) {
  ప్రతిజ్ఞా(!విలువ.label.ఖాళీలు() && "completion item label is required");
  llvm::json::Object ఫలితం{{"label", విలువ.label}};
  ఒకవేళ (విలువ.రకం_2 != Completion_అంశం_రకం::Missing)
    ఫలితం["kind"] = static_cast<పూర్ణాంకం>(విలువ.రకం_2);
  ఒకవేళ (!విలువ.detail.ఖాళీలు())
    ఫలితం["detail"] = విలువ.detail;
  ఒకవేళ (విలువ.documentation)
    ఫలితం["documentation"] = విలువ.documentation;
  ఒకవేళ (!విలువ.క్రమీకరించు_పాఠ్యం.ఖాళీలు())
    ఫలితం["sortText"] = విలువ.క్రమీకరించు_పాఠ్యం;
  ఒకవేళ (!విలువ.filter_పాఠ్యం.ఖాళీలు())
    ఫలితం["filterText"] = విలువ.filter_పాఠ్యం;
  ఒకవేళ (!విలువ.చొప్పించు_పాఠ్యం.ఖాళీలు())
    ఫలితం["insertText"] = విలువ.చొప్పించు_పాఠ్యం;
  ఒకవేళ (విలువ.చొప్పించు_పాఠ్యం_Format != చొప్పించు_పాఠ్యం_Format_2::Missing)
    ఫలితం["insertTextFormat"] = static_cast<పూర్ణాంకం>(విలువ.చొప్పించు_పాఠ్యం_Format);
  ఒకవేళ (విలువ.పాఠ్యం_Edit_2)
    ఫలితం["textEdit"] = *విలువ.పాఠ్యం_Edit_2;
  ఒకవేళ (!విలువ.additional_పాఠ్యం_Edits.ఖాళీలు()) {
    ఫలితం["additionalTextEdits"] =
        llvm::json::Array(విలువ.additional_పాఠ్యం_Edits);
  }
  ఒకవేళ (విలువ.deprecated)
    ఫలితం["deprecated"] = విలువ.deprecated;
  తిరిగి std::move(ఫలితం);
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os,
                                   స్థిరరాశి Completion_అంశం &విలువ) {
  తిరిగి Os << విలువ.label << " - " << గమ్యం_JSON(విలువ);
}

తార్కిక llvm::lsp::క్రియాకారకం<(స్థిరరాశి Completion_అంశం &Lhs,
                          స్థిరరాశి Completion_అంశం &Rhs) {
  తిరిగి (Lhs.క్రమీకరించు_పాఠ్యం.ఖాళీలు() ? Lhs.label : Lhs.క్రమీకరించు_పాఠ్యం) <
         (Rhs.క్రమీకరించు_పాఠ్యం.ఖాళీలు() ? Rhs.label : Rhs.క్రమీకరించు_పాఠ్యం);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి CompletionList &విలువ) {
  తిరిగి llvm::json::Object{
      {"isIncomplete", విలువ.isIncomplete},
      {"items", llvm::json::Array(విలువ.వస్తువులు)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         CompletionContext &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  పూర్ణాంకం Trigger_రకం;
  ఒకవేళ (!O || !O.map("triggerKind", Trigger_రకం) ||
      !mapOptOrNull(విలువ, "triggerCharacter", ఫలితం.triggerCharacter, మార్గం))
    తిరిగి అబద్ధం;
  ఫలితం.trigger_రకం = static_cast<Completion_Trigger_రకం>(Trigger_రకం);
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         CompletionParams &ఫలితం, llvm::json::మార్గం మార్గం) {
  ఒకవేళ (!fromJSON(విలువ, static_cast<పాఠ్యం_Document_Position_Params &>(ఫలితం), మార్గం))
    తిరిగి అబద్ధం;
  ఒకవేళ (స్థిరరాశి llvm::json::విలువ *Context = విలువ.getAsObject()->get("context"))
    తిరిగి fromJSON(*Context, ఫలితం.context, మార్గం.క్షేత్రం("context"));
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి ParameterInformation &విలువ) {
  ప్రతిజ్ఞా((విలువ.labelOffsets || !విలువ.labelString.ఖాళీలు()) &&
         "parameter information label is required");
  llvm::json::Object ఫలితం;
  ఒకవేళ (విలువ.labelOffsets)
    ఫలితం["label"] = llvm::json::Array(
        {విలువ.labelOffsets->మొదటి_2, విలువ.labelOffsets->రెండవ_2});
  లేకపోతే
    ఫలితం["label"] = విలువ.labelString;
  ఒకవేళ (!విలువ.documentation.ఖాళీలు())
    ఫలితం["documentation"] = విలువ.documentation;
  తిరిగి std::move(ఫలితం);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి SignatureInformation &విలువ) {
  ప్రతిజ్ఞా(!విలువ.label.ఖాళీలు() && "signature information label is required");
  llvm::json::Object ఫలితం{
      {"label", విలువ.label},
      {"parameters", llvm::json::Array(విలువ.parameters)},
  };
  ఒకవేళ (!విలువ.documentation.ఖాళీలు())
    ఫలితం["documentation"] = విలువ.documentation;
  తిరిగి std::move(ఫలితం);
}

raw_ostream &llvm::lsp::క్రియాకారకం<<(raw_ostream &Os,
                                   స్థిరరాశి SignatureInformation &విలువ) {
  తిరిగి Os << విలువ.label << " - " << గమ్యం_JSON(విలువ);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి SignatureHelp &విలువ) {
  ప్రతిజ్ఞా(విలువ.చురుకైన_Signature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  ప్రతిజ్ఞా(విలువ.చురుకైన_Parameter >= 0 &&
         "Unexpected negative value for active parameter index");
  తిరిగి llvm::json::Object{
      {"activeSignature", విలువ.చురుకైన_Signature},
      {"activeParameter", విలువ.చురుకైన_Parameter},
      {"signatures", llvm::json::Array(విలువ.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         DocumentLinkParams &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి DocumentLink &విలువ) {
  తిరిగి llvm::json::Object{
      {"range", విలువ.range},
      {"target", విలువ.లక్ష్యం},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         InlayHintsParams &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2) &&
         O.map("range", ఫలితం.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి InlayHint &విలువ) {
  తిరిగి llvm::json::Object{{"position", విలువ.position},
                            {"kind", (పూర్ణాంకం)విలువ.రకం_2},
                            {"label", విలువ.label},
                            {"paddingLeft", విలువ.padding_ఎడమ},
                            {"paddingRight", విలువ.padding_కుడి}};
}
తార్కిక llvm::lsp::క్రియాకారకం==(స్థిరరాశి InlayHint &Lhs, స్థిరరాశి InlayHint &Rhs) {
  తిరిగి std::tie(Lhs.position, Lhs.రకం_2, Lhs.label) ==
         std::tie(Rhs.position, Rhs.రకం_2, Rhs.label);
}
తార్కిక llvm::lsp::క్రియాకారకం<(స్థిరరాశి InlayHint &Lhs, స్థిరరాశి InlayHint &Rhs) {
  తిరిగి std::tie(Lhs.position, Lhs.రకం_2, Lhs.label) <
         std::tie(Rhs.position, Rhs.రకం_2, Rhs.label);
}

llvm::raw_ostream &llvm::lsp::క్రియాకారకం<<(llvm::raw_ostream &Os,
                                         Inlay_Hint_రకం విలువ) {
  ఎంపిక (విలువ) {
  స్థితి Inlay_Hint_రకం::Parameter:
    తిరిగి Os << "parameter";
  స్థితి Inlay_Hint_రకం::Type:
    తిరిగి Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         సంకేతం_Action_Context &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  ఒకవేళ (!O || !O.map("diagnostics", ఫలితం.diagnostics))
    తిరిగి అబద్ధం;
  O.map("only", ఫలితం.only);
  తిరిగి నిజం;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ,
                         సంకేతం_Action_Params &ఫలితం, llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("textDocument", ఫలితం.పాఠ్యం_Document_2) &&
         O.map("range", ఫలితం.range) && O.map("context", ఫలితం.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

తార్కిక llvm::lsp::fromJSON(స్థిరరాశి llvm::json::విలువ &విలువ, WorkspaceEdit &ఫలితం,
                         llvm::json::మార్గం మార్గం) {
  llvm::json::ObjectMapper O(విలువ, మార్గం);
  తిరిగి O && O.map("changes", ఫలితం.changes);
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి WorkspaceEdit &విలువ) {
  llvm::json::Object FileChanges;
  కొరకు (స్వయంచాలక &Change : విలువ.changes)
    FileChanges[Change.మొదటి_2] = llvm::json::Array(Change.రెండవ_2);
  తిరిగి llvm::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

స్థిరరాశి llvm::StringLiteral సంకేతం_Action::k_శీఘ్ర_Fix = "quickfix";
స్థిరరాశి llvm::StringLiteral సంకేతం_Action::kRefactor = "refactor";
స్థిరరాశి llvm::StringLiteral సంకేతం_Action::kInfo = "info";

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి సంకేతం_Action &విలువ) {
  llvm::json::Object సంకేతం_Action{{"title", విలువ.title}};
  ఒకవేళ (విలువ.రకం_2)
    సంకేతం_Action["kind"] = *విలువ.రకం_2;
  ఒకవేళ (విలువ.diagnostics)
    సంకేతం_Action["diagnostics"] = llvm::json::Array(*విలువ.diagnostics);
  ఒకవేళ (విలువ.isPreferred)
    సంకేతం_Action["isPreferred"] = నిజం;
  ఒకవేళ (విలువ.edit)
    సంకేతం_Action["edit"] = *విలువ.edit;
  తిరిగి std::move(సంకేతం_Action);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి Show_సందేశం_Params &Params) {
  స్వయంచాలక బహిర్గతం = llvm::json::Object{
      {"type", static_cast<పూర్ణాంకం>(Params.type)},
      {"message", Params.సందేశం_2},
  };
  ఒకవేళ (Params.actions)
    బహిర్గతం["actions"] = *Params.actions;
  తిరిగి బహిర్గతం;
}

llvm::json::విలువ llvm::lsp::గమ్యం_JSON(స్థిరరాశి సందేశం_Action_అంశం &Params) {
  తిరిగి llvm::json::Object{{"title", Params.title}};
}
