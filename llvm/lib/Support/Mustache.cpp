//===-- Mustache.cpp ------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Mustache.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <sstream>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_TYPE "mustache"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::mustache;

ਨਾਮਾਕਾਸ਼ {

ਵਰਤੋਂ Accessor = ArrayRef<StringRef>;

ਸਥਿਰ ਤਾਰਕਿਕ isFalsey(ਅਚਲ json::ਮੁੱਲ &V) {
  ਵਾਪਸ V.getAsNull() || (V.getAsBoolean() && !V.getAsBoolean().ਮੁੱਲ_2()) ||
         (V.getAsArray() && V.getAsArray()->ਖਾਲੀ_ਥਾਵਾਂ());
}

ਸਥਿਰ ਤਾਰਕਿਕ isContextFalsey(ਅਚਲ json::ਮੁੱਲ *V) {
  // A missing context (represented by a nullptr) is defined as falsey.
  ਜੇ (!V)
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ isFalsey(*V);
}

ਸਥਿਰ ਖਾਲੀ ਵੰਡੋ_And_Trim(StringRef Str, Small_ਸਦਿਸ਼_Impl<StringRef> &ਟੋਕਨ) {
  size_t CurrentPos = 0;
  ਜਦੋਂ (CurrentPos < Str.ਮਾਪ_2()) {
    // Find the next delimiter.
    size_t DelimiterPos = Str.ਲੱਭੋ('.', CurrentPos);

    // If no delimiter is found, process the rest of the string.
    ਜੇ (DelimiterPos == StringRef::npos)
      DelimiterPos = Str.ਮਾਪ_2();

    // Get the current part, which may have whitespace.
    StringRef Part = Str.ਟੁਕੜਾ(CurrentPos, DelimiterPos);

    // Manually trim the part without creating a new string object.
    size_t ਸ਼ੁਰੂ = Part.ਲੱਭੋ_ਪਹਿਲਾ_not_ਦਾ(" \t\r\n");
    ਜੇ (ਸ਼ੁਰੂ != StringRef::npos) {
      size_t ਸਿਰਾ_2 = Part.ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ(" \t\r\n");
      ਟੋਕਨ.ਧੱਕੋ_ਪਿੱਠ(Part.ਟੁਕੜਾ(ਸ਼ੁਰੂ, ਸਿਰਾ_2 + 1));
    }

    // Move past the delimiter for the next iteration.
    CurrentPos = DelimiterPos + 1;
  }
}

ਸਥਿਰ Accessor ਵੰਡੋ_Mustache_String(StringRef Str, MustacheContext &Ctx) {
  // We split the mustache string into an accessor.
  // For example:
  //    "a.b.c" would be split into {"a", "b", "c"}
  // We make an exception for a single dot which
  // refers to the current context.
  Small_ਸਦਿਸ਼<StringRef> ਟੋਕਨ;
  ਜੇ (Str == ".") {
    // "." is a special accessor that refers to the current context.
    // It's a literal, so it doesn't need to be saved.
    ਟੋਕਨ.ਧੱਕੋ_ਪਿੱਠ(".");
  } ਨਹੀਂਤਾਂ {
    ਵੰਡੋ_And_Trim(Str, ਟੋਕਨ);
  }
  // Now, allocate memory for the array of StringRefs in the arena.
  StringRef *Arena_ਟੋਕਨ = Ctx.Allocator.Allocate<StringRef>(ਟੋਕਨ.ਮਾਪ_2());
  // Copy the StringRefs from the stack vector to the arena.
  llvm::copy(ਟੋਕਨ, Arena_ਟੋਕਨ);
  // Return an ArrayRef pointing to the stable arena memory.
  ਵਾਪਸ ArrayRef<StringRef>(Arena_ਟੋਕਨ, ਟੋਕਨ.ਮਾਪ_2());
}
} // namespace

ਨਾਮਾਕਾਸ਼ llvm::mustache {

ਵਰਗ Mustache_Output_ਧਾਰਾ : ਜਨਤਕ raw_ostream {
ਜਨਤਕ:
  Mustache_Output_ਧਾਰਾ() = ਮੂਲ;
  ~Mustache_Output_ਧਾਰਾ() override = ਮੂਲ;

  ਆਭਾਸੀ ਖਾਲੀ suspendIndentation() {}
  ਆਭਾਸੀ ਖਾਲੀ resumeIndentation() {}

ਨਿਜੀ:
  ਖਾਲੀ anchor() override;
};

ਖਾਲੀ Mustache_Output_ਧਾਰਾ::anchor() {}

ਵਰਗ Raw_Mustache_Output_ਧਾਰਾ : ਜਨਤਕ Mustache_Output_ਧਾਰਾ {
ਜਨਤਕ:
  Raw_Mustache_Output_ਧਾਰਾ(raw_ostream &OS) : OS(OS) { ਸੈੱਟ_Unbuffered(); }

ਨਿਜੀ:
  raw_ostream &OS;

  ਖਾਲੀ write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) override {
    OS.ਲਿਖ(Ptr, ਮਾਪ);
  }
  uint64_t current_pos() ਅਚਲ override { ਵਾਪਸ OS.tell(); }
};

