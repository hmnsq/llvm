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

#ضمن "llvm/Support/raw_socket_stream.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/FileSystem.h"

#ضمن <atomic>
#ضمن <fcntl.h>
#ضمن <functional>

#که_نه_تعریف_شوی _WIN32
#ضمن <poll.h>
#ضمن <sys/socket.h>
#ضمن <sys/un.h>
#وإلا
#ضمن "llvm/Support/Windows/WindowsSupport.h"
// winsock2.h must be included before afunix.h. Briefly turn off clang-format to
// avoid error.
// clang-format off
#ضمن <winsock2.h>
#ضمن <afunix.h>
// clang-format on
#ضمن <io.h>
#د_که_پای // _WIN32

#إذا defined(HAVE_UNISTD_H)
#ضمن <unistd.h>
#د_که_پای

کارول نطاق llvm;

#که_تعریف_شوی _WIN32
WSABalancer::WSABalancer() {
  WSADATA Wsa_معلومات;
  ::memset(&Wsa_معلومات, 0, حجم(Wsa_معلومات));
  إذا (WSAStartup(MAKEWORD(2, 2), &Wsa_معلومات) != 0) {
    llvm::report_fatal_error("WSAStartup failed");
  }
}

WSABalancer::~WSABalancer() { WSACleanup(); }
#د_که_پای // _WIN32

ساكن std::error_کوډ get_وروستی_Socket_Error_کوډ() {
#که_تعریف_شوی _WIN32
  أرجع std::error_کوډ(::WSAGet_وروستی_Error(), std::system_category());
#وإلا
  أرجع errno_As_Error_کوډ();
#د_که_پای
}

#که_تعریف_شوی _WIN32
کارول NativeSocket = SOCKET;
#وإلا
کارول NativeSocket = صحيح;
#تعریف_کړه INVALID_SOCKET -1
#د_که_پای

ساكن صحيح closeSocket(NativeSocket Socket) {
#که_تعریف_شوی _WIN32
  أرجع ::closesocket(Socket);
#وإلا
  أرجع ::close(Socket);
#د_که_پای
}

ساكن Expected<sockaddr_un> سېټ_Socket_Addr(StringRef Socket_لار) {
  بنية sockaddr_un Addr;
  memset(&Addr, 0, حجم(Addr));
  Addr.sun_family = AF_UNIX;

  إذا (حجم(sockaddr_un::sun_لار) <= Socket_لار.اندازه_2())
    أرجع make_error<StringError>(
        std::make_error_کوډ(std::errc::filename_too_long),
        "Socket path exceeds sockaddr_un::sun_path size limit");

  strncpy(Addr.sun_لار, Socket_لار.str().c_str(), حجم(Addr.sun_لار) - 1);
  أرجع Addr;
}

ساكن Expected<صحيح> getSocketFD(StringRef Socket_لار) {
  NativeSocket Socket = socket(AF_UNIX, SOCK_بهیر, 0);
  إذا (Socket == INVALID_SOCKET) {
    أرجع llvm::make_error<StringError>(get_وروستی_Socket_Error_کوډ(),
                                         "Create socket failed");
  }

#که_تعریف_شوی __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, عدم, 0);
#د_که_پای
  Expected<بنية sockaddr_un> Addr = سېټ_Socket_Addr(Socket_لار);
  إذا (!Addr) {
    closeSocket(Socket);
    أرجع Addr.takeError();
  }

  إذا (::connect(Socket, (بنية sockaddr *)&*Addr, حجم(*Addr)) == -1) {
    // Grab the error code before closing, which may overwrite it.
    std::error_کوډ EC = get_وروستی_Socket_Error_کوډ();
    closeSocket(Socket);
    أرجع llvm::make_error<StringError>(EC, "Connect socket failed");
  }

#که_تعریف_شوی _WIN32
  أرجع _open_osfhandle(Socket, 0);
#وإلا
  أرجع Socket;
#د_که_پای // _WIN32
}

ListeningSocket::ListeningSocket(صحيح SocketFD, StringRef Socket_لار,
                                 صحيح PipeFD[2])
    : FD(SocketFD), Socket_لار(Socket_لار), PipeFD{PipeFD[0], PipeFD[1]} {}

ListeningSocket::ListeningSocket(ListeningSocket &&LS)
    : FD(LS.FD.بارول()), Socket_لار(LS.Socket_لار),
      PipeFD{LS.PipeFD[0], LS.PipeFD[1]} {

  LS.FD = -1;
  LS.Socket_لار.clear();
  LS.PipeFD[0] = -1;
  LS.PipeFD[1] = -1;
}

