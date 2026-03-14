// SPDX-License-Identifier: Unlicense

#pragma once

#include <stdio.h>

#include <crlib/string.h>
#include <crlib/files.h>
#include <crlib/logger.h>
#include <crlib/twin.h>
#include <crlib/platform.h>
#include <crlib/uniqueptr.h>
#include <crlib/random.h>
#include <crlib/thread.h>

#if defined(CR_WINDOWS)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <sys/types.h>
#if !defined(CR_PSVITA)
#  include <sys/uio.h>
#endif
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <errno.h>
#  include <netdb.h>
#  include <fcntl.h>
#endif

// status codes for http client
CR_DECLARE_SCOPED_ENUM (HttpClientResult,
   Continue = 100,
   SwitchingProtocol = 101,
   Processing = 102,
   EarlyHints = 103,

   Ok = 200,
   Created = 201,
   Accepted = 202,
   NonAuthoritativeInformation = 203,
   NoContent = 204,
   ResetContent = 205,
   PartialContent = 206,
   MultiStatus = 207,
   AlreadyReported = 208,
   ImUsed = 226,

   MultipleChoice = 300,
   MovedPermanently = 301,
   Found = 302,
   SeeOther = 303,
   NotModified = 304,
   UseProxy = 305,
   TemporaryRedirect = 307,
   PermanentRedirect = 308,

   BadRequest = 400,
   Unauthorized = 401,
   PaymentRequired = 402,
   Forbidden = 403,
   NotFound = 404,
   MethodNotAllowed = 405,
   NotAcceptable = 406,
   ProxyAuthenticationRequired = 407,
   RequestTimeout = 408,
   Conflict = 409,
   Gone = 410,
   LengthRequired = 411,
   PreconditionFailed = 412,
   PayloadTooLarge = 413,
   UriTooLong = 414,
   UnsupportedMediaType = 415,
   RangeNotSatisfiable = 416,
   ExpectationFailed = 417,
   ImaTeapot = 418,
   MisdirectedRequest = 421,
   UnprocessableEntity = 422,
   Locked = 423,
   FailedDependency = 424,
   TooEarly = 425,
   UpgradeRequired = 426,
   PreconditionRequired = 428,
   TooManyRequests = 429,
   RequestHeaderFieldsTooLarge = 431,
   UnavailableForLegalReasons = 451,

   InternalServerError = 500,
   NotImplemented = 501,
   BadGateway = 502,
   ServiceUnavailable = 503,
   GatewayTimeout = 504,
   HttpVersionNotSupported = 505,
   VariantAlsoNegotiates = 506,
   InsufficientStorage = 507,
   LoopDetected = 508,
   NotExtended = 510,
   NetworkAuthenticationRequired = 511,

   SocketError = -1,
   ConnectError = -2,
   HttpOnly = -3,
   Undefined = -4,
   NoLocalFile = -5,
   LocalFileExists = -6,
   NetworkUnavilable = -7
)

CR_NAMESPACE_BEGIN

namespace detail {

   // simple http uri omitting query-string and port
   struct HttpUri {
      String path, protocol, host;

   public:
      static HttpUri parse (StringRef uri) {
         HttpUri result;

         if (uri.empty ()) {
            return result;
         }
         const size_t protocol = uri.find ("://");

         if (protocol == String::InvalidIndex) {
            return result;
         }
         result.protocol = uri.substr (0, protocol);

         const size_t hostStart = protocol + 3;
         const size_t pathSlash = uri.find ("/", hostStart);

         if (pathSlash != String::InvalidIndex) {
            result.host = uri.substr (hostStart, pathSlash - hostStart);
            result.path = uri.substr (pathSlash + 1);
         }
         else {
            result.host = uri.substr (hostStart);
         }
         return result;
      }
   };

   struct SocketInit {
      static void start () {
#if defined(CR_WINDOWS)
         WSADATA wsa;

         if (WSAStartup (MAKEWORD (2, 2), &wsa) != 0) {
            logger.error ("Unable to initialize sockets.");
         }
#endif
      }

      static void stop () {
#if defined(CR_WINDOWS)
         WSACleanup ();
#endif
      }
   };
}

