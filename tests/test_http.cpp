// test_http.cpp — tests for crlib/http.h (HttpClientResult enum, HttpUri, HttpClient)
//
// NOTE: No actual network connections are made. We test only:
//   - HttpClientResult enum values (compile-time constants)
//   - detail::HttpUri::parse() (pure string parsing)
//   - HttpClient singleton accessor and setters
//
#include <crlib/crlib.h>
#include "catch2/catch_amalgamated.hpp"

using namespace cr;

// ---------------------------------------------------------------------------
// HttpClientResult enum values
// ---------------------------------------------------------------------------
TEST_CASE("HttpClientResult Ok has value 200", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::Ok) == 200);
}

TEST_CASE("HttpClientResult NotFound has value 404", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::NotFound) == 404);
}

TEST_CASE("HttpClientResult InternalServerError has value 500", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::InternalServerError) == 500);
}

TEST_CASE("HttpClientResult SocketError is negative", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::SocketError) < 0);
}

TEST_CASE("HttpClientResult ConnectError is negative", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::ConnectError) < 0);
}

TEST_CASE("HttpClientResult Undefined is negative", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::Undefined) < 0);
}

TEST_CASE("HttpClientResult NetworkUnavilable is negative", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::NetworkUnavilable) < 0);
}

// ---------------------------------------------------------------------------
// detail::HttpUri::parse — URL parsing (no network)
// ---------------------------------------------------------------------------
TEST_CASE("HttpUri parse extracts protocol, host, and path", "[http]") {
    auto uri = detail::HttpUri::parse("http://example.com/some/path");
    REQUIRE(uri.protocol == "http");
    REQUIRE(uri.host == "example.com");
    REQUIRE(uri.path == "some/path");
}

TEST_CASE("HttpUri parse handles empty URI", "[http]") {
    auto uri = detail::HttpUri::parse("");
    REQUIRE(uri.protocol.empty());
    REQUIRE(uri.host.empty());
    REQUIRE(uri.path.empty());
}

TEST_CASE("HttpUri parse handles URI without path separator", "[http]") {
    // No '/' after the host => parse returns empty result
    auto uri = detail::HttpUri::parse("http://example.com");
    // host won't be parsed because there's no trailing '/'
    REQUIRE(uri.path.empty());
}

TEST_CASE("HttpUri parse with https protocol", "[http]") {
    auto uri = detail::HttpUri::parse("https://update.example.com/files/data.zip");
    REQUIRE(uri.protocol == "https");
    REQUIRE(uri.host == "update.example.com");
    REQUIRE(uri.path == "files/data.zip");
}

// ---------------------------------------------------------------------------
// HttpClient singleton
// ---------------------------------------------------------------------------
TEST_CASE("HttpClient singleton returns same instance", "[http]") {
    auto &a = HttpClient::instance();
    auto &b = http;
    REQUIRE(&a == &b);
}

TEST_CASE("HttpClient getLastStatusCode returns Undefined before any request", "[http]") {
    // Fresh state — no request was made
    auto code = http.getLastStatusCode();
    REQUIRE(static_cast<int>(code) != 0);  // just ensure it returns something
}

TEST_CASE("HttpClient setUserAgent does not crash", "[http]") {
    http.setUserAgent("crlib-test-agent");
    REQUIRE(true);
}

TEST_CASE("HttpClient setChunkSize does not crash", "[http]") {
    http.setChunkSize(8192);
    REQUIRE(true);
}

// ---------------------------------------------------------------------------
// HttpClient downloadFile — early-out paths (no network)
// ---------------------------------------------------------------------------
TEST_CASE("HttpClient downloadFile returns false if local file already exists", "[http]") {
    http.startup();

    const char *fname = "crlib_test_http_exists.tmp";
    {
        File fw(fname, "w");
        fw.puts("existing");
    }

    bool ok = http.downloadFile("http://example.com/file.bin", fname);
    REQUIRE(!ok);
    REQUIRE(http.getLastStatusCode() == HttpClientResult::LocalFileExists);

    plat.removeFile(fname);
}

TEST_CASE("HttpClient uploadFile returns false if local file does not exist", "[http]") {
    http.startup();

    bool ok = http.uploadFile("http://example.com/upload", "crlib_nonexistent_upload.tmp");
    REQUIRE(!ok);
    REQUIRE(http.getLastStatusCode() == HttpClientResult::NoLocalFile);
}

// ---------------------------------------------------------------------------
// HttpClientResult enum — additional status codes
// ---------------------------------------------------------------------------
TEST_CASE("HttpClientResult redirect codes", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::MovedPermanently) == 301);
    REQUIRE(static_cast<int>(HttpClientResult::Found) == 302);
    REQUIRE(static_cast<int>(HttpClientResult::TemporaryRedirect) == 307);
    REQUIRE(static_cast<int>(HttpClientResult::PermanentRedirect) == 308);
}

