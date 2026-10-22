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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_socket_stream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <atomic>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <functional>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ <poll.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/socket.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/un.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#ਅਨ੍ਤਰ੍ਭਾਵ <winsock2.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <afunix.h>
// clang-format on
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ // _WIN32

#ਜੇ defined(HAVE_UNISTD_H)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_ਅੰਕੜੇ;
  ::memset(&Wsa_ਅੰਕੜੇ, 0, ਆਕਾਰ(Wsa_ਅੰਕੜੇ));
  ਜੇ (WSAStartup(MAKEWORD(2, 2), &Wsa_ਅੰਕੜੇ) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#ਜੇ_ਅੰਤ // _WIN32

ਸਥਿਰ std::error_ਕੋਡ get_ਆਖਰੀ_Socket_Error_ਕੋਡ() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਵਾਪਸ std::error_ਕੋਡ(::WSAGet_ਆਖਰੀ_Error(), std::system_category());
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ errno_As_Error_ਕੋਡ();
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
ਵਰਤੋਂ NativeSocket = SOCKET;
#ਨਹੀਂਤਾਂ
ਵਰਤੋਂ NativeSocket = ਪੂਰਨ;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INVALID_SOCKET -1
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ closeSocket(NativeSocket Socket) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਵਾਪਸ ::closesocket(Socket);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ::close(Socket);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ Expected<sockaddr_un> ਸੈੱਟ_Socket_Addr(StringRef Socket_ਰਸਤਾ) {
  ਬਣਤਰ sockaddr_un Addr;
  memset(&Addr, 0, ਆਕਾਰ(Addr));
  Addr.sun_family = AF_UNIX;

  ਜੇ (ਆਕਾਰ(sockaddr_un::sun_ਰਸਤਾ) <= Socket_ਰਸਤਾ.ਮਾਪ_2())
    ਵਾਪਸ make_error<StringError>(
        std::make_error_ਕੋਡ(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_ਰਸਤਾ, Socket_ਰਸਤਾ.str().c_str(), ਆਕਾਰ(Addr.sun_ਰਸਤਾ) - 1);
  ਵਾਪਸ Addr;
}

ਸਥਿਰ Expected<ਪੂਰਨ> getSocketFD(StringRef Socket_ਰਸਤਾ) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_ਧਾਰਾ, 0);
  ਜੇ (Socket == INVALID_SOCKET) {
    ਵਾਪਸ llvm::make_error<StringError>(get_ਆਖਰੀ_Socket_Error_ਕੋਡ(),
                                         "Create socket failed");
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ਨਿਰਾ, 0);
#ਜੇ_ਅੰਤ
  Expected<ਬਣਤਰ sockaddr_un> Addr = ਸੈੱਟ_Socket_Addr(Socket_ਰਸਤਾ);
  ਜੇ (!Addr) {
    closeSocket(Socket);
    ਵਾਪਸ Addr.takeError();
  }

  ਜੇ (::connect(Socket, (ਬਣਤਰ sockaddr *)&*Addr, ਆਕਾਰ(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_ਕੋਡ EC = get_ਆਖਰੀ_Socket_Error_ਕੋਡ();
    closeSocket(Socket);
    ਵਾਪਸ llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਵਾਪਸ _open_osfhandle(Socket, 0);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ Socket;
#ਜੇ_ਅੰਤ // _WIN32
}

ListeningSocket::ListeningSocket(ਪੂਰਨ SocketFD, StringRef Socket_ਰਸਤਾ,
                                 ਪੂਰਨ PipeFD[2])
    : FD(SocketFD), Socket_ਰਸਤਾ(Socket_ਰਸਤਾ), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.ਲੱਦੋ()), Socket_ਰਸਤਾ(LS.Socket_ਰਸਤਾ),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.Socket_ਰਸਤਾ.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef Socket_ਰਸਤਾ,
                                                      ਪੂਰਨ MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  ਜੇ (llvm::sys::fs::exists(Socket_ਰਸਤਾ)) {
    Expected<ਪੂਰਨ> MaybeFD = getSocketFD(Socket_ਰਸਤਾ);
    ਜੇ (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      ਵਾਪਸ llvm::make_error<StringError>(
          std::make_error_ਕੋਡ(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    ਵਾਪਸ llvm::make_error<StringError>(
        std::make_error_ਕੋਡ(std::errc::ਪਤਾ_ਵਿੱਚ_use),
        "Socket address unavailable");
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  WSABalancer _;
#ਜੇ_ਅੰਤ
  NativeSocket Socket = socket(AF_UNIX, SOCK_ਧਾਰਾ, 0);
  ਜੇ (Socket == INVALID_SOCKET)
    ਵਾਪਸ llvm::make_error<StringError>(get_ਆਖਰੀ_Socket_Error_ਕੋਡ(),
                                         "socket create failed");

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ਨਿਰਾ, 0);
#ਜੇ_ਅੰਤ
  Expected<ਬਣਤਰ sockaddr_un> Addr = ਸੈੱਟ_Socket_Addr(Socket_ਰਸਤਾ);
  ਜੇ (!Addr) {
    closeSocket(Socket);
    ਵਾਪਸ Addr.takeError();
  }

  ਜੇ (::bind(Socket, (ਬਣਤਰ sockaddr *)&*Addr, ਆਕਾਰ(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_ਕੋਡ EC = get_ਆਖਰੀ_Socket_Error_ਕੋਡ();
    closeSocket(Socket);
    ਵਾਪਸ llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  ਜੇ (::listen(Socket, MaxBacklog) == -1)
    ਵਾਪਸ llvm::make_error<StringError>(get_ਆਖਰੀ_Socket_Error_ਕੋਡ(),
                                         "Listen error");

  ਪੂਰਨ PipeFD[2];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  ਜੇ (::_pipe(PipeFD, 1, 0) == -1)
#ਨਹੀਂਤਾਂ
  ਜੇ (::ਪ੍ਰਣਾਲ(PipeFD) == -1)
#ਜੇ_ਅੰਤ // _WIN32
    ਵਾਪਸ llvm::make_error<StringError>(get_ਆਖਰੀ_Socket_Error_ਕੋਡ(),
                                         "pipe failed");

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਵਾਪਸ ListeningSocket{_open_osfhandle(Socket, 0), Socket_ਰਸਤਾ, PipeFD};
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ListeningSocket{Socket, Socket_ਰਸਤਾ, PipeFD};
#ਜੇ_ਅੰਤ // _WIN32
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
ਸਥਿਰ std::error_ਕੋਡ
manageTimeout(ਅਚਲ std::chrono::milliseconds &Timeout,
              ਅਚਲ std::function<ਪੂਰਨ()> &get_ਸਰਗਰਮ_FD,
              ਅਚਲ std::optional<ਪੂਰਨ> &CancelFD = std::nullopt) {
  ਬਣਤਰ pollfd FD[2];
  FD[0].events = POLLIN;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  SOCKET WinServerSock = _get_osfhandle(get_ਸਰਗਰਮ_FD());
  FD[0].fd = WinServerSock;
#ਨਹੀਂਤਾਂ
  FD[0].fd = get_ਸਰਗਰਮ_FD();
#ਜੇ_ਅੰਤ
  uint8_t FDCount = 1;
  ਜੇ (CancelFD.has_ਮੁੱਲ_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.ਮੁੱਲ_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  ਸਵੈਚਾਲਿਤ ਸ਼ੁਰੂ = std::chrono::steady_clock::now();
  ਸਵੈਚਾਲਿਤ ਬਚਿਆ_Timeout = Timeout;
  ਪੂਰਨ Poll_ਸਥਿਤੀ = 0;
  ਕਰੋ {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    ਜੇ (Poll_ਸਥਿਤੀ != 0 && Timeout != std::chrono::milliseconds(-1)) {
      ਸਵੈਚਾਲਿਤ ਕੁੱਲ_Elapsed_ਸਮਾਂ =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - ਸ਼ੁਰੂ);

      ਜੇ (ਕੁੱਲ_Elapsed_ਸਮਾਂ >= Timeout)
        ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::operation_would_ਖੰਡ);

      ਬਚਿਆ_Timeout = Timeout - ਕੁੱਲ_Elapsed_ਸਮਾਂ;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
    Poll_ਸਥਿਤੀ = WSAPoll(FD, FDCount, ਬਚਿਆ_Timeout.ਗਿਣਤੀ_2());
  } ਜਦੋਂ (Poll_ਸਥਿਤੀ == SOCKET_ERROR &&
           get_ਆਖਰੀ_Socket_Error_ਕੋਡ() == std::errc::interrupted);
#ਨਹੀਂਤਾਂ
    Poll_ਸਥਿਤੀ = ::poll(FD, FDCount, ਬਚਿਆ_Timeout.ਗਿਣਤੀ_2());
  } ਜਦੋਂ (Poll_ਸਥਿਤੀ == -1 &&
           get_ਆਖਰੀ_Socket_Error_ਕੋਡ() == std::errc::interrupted);
#ਜੇ_ਅੰਤ

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  ਜੇ (get_ਸਰਗਰਮ_FD() == -1 || (CancelFD.has_ਮੁੱਲ_2() && FD[1].revents & POLLIN))
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::operation_canceled);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਜੇ (Poll_ਸਥਿਤੀ == SOCKET_ERROR)
#ਨਹੀਂਤਾਂ
  ਜੇ (Poll_ਸਥਿਤੀ == -1)
#ਜੇ_ਅੰਤ
    ਵਾਪਸ get_ਆਖਰੀ_Socket_Error_ਕੋਡ();
  ਜੇ (Poll_ਸਥਿਤੀ == 0)
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::timed_ਬਾਹਰਲਾ);
  ਜੇ (FD[0].revents & POLLNVAL)
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::bad_file_descriptor);
  ਵਾਪਸ std::error_ਕੋਡ();
}

Expected<std::unique_ptr<raw_socket_ਧਾਰਾ>>
ListeningSocket::accept(ਅਚਲ std::chrono::milliseconds &Timeout) {
  ਸਵੈਚਾਲਿਤ get_ਸਰਗਰਮ_FD = [ਇਹ]() -> ਪੂਰਨ { ਵਾਪਸ FD; };
  std::error_ਕੋਡ TimeoutErr = manageTimeout(Timeout, get_ਸਰਗਰਮ_FD, PipeFD[0]);
  ਜੇ (TimeoutErr)
    ਵਾਪਸ llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  ਪੂਰਨ AcceptFD;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), ਨਿਰਾ, ਨਿਰਾ);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#ਨਹੀਂਤਾਂ
  AcceptFD = ::accept(FD, ਨਿਰਾ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ

  ਜੇ (AcceptFD == -1)
    ਵਾਪਸ llvm::make_error<StringError>(get_ਆਖਰੀ_Socket_Error_ਕੋਡ(),
                                         "Socket accept failed");
  ਵਾਪਸ std::make_unique<raw_socket_ਧਾਰਾ>(AcceptFD);
}

ਖਾਲੀ ListeningSocket::shutdown() {
  ਪੂਰਨ ObservedFD = FD.ਲੱਦੋ();

  ਜੇ (ObservedFD == -1)
    ਵਾਪਸ;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  ਜੇ (!FD.compare_exchange_strong(ObservedFD, -1))
    ਵਾਪਸ;

  ::close(ObservedFD);
  ::unlink(Socket_ਰਸਤਾ.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  ਅੱਖਰ Byte = 'A';
  ssize_t written = ::ਲਿਖ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (ਖਾਲੀ)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  ਜੇ (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  ਜੇ (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_ਧਾਰਾ::raw_socket_ਧਾਰਾ(ਪੂਰਨ SocketFD)
    : raw_fd_ਧਾਰਾ(SocketFD, ਸੱਚ) {}

raw_socket_ਧਾਰਾ::~raw_socket_ਧਾਰਾ() = ਮੂਲ;

Expected<std::unique_ptr<raw_socket_ਧਾਰਾ>>
raw_socket_ਧਾਰਾ::createConnectedUnix(StringRef Socket_ਰਸਤਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  WSABalancer _;
#ਜੇ_ਅੰਤ // _WIN32
  Expected<ਪੂਰਨ> FD = getSocketFD(Socket_ਰਸਤਾ);
  ਜੇ (!FD)
    ਵਾਪਸ FD.takeError();
  ਵਾਪਸ std::make_unique<raw_socket_ਧਾਰਾ>(*FD);
}

ssize_t raw_socket_ਧਾਰਾ::ਪਠ(ਅੱਖਰ *Ptr, size_t ਮਾਪ,
                                ਅਚਲ std::chrono::milliseconds &Timeout) {
  ਸਵੈਚਾਲਿਤ get_ਸਰਗਰਮ_FD = [ਇਹ]() -> ਪੂਰਨ { ਵਾਪਸ ਇਹ->get_fd(); };
  std::error_ਕੋਡ Err = manageTimeout(Timeout, get_ਸਰਗਰਮ_FD);
  // Mimic raw_fd_stream::read error handling behavior
  ਜੇ (Err) {
    raw_fd_ਧਾਰਾ::error_detected(Err);
    ਵਾਪਸ -1;
  }
  ਵਾਪਸ raw_fd_ਧਾਰਾ::ਪਠ(Ptr, ਮਾਪ);
}