class Socket final : public NonCopyable {
private:
#if defined(CR_WINDOWS)
   using SocketType = SOCKET;
#else
   using SocketType = int32_t;
#endif

private:
#if defined(CR_WINDOWS)
   static constexpr SocketType kInvalidSocket = INVALID_SOCKET;
#else
   static constexpr SocketType kInvalidSocket = -1;
#endif

private:
   SocketType socket_;
   uint32_t timeout_;

public:
   Socket () : socket_ (kInvalidSocket), timeout_ (2) {}

   ~Socket () {
      disconnect ();
   }

public:
   bool connect (StringRef hostname) {
      addrinfo  hints {}, *result = nullptr;
      plat.bzero (&hints, sizeof (hints));

      constexpr auto kNumericServ = 0x00000008;

      hints.ai_flags = kNumericServ;
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;

      if (getaddrinfo (hostname.chars (), "80", &hints, &result) != 0) {
         return false;
      }
      socket_ = socket (result->ai_family, result->ai_socktype, 0);

      if (socket_ == kInvalidSocket) {
         freeaddrinfo (result);
         return false;
      }

      // set timeouts
#if defined(CR_WINDOWS)
      DWORD tv = timeout_ * 1000;
#else
      timeval tv { static_cast <time_t> (timeout_), 0 };
#endif

      setsockopt (socket_, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast <char *> (&tv), static_cast <int32_t> (sizeof (tv)));
      setsockopt (socket_, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast <char *> (&tv), static_cast <int32_t> (sizeof (tv)));

      if (::connect (socket_, result->ai_addr, static_cast <int32_t> (result->ai_addrlen)) == -1) {
         disconnect ();
         freeaddrinfo (result);

         return false;
      }
      freeaddrinfo (result);

      return true;
   }

   void setTimeout (uint32_t timeout) {
      timeout_ = timeout;
   }

   void disconnect () {
      if (socket_ == kInvalidSocket) {
         return;
      }
#if defined(CR_WINDOWS)
      closesocket (socket_);
#else
      close (socket_);
#endif
      socket_ = kInvalidSocket;
   }

public:
   template <typename U> int32_t send (const U *buffer, int32_t length) const {
      return ::send (socket_, reinterpret_cast <const char *> (buffer), length, 0);
   }

   template <typename U> int32_t recv (U *buffer, int32_t length) {
      return ::recv (socket_, reinterpret_cast <char *> (buffer), length, 0);
   }

public:
   static int32_t CR_STDCALL sendto (int socket, const void *message, size_t length, int flags, const struct sockaddr *dest, int32_t destLength) {
#if defined(CR_WINDOWS)
      WSABUF buffer = { static_cast <ULONG> (length), const_cast <char *> (reinterpret_cast <const char *> (message)) };
      DWORD sendLength = 0;

      if (WSASendTo (socket, &buffer, 1, &sendLength, flags, dest, destLength, NULL, NULL) == SOCKET_ERROR) {
         errno = WSAGetLastError ();
         return -1;
      }
      return static_cast <int32_t> (sendLength);
#else
      iovec iov = { const_cast <void *> (message), length };
      msghdr msg {};

      msg.msg_name = reinterpret_cast <void *> (const_cast <struct sockaddr *> (dest));
      msg.msg_namelen = destLength;
      msg.msg_iov = &iov;
      msg.msg_iovlen = 1;

      return sendmsg (socket, &msg, flags);
#endif
   }
};

// simple http client for downloading/uploading files only
class HttpClient final : public Singleton <HttpClient> {
private:
   enum : int32_t {
      MaxReceiveErrors = 12,
      DefaultSocketTimeout = 5
   };

private:
   String userAgent_ = "crlib";
   HttpClientResult statusCode_ = HttpClientResult::Undefined;
   int32_t chunkSize_ = 4096;

   bool initialized_ {};
   bool hasConnection_ {};
   Mutex connectionMutex_;

public:
   HttpClient () = default;
   ~HttpClient () {
      detail::SocketInit::stop ();
   }

private:
   bool isHttps (StringRef protocol) {
      return protocol == "https" || protocol == "HTTPS";
   }

