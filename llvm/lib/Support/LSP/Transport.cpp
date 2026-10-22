//===--- JSONTransport.cpp - sending and receiving LSP messages over JSON -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/LSP/Transport.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/LSP/Logging.h"
#include "llvm/Support/LSP/Protocol.h"
#include <atomic>
#include <optional>
#include <system_error>
#include <utility>

using namespace llvm;
using namespace llvm::lsp;

//===----------------------------------------------------------------------===//
// Reply
//===----------------------------------------------------------------------===//

namespace {
/// Function object to reply to an LSP call.
/// Each instance must be called exactly once, otherwise:
///  - if there was no reply, an error reply is sent
///  - if there were multiple replies, only the first is sent
class Reply {
public:
  Reply(const llvm::json::Value &Id, StringRef Method, JSONTransport &Transport,
        std::mutex &TransportOutputMutex);
  Reply(Reply &&Other);
  Reply &operator=(Reply &&) = delete;
  Reply(const Reply &) = delete;
  Reply &operator=(const Reply &) = delete;

  void operator()(llvm::Expected<llvm::json::Value> Reply);

private:
  std::string Method;
  std::atomic<bool> Replied = {false};
  llvm::json::Value Id;
  JSONTransport *Transport;
  std::mutex &TransportOutputMutex;
};
} // namespace

Reply::Reply(const llvm::json::Value &Id, llvm::StringRef Method,
             JSONTransport &Transport, std::mutex &TransportOutputMutex)
    : Method(Method), Id(Id), Transport(&Transport),
      TransportOutputMutex(TransportOutputMutex) {}

Reply::Reply(Reply &&Other)
    : Method(Other.Method), Replied(Other.Replied.load()),
      Id(std::move(Other.Id)), Transport(Other.Transport),
      TransportOutputMutex(Other.TransportOutputMutex) {
  Other.Transport = nullptr;
}

void Reply::operator()(llvm::Expected<llvm::json::Value> Reply) {
  如果 (Replied.exchange(true)) {
    Logger::error("Replied twice to message {0}({1})", Method, Id);
    assert(false && "must reply to each call only once!");
    返回;
  }
  assert(Transport && "expected valid transport to reply to");

  std::lock_guard<std::mutex> TransportLock(TransportOutputMutex);
  如果 (Reply) {
    Logger::info("--> reply:{0}({1})", Method, Id);
    Transport->reply(std::move(Id), std::move(Reply));
  } 否则 {
    llvm::Error Error = Reply.takeError();
    Logger::info("--> reply:{0}({1}): {2}", Method, Id, Error);
    Transport->reply(std::move(Id), std::move(Error));
  }
}

//===----------------------------------------------------------------------===//
// MessageHandler
//===----------------------------------------------------------------------===//

// Keep handleParseError out of every parse<T> instantiation.
LLVM_ATTRIBUTE_NOINLINE llvm::Error
MessageHandler::handleParseError(const llvm::json::Value &Raw,
                                 StringRef PayloadName, StringRef PayloadKind,
                                 const llvm::json::Path::Root &Root) {
  // Dump the relevant parts of the broken message.
  std::string Context;
  llvm::raw_string_ostream Os(Context);
  Root.printErrorContext(Raw, Os);

  // Report the error (e.g. to the client).
  返回 llvm::make_error<LSPError>(
      llvm::formatv("failed to decode {0} {1}: {2}", PayloadName, PayloadKind,
                    fmt_consume(Root.getError())),
      ErrorCode::InvalidParams);
}

bool MessageHandler::onNotify(llvm::StringRef Method, llvm::json::Value Value) {
  Logger::info("--> {0}", Method);

  如果 (Method == "exit")
    返回 false;
  如果 (Method == "$cancel") {
    // TODO: Add support for cancelling requests.
  } 否则 {
    auto It = NotificationHandlers.find(Method);
    如果 (It != NotificationHandlers.end())
      It->second(std::move(Value));
  }
  返回 true;
}

bool MessageHandler::onCall(llvm::StringRef Method, llvm::json::Value Params,
                            llvm::json::Value Id) {
  Logger::info("--> {0}({1})", Method, Id);

  Reply Reply(Id, Method, Transport, TransportOutputMutex);

  auto It = MethodHandlers.find(Method);
  如果 (It != MethodHandlers.end()) {
    It->second(std::move(Params), std::move(Reply));
  } 否则 {
    Reply(llvm::make_error<LSPError>("method not found: " + Method.str(),
                                     ErrorCode::MethodNotFound));
  }
  返回 true;
}

