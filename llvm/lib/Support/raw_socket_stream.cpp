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

#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_socket_stream.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <functional>

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <sys/socket.h>
#ಅನ್ತರ್ಭಾವ <sys/un.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#ಅನ್ತರ್ಭಾವ <winsock2.h>
#ಅನ್ತರ್ಭಾವ <afunix.h>
// clang-format on
#ಅನ್ತರ್ಭಾವ <io.h>
#ಆದರೆ_ಅಂತ್ಯ // _WIN32

#ಆದರೆ defined(HAVE_UNISTD_H)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_ದತ್ತಾಂಶ;
  ::memset(&Wsa_ದತ್ತಾಂಶ, 0, ಗಾತ್ರ(Wsa_ದತ್ತಾಂಶ));
  ಆದರೆ (WSAStartup(MAKEWORD(2, 2), &Wsa_ದತ್ತಾಂಶ) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#ಆದರೆ_ಅಂತ್ಯ // _WIN32

ಸ್ಥಿರ std::error_ಸಂಕೇತ get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(::WSAGet_ಕೊನೆಯ_Error(), std::system_category());
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ errno_As_Error_ಸಂಕೇತ();
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
ಬಳಸಿ NativeSocket = SOCKET;
#ಇಲ್ಲದಿದ್ದರೆ
ಬಳಸಿ NativeSocket = ಪೂರ್ಣಾಂಕ;
#ವ್ಯಾಖ್ಯಾನಿಸು INVALID_SOCKET -1
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ closeSocket(NativeSocket Socket) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಹಿಂತಿರುಗಿ ::closesocket(Socket);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ::close(Socket);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ Expected<sockaddr_un> ಗಣ_Socket_Addr(StringRef Socket_ಮಾರ್ಗ) {
  ರಚನೆ sockaddr_un Addr;
  memset(&Addr, 0, ಗಾತ್ರ(Addr));
  Addr.sun_family = AF_UNIX;

  ಆದರೆ (ಗಾತ್ರ(sockaddr_un::sun_ಮಾರ್ಗ) <= Socket_ಮಾರ್ಗ.ಗಾತ್ರ_3())
    ಹಿಂತಿರುಗಿ make_error<StringError>(
        std::make_error_ಸಂಕೇತ(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_ಮಾರ್ಗ, Socket_ಮಾರ್ಗ.str().c_str(), ಗಾತ್ರ(Addr.sun_ಮಾರ್ಗ) - 1);
  ಹಿಂತಿರುಗಿ Addr;
}

ಸ್ಥಿರ Expected<ಪೂರ್ಣಾಂಕ> getSocketFD(StringRef Socket_ಮಾರ್ಗ) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_ಹರಿವು, 0);
  ಆದರೆ (Socket == INVALID_SOCKET) {
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ(),
                                         "Create socket failed");
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ಖಾಲಿ, 0);
#ಆದರೆ_ಅಂತ್ಯ
  Expected<ರಚನೆ sockaddr_un> Addr = ಗಣ_Socket_Addr(Socket_ಮಾರ್ಗ);
  ಆದರೆ (!Addr) {
    closeSocket(Socket);
    ಹಿಂತಿರುಗಿ Addr.takeError();
  }

  ಆದರೆ (::connect(Socket, (ರಚನೆ sockaddr *)&*Addr, ಗಾತ್ರ(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_ಸಂಕೇತ EC = get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ();
    closeSocket(Socket);
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಹಿಂತಿರುಗಿ _open_osfhandle(Socket, 0);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ Socket;
#ಆದರೆ_ಅಂತ್ಯ // _WIN32
}

ListeningSocket::ListeningSocket(ಪೂರ್ಣಾಂಕ SocketFD, StringRef Socket_ಮಾರ್ಗ,
                                 ಪೂರ್ಣಾಂಕ PipeFD[2])
    : FD(SocketFD), Socket_ಮಾರ್ಗ(Socket_ಮಾರ್ಗ), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.ತುಂಬು()), Socket_ಮಾರ್ಗ(LS.Socket_ಮಾರ್ಗ),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.Socket_ಮಾರ್ಗ.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef Socket_ಮಾರ್ಗ,
                                                      ಪೂರ್ಣಾಂಕ MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  ಆದರೆ (llvm::sys::fs::exists(Socket_ಮಾರ್ಗ)) {
    Expected<ಪೂರ್ಣಾಂಕ> MaybeFD = getSocketFD(Socket_ಮಾರ್ಗ);
    ಆದರೆ (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(
          std::make_error_ಸಂಕೇತ(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(
        std::make_error_ಸಂಕೇತ(std::errc::ವಿಳಾಸ_ಒಳ_use),
        "Socket address unavailable");
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  WSABalancer _;
#ಆದರೆ_ಅಂತ್ಯ
  NativeSocket Socket = socket(AF_UNIX, SOCK_ಹರಿವು, 0);
  ಆದರೆ (Socket == INVALID_SOCKET)
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ(),
                                         "socket create failed");

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, ಖಾಲಿ, 0);
#ಆದರೆ_ಅಂತ್ಯ
  Expected<ರಚನೆ sockaddr_un> Addr = ಗಣ_Socket_Addr(Socket_ಮಾರ್ಗ);
  ಆದರೆ (!Addr) {
    closeSocket(Socket);
    ಹಿಂತಿರುಗಿ Addr.takeError();
  }

  ಆದರೆ (::bind(Socket, (ರಚನೆ sockaddr *)&*Addr, ಗಾತ್ರ(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_ಸಂಕೇತ EC = get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ();
    closeSocket(Socket);
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  ಆದರೆ (::listen(Socket, MaxBacklog) == -1)
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ(),
                                         "Listen error");

  ಪೂರ್ಣಾಂಕ PipeFD[2];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  ಆದರೆ (::_pipe(PipeFD, 1, 0) == -1)
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (::ಪ್ರಣಾಲ(PipeFD) == -1)
#ಆದರೆ_ಅಂತ್ಯ // _WIN32
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ(),
                                         "pipe failed");

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಹಿಂತಿರುಗಿ ListeningSocket{_open_osfhandle(Socket, 0), Socket_ಮಾರ್ಗ, PipeFD};
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ListeningSocket{Socket, Socket_ಮಾರ್ಗ, PipeFD};
#ಆದರೆ_ಅಂತ್ಯ // _WIN32
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
ಸ್ಥಿರ std::error_ಸಂಕೇತ
manageTimeout(ಸ್ಥಿರಾಂಕ std::chrono::milliseconds &Timeout,
              ಸ್ಥಿರಾಂಕ std::function<ಪೂರ್ಣಾಂಕ()> &get_ಸಕ್ರಿಯ_FD,
              ಸ್ಥಿರಾಂಕ std::optional<ಪೂರ್ಣಾಂಕ> &CancelFD = std::nullopt) {
  ರಚನೆ pollfd FD[2];
  FD[0].events = POLLIN;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  SOCKET WinServerSock = _get_osfhandle(get_ಸಕ್ರಿಯ_FD());
  FD[0].fd = WinServerSock;
#ಇಲ್ಲದಿದ್ದರೆ
  FD[0].fd = get_ಸಕ್ರಿಯ_FD();
#ಆದರೆ_ಅಂತ್ಯ
  uint8_t FDCount = 1;
  ಆದರೆ (CancelFD.has_ಮೌಲ್ಯ_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.ಮೌಲ್ಯ_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  ಸ್ವಯಂಚಾಲಿತ ಆರಂಭ = std::chrono::steady_clock::now();
  ಸ್ವಯಂಚಾಲಿತ ಉಳಿದ_Timeout = Timeout;
  ಪೂರ್ಣಾಂಕ Poll_ಸ್ಥಿತಿ = 0;
  ಮಾಡು {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    ಆದರೆ (Poll_ಸ್ಥಿತಿ != 0 && Timeout != std::chrono::milliseconds(-1)) {
      ಸ್ವಯಂಚಾಲಿತ ಒಟ್ಟು_Elapsed_ಸಮಯ =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - ಆರಂಭ);

      ಆದರೆ (ಒಟ್ಟು_Elapsed_ಸಮಯ >= Timeout)
        ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::operation_would_ಖಂಡ);

      ಉಳಿದ_Timeout = Timeout - ಒಟ್ಟು_Elapsed_ಸಮಯ;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
    Poll_ಸ್ಥಿತಿ = WSAPoll(FD, FDCount, ಉಳಿದ_Timeout.ಎಣಿಕೆ_2());
  } ಆಗ (Poll_ಸ್ಥಿತಿ == SOCKET_ERROR &&
           get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ() == std::errc::interrupted);
#ಇಲ್ಲದಿದ್ದರೆ
    Poll_ಸ್ಥಿತಿ = ::poll(FD, FDCount, ಉಳಿದ_Timeout.ಎಣಿಕೆ_2());
  } ಆಗ (Poll_ಸ್ಥಿತಿ == -1 &&
           get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ() == std::errc::interrupted);
#ಆದರೆ_ಅಂತ್ಯ

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  ಆದರೆ (get_ಸಕ್ರಿಯ_FD() == -1 || (CancelFD.has_ಮೌಲ್ಯ_2() && FD[1].revents & POLLIN))
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::operation_canceled);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಆದರೆ (Poll_ಸ್ಥಿತಿ == SOCKET_ERROR)
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (Poll_ಸ್ಥಿತಿ == -1)
#ಆದರೆ_ಅಂತ್ಯ
    ಹಿಂತಿರುಗಿ get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ();
  ಆದರೆ (Poll_ಸ್ಥಿತಿ == 0)
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::timed_ಹೊರಹೋಗು);
  ಆದರೆ (FD[0].revents & POLLNVAL)
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::bad_file_descriptor);
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

Expected<std::unique_ptr<raw_socket_ಹರಿವು>>
ListeningSocket::accept(ಸ್ಥಿರಾಂಕ std::chrono::milliseconds &Timeout) {
  ಸ್ವಯಂಚಾಲಿತ get_ಸಕ್ರಿಯ_FD = [ಇದು]() -> ಪೂರ್ಣಾಂಕ { ಹಿಂತಿರುಗಿ FD; };
  std::error_ಸಂಕೇತ TimeoutErr = manageTimeout(Timeout, get_ಸಕ್ರಿಯ_FD, PipeFD[0]);
  ಆದರೆ (TimeoutErr)
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  ಪೂರ್ಣಾಂಕ AcceptFD;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), ಖಾಲಿ, ಖಾಲಿ);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#ಇಲ್ಲದಿದ್ದರೆ
  AcceptFD = ::accept(FD, ಖಾಲಿ, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (AcceptFD == -1)
    ಹಿಂತಿರುಗಿ llvm::make_error<StringError>(get_ಕೊನೆಯ_Socket_Error_ಸಂಕೇತ(),
                                         "Socket accept failed");
  ಹಿಂತಿರುಗಿ std::make_unique<raw_socket_ಹರಿವು>(AcceptFD);
}