ਵਰਗ Token {
ਜਨਤਕ:
  ਗਿਣਤੀ ਵਰਗ Type {
    ਪਾਠ_2,
    Variable,
    Partial,
    SectionOpen,
    SectionClose,
    InvertSectionOpen,
    UnescapeVariable,
    Comment,
    ਸੈੱਟ_Delimiter,
  };

  Token(StringRef Str)
      : TokenType(Type::ਪਾਠ_2), RawBody(Str), TokenBody(RawBody),
        Accessor_ਮੁੱਲ({}), Indentation(0) {};

  Token(StringRef RawBody, StringRef TokenBody, ਅੱਖਰ Identifier,
        MustacheContext &Ctx)
      : RawBody(RawBody), TokenBody(TokenBody), Indentation(0) {
    TokenType = getTokenType(Identifier);
    ਜੇ (TokenType == Type::Comment)
      ਵਾਪਸ;
    StringRef AccessorStr(ਇਹ->TokenBody);
    ਜੇ (TokenType != Type::Variable)
      AccessorStr = AccessorStr.substr(1);
    Accessor_ਮੁੱਲ = ਵੰਡੋ_Mustache_String(StringRef(AccessorStr).trim(), Ctx);
  }

  ArrayRef<StringRef> getAccessor() ਅਚਲ { ਵਾਪਸ Accessor_ਮੁੱਲ; }

  Type getType() ਅਚਲ { ਵਾਪਸ TokenType; }

  ਖਾਲੀ ਸੈੱਟ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; }

  size_t getIndentation() ਅਚਲ { ਵਾਪਸ Indentation; }

  ਸਥਿਰ Type getTokenType(ਅੱਖਰ Identifier) {
    ਚੋਣ (Identifier) {
    ਹਾਲਤ '#':
      ਵਾਪਸ Type::SectionOpen;
    ਹਾਲਤ '/':
      ਵਾਪਸ Type::SectionClose;
    ਹਾਲਤ '^':
      ਵਾਪਸ Type::InvertSectionOpen;
    ਹਾਲਤ '!':
      ਵਾਪਸ Type::Comment;
    ਹਾਲਤ '>':
      ਵਾਪਸ Type::Partial;
    ਹਾਲਤ '&':
      ਵਾਪਸ Type::UnescapeVariable;
    ਹਾਲਤ '=':
      ਵਾਪਸ Type::ਸੈੱਟ_Delimiter;
    ਮੂਲ:
      ਵਾਪਸ Type::Variable;
    }
  }

  Type TokenType;
  // RawBody is the original string that was tokenized.
  StringRef RawBody;
  // TokenBody is the original string with the identifier removed.
  StringRef TokenBody;
  ArrayRef<StringRef> Accessor_ਮੁੱਲ;
  size_t Indentation;
};

ਵਰਤੋਂ EscapeMap = DenseMap<ਅੱਖਰ, std::string>;

