//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "llvm/Support/Mustache.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>

#define DEBUG_TYPE "mustache"

using namespace llvm;
using namespace llvm::mustache;

namespace {

using Accessor = ArrayRef<StringRef>;

static bool isFalsey(const json::Value &V) {
  trả_về V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().value()) ||
         (V.getAsArray() && V.getAsArray()->empty());
}

static bool isContextFalsey(const json::Value *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  nếu (!V)
    trả_về true;
  trả_về isFalsey(*V);
}

static void splitAndTrim(StringRef Str, SmallVectorImpl<StringRef> &Tokens) {
  size_t CurrentPos = 0;
  trong_khi (CurrentPos < Str.size()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.find('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    nếu (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.size();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.slice(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t Start = Part.find_first_not_of(" \t\r\n");
    nếu (Start != StringRef::npos) {
      size_t End = Part.find_last_not_of(" \t\r\n");
      Tokens.push_back(Part.slice(Start, End + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

static Accessor splitMustacheString(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  SmallVector<StringRef> Tokens;
  nếu (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    Tokens.push_back(".");
  } không_thì {
    splitAndTrim(Str, Tokens);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *ArenaTokens = Ctx.Allocator.Allocate<StringRef>(Tokens.size());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(Tokens, ArenaTokens);
  // Return an ArrayRef pointing to the stable arena memory.
  trả_về ArrayRef<StringRef>(ArenaTokens, Tokens.size());
}
} // namespace

namespace llvm::mustache {

class MustacheOutputStream : public raw_ostream {
public:
  MustacheOutputStream() = default;
  ~MustacheOutputStream() override = default;

  virtual void suspendIndentation() {}
  virtual void resumeIndentation() {}

private:
  void anchor() override;
};

void MustacheOutputStream::anchor() {}

class RawMustacheOutputStream : public MustacheOutputStream {
public:
  RawMustacheOutputStream(raw_ostream &OS) : OS(OS) { SetUnbuffered(); }

private:
  raw_ostream &OS;

  void write_impl(const char *Ptr, size_t Size) override {
    OS.write(Ptr, Size);
  }
  uint64_t current_pos() const override { trả_về OS.tell(); }
};

class Token {
public:
  enum class Type {
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

  Token(StringRef RawBody, StringRef TokenBody, char Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    nếu (TokenType == Type::Comment)
      trả_về;
    StringRef AccessorStr(this->TokenBody);
    nếu (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    AccessorValue = splitMustacheString(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() const { trả_về AccessorValue; }

  Type getType() const { trả_về TokenType; }

  void setIndentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() const { trả_về Indentation; }

  static Type getTokenType(char Identifier) {
    switch (Identifier) {
    case '#':
      trả_về Type::SectionOpen;
    case '/':
      trả_về Type::SectionClose;
    case '^':
      trả_về Type::InvertSectionOpen;
    case '!':
      trả_về Type::Comment;
    case '>':
      trả_về Type::Partial;
    case '&':
      trả_về Type::UnescapeVariable;
    case '=':
      trả_về Type::SetDelimiter;
    default:
      trả_về Type::Variable;
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

using EscapeMap = DenseMap<char, std::string>;

class ASTNode : public ilist_node<ASTNode> {
public:
  enum Type {
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

  void addChild(AstPtr Child) { Children.push_back(Child); };

  void setRawBody(StringRef NewBody) { RawBody = NewBody; };

  void setIndentation(size_t NewIndentation) { Indentation = NewIndentation; };

  void render(const llvm::json::Value &Data, MustacheOutputStream &OS);

private:
  void renderLambdas(const llvm::json::Value &Contexts,
                     MustacheOutputStream &OS, Lambda &L);

  void renderSectionLambdas(const llvm::json::Value &Contexts,
                            MustacheOutputStream &OS, SectionLambda &L);

  void renderPartial(const llvm::json::Value &Contexts,
                     MustacheOutputStream &OS, ASTNode *Partial);

  void renderChild(const llvm::json::Value &Context, MustacheOutputStream &OS);

  const llvm::json::Value *findContext();

  void renderRoot(const json::Value &CurrentCtx, MustacheOutputStream &OS);
  void renderText(MustacheOutputStream &OS);
  void renderPartial(const json::Value &CurrentCtx, MustacheOutputStream &OS);
  void renderVariable(const json::Value &CurrentCtx, MustacheOutputStream &OS);
  void renderUnescapeVariable(const json::Value &CurrentCtx,
                              MustacheOutputStream &OS);
  void renderSection(const json::Value &CurrentCtx, MustacheOutputStream &OS);
  void renderInvertSection(const json::Value &CurrentCtx,
                           MustacheOutputStream &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *Parent;
  ASTNodeList Children;
  const ArrayRef<StringRef> AccessorValue;
  const llvm::json::Value *ParentContext;
};

// A wrapper for arena allocator for ASTNodes
static AstPtr createRootNode(MustacheContext &Ctx) {
  trả_về new (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

static AstPtr createNode(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *Parent) {
  trả_về new (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, Parent);
}

static AstPtr createTextNode(MustacheContext &Ctx, StringRef Body,
                             ASTNode *Parent) {
  trả_về new (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, Parent);
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
static bool hasTextBehind(size_t Idx, const ArrayRef<Token> &Tokens) {
  nếu (Idx == 0)
    trả_về true;

  size_t PrevIdx = Idx - 1;
  nếu (Tokens[PrevIdx].getType() != Token::Type::Text)
    trả_về true;

  const Token &PrevToken = Tokens[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  trả_về !TokenBody.ends_with("\n") && !(TokenBody.empty() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
static bool hasTextAhead(size_t Idx, const ArrayRef<Token> &Tokens) {
  nếu (Idx >= Tokens.size() - 1)
    trả_về true;

  size_t NextIdx = Idx + 1;
  nếu (Tokens[NextIdx].getType() != Token::Type::Text)
    trả_về true;

  const Token &NextToken = Tokens[NextIdx];
  StringRef TokenBody = StringRef(NextToken.RawBody).ltrim(" ");
  trả_về !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

static bool requiresCleanUp(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  trả_về T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::SetDelimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
static void stripTokenAhead(SmallVectorImpl<Token> &Tokens, size_t Idx) {
  Token &NextToken = Tokens[Idx + 1];
  StringRef NextTokenBody = NextToken.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  nếu (NextTokenBody.starts_with("\r\n"))
    NextToken.TokenBody = NextTokenBody.substr(2);
  không_thì nếu (NextTokenBody.starts_with("\n"))
    NextToken.TokenBody = NextTokenBody.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
void stripTokenBefore(SmallVectorImpl<Token> &Tokens, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = Tokens[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.size() - Unindented.size();
  PrevToken.TokenBody = Unindented;
  CurrentToken.setIndentation(Indentation);
}

struct Tag {
  enum class Kind {
    None,
    Normal, // {{...}}
    Triple, // {{{...}}}
  };

  Kind TagKind = Kind::None;
  StringRef Content;   // The content between the delimiters.
  StringRef FullMatch; // The entire tag, including delimiters.
  size_t StartPosition = StringRef::npos;
};

[[maybe_unused]] static const char *tagKindToString(Tag::Kind K) {
  switch (K) {
  case Tag::Kind::None:
    trả_về "None";
  case Tag::Kind::Normal:
    trả_về "Normal";
  case Tag::Kind::Triple:
    trả_về "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_unused]] static const char *jsonKindToString(json::Value::Kind K) {
  switch (K) {
  case json::Value::Kind::Null:
    trả_về "JSON_KIND_NULL";
  case json::Value::Kind::Boolean:
    trả_về "JSON_KIND_BOOLEAN";
  case json::Value::Kind::Number:
    trả_về "JSON_KIND_NUMBER";
  case json::Value::Kind::String:
    trả_về "JSON_KIND_STRING";
  case json::Value::Kind::Array:
    trả_về "JSON_KIND_ARRAY";
  case json::Value::Kind::Object:
    trả_về "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
static SmallVector<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  SmallVector<Token> Tokens;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t TextStart = 0;

  const StringLiteral TripleOpen("{{{");
  const StringLiteral TripleClose("}}}");

  trong_khi (Cursor < Template.size()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef TagOpen, TagClose;
    Tag::Kind Kind;

    // Determine which tag we've encountered.
    nếu (TemplateSuffix.starts_with(TripleOpen)) {
      Kind = Tag::Kind::Triple;
      TagOpen = TripleOpen;
      TagClose = TripleClose;
    } không_thì nếu (TemplateSuffix.starts_with(Open)) {
      Kind = Tag::Kind::Normal;
      TagOpen = Open;
      TagClose = Close;
    } không_thì {
      // Not at a tag, continue scanning.
      ++Cursor;
      continue;
    }

    // Found a tag, first add the preceding text.
    nếu (Cursor > TextStart)
      Tokens.emplace_back(Template.slice(TextStart, Cursor));

    // Find the closing tag.
    size_t EndPos = Template.find(TagClose, Cursor + TagOpen.size());
    nếu (EndPos == StringRef::npos) {
      // No closing tag, the rest is text.
      Tokens.emplace_back(Template.substr(Cursor));
      TextStart = Cursor = Template.size();
      break;
    }

    // Extract tag content and full match.
    size_t ContentStart = Cursor + TagOpen.size();
    StringRef Content = Template.substr(ContentStart, EndPos - ContentStart);
    StringRef FullMatch =
        Template.substr(Cursor, (EndPos + TagClose.size()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << FullMatch << ", Content: " << Content
                      << ", Kind: " << tagKindToString(Kind) << "\n");
    nếu (Kind == Tag::Kind::Triple) {
      Tokens.emplace_back(FullMatch, Ctx.Saver.save("&" + Content), '&', Ctx);
    } không_thì { // Normal Tag
      StringRef Interpolated = Content;
      nếu (!Interpolated.trim().starts_with("=")) {
        char Front = Interpolated.empty() ? ' ' : Interpolated.trim().front();
        Tokens.emplace_back(FullMatch, Interpolated, Front, Ctx);
      } không_thì { // Set Delimiter
        Tokens.emplace_back(FullMatch, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](char C) { trả_về C == '='; });
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
  nếu (TextStart < Template.size())
    Tokens.emplace_back(Template.substr(TextStart));

  // Fix up white spaces for standalone tags.
  size_t LastIdx = Tokens.size() - 1;
  cho (size_t Idx = 0, End = Tokens.size(); Idx < End; ++Idx) {
    Token &CurrentToken = Tokens[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    nếu (!requiresCleanUp(CurrentType))
      continue;

    bool HasTextBehind = hasTextBehind(Idx, Tokens);
    bool HasTextAhead = hasTextAhead(Idx, Tokens);

    nếu ((!HasTextAhead && !HasTextBehind) || (!HasTextAhead && Idx == 0))
      stripTokenAhead(Tokens, Idx);

    nếu ((!HasTextBehind && !HasTextAhead) || (!HasTextBehind && Idx == LastIdx))
      stripTokenBefore(Tokens, Idx, CurrentToken, CurrentType);
  }
  trả_về Tokens;
}

// Custom stream to escape strings.
class EscapeStringStream : public MustacheOutputStream {
public:
  explicit EscapeStringStream(llvm::raw_ostream &WrappedStream,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.keys().begin(), Escape.keys().end()),
        WrappedStream(WrappedStream) {
    SetUnbuffered();
  }

protected:
  void write_impl(const char *Ptr, size_t Size) override {
    StringRef Data(Ptr, Size);
    size_t Start = 0;
    trong_khi (Start < Size) {
      // Find the next character that needs to be escaped.
      size_t Next = Data.find_first_of(EscapeChars.str(), Start);

      // If no escapable characters are found, write the rest of the string.
      nếu (Next == StringRef::npos) {
        WrappedStream << Data.substr(Start);
        trả_về;
      }

      // Write the chunk of text before the escapable character.
      nếu (Next > Start)
        WrappedStream << Data.substr(Start, Next - Start);

      // Look up and write the escaped version of the character.
      WrappedStream << Escape[Data[Next]];
      Start = Next + 1;
    }
  }

  uint64_t current_pos() const override { trả_về WrappedStream.tell(); }

private:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &WrappedStream;
};

// Custom stream to add indentation used to for rendering partials.
class AddIndentationStringStream : public MustacheOutputStream {
public:
  explicit AddIndentationStringStream(raw_ostream &WrappedStream,
                                      size_t Indentation)
      : Indentation(Indentation), WrappedStream(WrappedStream),
        NeedsIndent(true), IsSuspended(false) {
    SetUnbuffered();
  }

  void suspendIndentation() override { IsSuspended = true; }
  void resumeIndentation() override { IsSuspended = false; }

protected:
  void write_impl(const char *Ptr, size_t Size) override {
    llvm::StringRef Data(Ptr, Size);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    cho (char C : Data) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      nếu (NeedsIndent && C != '\n') {
        WrappedStream << Indent;
        NeedsIndent = false;
      }
      WrappedStream << C;
      nếu (C == '\n' && !IsSuspended)
        NeedsIndent = true;
    }
  }

  uint64_t current_pos() const override { trả_về WrappedStream.tell(); }

private:
  size_t Indentation;
  raw_ostream &WrappedStream;
  bool NeedsIndent;
  bool IsSuspended;
};

class Parser {
public:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

private:
  void parseMustache(ASTNode *Parent);
  void parseSection(ASTNode *Parent, ASTNode::Type Ty, const Accessor &A);

  MustacheContext &Ctx;
  SmallVector<Token> Tokens;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

void Parser::parseSection(ASTNode *Parent, ASTNode::Type Ty,
                          const Accessor &A) {
  AstPtr CurrentNode = createNode(Ctx, Ty, A, Parent);
  size_t Start = CurrentPtr;
  parseMustache(CurrentNode);
  const size_t End = CurrentPtr - 1;

  size_t RawBodySize = 0;
  cho (size_t I = Start; I < End; ++I)
    RawBodySize += Tokens[I].RawBody.size();

  SmallString<128> RawBody;
  RawBody.reserve(RawBodySize);
  cho (std::size_t I = Start; I < End; ++I)
    RawBody += Tokens[I].RawBody;

  CurrentNode->setRawBody(Ctx.Saver.save(StringRef(RawBody)));
  Parent->addChild(CurrentNode);
}

AstPtr Parser::parse() {
  Tokens = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr RootNode = createRootNode(Ctx);
  parseMustache(RootNode);
  trả_về RootNode;
}

void Parser::parseMustache(ASTNode *Parent) {

  trong_khi (CurrentPtr < Tokens.size()) {
    Token CurrentToken = Tokens[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr CurrentNode;

    switch (CurrentToken.getType()) {
    case Token::Type::Text: {
      CurrentNode = createTextNode(Ctx, CurrentToken.TokenBody, Parent);
      Parent->addChild(CurrentNode);
      break;
    }
    case Token::Type::Variable: {
      CurrentNode = createNode(Ctx, ASTNode::Variable, A, Parent);
      Parent->addChild(CurrentNode);
      break;
    }
    case Token::Type::UnescapeVariable: {
      CurrentNode = createNode(Ctx, ASTNode::UnescapeVariable, A, Parent);
      Parent->addChild(CurrentNode);
      break;
    }
    case Token::Type::Partial: {
      CurrentNode = createNode(Ctx, ASTNode::Partial, A, Parent);
      CurrentNode->setIndentation(CurrentToken.getIndentation());
      Parent->addChild(CurrentNode);
      break;
    }
    case Token::Type::SectionOpen: {
      parseSection(Parent, ASTNode::Section, A);
      break;
    }
    case Token::Type::InvertSectionOpen: {
      parseSection(Parent, ASTNode::InvertSection, A);
      break;
    }
    case Token::Type::Comment:
    case Token::Type::SetDelimiter:
      break;
    case Token::Type::SectionClose:
      trả_về;
    }
  }
}
static void toMustacheString(const json::Value &Data, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << jsonKindToString(Data.kind()) << ", Data: " << Data
                    << "\n");
  switch (Data.kind()) {
  case json::Value::Null:
    trả_về;
  case json::Value::Number: {
    auto Num = *Data.getAsNumber();
    std::ostringstream SS;
    SS << Num;
    OS << SS.str();
    trả_về;
  }
  case json::Value::String: {
    OS << *Data.getAsString();
    trả_về;
  }

  case json::Value::Array: {
    auto Arr = *Data.getAsArray();
    nếu (Arr.empty())
      trả_về;
    [[fallthrough]];
  }
  case json::Value::Object:
  case json::Value::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.value(Data);
    break;
  }
  }
}

void ASTNode::renderRoot(const json::Value &CurrentCtx,
                         MustacheOutputStream &OS) {
  renderChild(CurrentCtx, OS);
}

void ASTNode::renderText(MustacheOutputStream &OS) { OS << Body; }

void ASTNode::renderPartial(const json::Value &CurrentCtx,
                            MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << AccessorValue[0]
                    << ", Indentation:" << Indentation << "\n");
  auto Partial = Ctx.Partials.find(AccessorValue[0]);
  nếu (Partial != Ctx.Partials.end())
    renderPartial(CurrentCtx, OS, Partial->getValue());
}

void ASTNode::renderVariable(const json::Value &CurrentCtx,
                             MustacheOutputStream &OS) {
  auto Lambda = Ctx.Lambdas.find(AccessorValue[0]);
  nếu (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->getValue());
  } không_thì nếu (const json::Value *ContextPtr = findContext()) {
    EscapeStringStream ES(OS, Ctx.Escapes);
    toMustacheString(*ContextPtr, ES);
  }
}

void ASTNode::renderUnescapeVariable(const json::Value &CurrentCtx,
                                     MustacheOutputStream &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << AccessorValue[0]
                    << "\n");
  auto Lambda = Ctx.Lambdas.find(AccessorValue[0]);
  nếu (Lambda != Ctx.Lambdas.end()) {
    renderLambdas(CurrentCtx, OS, Lambda->getValue());
  } không_thì nếu (const json::Value *ContextPtr = findContext()) {
    OS.suspendIndentation();
    toMustacheString(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

void ASTNode::renderSection(const json::Value &CurrentCtx,
                            MustacheOutputStream &OS) {
  auto SectionLambda = Ctx.SectionLambdas.find(AccessorValue[0]);
  nếu (SectionLambda != Ctx.SectionLambdas.end()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->getValue());
    trả_về;
  }

  const json::Value *ContextPtr = findContext();
  nếu (isContextFalsey(ContextPtr))
    trả_về;

  nếu (const json::Array *Arr = ContextPtr->getAsArray()) {
    cho (const json::Value &V : *Arr)
      renderChild(V, OS);
    trả_về;
  }
  renderChild(*ContextPtr, OS);
}

void ASTNode::renderInvertSection(const json::Value &CurrentCtx,
                                  MustacheOutputStream &OS) {
  bool IsLambda = Ctx.SectionLambdas.contains(AccessorValue[0]);
  const json::Value *ContextPtr = findContext();
  nếu (isContextFalsey(ContextPtr) && !IsLambda) {
    renderChild(CurrentCtx, OS);
  }
}

void ASTNode::render(const llvm::json::Value &Data, MustacheOutputStream &OS) {
  nếu (Ty != Root && Ty != Text && AccessorValue.empty())
    trả_về;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ParentContext = &Data;

  switch (Ty) {
  case Root:
    renderRoot(Data, OS);
    trả_về;
  case Text:
    renderText(OS);
    trả_về;
  case Partial:
    renderPartial(Data, OS);
    trả_về;
  case Variable:
    renderVariable(Data, OS);
    trả_về;
  case UnescapeVariable:
    renderUnescapeVariable(Data, OS);
    trả_về;
  case Section:
    renderSection(Data, OS);
    trả_về;
  case InvertSection:
    renderInvertSection(Data, OS);
    trả_về;
  }
  llvm_unreachable("Invalid ASTNode type");
}

const json::Value *ASTNode::findContext() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  nếu (AccessorValue.empty())
    trả_về nullptr;
  nếu (AccessorValue[0] == ".")
    trả_về ParentContext;

  const json::Object *CurrentContext = ParentContext->getAsObject();
  StringRef CurrentAccessor = AccessorValue[0];
  ASTNode *CurrentParent = Parent;

  trong_khi (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    nếu (CurrentParent->Ty != Root) {
      CurrentContext = CurrentParent->ParentContext->getAsObject();
      CurrentParent = CurrentParent->Parent;
      continue;
    }
    trả_về nullptr;
  }
  const json::Value *Context = nullptr;
  cho (auto [Idx, Acc] : enumerate(AccessorValue)) {
    const json::Value *CurrentValue = CurrentContext->get(Acc);
    nếu (!CurrentValue)
      trả_về nullptr;
    nếu (Idx < AccessorValue.size() - 1) {
      CurrentContext = CurrentValue->getAsObject();
      nếu (!CurrentContext)
        trả_về nullptr;
    } không_thì {
      Context = CurrentValue;
    }
  }
  trả_về Context;
}

void ASTNode::renderChild(const json::Value &Contexts,
                          MustacheOutputStream &OS) {
  cho (ASTNode &Child : Children)
    Child.render(Contexts, OS);
}

void ASTNode::renderPartial(const json::Value &Contexts,
                            MustacheOutputStream &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  AddIndentationStringStream IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

void ASTNode::renderLambdas(const llvm::json::Value &Contexts,
                            MustacheOutputStream &OS, Lambda &L) {
  json::Value LambdaResult = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(LambdaResult, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr LambdaNode = P.parse();

  EscapeStringStream ES(OS, Ctx.Escapes);
  nếu (Ty == Variable) {
    LambdaNode->render(Contexts, ES);
    trả_về;
  }
  LambdaNode->render(Contexts, OS);
}

void ASTNode::renderSectionLambdas(const llvm::json::Value &Contexts,
                                   MustacheOutputStream &OS, SectionLambda &L) {
  json::Value Return = L(RawBody.str());
  nếu (isFalsey(Return))
    trả_về;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  toMustacheString(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr LambdaNode = P.parse();
  LambdaNode->render(Contexts, OS);
}

void Template::render(const llvm::json::Value &Data, llvm::raw_ostream &OS) {
  RawMustacheOutputStream MOS(OS);
  Tree->render(Data, MOS);
}

void Template::registerPartial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr PartialTree = P.parse();
  Ctx.Partials.insert(std::make_pair(Name, PartialTree));
}

void Template::registerLambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

void Template::registerLambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

void Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  Tree = P.parse();
  // The default behavior is to escape html entities.
  const EscapeMap HtmlEntities = {{'&', "&amp;"},
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

Template::~Template() = default;

} // namespace llvm::mustache

#undef DEBUG_TYPE