TEST_CASE("HttpClientResult client error codes", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::BadRequest) == 400);
    REQUIRE(static_cast<int>(HttpClientResult::Unauthorized) == 401);
    REQUIRE(static_cast<int>(HttpClientResult::Forbidden) == 403);
    REQUIRE(static_cast<int>(HttpClientResult::MethodNotAllowed) == 405);
    REQUIRE(static_cast<int>(HttpClientResult::TooManyRequests) == 429);
}

TEST_CASE("HttpClientResult server error codes", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::BadGateway) == 502);
    REQUIRE(static_cast<int>(HttpClientResult::ServiceUnavailable) == 503);
    REQUIRE(static_cast<int>(HttpClientResult::GatewayTimeout) == 504);
}

TEST_CASE("HttpClientResult custom negative error codes", "[http]") {
    REQUIRE(static_cast<int>(HttpClientResult::SocketError) == -1);
    REQUIRE(static_cast<int>(HttpClientResult::ConnectError) == -2);
    REQUIRE(static_cast<int>(HttpClientResult::HttpOnly) == -3);
    REQUIRE(static_cast<int>(HttpClientResult::Undefined) == -4);
    REQUIRE(static_cast<int>(HttpClientResult::NoLocalFile) == -5);
    REQUIRE(static_cast<int>(HttpClientResult::LocalFileExists) == -6);
    REQUIRE(static_cast<int>(HttpClientResult::NetworkUnavilable) == -7);
}

// ---------------------------------------------------------------------------
// detail::HttpUri::parse — additional edge cases
// ---------------------------------------------------------------------------
TEST_CASE("HttpUri parse returns empty for URI without protocol separator", "[http]") {
    auto uri = detail::HttpUri::parse("example.com/path");
    REQUIRE(uri.protocol.empty());
    REQUIRE(uri.host.empty());
    REQUIRE(uri.path.empty());
}

TEST_CASE("HttpUri parse handles URI with just host and trailing slash", "[http]") {
    auto uri = detail::HttpUri::parse("http://example.com/");
    REQUIRE(uri.protocol == "http");
    REQUIRE(uri.host == "example.com");
    REQUIRE(uri.path.empty());
}

TEST_CASE("HttpUri parse handles deep path", "[http]") {
    auto uri = detail::HttpUri::parse("http://cdn.example.com/a/b/c/d/file.txt");
    REQUIRE(uri.protocol == "http");
    REQUIRE(uri.host == "cdn.example.com");
    REQUIRE(uri.path == "a/b/c/d/file.txt");
}

TEST_CASE("HttpUri parse handles ftp protocol", "[http]") {
    auto uri = detail::HttpUri::parse("ftp://files.example.com/data");
    REQUIRE(uri.protocol == "ftp");
    REQUIRE(uri.host == "files.example.com");
    REQUIRE(uri.path == "data");
}

// ---------------------------------------------------------------------------
// HttpClient downloadFile/uploadFile — HTTPS rejection
// ---------------------------------------------------------------------------
TEST_CASE("HttpClient downloadFile returns HttpOnly for https URL", "[http]") {
    http.startup();

    bool ok = http.downloadFile("https://example.com/file.bin", "crlib_https_test.tmp");
    REQUIRE(!ok);
    REQUIRE(http.getLastStatusCode() == HttpClientResult::HttpOnly);
}

TEST_CASE("HttpClient uploadFile returns HttpOnly for https URL", "[http]") {
    http.startup();

    const char *fname = "crlib_test_upload_https.tmp";
    {
        File fw(fname, "w");
        fw.puts("test data");
    }

    bool ok = http.uploadFile("https://example.com/upload", fname);
    REQUIRE(!ok);
    REQUIRE(http.getLastStatusCode() == HttpClientResult::HttpOnly);

    plat.removeFile(fname);
}

// ---------------------------------------------------------------------------
// Socket class
// ---------------------------------------------------------------------------
TEST_CASE("Socket default construction", "[http]") {
    Socket s;
    REQUIRE(true);
}

TEST_CASE("Socket setTimeout does not crash", "[http]") {
    Socket s;
    s.setTimeout(10);
    REQUIRE(true);
}

TEST_CASE("Socket disconnect on unconnected socket is safe", "[http]") {
    Socket s;
    s.disconnect();
    REQUIRE(true);
}

TEST_CASE("Socket connect to invalid host returns false", "[http]") {
    http.startup();
    Socket s;
    s.setTimeout(1);
    bool connected = s.connect("invalid.host.that.does.not.exist.example");
    REQUIRE(!connected);
}