Expected<ListeningSocket> ListeningSocket::createUnix(StringRef Socket_لار,
                                                      صحيح MaxBacklog) {

  // Handle instances where the target socket address already exists and
  // differentiate between a preexisting file with and without a bound socket
  //
  // ::bind will return std::errc:address_in_use if a file at the socket address
  // already exists (e.g., the file was not properly unlinked due to a crash)
  // even if another socket has not yet binded to that address
  إذا (llvm::sys::fs::exists(Socket_لار)) {
    Expected<صحيح> MaybeFD = getSocketFD(Socket_لار);
    إذا (!MaybeFD) {

      // Regardless of the error, notify the caller that a file already exists
      // at the desired socket address and that there is no bound socket at that
      // address. The file must be removed before ::bind can use the address
      consumeError(MaybeFD.takeError());
      أرجع llvm::make_error<StringError>(
          std::make_error_کوډ(std::errc::file_exists),
          "Socket address unavailable");
    }
    ::close(std::move(*MaybeFD));

    // Notify caller that the provided socket address already has a bound socket
    أرجع llvm::make_error<StringError>(
        std::make_error_کوډ(std::errc::پته_دننه_use),
        "Socket address unavailable");
  }

#که_تعریف_شوی _WIN32
  WSABalancer _;
#د_که_پای
  NativeSocket Socket = socket(AF_UNIX, SOCK_بهیر, 0);
  إذا (Socket == INVALID_SOCKET)
    أرجع llvm::make_error<StringError>(get_وروستی_Socket_Error_کوډ(),
                                         "socket create failed");

#که_تعریف_شوی __CYGWIN__
  // On Cygwin, UNIX sockets involve a handshake between connect and accept
  // to enable SO_PEERCRED/getpeereid handling.  This necessitates accept being
  // called before connect can return, but at least the tests in
  // llvm/unittests/Support/raw_socket_stream_test do both on the same thread
  // (first connect and then accept), resulting in a deadlock.  This call turns
  // off the handshake (and SO_PEERCRED/getpeereid support).
  setsockopt(Socket, SOL_SOCKET, SO_PEERCRED, عدم, 0);
#د_که_پای
  Expected<بنية sockaddr_un> Addr = سېټ_Socket_Addr(Socket_لار);
  إذا (!Addr) {
    closeSocket(Socket);
    أرجع Addr.takeError();
  }

  إذا (::bind(Socket, (بنية sockaddr *)&*Addr, حجم(*Addr)) == -1) {
    // Grab error code from call to ::bind before closing the socket
    std::error_کوډ EC = get_وروستی_Socket_Error_کوډ();
    closeSocket(Socket);
    أرجع llvm::make_error<StringError>(EC, "Bind error");
  }

  // Mark socket as passive so incoming connections can be accepted
  إذا (::listen(Socket, MaxBacklog) == -1)
    أرجع llvm::make_error<StringError>(get_وروستی_Socket_Error_کوډ(),
                                         "Listen error");

  صحيح PipeFD[2];
#که_تعریف_شوی _WIN32
  // Reserve 1 byte for the pipe and use default textmode
  إذا (::_pipe(PipeFD, 1, 0) == -1)
#وإلا
  إذا (::أنبوب(PipeFD) == -1)
#د_که_پای // _WIN32
    أرجع llvm::make_error<StringError>(get_وروستی_Socket_Error_کوډ(),
                                         "pipe failed");

#که_تعریف_شوی _WIN32
  أرجع ListeningSocket{_open_osfhandle(Socket, 0), Socket_لار, PipeFD};
#وإلا
  أرجع ListeningSocket{Socket, Socket_لار, PipeFD};
#د_که_پای // _WIN32
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
ساكن std::error_کوډ
manageTimeout(ثابت std::chrono::milliseconds &Timeout,
              ثابت std::function<صحيح()> &get_فعال_FD,
              ثابت std::optional<صحيح> &CancelFD = std::nullopt) {
  بنية pollfd FD[2];
  FD[0].events = POLLIN;
#که_تعریف_شوی _WIN32
  SOCKET WinServerSock = _get_osfhandle(get_فعال_FD());
  FD[0].fd = WinServerSock;
#وإلا
  FD[0].fd = get_فعال_FD();
#د_که_پای
  uint8_t FDCount = 1;
  إذا (CancelFD.has_ارزښت_2()) {
    FD[1].events = POLLIN;
    FD[1].fd = CancelFD.ارزښت_2();
    FDCount++;
  }

  // Keep track of how much time has passed in case ::poll or WSAPoll are
  // interupted by a signal and need to be recalled
  خپلکاری پیل = std::chrono::steady_clock::now();
  خپلکاری پاتې_شوی_Timeout = Timeout;
  صحيح Poll_حالت = 0;
  افعل {
    // If Timeout is -1 then poll should block and RemainingTimeout does not
    // need to be recalculated
    إذا (Poll_حالت != 0 && Timeout != std::chrono::milliseconds(-1)) {
      خپلکاری ټول_Elapsed_وخت =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - پیل);

      إذا (ټول_Elapsed_وخت >= Timeout)
        أرجع std::make_error_کوډ(std::errc::operation_would_بلاک);

      پاتې_شوی_Timeout = Timeout - ټول_Elapsed_وخت;
    }
#که_تعریف_شوی _WIN32
    Poll_حالت = WSAPoll(FD, FDCount, پاتې_شوی_Timeout.شمېر());
  } طالما (Poll_حالت == SOCKET_ERROR &&
           get_وروستی_Socket_Error_کوډ() == std::errc::interrupted);
#وإلا
    Poll_حالت = ::poll(FD, FDCount, پاتې_شوی_Timeout.شمېر());
  } طالما (Poll_حالت == -1 &&
           get_وروستی_Socket_Error_کوډ() == std::errc::interrupted);
