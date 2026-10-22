//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ضمن "llvm/Support/Mustache.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <sstream>

#define DEBUG_TYPE "mustache"

using نطاق llvm;
using نطاق llvm::mustache;

نطاق {

using Accessor = ArrayRef<StringRef>;

ساكن منطقي isFalsey(ثابت json::Value &V) {
  ܦܢܐ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().value()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

ساكن منطقي isContextFalsey(ثابت json::Value *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ܐܢ (!V)
    ܦܢܐ ܫܪܪܐ;
  ܦܢܐ isFalsey(*V);
}

ساكن فراغ splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  ܟܡܐ (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ܐܢ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    ܐܢ (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.push_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ساكن Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  ܐܢ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.push_back(".");
  } ܐܚܪܢܐ {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  ܦܢܐ ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

نطاق llvm::mustache {

صنف MustacheOutputStream : عام raw_ostream {
عام:
  MustacheOutputStream() = افتراضي;
  ~MustacheOutputStream() override = افتراضي;

  virtual فراغ suspendIndentation() {}
  virtual فراغ resumeIndentation() {}

خاص:
  فراغ anchor() override;
};

فراغ MustacheOutputStream::anchor() {}

صنف RawMustacheOutputStream : عام MustacheOutputStream {
عام:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

خاص:
  raw_ostream &OS;

  فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
    OS.اكتب(Ptr, Size);
  }
  uint64_t current_pos() ثابت override { ܦܢܐ OS.tell(); }
};

صنف Token {
عام:
  تعداد صنف Type {
    Text,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    SetDelimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::Text), RawBody(Str), TokenBody(RawBody),
        AccessorValue({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ܐܢ (TokenType == Type::Comment)
      ܦܢܐ;
    StringRef AccessorStr(هذا->TokenBody);
    ܐܢ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    AccessorValue = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { ܦܢܐ AccessorValue; }

  Type getType() ثابت { ܦܢܐ TokenType; }

  فراغ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { ܦܢܐ Indentation; }

  ساكن Type getTokenType(حرف Identifier) {
    بدل (Identifier) {
    حالة '#':
      ܦܢܐ Type::SectionOpen;
    حالة '/':
      ܦܢܐ Type::SectionClose;
    حالة '^':
      ܦܢܐ Type::InvertSectionOpen;
    حالة '!':
      ܦܢܐ Type::Comment;
    حالة '>':
      ܦܢܐ Type::Partial;
    حالة '&':
      ܦܢܐ Type::UnescapeVariable;
    حالة '=':
      ܦܢܐ Type::SetDelimiter;
    افتراضي:
      ܦܢܐ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> AccessorValue;
  size_t Indentation;
};

using EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_node<ASTNode> {
عام:
  تعداد Type {
    Root,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::Root), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), AccessorValue(Accessor),
        ParentContext(nullptr) {}

  فراغ addChild(AstPtr Child) { Children.push_back(Child); };

  فراغ setRawBody(StringRef NewBody) { RawBody = NewBody; };

  فراغ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::Value &Data, MustacheOutputStream &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::Value &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::Value &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::Value &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  فراغ renderChild(ثابت llvm::json::Value &Context, MustacheOutputStream &OS);

  ثابت llvm::json::Value *findContext();

  فراغ renderRoot(ثابت json::Value &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderText(MustacheOutputStream &OS);
  فراغ renderPartial(ثابت json::Value &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderVariable(ثابت json::Value &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderUnescapeVariable(ثابت json::Value &CurrentCtx,
                              MustacheOutputStream &OS);
  فراغ renderSection(ثابت json::Value &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderInvertSection(ثابت json::Value &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> AccessorValue;
  ثابت llvm::json::Value *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr createRootNode(MustacheContext &Ctx) {
  ܦܢܐ جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr createNode(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  ܦܢܐ جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

ساكن AstPtr createTextNode(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  ܦܢܐ جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
}

// Function to check if there is meaningful text behind.
// We determine if a token has meaningful text behind
// if the right of previous token contains anything that is
// not a newline.
// For example:
//  "Stuff {{#Section}}" (returns true)
//   vs
//  "{{#Section}} \n" (returns false)
// We make an exception for when previous token is empty
// and the current token is the second token.
// For example:
//  "{{#Section}}"
ساكن منطقي hasTextBehind(size_t Idx, ثابت ArrayRef<Token> &Tokens) {
  ܐܢ (Idx == 0)
    ܦܢܐ ܫܪܪܐ;

  size_t PrevIdx = Idx - 1;
  ܐܢ (Tokens[PrevIdx].getType() != Token::Type::Text)
    ܦܢܐ ܫܪܪܐ;

  ثابت Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ܦܢܐ !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي hasTextAhead(size_t Idx, ثابت ArrayRef<Token> &Tokens) {
  ܐܢ (Idx >= Tokens.size() - 1)
    ܦܢܐ ܫܪܪܐ;

  size_t NextIdx = Idx + 1;
  ܐܢ (Tokens[NextIdx].getType() != Token::Type::Text)
    ܦܢܐ ܫܪܪܐ;

  ثابت Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  ܦܢܐ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ܦܢܐ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ساكن فراغ stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ܐܢ (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  ܐܚܪܢܐ ܐܢ (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
فراغ stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

بنية Tag {
  تعداد صنف Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] ساكن ثابت حرف *tagKindToString(Tag::Kind K) {
  بدل (K) {
  حالة Tag::Kind::None:
    ܦܢܐ "None";
  حالة Tag::Kind::Normal:
    ܦܢܐ "Normal";
  حالة Tag::Kind::Triple:
    ܦܢܐ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] ساكن ثابت حرف *jsonKindToString(json::Value::Kind K) {
  بدل (K) {
  حالة json::Value::Kind::Null:
    ܦܢܐ "JSON_KIND_NULL";
  حالة json::Value::Kind::Boolean:
    ܦܢܐ "JSON_KIND_BOOLEAN";
  حالة json::Value::Kind::Number:
    ܦܢܐ "JSON_KIND_NUMBER";
  حالة json::Value::Kind::String:
    ܦܢܐ "JSON_KIND_STRING";
  حالة json::Value::Kind::Array:
    ܦܢܐ "JSON_KIND_ARRAY";
  حالة json::Value::Kind::Object:
    ܦܢܐ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ساكن SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  ثابت StringLiteral TripleOpen("{{{");
  ثابت StringLiteral TripleClose("}}}");

  ܟܡܐ (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    ܐܢ (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } ܐܚܪܢܐ ܐܢ (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } ܐܚܪܢܐ {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    ܐܢ (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    ܐܢ (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      اكسر;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    ܐܢ (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ܐܚܪܢܐ { // Normal Tag
      StringRef Interpolated = Content;
      ܐܢ (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } ܐܚܪܢܐ { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { ܦܢܐ C == '='; });
        DelimSpec = DelimSpec.trim();

        auto [NewOpen, NewClose] = DelimSpec.split(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += FullMatch.size();
    TextStart = Cursor;
  }

  // Add any remaining text after the last tag.
  ܐܢ (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  لكل (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ܐܢ (!requiresCleanUp(CurrentType))
      استمر;

    منطقي HasTextBehind = hasTextBehind(Idx, Tokens);
    منطقي HasTextAhead = hasTextAhead(Idx, Tokens);

    ܐܢ ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    ܐܢ ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  ܦܢܐ Tokens;
}

// Custom stream to escape strings.
صنف EscapeStringStream : عام MustacheOutputStream {
عام:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.keys().begin(), Escape.keys().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    ܟܡܐ (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      ܐܢ (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        ܦܢܐ;
      }

      // Write the chunk of text before the escapable character.
      ܐܢ (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ثابت override { ܦܢܐ WrappedStream.tell(); }

خاص:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
صنف AddIndentationStringStream : عام MustacheOutputStream {
عام:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(ܫܪܪܐ), IsSuspended(ܕܓܠܐ) {
    SetUnbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = ܫܪܪܐ; }
  فراغ resumeIndentation() override { IsSuspended = ܕܓܠܐ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ܐܢ (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = ܕܓܠܐ;
      }
      WrappedStream << C;
      ܐܢ (C == '\n' && !IsSuspended)
        NeedsIndent = ܫܪܪܐ;
    }
  }

  uint64_t current_pos() ثابت override { ܦܢܐ WrappedStream.tell(); }

خاص:
  size_t Indentation;
  raw_ostream &WrappedStream;
  منطقي NeedsIndent;
  منطقي IsSuspended;
};

صنف Parser {
عام:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

خاص:
  فراغ parseMustache(ASTNode *Parent);
  فراغ parseSection(ASTNode *Parent, ASTNode::Type Ty, ثابت Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

فراغ Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          ثابت Accessor &A) {
  AstPtr CurrentNode = createNode(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(CurrentNode);
  ثابت size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  لكل (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  لكل (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  CurrentNode->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(CurrentNode);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr RootNode = createRootNode(Ctx);
  parseMustache(RootNode);
  ܦܢܐ RootNode;
}

فراغ Parser::parseMustache(ASTNode *Parent) {

  ܟܡܐ (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr CurrentNode;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::Text: {
      CurrentNode = createTextNode(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(CurrentNode);
      اكسر;
    }
    حالة Token::Type::Variable: {
      CurrentNode = createNode(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(CurrentNode);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      CurrentNode = createNode(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(CurrentNode);
      اكسر;
    }
    حالة Token::Type::Partial: {
      CurrentNode = createNode(Ctx, ASTNode::Partial, A, Parent);
      CurrentNode->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(CurrentNode);
      اكسر;
    }
    حالة Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      اكسر;
    }
    حالة Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      اكسر;
    }
    حالة Token::Type::Comment:
    حالة Token::Type::SetDelimiter:
      اكسر;
    حالة Token::Type::SectionClose:
      ܦܢܐ;
    }
  }
}
ساكن فراغ toMustacheString(ثابت json::Value &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  بدل (Data.kind()) {
  حالة json::Value::Null:
    ܦܢܐ;
  حالة json::Value::Number: {
    auto Num = *Data.getAsNumber();
    std::ostringstream SS;
    SS << Num;
    OS << SS.str();
    ܦܢܐ;
  }
  حالة json::Value::String: {
    OS << *Data.getAsString();
    ܦܢܐ;
  }

  حالة json::Value::Array: {
    auto Arr = *Data.getAsArray();
    ܐܢ (Arr.empty())
      ܦܢܐ;
    [[fallthrough]];
  }
  حالة json::Value::Object:
  حالة json::Value::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.value(Data);
    اكسر;
  }
  }
}

فراغ ASTNode::renderRoot(ثابت json::Value &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

فراغ ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::Value &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << AccessorValue[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(AccessorValue[0]);
  ܐܢ (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->getValue());
}

فراغ ASTNode::renderVariable(ثابت json::Value &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(AccessorValue[0]);
  ܐܢ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->getValue());
  } ܐܚܪܢܐ ܐܢ (ثابت json::Value *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::Value &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << AccessorValue[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(AccessorValue[0]);
  ܐܢ (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->getValue());
  } ܐܚܪܢܐ ܐܢ (ثابت json::Value *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::Value &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(AccessorValue[0]);
  ܐܢ (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->getValue());
    ܦܢܐ;
  }

  ثابت json::Value *ContextPtr = findContext();
  ܐܢ (isContextFalsey(ContextPtr))
    ܦܢܐ;

  ܐܢ (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::Value &V : *Arr)
      renderChild(V, OS);
    ܦܢܐ;
  }
  renderChild(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::Value &CurrentCtx,
                                  MustacheOutputStream &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.contains(AccessorValue[0]);
  ثابت json::Value *ContextPtr = findContext();
  ܐܢ (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::Value &Data, MustacheOutputStream &OS) {
  ܐܢ (Ty != Root && Ty != Text && AccessorValue.empty())
    ܦܢܐ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  بدل (Ty) {
  حالة Root:
    renderRoot(Data, OS);
    ܦܢܐ;
  حالة Text:
    renderText(OS);
    ܦܢܐ;
  حالة Partial:
    renderPartial(Data, OS);
    ܦܢܐ;
  حالة Variable:
    renderVariable(Data, OS);
    ܦܢܐ;
  حالة UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    ܦܢܐ;
  حالة Section:
    renderSection(Data, OS);
    ܦܢܐ;
  حالة InvertSection:
    renderInvertSection(Data, OS);
    ܦܢܐ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::Value *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ܐܢ (AccessorValue.empty())
    ܦܢܐ nullptr;
  ܐܢ (AccessorValue[0] == ".")
    ܦܢܐ ParentContext;

  ثابت json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = AccessorValue[0];
  ASTNode *CurrentParent = Parent;

  ܟܡܐ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ܐܢ (CurrentParent->Ty != Root) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      استمر;
    }
    ܦܢܐ nullptr;
  }
  ثابت json::Value *Context = nullptr;
  لكل (auto [Idx, Acc] : enumerate(AccessorValue)) {
    ثابت json::Value *CurrentValue = CurrentContext->get(Acc);
    ܐܢ (!CurrentValue)
      ܦܢܐ nullptr;
    ܐܢ (Idx < AccessorValue.size() - 1) {
      CurrentContext = CurrentValue->getAsObject();
      ܐܢ (!CurrentContext)
        ܦܢܐ nullptr;
    } ܐܚܪܢܐ {
      Context = CurrentValue;
    }
  }
  ܦܢܐ Context;
}

فراغ ASTNode::renderChild(ثابت json::Value &Contexts,
                          MustacheOutputStream &OS) {
  لكل (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::Value &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::Value &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::Value LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr LambdaNode = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  ܐܢ (Ty == Variable) {
    LambdaNode->render(Contexts, ES);
    ܦܢܐ;
  }
  LambdaNode->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::Value &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::Value Return = L(RawBody.str());
  ܐܢ (isFalsey(Return))
    ܦܢܐ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr LambdaNode = P.parse();
  LambdaNode->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::Value &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  Tree->render(Data, MOS);
}

فراغ Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr PartialTree = P.parse();
  Ctx.Partials.insert(std::make_pair(Name, PartialTree));
}

فراغ Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

فراغ Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

فراغ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  Tree = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), Tree(Other.Tree) {
  Other.Tree = nullptr;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#undef DEBUG_TYPE