bool MessageHandler::onReply(llvm::json::Value Id,
                             llvm::Expected<llvm::json::Value> Result) {
  // Find the response handler in the mapping. If it exists, move it out of the
  // mapping and erase it.
  ResponseHandlerTy ResponseHandler;
  {
    std::lock_guard<std::mutex> responseHandlersLock(ResponseHandlersMutex);
    auto It = ResponseHandlers.find(debugString(Id));
    如果 (It != ResponseHandlers.end()) {
      ResponseHandler = std::move(It->second);
      ResponseHandlers.erase(It);
    }
  }

  // If we found a response handler, invoke it. Otherwise, log an error.
  如果 (ResponseHandler.second) {
    Logger::info("--> reply:{0}({1})", ResponseHandler.first, Id);
    ResponseHandler.second(std::move(Id), std::move(Result));
  } 否则 {
    Logger::error(
        "received a reply with ID {0}, but there was no such outgoing request",
        Id);
    如果 (!Result)
      llvm::consumeError(Result.takeError());
  }
  返回 true;
}

//===----------------------------------------------------------------------===//
// JSONTransport
//===----------------------------------------------------------------------===//

/// Encode the given error as a JSON object.
static llvm::json::Object encodeError(llvm::Error Error) {
  std::string Message;
  ErrorCode Code = ErrorCode::UnknownErrorCode;
  auto HandlerFn = [&](const LSPError &LspError) -> llvm::Error {
    Message = LspError.message;
    Code = LspError.code;
    返回 llvm::Error::success();
  };
  如果 (llvm::Error Unhandled = llvm::handleErrors(std::move(Error), HandlerFn))
    Message = llvm::toString(std::move(Unhandled));

  返回 llvm::json::Object{
      {"message", std::move(Message)},
      {"code", int64_t(Code)},
  };
}

/// Decode the given JSON object into an error.
llvm::Error decodeError(const llvm::json::Object &O) {
  StringRef Msg = O.getString("message").value_or("Unspecified error");
  如果 (std::optional<int64_t> Code = O.getInteger("code"))
    返回 llvm::make_error<LSPError>(Msg.str(), ErrorCode(*Code));
  返回 llvm::make_error<llvm::StringError>(llvm::inconvertibleErrorCode(),
                                             Msg.str());
}

void JSONTransport::notify(StringRef Method, llvm::json::Value Params) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
void JSONTransport::call(StringRef Method, llvm::json::Value Params,
                         llvm::json::Value Id) {
  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"method", Method},
      {"params", std::move(Params)},
  });
}
void JSONTransport::reply(llvm::json::Value Id,
                          llvm::Expected<llvm::json::Value> Result) {
  如果 (Result) {
    返回 sendMessage(llvm::json::Object{
        {"jsonrpc", "2.0"},
        {"id", std::move(Id)},
        {"result", std::move(*Result)},
    });
  }

  sendMessage(llvm::json::Object{
      {"jsonrpc", "2.0"},
      {"id", std::move(Id)},
      {"error", encodeError(Result.takeError())},
  });
}

llvm::Error JSONTransport::run(MessageHandler &Handler) {
  std::string Json;
  当 (!In->isEndOfInput()) {
    如果 (In->hasError()) {
      返回 llvm::errorCodeToError(
          std::error_code(errno, std::system_category()));
    }

    如果 (succeeded(In->readMessage(Json))) {
      如果 (llvm::Expected<llvm::json::Value> Doc = llvm::json::parse(Json)) {
        如果 (!handleMessage(std::move(*Doc), Handler))
          返回 llvm::Error::success();
      } 否则 {
        Logger::error("JSON parse error: {0}", llvm::toString(Doc.takeError()));
      }
    }
  }
  返回 llvm::errorCodeToError(std::make_error_code(std::errc::io_error));
}

void JSONTransport::sendMessage(llvm::json::Value Msg) {
  OutputBuffer.clear();
  llvm::raw_svector_ostream os(OutputBuffer);
  os << llvm::formatv(PrettyOutput ? "{0:2}\n" : "{0}", Msg);
  Out << "Content-Length: " << OutputBuffer.size() << "\r\n\r\n"
      << OutputBuffer;
  Out.flush();
  Logger::debug(">>> {0}\n", OutputBuffer);
}