   bool checkConnection () {
      MutexScopedLock lock (connectionMutex_);
      return hasConnection_;
   }

   HttpClientResult parseResponseHeader (Socket *socket, uint8_t *buffer) {
      int32_t pos = 0, symbols = 0, errors = 0;

      // parse response header byte-by-byte until we see a blank line (\r\n\r\n)
      while (pos < chunkSize_) {
         if (socket->recv (&buffer[pos], 1) < 1) {
            if (++errors > MaxReceiveErrors) {
               break;
            }
            continue;
         }

         switch (buffer[pos]) {
         case '\r':
            break;

         case '\n':
            if (symbols == 0) {
               ++pos;
               goto done;
            }
            symbols = 0;
            break;

         default:
            ++symbols;
            break;
         }
         ++pos;
      }
   done:
      String response { reinterpret_cast <const char *> (buffer), static_cast <size_t> (pos) };
      const size_t responseCodeStart = response.find ("HTTP/1.");

      if (responseCodeStart != String::InvalidIndex) {
         String respCode = response.substr (responseCodeStart + cr::bufsize ("HTTP/1.1 "), 3);
         respCode.trim ();

         return static_cast <HttpClientResult> (respCode.as <int> ());
      }
      return HttpClientResult::NotFound;
   }

   String buildRequest (StringRef method, const detail::HttpUri &uri, size_t contentLength = 0, StringRef contentType = "", StringRef extraHeaders = "") {
      String request;

      request.appendf ("%s /%s HTTP/1.1\r\n", method.chars (), uri.path);
      request.appendf ("Host: %s\r\n", uri.host);
      request.appendf ("User-Agent: %s\r\n", userAgent_);
      request.append ("Accept: */*\r\n");
      request.append ("Connection: close\r\n");

      if (contentLength > 0) {
         request.appendf ("Content-Length: %zu\r\n", contentLength);
      }
      if (!contentType.empty ()) {
         request.appendf ("Content-Type: %s\r\n", contentType.chars ());
      }
      if (!extraHeaders.empty ()) {
         request.append (extraHeaders);
      }
      request.append ("\r\n");

      return request;
   }

public:
   void startup (StringRef hostCheck = "", StringRef errMessageIfHostDown = "", uint32_t timeout = DefaultSocketTimeout) {
      detail::SocketInit::start ();

      initialized_ = true;

      if (hostCheck.empty ()) {
         MutexScopedLock lock (connectionMutex_);
         hasConnection_ = true;
         return;
      }
      {
         MutexScopedLock lock (connectionMutex_);
         hasConnection_ = false;
      }
      String hostCopy { hostCheck };
      String errCopy { errMessageIfHostDown };

      Thread hostCheckThread { [this, hostCopy, errCopy, timeout] () {
         auto socket = cr::makeUnique <Socket> ();
         socket->setTimeout (timeout);

         const bool connected = socket->connect (hostCopy);

         MutexScopedLock lock (connectionMutex_);
         hasConnection_ = connected;

         if (!connected && !errCopy.empty ()) {
            logger.message (errCopy.chars ());
         }
      } };
      hostCheckThread.detach ();
   }

   // simple blocked download
   bool downloadFile (StringRef url, StringRef localPath, int32_t timeout = DefaultSocketTimeout) {
      if (plat.win && !initialized_) {
         plat.abort ("Sockets not initialized.");
      }

      if (!checkConnection ()) {
         statusCode_ = HttpClientResult::NetworkUnavilable;
         return false;
      }

      if (plat.fileExists (localPath.chars ())) {
         statusCode_ = HttpClientResult::LocalFileExists;
         return false;
      }
      auto uri = detail::HttpUri::parse (url);

      if (isHttps (uri.protocol)) {
         statusCode_ = HttpClientResult::HttpOnly;
         return false;
      }
      auto socket = cr::makeUnique <Socket> ();
      socket->setTimeout (timeout);

      if (!socket->connect (uri.host)) {
         statusCode_ = HttpClientResult::ConnectError;
         return false;
      }

      String request = buildRequest ("GET", uri);

      if (socket->send (request.chars (), static_cast <int32_t> (request.length ())) < 1) {
         statusCode_ = HttpClientResult::SocketError;
         return false;
      }
      SmallArray <uint8_t> buffer (chunkSize_);
      statusCode_ = parseResponseHeader (socket.get (), buffer.data ());

      if (statusCode_ != HttpClientResult::Ok) {
         return false;
      }

      // receive the file
      File file (localPath, "wb");

      if (!file) {
         statusCode_ = HttpClientResult::Undefined;
         return false;
      }
      int32_t length = 0;
      int32_t errors = 0;

      for (;;) {
         length = socket->recv (buffer.data (), chunkSize_);

         if (length > 0) {
            file.write (buffer.data (), length);
         }
         else if (++errors > MaxReceiveErrors) {
            break;
         }
      }
      statusCode_ = HttpClientResult::Ok;

      return true;
   }