ਵਰਗ ASTNode : ਜਨਤਕ ilist_ਗੰਢ<ASTNode> {
ਜਨਤਕ:
  ਗਿਣਤੀ Type {
    ਜੜ੍ਹ_2,
    ਪਾਠ_2,
    Partial,
    Variable,
    UnescapeVariable,
    Section,
    InvertSection,
  };

  ASTNode(MustacheContext &Ctx)
      : Ctx(Ctx), Ty(Type::ਜੜ੍ਹ_2), ਮਾਪਾ_2(ਖਾਲੀ_ਸੂਚਕ), ਮਾਪਾ_Context(ਖਾਲੀ_ਸੂਚਕ) {}

  ASTNode(MustacheContext &Ctx, StringRef Body, ASTNode *ਮਾਪਾ_2)
      : Ctx(Ctx), Ty(Type::ਪਾਠ_2), Body(Body), ਮਾਪਾ_2(ਮਾਪਾ_2),
        ਮਾਪਾ_Context(ਖਾਲੀ_ਸੂਚਕ) {}

  // Constructor for Section/InvertSection/Variable/UnescapeVariable Nodes
  ASTNode(MustacheContext &Ctx, Type Ty, ArrayRef<StringRef> Accessor,
          ASTNode *ਮਾਪਾ_2)
      : Ctx(Ctx), Ty(Ty), ਮਾਪਾ_2(ਮਾਪਾ_2), Accessor_ਮੁੱਲ(Accessor),
        ਮਾਪਾ_Context(ਖਾਲੀ_ਸੂਚਕ) {}

  ਖਾਲੀ ਜੋੜੋ_ਬੱਚਾ(AstPtr ਬੱਚਾ) { Children.ਧੱਕੋ_ਪਿੱਠ(ਬੱਚਾ); };

  ਖਾਲੀ ਸੈੱਟ_Raw_Body(StringRef NewBody) { RawBody = NewBody; };

  ਖਾਲੀ ਸੈੱਟ_Indentation(size_t NewIndentation) { Indentation = NewIndentation; };

  ਖਾਲੀ render(ਅਚਲ llvm::json::ਮੁੱਲ &ਅੰਕੜੇ_2, Mustache_Output_ਧਾਰਾ &OS);

ਨਿਜੀ:
  ਖਾਲੀ renderLambdas(ਅਚਲ llvm::json::ਮੁੱਲ &Contexts,
                     Mustache_Output_ਧਾਰਾ &OS, Lambda &L);

  ਖਾਲੀ renderSectionLambdas(ਅਚਲ llvm::json::ਮੁੱਲ &Contexts,
                            Mustache_Output_ਧਾਰਾ &OS, SectionLambda &L);

  ਖਾਲੀ renderPartial(ਅਚਲ llvm::json::ਮੁੱਲ &Contexts,
                     Mustache_Output_ਧਾਰਾ &OS, ASTNode *Partial);

  ਖਾਲੀ render_ਬੱਚਾ(ਅਚਲ llvm::json::ਮੁੱਲ &Context, Mustache_Output_ਧਾਰਾ &OS);

  ਅਚਲ llvm::json::ਮੁੱਲ *ਲੱਭੋ_Context();

  ਖਾਲੀ render_ਜੜ੍ਹ(ਅਚਲ json::ਮੁੱਲ &CurrentCtx, Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ render_ਪਾਠ(Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ renderPartial(ਅਚਲ json::ਮੁੱਲ &CurrentCtx, Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ renderVariable(ਅਚਲ json::ਮੁੱਲ &CurrentCtx, Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ renderUnescapeVariable(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                              Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ renderSection(ਅਚਲ json::ਮੁੱਲ &CurrentCtx, Mustache_Output_ਧਾਰਾ &OS);
  ਖਾਲੀ renderInvertSection(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                           Mustache_Output_ਧਾਰਾ &OS);

  MustacheContext &Ctx;
  Type Ty;
  size_t Indentation = 0;
  StringRef RawBody;
  StringRef Body;
  ASTNode *ਮਾਪਾ_2;
  ASTNodeList Children;
  ਅਚਲ ArrayRef<StringRef> Accessor_ਮੁੱਲ;
  ਅਚਲ llvm::json::ਮੁੱਲ *ਮਾਪਾ_Context;
};

// A wrapper for arena allocator for ASTNodes
ਸਥਿਰ AstPtr create_ਜੜ੍ਹ_ਗੰਢ(MustacheContext &Ctx) {
  ਵਾਪਸ ਨਵਾਂ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx);
}

ਸਥਿਰ AstPtr create_ਗੰਢ(MustacheContext &Ctx, ASTNode::Type T,
                         ArrayRef<StringRef> A, ASTNode *ਮਾਪਾ_2) {
  ਵਾਪਸ ਨਵਾਂ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, T, A, ਮਾਪਾ_2);
}

ਸਥਿਰ AstPtr create_ਪਾਠ_ਗੰਢ(MustacheContext &Ctx, StringRef Body,
                             ASTNode *ਮਾਪਾ_2) {
  ਵਾਪਸ ਨਵਾਂ (Ctx.Allocator.Allocate<ASTNode>()) ASTNode(Ctx, Body, ਮਾਪਾ_2);
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
ਸਥਿਰ ਤਾਰਕਿਕ has_ਪਾਠ_Behind(size_t Idx, ਅਚਲ ArrayRef<Token> &ਟੋਕਨ) {
  ਜੇ (Idx == 0)
    ਵਾਪਸ ਸੱਚ;

  size_t PrevIdx = Idx - 1;
  ਜੇ (ਟੋਕਨ[PrevIdx].getType() != Token::Type::ਪਾਠ_2)
    ਵਾਪਸ ਸੱਚ;

  ਅਚਲ Token &PrevToken = ਟੋਕਨ[PrevIdx];
  StringRef TokenBody = StringRef(PrevToken.RawBody).rtrim(" \r\t\v");
  ਵਾਪਸ !TokenBody.ਸਿਰੇ_with("\n") && !(TokenBody.ਖਾਲੀ_ਥਾਵਾਂ() && Idx == 1);
}

// Function to check if there's no meaningful text ahead.
// We determine if a token has text ahead if the left of previous
// token does not start with a newline.
ਸਥਿਰ ਤਾਰਕਿਕ has_ਪਾਠ_Ahead(size_t Idx, ਅਚਲ ArrayRef<Token> &ਟੋਕਨ) {
  ਜੇ (Idx >= ਟੋਕਨ.ਮਾਪ_2() - 1)
    ਵਾਪਸ ਸੱਚ;

  size_t ਅਗਲਾ_Idx = Idx + 1;
  ਜੇ (ਟੋਕਨ[ਅਗਲਾ_Idx].getType() != Token::Type::ਪਾਠ_2)
    ਵਾਪਸ ਸੱਚ;

  ਅਚਲ Token &ਅਗਲਾ_Token = ਟੋਕਨ[ਅਗਲਾ_Idx];
  StringRef TokenBody = StringRef(ਅਗਲਾ_Token.RawBody).ltrim(" ");
  ਵਾਪਸ !TokenBody.starts_with("\r\n") && !TokenBody.starts_with("\n");
}

ਸਥਿਰ ਤਾਰਕਿਕ requires_Clean_ਉੱਪਰ(Token::Type T) {
  // We must clean up all the tokens that could contain child nodes.
  ਵਾਪਸ T == Token::Type::SectionOpen || T == Token::Type::InvertSectionOpen ||
         T == Token::Type::SectionClose || T == Token::Type::Comment ||
         T == Token::Type::Partial || T == Token::Type::ਸੈੱਟ_Delimiter;
}

// Adjust next token body if there is no text ahead.
// For example:
// The template string
//  "{{! Comment }} \nLine 2"
// would be considered as no text ahead and should be rendered as
//  " Line 2"
ਸਥਿਰ ਖਾਲੀ stripTokenAhead(Small_ਸਦਿਸ਼_Impl<Token> &ਟੋਕਨ, size_t Idx) {
  Token &ਅਗਲਾ_Token = ਟੋਕਨ[Idx + 1];
  StringRef ਅਗਲਾ_Token_Body = ਅਗਲਾ_Token.TokenBody;
  // Cut off the leading newline which could be \n or \r\n.
  ਜੇ (ਅਗਲਾ_Token_Body.starts_with("\r\n"))
    ਅਗਲਾ_Token.TokenBody = ਅਗਲਾ_Token_Body.substr(2);
  ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ_Token_Body.starts_with("\n"))
    ਅਗਲਾ_Token.TokenBody = ਅਗਲਾ_Token_Body.substr(1);
}

// Adjust previous token body if there no text behind.
// For example:
//  The template string
//  " \t{{#section}}A{{/section}}"
// would be considered as having no text ahead and would be render as:
//  "A"
ਖਾਲੀ stripTokenBefore(Small_ਸਦਿਸ਼_Impl<Token> &ਟੋਕਨ, size_t Idx,
                      Token &CurrentToken, Token::Type CurrentType) {
  Token &PrevToken = ਟੋਕਨ[Idx - 1];
  StringRef PrevTokenBody = PrevToken.TokenBody;
  StringRef Unindented = PrevTokenBody.rtrim(" \r\t\v");
  size_t Indentation = PrevTokenBody.ਮਾਪ_2() - Unindented.ਮਾਪ_2();
  PrevToken.TokenBody = Unindented;
  CurrentToken.ਸੈੱਟ_Indentation(Indentation);
}

ਬਣਤਰ ਟੈਗ {
  ਗਿਣਤੀ ਵਰਗ ਕਿਸਮ_3 {
    None,
    ਸਾਧਾਰਨ, // {{...}}
    Triple, // {{{...}}}
  };

  ਕਿਸਮ_3 ਟੈਗ_ਕਿਸਮ = ਕਿਸਮ_3::None;
  StringRef Content;   // The content between the delimiters.
  StringRef ਭਰਿਆ_ਮੇਲ; // The entire tag, including delimiters.
  size_t ਸ਼ੁਰੂ_Position = StringRef::npos;
};

[[maybe_ਅਣਵਰਤਿਆ]] ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਟੈਗ_ਕਿਸਮ_ਮੰਜ਼ਿਲ_String(ਟੈਗ::ਕਿਸਮ_3 K) {
  ਚੋਣ (K) {
  ਹਾਲਤ ਟੈਗ::ਕਿਸਮ_3::None:
    ਵਾਪਸ "None";
  ਹਾਲਤ ਟੈਗ::ਕਿਸਮ_3::ਸਾਧਾਰਨ:
    ਵਾਪਸ "Normal";
  ਹਾਲਤ ਟੈਗ::ਕਿਸਮ_3::Triple:
    ਵਾਪਸ "Triple";
  }
  llvm_unreachable("Unknown Tag::Kind");
}

[[maybe_ਅਣਵਰਤਿਆ]] ਸਥਿਰ ਅਚਲ ਅੱਖਰ *json_ਕਿਸਮ_ਮੰਜ਼ਿਲ_String(json::ਮੁੱਲ::ਕਿਸਮ_3 K) {
  ਚੋਣ (K) {
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::Null:
    ਵਾਪਸ "JSON_KIND_NULL";
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::Boolean:
    ਵਾਪਸ "JSON_KIND_BOOLEAN";
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::ਸੰਖਿਆ:
    ਵਾਪਸ "JSON_KIND_NUMBER";
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::String:
    ਵਾਪਸ "JSON_KIND_STRING";
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::Array:
    ਵਾਪਸ "JSON_KIND_ARRAY";
  ਹਾਲਤ json::ਮੁੱਲ::ਕਿਸਮ_3::Object:
    ਵਾਪਸ "JSON_KIND_OBJECT";
  }
  llvm_unreachable("Unknown json::Value::Kind");
}

// Simple tokenizer that splits the template into tokens.
ਸਥਿਰ Small_ਸਦਿਸ਼<Token> tokenize(StringRef Template, MustacheContext &Ctx) {
  LLVM_DEBUG(dbgs() << "[Tokenize Template] \"" << Template << "\"\n");
  Small_ਸਦਿਸ਼<Token> ਟੋਕਨ;
  SmallString<8> Open("{{");
  SmallString<8> Close("}}");
  size_t Cursor = 0;
  size_t ਪਾਠ_ਸ਼ੁਰੂ = 0;

  ਅਚਲ StringLiteral TripleOpen("{{{");
  ਅਚਲ StringLiteral TripleClose("}}}");

  ਜਦੋਂ (Cursor < Template.ਮਾਪ_2()) {
    StringRef TemplateSuffix = Template.substr(Cursor);
    StringRef ਟੈਗ_Open, ਟੈਗ_Close;
    ਟੈਗ::ਕਿਸਮ_3 ਕਿਸਮ_3;

    // Determine which tag we've encountered.
    ਜੇ (TemplateSuffix.starts_with(TripleOpen)) {
      ਕਿਸਮ_3 = ਟੈਗ::ਕਿਸਮ_3::Triple;
      ਟੈਗ_Open = TripleOpen;
      ਟੈਗ_Close = TripleClose;
    } ਨਹੀਂਤਾਂ ਜੇ (TemplateSuffix.starts_with(Open)) {
      ਕਿਸਮ_3 = ਟੈਗ::ਕਿਸਮ_3::ਸਾਧਾਰਨ;
      ਟੈਗ_Open = Open;
      ਟੈਗ_Close = Close;
    } ਨਹੀਂਤਾਂ {
      // Not at a tag, continue scanning.
      ++Cursor;
      ਜਾਰੀ;
    }

    // Found a tag, first add the preceding text.
    ਜੇ (Cursor > ਪਾਠ_ਸ਼ੁਰੂ)
      ਟੋਕਨ.emplace_ਪਿੱਠ(Template.ਟੁਕੜਾ(ਪਾਠ_ਸ਼ੁਰੂ, Cursor));

    // Find the closing tag.
    size_t ਸਿਰਾ_Pos = Template.ਲੱਭੋ(ਟੈਗ_Close, Cursor + ਟੈਗ_Open.ਮਾਪ_2());
    ਜੇ (ਸਿਰਾ_Pos == StringRef::npos) {
      // No closing tag, the rest is text.
      ਟੋਕਨ.emplace_ਪਿੱਠ(Template.substr(Cursor));
      ਪਾਠ_ਸ਼ੁਰੂ = Cursor = Template.ਮਾਪ_2();
      ਰੋਕੋ;
    }

    // Extract tag content and full match.
    size_t Content_ਸ਼ੁਰੂ = Cursor + ਟੈਗ_Open.ਮਾਪ_2();
    StringRef Content = Template.substr(Content_ਸ਼ੁਰੂ, ਸਿਰਾ_Pos - Content_ਸ਼ੁਰੂ);
    StringRef ਭਰਿਆ_ਮੇਲ =
        Template.substr(Cursor, (ਸਿਰਾ_Pos + ਟੈਗ_Close.ਮਾਪ_2()) - Cursor);

    // Process the tag (inlined logic from processTag).
    LLVM_DEBUG(dbgs() << "[Tag] " << ਭਰਿਆ_ਮੇਲ << ", Content: " << Content
                      << ", Kind: " << ਟੈਗ_ਕਿਸਮ_ਮੰਜ਼ਿਲ_String(ਕਿਸਮ_3) << "\n");
    ਜੇ (ਕਿਸਮ_3 == ਟੈਗ::ਕਿਸਮ_3::Triple) {
      ਟੋਕਨ.emplace_ਪਿੱਠ(ਭਰਿਆ_ਮੇਲ, Ctx.Saver.save("&" + Content), '&', Ctx);
    } ਨਹੀਂਤਾਂ { // Normal Tag
      StringRef Interpolated = Content;
      ਜੇ (!Interpolated.trim().starts_with("=")) {
        ਅੱਖਰ Front = Interpolated.ਖਾਲੀ_ਥਾਵਾਂ() ? ' ' : Interpolated.trim().front();
        ਟੋਕਨ.emplace_ਪਿੱਠ(ਭਰਿਆ_ਮੇਲ, Interpolated, Front, Ctx);
      } ਨਹੀਂਤਾਂ { // Set Delimiter
        ਟੋਕਨ.emplace_ਪਿੱਠ(ਭਰਿਆ_ਮੇਲ, Interpolated, '=', Ctx);
        StringRef DelimSpec = Interpolated.trim();
        DelimSpec = DelimSpec.drop_front(1);
        DelimSpec = DelimSpec.take_until([](ਅੱਖਰ C) { ਵਾਪਸ C == '='; });
        DelimSpec = DelimSpec.trim();

        ਸਵੈਚਾਲਿਤ [NewOpen, NewClose] = DelimSpec.ਵੰਡੋ(' ');
        LLVM_DEBUG(dbgs() << "[Set Delimiter] NewOpen: " << NewOpen
                          << ", NewClose: " << NewClose << "\n");
        Open = NewOpen;
        Close = NewClose;
      }
    }

    // Move past the tag for the next iteration.
    Cursor += ਭਰਿਆ_ਮੇਲ.ਮਾਪ_2();
    ਪਾਠ_ਸ਼ੁਰੂ = Cursor;
  }

  // Add any remaining text after the last tag.
  ਜੇ (ਪਾਠ_ਸ਼ੁਰੂ < Template.ਮਾਪ_2())
    ਟੋਕਨ.emplace_ਪਿੱਠ(Template.substr(ਪਾਠ_ਸ਼ੁਰੂ));

  // Fix up white spaces for standalone tags.
  size_t ਆਖਰੀ_Idx = ਟੋਕਨ.ਮਾਪ_2() - 1;
  ਲਈ (size_t Idx = 0, ਸਿਰਾ_2 = ਟੋਕਨ.ਮਾਪ_2(); Idx < ਸਿਰਾ_2; ++Idx) {
    Token &CurrentToken = ਟੋਕਨ[Idx];
    Token::Type CurrentType = CurrentToken.getType();
    ਜੇ (!requires_Clean_ਉੱਪਰ(CurrentType))
      ਜਾਰੀ;

    ਤਾਰਕਿਕ Has_ਪਾਠ_Behind = has_ਪਾਠ_Behind(Idx, ਟੋਕਨ);
    ਤਾਰਕਿਕ Has_ਪਾਠ_Ahead = has_ਪਾਠ_Ahead(Idx, ਟੋਕਨ);

    ਜੇ ((!Has_ਪਾਠ_Ahead && !Has_ਪਾਠ_Behind) || (!Has_ਪਾਠ_Ahead && Idx == 0))
      stripTokenAhead(ਟੋਕਨ, Idx);

    ਜੇ ((!Has_ਪਾਠ_Behind && !Has_ਪਾਠ_Ahead) || (!Has_ਪਾਠ_Behind && Idx == ਆਖਰੀ_Idx))
      stripTokenBefore(ਟੋਕਨ, Idx, CurrentToken, CurrentType);
  }
  ਵਾਪਸ ਟੋਕਨ;
}

// Custom stream to escape strings.
ਵਰਗ Escape_String_ਧਾਰਾ : ਜਨਤਕ Mustache_Output_ਧਾਰਾ {
ਜਨਤਕ:
  ਸਪਸ਼ਟ Escape_String_ਧਾਰਾ(llvm::raw_ostream &Wrapped_ਧਾਰਾ,
                              EscapeMap &Escape)
      : Escape(Escape), EscapeChars(Escape.ਕੁੰਜੀਆਂ().begin(), Escape.ਕੁੰਜੀਆਂ().ਸਿਰਾ()),
        Wrapped_ਧਾਰਾ(Wrapped_ਧਾਰਾ) {
    ਸੈੱਟ_Unbuffered();
  }

ਸੁਰੱਖਿਅਤ:
  ਖਾਲੀ write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) override {
    StringRef ਅੰਕੜੇ_2(Ptr, ਮਾਪ);
    size_t ਸ਼ੁਰੂ = 0;
    ਜਦੋਂ (ਸ਼ੁਰੂ < ਮਾਪ) {
      // Find the next character that needs to be escaped.
      size_t ਅਗਲਾ_2 = ਅੰਕੜੇ_2.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(EscapeChars.str(), ਸ਼ੁਰੂ);

      // If no escapable characters are found, write the rest of the string.
      ਜੇ (ਅਗਲਾ_2 == StringRef::npos) {
        Wrapped_ਧਾਰਾ << ਅੰਕੜੇ_2.substr(ਸ਼ੁਰੂ);
        ਵਾਪਸ;
      }

      // Write the chunk of text before the escapable character.
      ਜੇ (ਅਗਲਾ_2 > ਸ਼ੁਰੂ)
        Wrapped_ਧਾਰਾ << ਅੰਕੜੇ_2.substr(ਸ਼ੁਰੂ, ਅਗਲਾ_2 - ਸ਼ੁਰੂ);

      // Look up and write the escaped version of the character.
      Wrapped_ਧਾਰਾ << Escape[ਅੰਕੜੇ_2[ਅਗਲਾ_2]];
      ਸ਼ੁਰੂ = ਅਗਲਾ_2 + 1;
    }
  }

  uint64_t current_pos() ਅਚਲ override { ਵਾਪਸ Wrapped_ਧਾਰਾ.tell(); }

ਨਿਜੀ:
  EscapeMap &Escape;
  SmallString<8> EscapeChars;
  llvm::raw_ostream &Wrapped_ਧਾਰਾ;
};

// Custom stream to add indentation used to for rendering partials.
ਵਰਗ ਜੋੜੋ_Indentation_String_ਧਾਰਾ : ਜਨਤਕ Mustache_Output_ਧਾਰਾ {
ਜਨਤਕ:
  ਸਪਸ਼ਟ ਜੋੜੋ_Indentation_String_ਧਾਰਾ(raw_ostream &Wrapped_ਧਾਰਾ,
                                      size_t Indentation)
      : Indentation(Indentation), Wrapped_ਧਾਰਾ(Wrapped_ਧਾਰਾ),
        NeedsIndent(ਸੱਚ), IsSuspended(ਝੂਠ) {
    ਸੈੱਟ_Unbuffered();
  }

  ਖਾਲੀ suspendIndentation() override { IsSuspended = ਸੱਚ; }
  ਖਾਲੀ resumeIndentation() override { IsSuspended = ਝੂਠ; }

ਸੁਰੱਖਿਅਤ:
  ਖਾਲੀ write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) override {
    llvm::StringRef ਅੰਕੜੇ_2(Ptr, ਮਾਪ);
    SmallString<0> Indent;
    Indent.resize(Indentation, ' ');

    ਲਈ (ਅੱਖਰ C : ਅੰਕੜੇ_2) {
      LLVM_DEBUG(dbgs() << "[Indentation Stream] NeedsIndent:" << NeedsIndent
                        << ", C:'" << C << "', Indentation:" << Indentation
                        << "\n");
      ਜੇ (NeedsIndent && C != '\n') {
        Wrapped_ਧਾਰਾ << Indent;
        NeedsIndent = ਝੂਠ;
      }
      Wrapped_ਧਾਰਾ << C;
      ਜੇ (C == '\n' && !IsSuspended)
        NeedsIndent = ਸੱਚ;
    }
  }

  uint64_t current_pos() ਅਚਲ override { ਵਾਪਸ Wrapped_ਧਾਰਾ.tell(); }

ਨਿਜੀ:
  size_t Indentation;
  raw_ostream &Wrapped_ਧਾਰਾ;
  ਤਾਰਕਿਕ NeedsIndent;
  ਤਾਰਕਿਕ IsSuspended;
};

ਵਰਗ Parser {
ਜਨਤਕ:
  Parser(StringRef TemplateStr, MustacheContext &Ctx)
      : Ctx(Ctx), TemplateStr(TemplateStr) {}

  AstPtr parse();

ਨਿਜੀ:
  ਖਾਲੀ parseMustache(ASTNode *ਮਾਪਾ_2);
  ਖਾਲੀ parseSection(ASTNode *ਮਾਪਾ_2, ASTNode::Type Ty, ਅਚਲ Accessor &A);

  MustacheContext &Ctx;
  Small_ਸਦਿਸ਼<Token> ਟੋਕਨ;
  size_t CurrentPtr;
  StringRef TemplateStr;
};

ਖਾਲੀ Parser::parseSection(ASTNode *ਮਾਪਾ_2, ASTNode::Type Ty,
                          ਅਚਲ Accessor &A) {
  AstPtr Current_ਗੰਢ = create_ਗੰਢ(Ctx, Ty, A, ਮਾਪਾ_2);
  size_t ਸ਼ੁਰੂ = CurrentPtr;
  parseMustache(Current_ਗੰਢ);
  ਅਚਲ size_t ਸਿਰਾ_2 = CurrentPtr - 1;

  size_t Raw_Body_ਮਾਪ = 0;
  ਲਈ (size_t I = ਸ਼ੁਰੂ; I < ਸਿਰਾ_2; ++I)
    Raw_Body_ਮਾਪ += ਟੋਕਨ[I].RawBody.ਮਾਪ_2();

  SmallString<128> RawBody;
  RawBody.reserve(Raw_Body_ਮਾਪ);
  ਲਈ (std::size_t I = ਸ਼ੁਰੂ; I < ਸਿਰਾ_2; ++I)
    RawBody += ਟੋਕਨ[I].RawBody;

  Current_ਗੰਢ->ਸੈੱਟ_Raw_Body(Ctx.Saver.save(StringRef(RawBody)));
  ਮਾਪਾ_2->ਜੋੜੋ_ਬੱਚਾ(Current_ਗੰਢ);
}

AstPtr Parser::parse() {
  ਟੋਕਨ = tokenize(TemplateStr, Ctx);
  CurrentPtr = 0;
  AstPtr ਜੜ੍ਹ_ਗੰਢ = create_ਜੜ੍ਹ_ਗੰਢ(Ctx);
  parseMustache(ਜੜ੍ਹ_ਗੰਢ);
  ਵਾਪਸ ਜੜ੍ਹ_ਗੰਢ;
}

ਖਾਲੀ Parser::parseMustache(ASTNode *ਮਾਪਾ_2) {

  ਜਦੋਂ (CurrentPtr < ਟੋਕਨ.ਮਾਪ_2()) {
    Token CurrentToken = ਟੋਕਨ[CurrentPtr];
    CurrentPtr++;
    ArrayRef<StringRef> A = CurrentToken.getAccessor();
    AstPtr Current_ਗੰਢ;

    ਚੋਣ (CurrentToken.getType()) {
    ਹਾਲਤ Token::Type::ਪਾਠ_2: {
      Current_ਗੰਢ = create_ਪਾਠ_ਗੰਢ(Ctx, CurrentToken.TokenBody, ਮਾਪਾ_2);
      ਮਾਪਾ_2->ਜੋੜੋ_ਬੱਚਾ(Current_ਗੰਢ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::Variable: {
      Current_ਗੰਢ = create_ਗੰਢ(Ctx, ASTNode::Variable, A, ਮਾਪਾ_2);
      ਮਾਪਾ_2->ਜੋੜੋ_ਬੱਚਾ(Current_ਗੰਢ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::UnescapeVariable: {
      Current_ਗੰਢ = create_ਗੰਢ(Ctx, ASTNode::UnescapeVariable, A, ਮਾਪਾ_2);
      ਮਾਪਾ_2->ਜੋੜੋ_ਬੱਚਾ(Current_ਗੰਢ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::Partial: {
      Current_ਗੰਢ = create_ਗੰਢ(Ctx, ASTNode::Partial, A, ਮਾਪਾ_2);
      Current_ਗੰਢ->ਸੈੱਟ_Indentation(CurrentToken.getIndentation());
      ਮਾਪਾ_2->ਜੋੜੋ_ਬੱਚਾ(Current_ਗੰਢ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::SectionOpen: {
      parseSection(ਮਾਪਾ_2, ASTNode::Section, A);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::InvertSectionOpen: {
      parseSection(ਮਾਪਾ_2, ASTNode::InvertSection, A);
      ਰੋਕੋ;
    }
    ਹਾਲਤ Token::Type::Comment:
    ਹਾਲਤ Token::Type::ਸੈੱਟ_Delimiter:
      ਰੋਕੋ;
    ਹਾਲਤ Token::Type::SectionClose:
      ਵਾਪਸ;
    }
  }
}
ਸਥਿਰ ਖਾਲੀ ਮੰਜ਼ਿਲ_Mustache_String(ਅਚਲ json::ਮੁੱਲ &ਅੰਕੜੇ_2, raw_ostream &OS) {
  LLVM_DEBUG(dbgs() << "[To Mustache String] Kind: "
                    << json_ਕਿਸਮ_ਮੰਜ਼ਿਲ_String(ਅੰਕੜੇ_2.ਕਿਸਮ_2()) << ", Data: " << ਅੰਕੜੇ_2
                    << "\n");
  ਚੋਣ (ਅੰਕੜੇ_2.ਕਿਸਮ_2()) {
  ਹਾਲਤ json::ਮੁੱਲ::Null:
    ਵਾਪਸ;
  ਹਾਲਤ json::ਮੁੱਲ::ਸੰਖਿਆ: {
    ਸਵੈਚਾਲਿਤ ਅੰਕ = *ਅੰਕੜੇ_2.get_As_ਸੰਖਿਆ();
    std::ostringstream SS;
    SS << ਅੰਕ;
    OS << SS.str();
    ਵਾਪਸ;
  }
  ਹਾਲਤ json::ਮੁੱਲ::String: {
    OS << *ਅੰਕੜੇ_2.getAsString();
    ਵਾਪਸ;
  }

  ਹਾਲਤ json::ਮੁੱਲ::Array: {
    ਸਵੈਚਾਲਿਤ Arr = *ਅੰਕੜੇ_2.getAsArray();
    ਜੇ (Arr.ਖਾਲੀ_ਥਾਵਾਂ())
      ਵਾਪਸ;
    [[fallthrough]];
  }
  ਹਾਲਤ json::ਮੁੱਲ::Object:
  ਹਾਲਤ json::ਮੁੱਲ::Boolean: {
    llvm::json::OStream JOS(OS, 2);
    JOS.ਮੁੱਲ_2(ਅੰਕੜੇ_2);
    ਰੋਕੋ;
  }
  }
}

ਖਾਲੀ ASTNode::render_ਜੜ੍ਹ(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                         Mustache_Output_ਧਾਰਾ &OS) {
  render_ਬੱਚਾ(CurrentCtx, OS);
}

ਖਾਲੀ ASTNode::render_ਪਾਠ(Mustache_Output_ਧਾਰਾ &OS) { OS << Body; }

ਖਾਲੀ ASTNode::renderPartial(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                            Mustache_Output_ਧਾਰਾ &OS) {
  LLVM_DEBUG(dbgs() << "[Render Partial] Accessor:" << Accessor_ਮੁੱਲ[0]
                    << ", Indentation:" << Indentation << "\n");
  ਸਵੈਚਾਲਿਤ Partial = Ctx.Partials.ਲੱਭੋ(Accessor_ਮੁੱਲ[0]);
  ਜੇ (Partial != Ctx.Partials.ਸਿਰਾ())
    renderPartial(CurrentCtx, OS, Partial->get_ਮੁੱਲ());
}

ਖਾਲੀ ASTNode::renderVariable(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                             Mustache_Output_ਧਾਰਾ &OS) {
  ਸਵੈਚਾਲਿਤ Lambda = Ctx.Lambdas.ਲੱਭੋ(Accessor_ਮੁੱਲ[0]);
  ਜੇ (Lambda != Ctx.Lambdas.ਸਿਰਾ()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ਮੁੱਲ());
  } ਨਹੀਂਤਾਂ ਜੇ (ਅਚਲ json::ਮੁੱਲ *ContextPtr = ਲੱਭੋ_Context()) {
    Escape_String_ਧਾਰਾ ES(OS, Ctx.Escapes);
    ਮੰਜ਼ਿਲ_Mustache_String(*ContextPtr, ES);
  }
}

ਖਾਲੀ ASTNode::renderUnescapeVariable(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                                     Mustache_Output_ਧਾਰਾ &OS) {
  LLVM_DEBUG(dbgs() << "[Render UnescapeVariable] Accessor:" << Accessor_ਮੁੱਲ[0]
                    << "\n");
  ਸਵੈਚਾਲਿਤ Lambda = Ctx.Lambdas.ਲੱਭੋ(Accessor_ਮੁੱਲ[0]);
  ਜੇ (Lambda != Ctx.Lambdas.ਸਿਰਾ()) {
    renderLambdas(CurrentCtx, OS, Lambda->get_ਮੁੱਲ());
  } ਨਹੀਂਤਾਂ ਜੇ (ਅਚਲ json::ਮੁੱਲ *ContextPtr = ਲੱਭੋ_Context()) {
    OS.suspendIndentation();
    ਮੰਜ਼ਿਲ_Mustache_String(*ContextPtr, OS);
    OS.resumeIndentation();
  }
}

ਖਾਲੀ ASTNode::renderSection(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                            Mustache_Output_ਧਾਰਾ &OS) {
  ਸਵੈਚਾਲਿਤ SectionLambda = Ctx.SectionLambdas.ਲੱਭੋ(Accessor_ਮੁੱਲ[0]);
  ਜੇ (SectionLambda != Ctx.SectionLambdas.ਸਿਰਾ()) {
    renderSectionLambdas(CurrentCtx, OS, SectionLambda->get_ਮੁੱਲ());
    ਵਾਪਸ;
  }

  ਅਚਲ json::ਮੁੱਲ *ContextPtr = ਲੱਭੋ_Context();
  ਜੇ (isContextFalsey(ContextPtr))
    ਵਾਪਸ;

  ਜੇ (ਅਚਲ json::Array *Arr = ContextPtr->getAsArray()) {
    ਲਈ (ਅਚਲ json::ਮੁੱਲ &V : *Arr)
      render_ਬੱਚਾ(V, OS);
    ਵਾਪਸ;
  }
  render_ਬੱਚਾ(*ContextPtr, OS);
}

ਖਾਲੀ ASTNode::renderInvertSection(ਅਚਲ json::ਮੁੱਲ &CurrentCtx,
                                  Mustache_Output_ਧਾਰਾ &OS) {
  ਤਾਰਕਿਕ IsLambda = Ctx.SectionLambdas.ਸ਼ਾਮਲ_ਹੈ(Accessor_ਮੁੱਲ[0]);
  ਅਚਲ json::ਮੁੱਲ *ContextPtr = ਲੱਭੋ_Context();
  ਜੇ (isContextFalsey(ContextPtr) && !IsLambda) {
    render_ਬੱਚਾ(CurrentCtx, OS);
  }
}

ਖਾਲੀ ASTNode::render(ਅਚਲ llvm::json::ਮੁੱਲ &ਅੰਕੜੇ_2, Mustache_Output_ਧਾਰਾ &OS) {
  ਜੇ (Ty != ਜੜ੍ਹ_2 && Ty != ਪਾਠ_2 && Accessor_ਮੁੱਲ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;
  // Set the parent context to the incoming context so that we
  // can walk up the context tree correctly in findContext().
  ਮਾਪਾ_Context = &ਅੰਕੜੇ_2;

  ਚੋਣ (Ty) {
  ਹਾਲਤ ਜੜ੍ਹ_2:
    render_ਜੜ੍ਹ(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  ਹਾਲਤ ਪਾਠ_2:
    render_ਪਾਠ(OS);
    ਵਾਪਸ;
  ਹਾਲਤ Partial:
    renderPartial(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  ਹਾਲਤ Variable:
    renderVariable(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  ਹਾਲਤ UnescapeVariable:
    renderUnescapeVariable(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  ਹਾਲਤ Section:
    renderSection(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  ਹਾਲਤ InvertSection:
    renderInvertSection(ਅੰਕੜੇ_2, OS);
    ਵਾਪਸ;
  }
  llvm_unreachable("Invalid ASTNode type");
}

ਅਚਲ json::ਮੁੱਲ *ASTNode::ਲੱਭੋ_Context() {
  // The mustache spec allows for dot notation to access nested values
  // a single dot refers to the current context.
  // We attempt to find the JSON context in the current node, if it is not
  // found, then we traverse the parent nodes to find the context until we
  // reach the root node or the context is found.
  ਜੇ (Accessor_ਮੁੱਲ.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  ਜੇ (Accessor_ਮੁੱਲ[0] == ".")
    ਵਾਪਸ ਮਾਪਾ_Context;

  ਅਚਲ json::Object *CurrentContext = ਮਾਪਾ_Context->getAsObject();
  StringRef CurrentAccessor = Accessor_ਮੁੱਲ[0];
  ASTNode *Current_ਮਾਪਾ = ਮਾਪਾ_2;

  ਜਦੋਂ (!CurrentContext || !CurrentContext->get(CurrentAccessor)) {
    ਜੇ (Current_ਮਾਪਾ->Ty != ਜੜ੍ਹ_2) {
      CurrentContext = Current_ਮਾਪਾ->ਮਾਪਾ_Context->getAsObject();
      Current_ਮਾਪਾ = Current_ਮਾਪਾ->ਮਾਪਾ_2;
      ਜਾਰੀ;
    }
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }
  ਅਚਲ json::ਮੁੱਲ *Context = ਖਾਲੀ_ਸੂਚਕ;
  ਲਈ (ਸਵੈਚਾਲਿਤ [Idx, Acc] : enumerate(Accessor_ਮੁੱਲ)) {
    ਅਚਲ json::ਮੁੱਲ *Current_ਮੁੱਲ = CurrentContext->get(Acc);
    ਜੇ (!Current_ਮੁੱਲ)
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    ਜੇ (Idx < Accessor_ਮੁੱਲ.ਮਾਪ_2() - 1) {
      CurrentContext = Current_ਮੁੱਲ->getAsObject();
      ਜੇ (!CurrentContext)
        ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    } ਨਹੀਂਤਾਂ {
      Context = Current_ਮੁੱਲ;
    }
  }
  ਵਾਪਸ Context;
}

ਖਾਲੀ ASTNode::render_ਬੱਚਾ(ਅਚਲ json::ਮੁੱਲ &Contexts,
                          Mustache_Output_ਧਾਰਾ &OS) {
  ਲਈ (ASTNode &ਬੱਚਾ : Children)
    ਬੱਚਾ.render(Contexts, OS);
}

ਖਾਲੀ ASTNode::renderPartial(ਅਚਲ json::ਮੁੱਲ &Contexts,
                            Mustache_Output_ਧਾਰਾ &OS, ASTNode *Partial) {
  LLVM_DEBUG(dbgs() << "[Render Partial Indentation] Indentation: " << Indentation << "\n");
  ਜੋੜੋ_Indentation_String_ਧਾਰਾ IS(OS, Indentation);
  Partial->render(Contexts, IS);
}

ਖਾਲੀ ASTNode::renderLambdas(ਅਚਲ llvm::json::ਮੁੱਲ &Contexts,
                            Mustache_Output_ਧਾਰਾ &OS, Lambda &L) {
  json::ਮੁੱਲ Lambda_ਨਤੀਜਾ = L();
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  ਮੰਜ਼ਿਲ_Mustache_String(Lambda_ਨਤੀਜਾ, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ਗੰਢ = P.parse();

  Escape_String_ਧਾਰਾ ES(OS, Ctx.Escapes);
  ਜੇ (Ty == Variable) {
    Lambda_ਗੰਢ->render(Contexts, ES);
    ਵਾਪਸ;
  }
  Lambda_ਗੰਢ->render(Contexts, OS);
}

ਖਾਲੀ ASTNode::renderSectionLambdas(ਅਚਲ llvm::json::ਮੁੱਲ &Contexts,
                                   Mustache_Output_ਧਾਰਾ &OS, SectionLambda &L) {
  json::ਮੁੱਲ Return = L(RawBody.str());
  ਜੇ (isFalsey(Return))
    ਵਾਪਸ;
  std::string LambdaStr;
  raw_string_ostream Output(LambdaStr);
  ਮੰਜ਼ਿਲ_Mustache_String(Return, Output);
  Parser P(LambdaStr, Ctx);
  AstPtr Lambda_ਗੰਢ = P.parse();
  Lambda_ਗੰਢ->render(Contexts, OS);
}

ਖਾਲੀ Template::render(ਅਚਲ llvm::json::ਮੁੱਲ &ਅੰਕੜੇ_2, llvm::raw_ostream &OS) {
  Raw_Mustache_Output_ਧਾਰਾ MOS(OS);
  ਰੁੱਖ->render(ਅੰਕੜੇ_2, MOS);
}

ਖਾਲੀ Template::ਰਜਿਸਟਰ_Partial(std::string Name, std::string Partial) {
  StringRef SavedPartial = Ctx.Saver.save(Partial);
  Parser P(SavedPartial, Ctx);
  AstPtr Partial_ਰੁੱਖ = P.parse();
  Ctx.Partials.ਪਾਓ(std::make_pair(Name, Partial_ਰੁੱਖ));
}

ਖਾਲੀ Template::ਰਜਿਸਟਰ_Lambda(std::string Name, Lambda L) {
  Ctx.Lambdas[Name] = std::move(L);
}

ਖਾਲੀ Template::ਰਜਿਸਟਰ_Lambda(std::string Name, SectionLambda L) {
  Ctx.SectionLambdas[Name] = std::move(L);
}

ਖਾਲੀ Template::overrideEscapeCharacters(EscapeMap E) {
  Ctx.Escapes = std::move(E);
}

Template::Template(StringRef TemplateStr, MustacheContext &Ctx) : Ctx(Ctx) {
  Parser P(TemplateStr, Ctx);
  ਰੁੱਖ = P.parse();
  // The default behavior is to escape html entities.
  ਅਚਲ EscapeMap HtmlEntities = {{'&', "&amp;"},
                                  {'<', "&lt;"},
                                  {'>', "&gt;"},
                                  {'"', "&quot;"},
                                  {'\'', "&#39;"}};
  overrideEscapeCharacters(HtmlEntities);
}

Template::Template(Template &&Other) noexcept
    : Ctx(Other.Ctx), ਰੁੱਖ(Other.ਰੁੱਖ) {
  Other.ਰੁੱਖ = ਖਾਲੀ_ਸੂਚਕ;
}

Template::~Template() = ਮੂਲ;

} // namespace llvm::mustache

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ DEBUG_TYPE