ಶೂನ್ಯ ListeningSocket::shutdown() {
  ಪೂರ್ಣಾಂಕ ObservedFD = FD.ತುಂಬು();

  ಆದರೆ (ObservedFD == -1)
    ಹಿಂತಿರುಗಿ;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  ಆದರೆ (!FD.compare_exchange_strong(ObservedFD, -1))
    ಹಿಂತಿರುಗಿ;

  ::close(ObservedFD);
  ::unlink(Socket_ಮಾರ್ಗ.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  ಅಕ್ಷರ Byte = 'A';
  ssize_t written = ::ಲಿಖ(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (ಶೂನ್ಯ)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  ಆದರೆ (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  ಆದರೆ (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_ಹರಿವು::raw_socket_ಹರಿವು(ಪೂರ್ಣಾಂಕ SocketFD)
    : raw_fd_ಹರಿವು(SocketFD, ನಿಜ) {}

raw_socket_ಹರಿವು::~raw_socket_ಹರಿವು() = ಪೂರ್ವನಿಯೋಜಿತ;

Expected<std::unique_ptr<raw_socket_ಹರಿವು>>
raw_socket_ಹರಿವು::createConnectedUnix(StringRef Socket_ಮಾರ್ಗ) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  WSABalancer _;
#ಆದರೆ_ಅಂತ್ಯ // _WIN32
  Expected<ಪೂರ್ಣಾಂಕ> FD = getSocketFD(Socket_ಮಾರ್ಗ);
  ಆದರೆ (!FD)
    ಹಿಂತಿರುಗಿ FD.takeError();
  ಹಿಂತಿರುಗಿ std::make_unique<raw_socket_ಹರಿವು>(*FD);
}

ssize_t raw_socket_ಹರಿವು::ಪಠ(ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2,
                                ಸ್ಥಿರಾಂಕ std::chrono::milliseconds &Timeout) {
  ಸ್ವಯಂಚಾಲಿತ get_ಸಕ್ರಿಯ_FD = [ಇದು]() -> ಪೂರ್ಣಾಂಕ { ಹಿಂತಿರುಗಿ ಇದು->get_fd(); };
  std::error_ಸಂಕೇತ Err = manageTimeout(Timeout, get_ಸಕ್ರಿಯ_FD);
  // Mimic raw_fd_stream::read error handling behavior
  ಆದರೆ (Err) {
    raw_fd_ಹರಿವು::error_detected(Err);
    ಹಿಂತಿರುಗಿ -1;
  }
  ಹಿಂತಿರುಗಿ raw_fd_ಹರಿವು::ಪಠ(Ptr, ಗಾತ್ರ_2);
}
