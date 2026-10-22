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

#അന്തര്ഭാവ "llvm/Support/raw_socket_stream.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"

#അന്തര്ഭാവ <atomic>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <functional>

#ifndef _WIN32
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <sys/socket.h>
#അന്തര്ഭാവ <sys/un.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#അന്തര്ഭാവ <winsock2.h>
#അന്തര്ഭാവ <afunix.h>
// clang-format on
#അന്തര്ഭാവ <io.h>
#endif // _WIN32

#എങ്കിൽ defined(HAVE_UNISTD_H)
#അന്തര്ഭാവ <unistd.h>
#endif

using നാമാകാശ llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, വലുപ്പം(WsaData));
  എങ്കിൽ (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

സ്ഥിതം std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  തിരികെ std::error_code(::WSAGetLastError(), std::system_category());
#അല്ലെങ്കിൽ
  തിരികെ errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#അല്ലെങ്കിൽ
using NativeSocket = പൂർണ്ണസംഖ്യ;
#define INVALID_SOCKET -1
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  തിരികെ ::closesocket(Socket);
#അല്ലെങ്കിൽ
  തിരികെ ::close(Socket);
#endif
}

സ്ഥിതം Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  ഘടന sockaddr_un Addr;
  memset(&Addr, 0, വലുപ്പം(Addr));
  Addr.sun_family = AF_UNIX;

  എങ്കിൽ (വലുപ്പം(sockaddr_un::sun_path) <= SocketPath.size())
    തിരികെ make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), വലുപ്പം(Addr.sun_path) - 1);
  തിരികെ Addr;
}

സ്ഥിതം Expected<പൂർണ്ണസംഖ്യ> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  എങ്കിൽ (Socket == INVALID_SOCKET) {
    തിരികെ llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ശൂന്യ, 0);
#endif
  Expected<ഘടന sockaddr_un> Addr = setSocketAddr(SocketPath);
  എങ്കിൽ (!Addr) {
    closeSocket(Socket);
    തിരികെ Addr.takeError();
  }

  എങ്കിൽ (::connect(Socket, (ഘടന sockaddr *)&*Addr, വലുപ്പം(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    തിരികെ llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  തിരികെ _open_osfhandle(Socket, 0);
#അല്ലെങ്കിൽ
  തിരികെ Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(പൂർണ്ണസംഖ്യ SocketFD, StringRef SocketPath,
                                 പൂർണ്ണസംഖ്യ PipeFD[2])
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
                                                      പൂർണ്ണസംഖ്യ MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  എങ്കിൽ (llvm::sys::fs::exists(SocketPath)) {
    Expected<പൂർണ്ണസംഖ്യ> MaybeFD = getSocketFD(SocketPath);
    എങ്കിൽ (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      തിരികെ llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    തിരികെ llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  എങ്കിൽ (Socket == INVALID_SOCKET)
    തിരികെ llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ശൂന്യ, 0);
#endif
  Expected<ഘടന sockaddr_un> Addr = setSocketAddr(SocketPath);
  എങ്കിൽ (!Addr) {
    closeSocket(Socket);
    തിരികെ Addr.takeError();
  }

  എങ്കിൽ (::bind(Socket, (ഘടന sockaddr *)&*Addr, വലുപ്പം(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    തിരികെ llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  എങ്കിൽ (::listen(Socket, MaxBacklog) == -1)
    തിരികെ llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  പൂർണ്ണസംഖ്യ PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  എങ്കിൽ (::_pipe(PipeFD, 1, 0) == -1)
#അല്ലെങ്കിൽ
  എങ്കിൽ (::പ്രണാല(PipeFD) == -1)
#endif // _WIN32
    തിരികെ llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  തിരികെ ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#അല്ലെങ്കിൽ
  തിരികെ ListeningSocket{Socket, SocketPath, PipeFD};
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
സ്ഥിതം std::error_code
manageTimeout(സ്ഥിരം std::chrono::milliseconds &Timeout,
              സ്ഥിരം std::function<പൂർണ്ണസംഖ്യ()> &getActiveFD,
              സ്ഥിരം std::optional<പൂർണ്ണസംഖ്യ> &CancelFD = std::nullopt) {
  ഘടന pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#അല്ലെങ്കിൽ
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  എങ്കിൽ (CancelFD.has_മൂല്യം_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.മൂല്യം_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  പൂർണ്ണസംഖ്യ PollStatus = 0;
  ചെയ്യുക {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    എങ്കിൽ (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      എങ്കിൽ (TotalElapsedTime >= Timeout)
        തിരികെ std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.എണ്ണം());
  } ആകുമ്പോൾ (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#അല്ലെങ്കിൽ
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.എണ്ണം());
  } ആകുമ്പോൾ (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  എങ്കിൽ (getActiveFD() == -1 || (CancelFD.has_മൂല്യം_2() && FD[1].revents & POLLIN))
    തിരികെ std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  എങ്കിൽ (PollStatus == SOCKET_ERROR)
#അല്ലെങ്കിൽ
  എങ്കിൽ (PollStatus == -1)
#endif
    തിരികെ getLastSocketErrorCode();
  എങ്കിൽ (PollStatus == 0)
    തിരികെ std::make_error_code(std::errc::timed_out);
  എങ്കിൽ (FD[0].revents & POLLNVAL)
    തിരികെ std::make_error_code(std::errc::bad_file_descriptor);
  തിരികെ std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(സ്ഥിരം std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [ഇത്]() -> പൂർണ്ണസംഖ്യ { തിരികെ FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  എങ്കിൽ (TimeoutErr)
    തിരികെ llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  പൂർണ്ണസംഖ്യ AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), ശൂന്യ, ശൂന്യ);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#അല്ലെങ്കിൽ
  AcceptFD = ::accept(FD, ശൂന്യ, ശൂന്യ);
#endif

  എങ്കിൽ (AcceptFD == -1)
    തിരികെ llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  തിരികെ std::make_unique<raw_socket_stream>(AcceptFD);
}

ശൂന്യം ListeningSocket::shutdown() {
  പൂർണ്ണസംഖ്യ ObservedFD = FD.load();

  എങ്കിൽ (ObservedFD == -1)
    തിരികെ;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  എങ്കിൽ (!FD.compare_exchange_strong(ObservedFD, -1))
    തിരികെ;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  അക്ഷരം Byte = 'A';
  ssize_t written = ::ലിഖ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (ശൂന്യം)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  എങ്കിൽ (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  എങ്കിൽ (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(പൂർണ്ണസംഖ്യ SocketFD)
    : raw_fd_stream(SocketFD, സത്യം) {}

raw_socket_stream::~raw_socket_stream() = സ്ഥിരസ്ഥിതി;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<പൂർണ്ണസംഖ്യ> FD = getSocketFD(SocketPath);
  എങ്കിൽ (!FD)
    തിരികെ FD.takeError();
  തിരികെ std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::പഠ(അക്ഷരം *Ptr, size_t Size,
                                സ്ഥിരം std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [ഇത്]() -> പൂർണ്ണസംഖ്യ { തിരികെ ഇത്->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  എങ്കിൽ (Err) {
    raw_fd_stream::error_detected(Err);
    തിരികെ -1;
  }
  തിരികെ raw_fd_stream::പഠ(Ptr, Size);
}
