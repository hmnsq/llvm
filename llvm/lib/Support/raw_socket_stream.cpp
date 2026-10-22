//===-- llvm/Support/raw_socket_stream.cpp - Socket streams --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains raw_ostream implementations for streams to communicate
// via UNIX sockets
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/raw_socket_stream.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"

#include <atomic>
#include <fcntl.h>
#include <functional>

#ifndef _WIN32
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#không_thì
#include "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#include <winsock2.h>
#include <afunix.h>
// clang-format on
#include <io.h>
#endif // _WIN32

#nếu defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

using namespace llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, sizeof(WsaData));
  nếu (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

static std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  trả_về std::error_code(::WSAGetLastError(), std::system_category());
#không_thì
  trả_về errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#không_thì
using NativeSocket = số_nguyên;
#define INVALID_SOCKET -1
#endif

static số_nguyên closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  trả_về ::closesocket(Socket);
#không_thì
  trả_về ::close(Socket);
#endif
}

static Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  struct sockaddr_un Addr;
  memset(&Addr, 0, sizeof(Addr));
  Addr.sun_family = AF_UNIX;

  nếu (sizeof(sockaddr_un::sun_path) <= SocketPath.size())
    trả_về make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), sizeof(Addr.sun_path) - 1);
  trả_về Addr;
}

static Expected<số_nguyên> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  nếu (Socket == INVALID_SOCKET) {
    trả_về llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, NULL, 0);
#endif
  Expected<struct sockaddr_un> Addr = setSocketAddr(SocketPath);
  nếu (!Addr) {
    closeSocket(Socket);
    trả_về Addr.takeError();
  }

  nếu (::connect(Socket, (struct sockaddr *)&*Addr, sizeof(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    trả_về llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  trả_về _open_osfhandle(Socket, 0);
#không_thì
  trả_về Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(số_nguyên SocketFD, StringRef SocketPath,
                                 số_nguyên PipeFD[2])
    : FD(SocketFD), SocketPath(SocketPath), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.load()), SocketPath(LS.SocketPath),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.SocketPath.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef SocketPath,
                                                      số_nguyên MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  nếu (llvm::sys::fs::exists(SocketPath)) {
    Expected<số_nguyên> MaybeFD = getSocketFD(SocketPath);
    nếu (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      trả_về llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    trả_về llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  nếu (Socket == INVALID_SOCKET)
    trả_về llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, NULL, 0);
#endif
  Expected<struct sockaddr_un> Addr = setSocketAddr(SocketPath);
  nếu (!Addr) {
    closeSocket(Socket);
    trả_về Addr.takeError();
  }

  nếu (::bind(Socket, (struct sockaddr *)&*Addr, sizeof(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    trả_về llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  nếu (::listen(Socket, MaxBacklog) == -1)
    trả_về llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  số_nguyên PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  nếu (::_pipe(PipeFD, 1, 0) == -1)
#không_thì
  nếu (::pipe(PipeFD) == -1)
#endif // _WIN32
    trả_về llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  trả_về ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#không_thì
  trả_về ListeningSocket{Socket, SocketPath, PipeFD};
#endif // _WIN32
}

// If a file descriptor being monitored by ::poll is closed by another thread,
// the result is unspecified. In the case ::poll does not unblock and return,
// when ActiveFD is closed, you can provide another file descriptor via CancelFD
// that when written to will cause poll to return. Typically CancelFD is the
// read end of a unidirectional pipe.
//
// Timeout should be -1 to block indefinitly
//
// getActiveFD is a callback to handle ActiveFD's of std::atomic<int> and int
static std::error_code
manageTimeout(const std::chrono::milliseconds &Timeout,
              const std::function<số_nguyên()> &getActiveFD,
              const std::optional<số_nguyên> &CancelFD = std::nullopt) {
  struct pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#không_thì
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  nếu (CancelFD.has_value()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.value();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  số_nguyên PollStatus = 0;
  do {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    nếu (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      nếu (TotalElapsedTime >= Timeout)
        trả_về std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.count());
  } trong_khi (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#không_thì
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.count());
  } trong_khi (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  nếu (getActiveFD() == -1 || (CancelFD.has_value() && FD[1].revents & POLLIN))
    trả_về std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  nếu (PollStatus == SOCKET_ERROR)
#không_thì
  nếu (PollStatus == -1)
#endif
    trả_về getLastSocketErrorCode();
  nếu (PollStatus == 0)
    trả_về std::make_error_code(std::errc::timed_out);
  nếu (FD[0].revents & POLLNVAL)
    trả_về std::make_error_code(std::errc::bad_file_descriptor);
  trả_về std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(const std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [this]() -> số_nguyên { trả_về FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  nếu (TimeoutErr)
    trả_về llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  số_nguyên AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), NULL, NULL);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#không_thì
  AcceptFD = ::accept(FD, NULL, NULL);
#endif

  nếu (AcceptFD == -1)
    trả_về llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  trả_về std::make_unique<raw_socket_stream>(AcceptFD);
}

void ListeningSocket::shutdown() {
  số_nguyên ObservedFD = FD.load();

  nếu (ObservedFD == -1)
    trả_về;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  nếu (!FD.compare_exchange_strong(ObservedFD, -1))
    trả_về;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  char Byte = 'A';
  ssize_t written = ::write(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (void)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  nếu (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  nếu (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(số_nguyên SocketFD)
    : raw_fd_stream(SocketFD, true) {}

raw_socket_stream::~raw_socket_stream() = default;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<số_nguyên> FD = getSocketFD(SocketPath);
  nếu (!FD)
    trả_về FD.takeError();
  trả_về std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::read(char *Ptr, size_t Size,
                                const std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [this]() -> số_nguyên { trả_về this->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  nếu (Err) {
    raw_fd_stream::error_detected(Err);
    trả_về -1;
  }
  trả_về raw_fd_stream::read(Ptr, Size);
}