bool JSONTransport::handleMessage(llvm::json::Value Msg,
                                  MessageHandler &Handler) {
  // Message must be an object with "jsonrpc":"2.0".
  llvm::json::Object *Object = Msg.getAsObject();
  如果 (!Object ||
      Object->getString("jsonrpc") != std::optional<StringRef>("2.0"))
    返回 false;

  // `id` may be any JSON value. If absent, this is a notification.
  std::optional<llvm::json::Value> Id;
  如果 (llvm::json::Value *I = Object->get("id"))
    Id = std::move(*I);
  std::optional<StringRef> Method = Object->getString("method");

  // This is a response.
  如果 (!Method) {
    如果 (!Id)
      返回 false;
    如果 (auto *Err = Object->getObject("error"))
      返回 Handler.onReply(std::move(*Id), decodeError(*Err));
    // result should be given, use null if not.
    llvm::json::Value Result = nullptr;
    如果 (llvm::json::Value *R = Object->get("result"))
      Result = std::move(*R);
    返回 Handler.onReply(std::move(*Id), std::move(Result));
  }

  // Params should be given, use null if not.
  llvm::json::Value Params = nullptr;
  如果 (llvm::json::Value *P = Object->get("params"))
    Params = std::move(*P);

  如果 (Id)
    返回 Handler.onCall(*Method, std::move(Params), std::move(*Id));
  返回 Handler.onNotify(*Method, std::move(Params));
}

/// Tries to read a line up to and including \n.
/// If failing, feof(), ferror(), or shutdownRequested() will be set.
LogicalResult readLine(std::FILE *In, SmallVectorImpl<char> &Out) {
  // Big enough to hold any reasonable header line. May not fit content lines
  // in delimited mode, but performance doesn't matter for that mode.
  static constexpr 整数 BufSize = 128;
  size_t Size = 0;
  Out.clear();
  为 (;;) {
    Out.resize_for_overwrite(Size + BufSize);
    如果 (!std::fgets(&Out[Size], BufSize, In))
      返回 failure();

    clearerr(In);

    // If the line contained null bytes, anything after it (including \n) will
    // be ignored. Fortunately this is not a legal header or JSON.
    size_t Read = std::strlen(&Out[Size]);
    如果 (Read > 0 && Out[Size + Read - 1] == '\n') {
      Out.resize(Size + Read);
      返回 success();
    }
    Size += Read;
  }
}

// Returns std::nullopt when:
//  - ferror(), feof(), or shutdownRequested() are set.
//  - Content-Length is missing or empty (protocol error)
LogicalResult
JSONTransportInputOverFile::readStandardMessage(std::string &Json) {
  // A Language Server Protocol message starts with a set of HTTP headers,
  // delimited  by \r\n, and terminated by an empty line (\r\n).
  unsigned long long ContentLength = 0;
  llvm::SmallString<128> Line;
  当 (true) {
    如果 (feof(In) || hasError() || failed(readLine(In, Line)))
      返回 failure();

    // Content-Length is a mandatory header, and the only one we handle.
    StringRef LineRef = Line;
    如果 (LineRef.consume_front("Content-Length: ")) {
      llvm::getAsUnsignedInteger(LineRef.trim(), 0, ContentLength);
    } 否则 如果 (!LineRef.trim().empty()) {
      // It's another header, ignore it.
      continue;
    } 否则 {
      // An empty line indicates the end of headers. Go ahead and read the JSON.
      break;
    }
  }

  // The fuzzer likes crashing us by sending "Content-Length: 9999999999999999"
  如果 (ContentLength == 0 || ContentLength > 1 << 30)
    返回 failure();

  Json.resize(ContentLength);
  为 (size_t Pos = 0, Read; Pos < ContentLength; Pos += Read) {
    Read = std::fread(&Json[Pos], 1, ContentLength - Pos, In);
    如果 (Read == 0)
      返回 failure();

    // If we're done, the error was transient. If we're not done, either it was
    // transient or we'll see it again on retry.
    clearerr(In);
    Pos += Read;
  }
  返回 success();
}

/// For lit tests we support a simplified syntax:
/// - messages are delimited by '// -----' on a line by itself
/// - lines starting with // are ignored.
/// This is a testing path, so favor simplicity over performance here.
/// When returning failure: feof(), ferror(), or shutdownRequested() will be
/// set.
LogicalResult
JSONTransportInputOverFile::readDelimitedMessage(std::string &Json) {
  Json.clear();
  llvm::SmallString<128> Line;
  当 (succeeded(readLine(In, Line))) {
    StringRef LineRef = Line.str().trim();
    如果 (LineRef.starts_with("//")) {
      // Found a delimiter for the message.
      如果 (LineRef == "// -----")
        break;
      continue;
    }

    Json += Line;
  }

  返回 failure(ferror(In));
}