#د_که_پای

  // If ActiveFD equals -1 or CancelFD has data to be read then the operation
  // has been canceled by another thread
  إذا (get_فعال_FD() == -1 || (CancelFD.has_ارزښت_2() && FD[1].revents & POLLIN))
    أرجع std::make_error_کوډ(std::errc::operation_canceled);
#که_تعریف_شوی _WIN32
  إذا (Poll_حالت == SOCKET_ERROR)
#وإلا
  إذا (Poll_حالت == -1)
#د_که_پای
    أرجع get_وروستی_Socket_Error_کوډ();
  إذا (Poll_حالت == 0)
    أرجع std::make_error_کوډ(std::errc::timed_بهر);
  إذا (FD[0].revents & POLLNVAL)
    أرجع std::make_error_کوډ(std::errc::bad_file_descriptor);
  أرجع std::error_کوډ();
}

Expected<std::unique_ptr<raw_socket_بهیر>>
ListeningSocket::accept(ثابت std::chrono::milliseconds &Timeout) {
  خپلکاری get_فعال_FD = [هذا]() -> صحيح { أرجع FD; };
  std::error_کوډ TimeoutErr = manageTimeout(Timeout, get_فعال_FD, PipeFD[0]);
  إذا (TimeoutErr)
    أرجع llvm::make_error<StringError>(TimeoutErr, "Timeout error");

  صحيح AcceptFD;
#که_تعریف_شوی _WIN32
  SOCKET WinAcceptSock = ::accept(_get_osfhandle(FD), عدم, عدم);
  AcceptFD = _open_osfhandle(WinAcceptSock, 0);
#وإلا
  AcceptFD = ::accept(FD, عدم, عدم);
#د_که_پای

  إذا (AcceptFD == -1)
    أرجع llvm::make_error<StringError>(get_وروستی_Socket_Error_کوډ(),
                                         "Socket accept failed");
  أرجع std::make_unique<raw_socket_بهیر>(AcceptFD);
}

فراغ ListeningSocket::shutdown() {
  صحيح ObservedFD = FD.بارول();

  إذا (ObservedFD == -1)
    أرجع;

  // If FD equals ObservedFD set FD to -1; If FD doesn't equal ObservedFD then
  // another thread is responsible for shutdown so return
  إذا (!FD.compare_exchange_strong(ObservedFD, -1))
    أرجع;

  ::close(ObservedFD);
  ::unlink(Socket_لار.c_str());

  // Ensure ::poll returns if shutdown is called by a separate thread
  حرف Byte = 'A';
  ssize_t written = ::اكتب(PipeFD[1], &Byte, 1);

  // Ignore any write() error
  (فراغ)written;
}

ListeningSocket::~ListeningSocket() {
  shutdown();

  // Close the pipe's FDs in the destructor instead of within
  // ListeningSocket::shutdown to avoid unnecessary synchronization issues that
  // would occur as PipeFD's values would have to be changed to -1
  //
  // The move constructor sets PipeFD to -1
  إذا (PipeFD[0] != -1)
    ::close(PipeFD[0]);
  إذا (PipeFD[1] != -1)
    ::close(PipeFD[1]);
}

//===----------------------------------------------------------------------===//
//  raw_socket_stream
//===----------------------------------------------------------------------===//

raw_socket_بهیر::raw_socket_بهیر(صحيح SocketFD)
    : raw_fd_بهیر(SocketFD, ریښتیا) {}

raw_socket_بهیر::~raw_socket_بهیر() = افتراضي;

Expected<std::unique_ptr<raw_socket_بهیر>>
raw_socket_بهیر::createConnectedUnix(StringRef Socket_لار) {
#که_تعریف_شوی _WIN32
  WSABalancer _;
#د_که_پای // _WIN32
  Expected<صحيح> FD = getSocketFD(Socket_لار);
  إذا (!FD)
    أرجع FD.takeError();
  أرجع std::make_unique<raw_socket_بهیر>(*FD);
}

ssize_t raw_socket_بهیر::اقرأ(حرف *Ptr, size_t اندازه,
                                ثابت std::chrono::milliseconds &Timeout) {
  خپلکاری get_فعال_FD = [هذا]() -> صحيح { أرجع هذا->get_fd(); };
  std::error_کوډ Err = manageTimeout(Timeout, get_فعال_FD);
  // Mimic raw_fd_stream::read error handling behavior
  إذا (Err) {
    raw_fd_بهیر::error_detected(Err);
    أرجع -1;
  }
  أرجع raw_fd_بهیر::اقرأ(Ptr, اندازه);
}
