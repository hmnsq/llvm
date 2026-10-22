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

#અન્તર્ભાવ "llvm/Support/raw_socket_stream.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"

#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <functional>

#ifndef _WIN32
#અન્તર્ભાવ <poll.h>
#અન્તર્ભાવ <sys/socket.h>
#અન્તર્ભાવ <sys/un.h>
#નહીંતર
#અન્તર્ભાવ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#અન્તર્ભાવ <winsock2.h>
#અન્તર્ભાવ <afunix.h>
// clang-format on
#અન્તર્ભાવ <io.h>
#endif // _WIN32

#જો defined(HAVE_UNISTD_H)
#અન્તર્ભાવ <unistd.h>
#endif

using નામાકાશ llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, કદ(WsaData));
  જો (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

સ્થિર std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  પાછા std::error_code(::WSAGetLastError(), std::system_category());
#નહીંતર
  પાછા errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#નહીંતર
using NativeSocket = પૂર્ણાંક;
#define INVALID_SOCKET -1
#endif

સ્થિર પૂર્ણાંક closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  પાછા ::closesocket(Socket);
#નહીંતર
  પાછા ::close(Socket);
#endif
}

સ્થિર Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  સંરચના sockaddr_un Addr;
  memset(&Addr, 0, કદ(Addr));
  Addr.sun_family = AF_UNIX;

  જો (કદ(sockaddr_un::sun_path) <= SocketPath.size())
    પાછા make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), કદ(Addr.sun_path) - 1);
  પાછા Addr;
}

સ્થિર Expected<પૂર્ણાંક> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  જો (Socket == INVALID_SOCKET) {
    પાછા llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ખાલી, 0);
#endif
  Expected<સંરચના sockaddr_un> Addr = setSocketAddr(SocketPath);
  જો (!Addr) {
    closeSocket(Socket);
    પાછા Addr.takeError();
  }

  જો (::connect(Socket, (સંરચના sockaddr *)&*Addr, કદ(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    પાછા llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  પાછા _open_osfhandle(Socket, 0);
#નહીંતર
  પાછા Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(પૂર્ણાંક SocketFD, StringRef SocketPath,
                                 પૂર્ણાંક PipeFD[2])
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
                                                      પૂર્ણાંક MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  જો (llvm::sys::fs::exists(SocketPath)) {
    Expected<પૂર્ણાંક> MaybeFD = getSocketFD(SocketPath);
    જો (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      પાછા llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    પાછા llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  જો (Socket == INVALID_SOCKET)
    પાછા llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ખાલી, 0);
#endif
  Expected<સંરચના sockaddr_un> Addr = setSocketAddr(SocketPath);
  જો (!Addr) {
    closeSocket(Socket);
    પાછા Addr.takeError();
  }

  જો (::bind(Socket, (સંરચના sockaddr *)&*Addr, કદ(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    પાછા llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  જો (::listen(Socket, MaxBacklog) == -1)
    પાછા llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  પૂર્ણાંક PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  જો (::_pipe(PipeFD, 1, 0) == -1)
#નહીંતર
  જો (::પ્રણાલ(PipeFD) == -1)
#endif // _WIN32
    પાછા llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  પાછા ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#નહીંતર
  પાછા ListeningSocket{Socket, SocketPath, PipeFD};
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
સ્થિર std::error_code
manageTimeout(અચલ std::chrono::milliseconds &Timeout,
              અચલ std::function<પૂર્ણાંક()> &getActiveFD,
              અચલ std::optional<પૂર્ણાંક> &CancelFD = std::nullopt) {
  સંરચના pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#નહીંતર
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  જો (CancelFD.has_કિંમત_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.કિંમત_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  પૂર્ણાંક PollStatus = 0;
  કરો {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    જો (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      જો (TotalElapsedTime >= Timeout)
        પાછા std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.ગણતરી());
  } જ્યારે (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#નહીંતર
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.ગણતરી());
  } જ્યારે (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  જો (getActiveFD() == -1 || (CancelFD.has_કિંમત_2() && FD[1].revents & POLLIN))
    પાછા std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  જો (PollStatus == SOCKET_ERROR)
#નહીંતર
  જો (PollStatus == -1)
#endif
    પાછા getLastSocketErrorCode();
  જો (PollStatus == 0)
    પાછા std::make_error_code(std::errc::timed_out);
  જો (FD[0].revents & POLLNVAL)
    પાછા std::make_error_code(std::errc::bad_file_descriptor);
  પાછા std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(અચલ std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [આ]() -> પૂર્ણાંક { પાછા FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  જો (TimeoutErr)
    પાછા llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  પૂર્ણાંક AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), ખાલી, ખાલી);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#નહીંતર
  AcceptFD = ::accept(FD, ખાલી, ખાલી);
#endif

  જો (AcceptFD == -1)
    પાછા llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  પાછા std::make_unique<raw_socket_stream>(AcceptFD);
}

શૂન્ય ListeningSocket::shutdown() {
  પૂર્ણાંક ObservedFD = FD.load();

  જો (ObservedFD == -1)
    પાછા;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  જો (!FD.compare_exchange_strong(ObservedFD, -1))
    પાછા;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  અક્ષર Byte = 'A';
  ssize_t written = ::લિખ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (શૂન્ય)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  જો (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  જો (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(પૂર્ણાંક SocketFD)
    : raw_fd_stream(SocketFD, સાચું) {}

raw_socket_stream::~raw_socket_stream() = મૂળભૂત;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<પૂર્ણાંક> FD = getSocketFD(SocketPath);
  જો (!FD)
    પાછા FD.takeError();
  પાછા std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::પઠ(અક્ષર *Ptr, size_t Size,
                                અચલ std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [આ]() -> પૂર્ણાંક { પાછા આ->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  જો (Err) {
    raw_fd_stream::error_detected(Err);
    પાછા -1;
  }
  પાછા raw_fd_stream::પઠ(Ptr, Size);
}
