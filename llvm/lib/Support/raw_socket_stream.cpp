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

#অন্তর্ভাৱ "llvm/Support/raw_socket_stream.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/Error.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"

#অন্তর্ভাৱ <atomic>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <functional>

#ifndef _WIN32
#অন্তর্ভাৱ <poll.h>
#অন্তর্ভাৱ <sys/socket.h>
#অন্তর্ভাৱ <sys/un.h>
#অন্যথা
#অন্তর্ভাৱ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#অন্তর্ভাৱ <winsock2.h>
#অন্তর্ভাৱ <afunix.h>
// clang-format on
#অন্তর্ভাৱ <io.h>
#endif // _WIN32

#যদি defined(HAVE_UNISTD_H)
#অন্তর্ভাৱ <unistd.h>
#endif

using নামাকাশ llvm;

#ifdef _WIN32
WSABalancer::WSABalancer() {
  WSADATA WsaData;
  ::memset(&WsaData, 0, পরিমাণ(WsaData));
  যদি (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#endif // _WIN32

স্থির std::error_code getLastSocketErrorCode() {
#ifdef _WIN32
  ঘূৰাই std::error_code(::WSAGetLastError(), std::system_category());
#অন্যথা
  ঘূৰাই errnoAsErrorCode();
#endif
}

#ifdef _WIN32
using NativeSocket = SOCKET;
#অন্যথা
using NativeSocket = পূর্ণাঙ্ক;
#define INVALID_SOCKET -1
#endif

স্থির পূর্ণাঙ্ক closeSocket(NativeSocket Socket) {
#ifdef _WIN32
  ঘূৰাই ::closesocket(Socket);
#অন্যথা
  ঘূৰাই ::close(Socket);
#endif
}

স্থির Expected<sockaddr_un> setSocketAddr(StringRef SocketPath) {
  সংরচনা sockaddr_un Addr;
  memset(&Addr, 0, পরিমাণ(Addr));
  Addr.sun_family = AF_UNIX;

  যদি (পরিমাণ(sockaddr_un::sun_path) <= SocketPath.size())
    ঘূৰাই make_error<StringError>(
        std::make_error_code(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_path, SocketPath.str().c_str(), পরিমাণ(Addr.sun_path) - 1);
  ঘূৰাই Addr;
}

স্থির Expected<পূর্ণাঙ্ক> getSocketFD(StringRef SocketPath) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  যদি (Socket == INVALID_SOCKET) {
    ঘূৰাই llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Create socket failed");
  }

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, রিক্ত, 0);
#endif
  Expected<সংরচনা sockaddr_un> Addr = setSocketAddr(SocketPath);
  যদি (!Addr) {
    closeSocket(Socket);
    ঘূৰাই Addr.takeError();
  }

  যদি (::connect(Socket, (সংরচনা sockaddr *)&*Addr, পরিমাণ(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    ঘূৰাই llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ifdef _WIN32
  ঘূৰাই _open_osfhandle(Socket, 0);
#অন্যথা
  ঘূৰাই Socket;
#endif // _WIN32
}

ListeningSocket::ListeningSocket(পূর্ণাঙ্ক SocketFD, StringRef SocketPath,
                                 পূর্ণাঙ্ক PipeFD[2])
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
                                                      পূর্ণাঙ্ক MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  যদি (llvm::sys::fs::exists(SocketPath)) {
    Expected<পূর্ণাঙ্ক> MaybeFD = getSocketFD(SocketPath);
    যদি (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      ঘূৰাই llvm::make_error<StringError>(
          std::make_error_code(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    ঘূৰাই llvm::make_error<StringError>(
        std::make_error_code(std::errc::address_in_use),
        "Socket address unavailable");
  }

#ifdef _WIN32
  WSABalancer _;
#endif
  NativeSocket Socket = socket(AF_UNIX, SOCK_STREAM, 0);
  যদি (Socket == INVALID_SOCKET)
    ঘূৰাই llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "socket create failed");

#ifdef __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, রিক্ত, 0);
#endif
  Expected<সংরচনা sockaddr_un> Addr = setSocketAddr(SocketPath);
  যদি (!Addr) {
    closeSocket(Socket);
    ঘূৰাই Addr.takeError();
  }

  যদি (::bind(Socket, (সংরচনা sockaddr *)&*Addr, পরিমাণ(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_code EC = getLastSocketErrorCode();
    closeSocket(Socket);
    ঘূৰাই llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  যদি (::listen(Socket, MaxBacklog) == -1)
    ঘূৰাই llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Listen error");

  পূর্ণাঙ্ক PipeFD[2];
#ifdef _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  যদি (::_pipe(PipeFD, 1, 0) == -1)
#অন্যথা
  যদি (::প্রণাল(PipeFD) == -1)
#endif // _WIN32
    ঘূৰাই llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "pipe failed");

#ifdef _WIN32
  ঘূৰাই ListeningSocket{_open_osfhandle(Socket, 0), SocketPath, PipeFD};
#অন্যথা
  ঘূৰাই ListeningSocket{Socket, SocketPath, PipeFD};
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
স্থির std::error_code
manageTimeout(ধ্রুৱ std::chrono::milliseconds &Timeout,
              ধ্রুৱ std::function<পূর্ণাঙ্ক()> &getActiveFD,
              ধ্রুৱ std::optional<পূর্ণাঙ্ক> &CancelFD = std::nullopt) {
  সংরচনা pollfd FD[2];
  FD[0].events = POLLIN;
#ifdef _WIN32
  SOCKET WinServerSock = _get_osfhandle(getActiveFD());
  FD[0].fd = WinServerSock;
#অন্যথা
  FD[0].fd = getActiveFD();
#endif
  uint8_t FDCount = 1;
  যদি (CancelFD.has_মান_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.মান_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  auto Start = std::chrono::steady_clock::now();
  auto RemainingTimeout = Timeout;
  পূর্ণাঙ্ক PollStatus = 0;
  কৰা {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    যদি (PollStatus != 0 && Timeout != std::chrono::milliseconds(-1)) {
      auto TotalElapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - Start);

      যদি (TotalElapsedTime >= Timeout)
        ঘূৰাই std::make_error_code(std::errc::operation_would_block);

      RemainingTimeout = Timeout - TotalElapsedTime;
    }
#ifdef _WIN32
    PollStatus = WSAPoll(FD, FDCount, RemainingTimeout.গুনতি());
  } যেতিয়ালৈ (PollStatus == SOCKET_ERROR &&
           getLastSocketErrorCode() == std::errc::interrupted);
#অন্যথা
    PollStatus = ::poll(FD, FDCount, RemainingTimeout.গুনতি());
  } যেতিয়ালৈ (PollStatus == -1 &&
           getLastSocketErrorCode() == std::errc::interrupted);
#endif

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  যদি (getActiveFD() == -1 || (CancelFD.has_মান_2() && FD[1].revents & POLLIN))
    ঘূৰাই std::make_error_code(std::errc::operation_canceled);
#ifdef _WIN32
  যদি (PollStatus == SOCKET_ERROR)
#অন্যথা
  যদি (PollStatus == -1)
#endif
    ঘূৰাই getLastSocketErrorCode();
  যদি (PollStatus == 0)
    ঘূৰাই std::make_error_code(std::errc::timed_out);
  যদি (FD[0].revents & POLLNVAL)
    ঘূৰাই std::make_error_code(std::errc::bad_file_descriptor);
  ঘূৰাই std::error_code();
}

Expected<std::unique_ptr<raw_socket_stream>>
ListeningSocket::accept(ধ্রুৱ std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [এতত্]() -> পূর্ণাঙ্ক { ঘূৰাই FD; };
  std::error_code TimeoutErr = manageTimeout(Timeout, getActiveFD, PipeFD[0]);
  যদি (TimeoutErr)
    ঘূৰাই llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  পূর্ণাঙ্ক AcceptFD;
#ifdef _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), রিক্ত, রিক্ত);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#অন্যথা
  AcceptFD = ::accept(FD, রিক্ত, রিক্ত);
#endif

  যদি (AcceptFD == -1)
    ঘূৰাই llvm::make_error<StringError>(getLastSocketErrorCode(),
                                         "Socket accept failed");
  ঘূৰাই std::make_unique<raw_socket_stream>(AcceptFD);
}

শূন্য ListeningSocket::shutdown() {
  পূর্ণাঙ্ক ObservedFD = FD.load();

  যদি (ObservedFD == -1)
    ঘূৰাই;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  যদি (!FD.compare_exchange_strong(ObservedFD, -1))
    ঘূৰাই;

  ::close(ObservedFD);
  ::unlink(SocketPath.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  ৱর্ণ Byte = 'A';
  ssize_t written = ::লিখ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (শূন্য)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  যদি (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  যদি (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_stream::raw_socket_stream(পূর্ণাঙ্ক SocketFD)
    : raw_fd_stream(SocketFD, সঁচা) {}

raw_socket_stream::~raw_socket_stream() = স্ৱভাৱ;

Expected<std::unique_ptr<raw_socket_stream>>
raw_socket_stream::createConnectedUnix(StringRef SocketPath) {
#ifdef _WIN32
  WSABalancer _;
#endif // _WIN32
  Expected<পূর্ণাঙ্ক> FD = getSocketFD(SocketPath);
  যদি (!FD)
    ঘূৰাই FD.takeError();
  ঘূৰাই std::make_unique<raw_socket_stream>(*FD);
}

ssize_t raw_socket_stream::পঠ(ৱর্ণ *Ptr, size_t Size,
                                ধ্রুৱ std::chrono::milliseconds &Timeout) {
  auto getActiveFD = [এতত্]() -> পূর্ণাঙ্ক { ঘূৰাই এতত্->get_fd(); };
  std::error_code Err = manageTimeout(Timeout, getActiveFD);
  // Mimic raw_fd_stream::read error handling behavior
  যদি (Err) {
    raw_fd_stream::error_detected(Err);
    ঘূৰাই -1;
  }
  ঘূৰাই raw_fd_stream::পঠ(Ptr, Size);
}
