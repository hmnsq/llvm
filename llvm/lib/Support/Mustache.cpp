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

ساكن منطقي isFalsey(ثابت json::بەها &V) {
  أرجع V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().بەها_2()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

ساكن منطقي isContextFalsey(ثابت json::بەها *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  إذا (!V)
    أرجع true;
  أرجع isFalsey(*V);
}

ساكن فراغ splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  طالما (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    إذا (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    إذا (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.پاڵنان_back(Part.slice(Start, End + 1));
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
  إذا (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.پاڵنان_back(".");
  } وإلا {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  أرجع ArrayRef<StringRef>(ArenaTokens, Tokens.size());
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
  uint64_t current_pos() ثابت override { أرجع OS.tell(); }
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
        Accessor_بەها({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, حرف Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    إذا (TokenType == Type::Comment)
      أرجع;
    StringRef AccessorStr(هذا->TokenBody);
    إذا (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_بەها = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ثابت { أرجع Accessor_بەها; }

  Type getType() ثابت { أرجع TokenType; }

  فراغ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ثابت { أرجع Indentation; }

  ساكن Type getTokenType(حرف Identifier) {
    بدل (Identifier) {
    حالة '#':
      أرجع Type::SectionOpen;
    حالة '/':
      أرجع Type::SectionClose;
    حالة '^':
      أرجع Type::InvertSectionOpen;
    حالة '!':
      أرجع Type::Comment;
    حالة '>':
      أرجع Type::Partial;
    حالة '&':
      أرجع Type::UnescapeVariable;
    حالة '=':
      أرجع Type::SetDelimiter;
    افتراضي:
      أرجع Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_بەها;
  size_t Indentation;
};

using EscapeMap = DenseMap<حرف, std::string>;

صنف ASTNode : عام ilist_گرێ<ASTNode> {
عام:
  تعداد Type {
    ڕەگ_2,
    Text,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ڕەگ_2), Parent(nullptr), ParentContext(nullptr) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *Parent)
      : Ctx(Ctx), Ty(Type::Text), Body(Body), Parent(Parent),
        ParentContext(nullptr) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *Parent)
      : Ctx(Ctx), Ty(Ty), Parent(Parent), Accessor_بەها(Accessor),
        ParentContext(nullptr) {}

  فراغ addChild(AstPtr Child) { Children.پاڵنان_back(Child); };

  فراغ setRawBody(StringRef NewBody) { RawBody = NewBody; };

  فراغ setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  فراغ render(ثابت llvm::json::بەها &Data, MustacheOutputStream &OS);

خاص:
  فراغ renderLambdas(ثابت llvm::json::بەها &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  فراغ renderSectionLambdas(ثابت llvm::json::بەها &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  فراغ renderPartial(ثابت llvm::json::بەها &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  فراغ renderChild(ثابت llvm::json::بەها &Context, MustacheOutputStream &OS);

  ثابت llvm::json::بەها *findContext();

  فراغ render_ڕەگ(ثابت json::بەها &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderText(MustacheOutputStream &OS);
  فراغ renderPartial(ثابت json::بەها &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderVariable(ثابت json::بەها &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderUnescapeVariable(ثابت json::بەها &CurrentCtx,
                              MustacheOutputStream &OS);
  فراغ renderSection(ثابت json::بەها &CurrentCtx, MustacheOutputStream &OS);
  فراغ renderInvertSection(ثابت json::بەها &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  ثابت ArrayRef<StringRef> Accessor_بەها;
  ثابت llvm::json::بەها *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
ساكن AstPtr create_ڕەگ_گرێ(MustacheContext &Ctx) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ساكن AstPtr create_گرێ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

ساكن AstPtr create_Text_گرێ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  أرجع جديد (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
  إذا (Idx == 0)
    أرجع true;

  size_t PrevIdx = Idx - 1;
  إذا (Tokens[PrevIdx].getType() != Token::Type::Text)
    أرجع true;

  ثابت Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  أرجع !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ساكن منطقي hasTextAhead(size_t Idx, ثابت ArrayRef<Token> &Tokens) {
  إذا (Idx >= Tokens.size() - 1)
    أرجع true;

  size_t NextIdx = Idx + 1;
  إذا (Tokens[NextIdx].getType() != Token::Type::Text)
    أرجع true;

  ثابت Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  أرجع !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ساكن منطقي requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  أرجع T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
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
  إذا (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  وإلا إذا (NextTokenBody.starts_with("\n"))
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
    أرجع "None";
  حالة Tag::Kind::Normal:
    أرجع "Normal";
  حالة Tag::Kind::Triple:
    أرجع "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] ساكن ثابت حرف *jsonKindToString(json::بەها::Kind K) {
  بدل (K) {
  حالة json::بەها::Kind::Null:
    أرجع "JSON_KIND_NULL";
  حالة json::بەها::Kind::Boolean:
    أرجع "JSON_KIND_BOOLEAN";
  حالة json::بەها::Kind::ژمار:
    أرجع "JSON_KIND_NUMBER";
  حالة json::بەها::Kind::String:
    أرجع "JSON_KIND_STRING";
  حالة json::بەها::Kind::Array:
    أرجع "JSON_KIND_ARRAY";
  حالة json::بەها::Kind::Object:
    أرجع "JSON_KIND_OBJECT";
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

  طالما (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    إذا (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } وإلا إذا (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } وإلا {
      // Not at a tag, continue scanning.
      ++Cursor;
      استمر;
    }

    // Found a tag, first add the preceding text.
    إذا (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    إذا (EndPos == StringRef::npos) {
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
    إذا (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } وإلا { // Normal Tag
      StringRef Interpolated = Content;
      إذا (!Interpolated.trim().starts_with("=")) {
        حرف Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } وإلا { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](حرف C) { أرجع C == '='; });
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
  إذا (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  لكل (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    إذا (!requiresCleanUp(CurrentType))
      استمر;

    منطقي HasTextBehind = hasTextBehind(Idx, Tokens);
    منطقي HasTextAhead = hasTextAhead(Idx, Tokens);

    إذا ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    إذا ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  أرجع Tokens;
}

// Custom stream to escape strings.
صنف EscapeStringStream : عام MustacheOutputStream {
عام:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.کلیلەکان().begin(), Escape.کلیلەکان().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    طالما (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      إذا (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        أرجع;
      }

      // Write the chunk of text before the escapable character.
      إذا (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() ثابت override { أرجع WrappedStream.tell(); }

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
        NeedsIndent(true), IsSuspended(خطأ) {
    SetUnbuffered();
  }

  فراغ suspendIndentation() override { IsSuspended = true; }
  فراغ resumeIndentation() override { IsSuspended = خطأ; }

محمي:
  فراغ write_impl(ثابت حرف *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    لكل (حرف C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      إذا (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = خطأ;
      }
      WrappedStream << C;
      إذا (C == '\n' && !IsSuspended)
        NeedsIndent = true;
    }
  }

  uint64_t current_pos() ثابت override { أرجع WrappedStream.tell(); }

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
  AstPtr Current_گرێ = create_گرێ(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(Current_گرێ);
  ثابت size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  لكل (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  لكل (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  Current_گرێ->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(Current_گرێ);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ڕەگ_گرێ = create_ڕەگ_گرێ(Ctx);
  parseMustache(ڕەگ_گرێ);
  أرجع ڕەگ_گرێ;
}

فراغ Parser::parseMustache(ASTNode *Parent) {

  طالما (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_گرێ;

    بدل (CurrentToken.getType()) {
    حالة Token::Type::Text: {
      Current_گرێ = create_Text_گرێ(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(Current_گرێ);
      اكسر;
    }
    حالة Token::Type::Variable: {
      Current_گرێ = create_گرێ(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(Current_گرێ);
      اكسر;
    }
    حالة Token::Type::UnescapeVariable: {
      Current_گرێ = create_گرێ(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(Current_گرێ);
      اكسر;
    }
    حالة Token::Type::Partial: {
      Current_گرێ = create_گرێ(Ctx, ASTNode::Partial, A, Parent);
      Current_گرێ->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(Current_گرێ);
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
      أرجع;
    }
  }
}
ساكن فراغ toMustacheString(ثابت json::بەها &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  بدل (Data.kind()) {
  حالة json::بەها::Null:
    أرجع;
  حالة json::بەها::ژمار: {
    auto ژمارە = *Data.get_As_ژمار();
    std::ostringstream SS;
    SS << ژمارە;
    OS << SS.str();
    أرجع;
  }
  حالة json::بەها::String: {
    OS << *Data.getAsString();
    أرجع;
  }

  حالة json::بەها::Array: {
    auto Arr = *Data.getAsArray();
    إذا (Arr.empty())
      أرجع;
    [[fallthrough]];
  }
  حالة json::بەها::Object:
  حالة json::بەها::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.بەها_2(Data);
    اكسر;
  }
  }
}

فراغ ASTNode::render_ڕەگ(ثابت json::بەها &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

فراغ ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

فراغ ASTNode::renderPartial(ثابت json::بەها &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_بەها[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(Accessor_بەها[0]);
  إذا (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->get_بەها());
}

فراغ ASTNode::renderVariable(ثابت json::بەها &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(Accessor_بەها[0]);
  إذا (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_بەها());
  } وإلا إذا (ثابت json::بەها *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

فراغ ASTNode::renderUnescapeVariable(ثابت json::بەها &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_بەها[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(Accessor_بەها[0]);
  إذا (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_بەها());
  } وإلا إذا (ثابت json::بەها *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

فراغ ASTNode::renderSection(ثابت json::بەها &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(Accessor_بەها[0]);
  إذا (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_بەها());
    أرجع;
  }

  ثابت json::بەها *ContextPtr = findContext();
  إذا (isContextFalsey(ContextPtr))
    أرجع;

  إذا (ثابت json::Array *Arr = ContextPtr->getAsArray()) {
    لكل (ثابت json::بەها &V : *Arr)
      renderChild(V, OS);
    أرجع;
  }
  renderChild(*ContextPtr, OS);
}

فراغ ASTNode::renderInvertSection(ثابت json::بەها &CurrentCtx,
                                  MustacheOutputStream &OS) {
  منطقي IsLambda = Ctx.SectionLambdas.contains(Accessor_بەها[0]);
  ثابت json::بەها *ContextPtr = findContext();
  إذا (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

فراغ ASTNode::render(ثابت llvm::json::بەها &Data, MustacheOutputStream &OS) {
  إذا (Ty != ڕەگ_2 && Ty != Text && Accessor_بەها.empty())
    أرجع;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  بدل (Ty) {
  حالة ڕەگ_2:
    render_ڕەگ(Data, OS);
    أرجع;
  حالة Text:
    renderText(OS);
    أرجع;
  حالة Partial:
    renderPartial(Data, OS);
    أرجع;
  حالة Variable:
    renderVariable(Data, OS);
    أرجع;
  حالة UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    أرجع;
  حالة Section:
    renderSection(Data, OS);
    أرجع;
  حالة InvertSection:
    renderInvertSection(Data, OS);
    أرجع;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ثابت json::بەها *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  إذا (Accessor_بەها.empty())
    أرجع nullptr;
  إذا (Accessor_بەها[0] == ".")
    أرجع ParentContext;

  ثابت json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = Accessor_بەها[0];
  ASTNode *CurrentParent = Parent;

  طالما (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    إذا (CurrentParent->Ty != ڕەگ_2) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      استمر;
    }
    أرجع nullptr;
  }
  ثابت json::بەها *Context = nullptr;
  لكل (auto [Idx, Acc] : enumerate(Accessor_بەها)) {
    ثابت json::بەها *Current_بەها = CurrentContext->get(Acc);
    إذا (!Current_بەها)
      أرجع nullptr;
    إذا (Idx < Accessor_بەها.size() - 1) {
      CurrentContext = Current_بەها->getAsObject();
      إذا (!CurrentContext)
        أرجع nullptr;
    } وإلا {
      Context = Current_بەها;
    }
  }
  أرجع Context;
}

فراغ ASTNode::renderChild(ثابت json::بەها &Contexts,
                          MustacheOutputStream &OS) {
  لكل (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

فراغ ASTNode::renderPartial(ثابت json::بەها &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

فراغ ASTNode::renderLambdas(ثابت llvm::json::بەها &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::بەها LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرێ = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  إذا (Ty == Variable) {
    Lambda_گرێ->render(Contexts, ES);
    أرجع;
  }
  Lambda_گرێ->render(Contexts, OS);
}

فراغ ASTNode::renderSectionLambdas(ثابت llvm::json::بەها &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::بەها Return = L(RawBody.str());
  إذا (isFalsey(Return))
    أرجع;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_گرێ = P.parse();
  Lambda_گرێ->render(Contexts, OS);
}

فراغ Template::render(ثابت llvm::json::بەها &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  درەخت->render(Data, MOS);
}

فراغ Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_درەخت = P.parse();
  Ctx.Partials.زیادکردن(std::make_pair(Name, Partial_درەخت));
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
  درەخت = P.parse();
  // The default behavior is to escape html entities.
  ثابت EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), درەخت(Other.درەخت) {
  Other.درەخت = nullptr;
}

Template::~Template() = افتراضي;

} // namespace llvm::mustache

#undef DEBUG_TYPE