   bool uploadFile (StringRef url, StringRef localPath, const int32_t timeout = DefaultSocketTimeout) {
      if (plat.win && !initialized_) {
         plat.abort ("Sockets not initialized.");
      }

      if (!checkConnection ()) {
         statusCode_ = HttpClientResult::NetworkUnavilable;
         return false;
      }

      if (!plat.fileExists (localPath.chars ())) {
         statusCode_ = HttpClientResult::NoLocalFile;
         return false;
      }
      auto uri = detail::HttpUri::parse (url);

      if (isHttps (uri.protocol)) {
         statusCode_ = HttpClientResult::HttpOnly;
         return false;
      }
      auto socket = cr::makeUnique <Socket> ();
      socket->setTimeout (timeout);

      if (!socket->connect (uri.host)) {
         statusCode_ = HttpClientResult::ConnectError;
         return false;
      }

      File file (localPath, "rb");

      if (!file) {
         statusCode_ = HttpClientResult::Undefined;
         return false;
      }
      String boundaryName = localPath;
      const size_t boundarySlash = localPath.findLastOf ("\\/");

      if (boundarySlash != String::InvalidIndex) {
         boundaryName = localPath.substr (boundarySlash + 1);
      }
      StringRef boundaryLine = strings.format ("---crlib_upload_boundary_%d%d%d%d", rg (0, 9), rg (0, 9), rg (0, 9), rg (0, 9));

      String start, end;
      start.appendf ("--%s\r\n", boundaryLine);
      start.appendf ("Content-Disposition: form-data; name='file'; filename='%s'\r\n", boundaryName);
      start.append ("Content-Type: application/octet-stream\r\n\r\n");

      end.appendf ("\r\n--%s--\r\n\r\n", boundaryLine);

      const auto contentLength = static_cast <size_t> (file.length ()) + start.length () + end.length ();
      StringRef contentType = strings.format ("multipart/form-data; boundary=%s", boundaryLine);
      String request = buildRequest ("POST", uri, contentLength, contentType);

      if (socket->send (request.chars (), static_cast <int32_t> (request.length ())) < 1) {
         statusCode_ = HttpClientResult::SocketError;
         return false;
      }

      if (socket->send (start.chars (), static_cast <int32_t> (start.length ())) < 1) {
         statusCode_ = HttpClientResult::SocketError;
         return false;
      }
      SmallArray <uint8_t> buffer (chunkSize_);
      int32_t length = 0;

      for (;;) {
         length = static_cast <int32_t> (file.read (buffer.data (), 1, chunkSize_));

         if (length > 0) {
            socket->send (buffer.data (), length);
         }
         else {
            break;
         }
      }

      if (socket->send (end.chars (), static_cast <int32_t> (end.length ())) < 1) {
         statusCode_ = HttpClientResult::SocketError;
         return false;
      }
      statusCode_ = parseResponseHeader (socket.get (), buffer.data ());
      return statusCode_ == HttpClientResult::Ok;
   }

public:
   void setUserAgent (StringRef ua) {
      userAgent_ = ua;
   }

   HttpClientResult getLastStatusCode () {
      return statusCode_;
   }

   void setChunkSize (int32_t chunkSize) {
      chunkSize_ = chunkSize;
   }
};

// expose global http client
CR_EXPOSE_GLOBAL_SINGLETON (HttpClient, http);

CR_NAMESPACE_END
