#include "tgbotcpp/net/OpenSslHttpClient.hpp"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include <cctype>
#include <cstring>
#include <stdexcept>
#include <string>

#include "net/Url.hpp"

namespace tgbotcpp::net {

// ---------------------------------------------------------------------------
// URL parsing
// ---------------------------------------------------------------------------

ParsedUrl parseUrl(const std::string& url) {
    auto schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        throw std::runtime_error("parseUrl: missing scheme in '" + url + "'");
    }
    std::string scheme = url.substr(0, schemeEnd);
    std::string defaultPort;
    if (scheme == "https") {
        defaultPort = "443";
    } else if (scheme == "http") {
        defaultPort = "80";
    } else {
        throw std::runtime_error("parseUrl: unsupported scheme '" + scheme + "'");
    }

    std::size_t authorityStart = schemeEnd + 3;
    std::size_t pathStart = url.find('/', authorityStart);
    std::string authority = url.substr(authorityStart, pathStart == std::string::npos
                                                           ? std::string::npos
                                                           : pathStart - authorityStart);
    if (authority.empty()) {
        throw std::runtime_error("parseUrl: empty host in '" + url + "'");
    }

    ParsedUrl parsed;
    auto colon = authority.find(':');
    if (colon == std::string::npos) {
        parsed.host = authority;
        parsed.port = defaultPort;
    } else {
        parsed.host = authority.substr(0, colon);
        parsed.port = authority.substr(colon + 1);
        if (parsed.host.empty() || parsed.port.empty()) {
            throw std::runtime_error("parseUrl: malformed host:port in '" + url + "'");
        }
    }

    parsed.target = pathStart == std::string::npos ? "/" : url.substr(pathStart);
    return parsed;
}

namespace {

// ---------------------------------------------------------------------------
// Small RAII / error helpers
// ---------------------------------------------------------------------------

// Drains and formats the OpenSSL error queue.
std::string opensslError(const std::string& context) {
    std::string message = context;
    unsigned long code;
    while ((code = ERR_get_error()) != 0) {
        char buf[256];
        ERR_error_string_n(code, buf, sizeof(buf));
        message += ": ";
        message += buf;
    }
    return message;
}

// Owns a connected socket fd, closing it on scope exit.
class SocketGuard {
public:
    explicit SocketGuard(int fd = -1) : fd_(fd) {}
    ~SocketGuard() { reset(); }
    SocketGuard(const SocketGuard&) = delete;
    SocketGuard& operator=(const SocketGuard&) = delete;

    int get() const { return fd_; }
    void reset(int fd = -1) {
        if (fd_ >= 0) {
            ::close(fd_);
        }
        fd_ = fd;
    }
    int release() {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

private:
    int fd_;
};

// Opens a TCP connection to host:port, trying each resolved address.
int connectTcp(const std::string& host, const std::string& port) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    int rc = ::getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (rc != 0) {
        throw std::runtime_error("DNS resolution failed for " + host + ": " +
                                 gai_strerror(rc));
    }

    SocketGuard sock;
    for (addrinfo* ai = result; ai != nullptr; ai = ai->ai_next) {
        int fd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) {
            continue;
        }
        if (::connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) {
            sock.reset(fd);
            break;
        }
        ::close(fd);
    }
    ::freeaddrinfo(result);

    if (sock.get() < 0) {
        throw std::runtime_error("Could not connect to " + host + ":" + port);
    }
    return sock.release();
}

std::string headerValue(const std::string& headers, const std::string& name) {
    std::string lowerHeaders;
    lowerHeaders.reserve(headers.size());
    for (char c : headers) {
        lowerHeaders.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    std::string needle = "\r\n" + name + ":";  // headers are joined to a leading CRLF below
    auto pos = lowerHeaders.find(needle);
    if (pos == std::string::npos) {
        return {};
    }
    pos += needle.size();
    auto end = headers.find("\r\n", pos);
    std::string value = headers.substr(pos, end - pos);
    // Trim surrounding whitespace.
    auto start = value.find_first_not_of(" \t");
    auto last = value.find_last_not_of(" \t");
    if (start == std::string::npos) {
        return {};
    }
    return value.substr(start, last - start + 1);
}

// Decodes an HTTP/1.1 chunked transfer-encoded body.
std::string dechunk(const std::string& body) {
    std::string out;
    std::size_t pos = 0;
    while (pos < body.size()) {
        auto lineEnd = body.find("\r\n", pos);
        if (lineEnd == std::string::npos) {
            break;
        }
        std::string sizeHex = body.substr(pos, lineEnd - pos);
        // A chunk-size may carry extensions after ';'.
        auto semicolon = sizeHex.find(';');
        if (semicolon != std::string::npos) {
            sizeHex = sizeHex.substr(0, semicolon);
        }
        std::size_t chunkSize = 0;
        try {
            chunkSize = static_cast<std::size_t>(std::stoul(sizeHex, nullptr, 16));
        } catch (const std::exception&) {
            throw std::runtime_error("HTTP: invalid chunk size");
        }
        if (chunkSize == 0) {
            break;
        }
        std::size_t dataStart = lineEnd + 2;
        if (dataStart + chunkSize > body.size()) {
            throw std::runtime_error("HTTP: truncated chunk");
        }
        out.append(body, dataStart, chunkSize);
        pos = dataStart + chunkSize + 2;  
    }
    return out;
}

std::string extractBody(const std::string& response) {
    auto headerEnd = response.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        throw std::runtime_error("HTTP: malformed response (no header terminator)");
    }
    // Prepend CRLF so headerValue's "\r\n<name>:" needle also matches the first
    // header line.
    std::string headers = "\r\n" + response.substr(0, headerEnd);
    std::string body = response.substr(headerEnd + 4);

    std::string transferEncoding = headerValue(headers, "transfer-encoding");
    for (char& c : transferEncoding) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    if (transferEncoding.find("chunked") != std::string::npos) {
        return dechunk(body);
    }

    std::string contentLength = headerValue(headers, "content-length");
    if (!contentLength.empty()) {
        std::size_t length = 0;
        try {
            length = static_cast<std::size_t>(std::stoul(contentLength));
        } catch (const std::exception&) {
            throw std::runtime_error("HTTP: invalid Content-Length");
        }
        return body.substr(0, length);
    }

    return body; 
}

}  // namespace

// ---------------------------------------------------------------------------
// OpenSslHttpClient
// ---------------------------------------------------------------------------

struct OpenSslHttpClient::Impl {
    SSL_CTX* ctx = nullptr;

    Impl() {
        ctx = SSL_CTX_new(TLS_client_method());
        if (ctx == nullptr) {
            throw std::runtime_error(opensslError("SSL_CTX_new failed"));
        }
        SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
        if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
            SSL_CTX_free(ctx);
            throw std::runtime_error(opensslError("loading default CA paths failed"));
        }
    }

    ~Impl() {
        if (ctx != nullptr) {
            SSL_CTX_free(ctx);
        }
    }

    std::string request(const std::string& method, const std::string& url,
                        const std::string& body, const std::string& contentType) {
        ParsedUrl parsed = parseUrl(url);

        SocketGuard sock(connectTcp(parsed.host, parsed.port));

        SSL* ssl = SSL_new(ctx);
        if (ssl == nullptr) {
            throw std::runtime_error(opensslError("SSL_new failed"));
        }
        // RAII for the SSL handle.
        struct SslGuard {
            SSL* ssl;
            ~SslGuard() {
                if (ssl != nullptr) {
                    SSL_free(ssl);
                }
            }
        } sslGuard{ssl};

        SSL_set_fd(ssl, sock.get());
        SSL_set_tlsext_host_name(ssl, parsed.host.c_str());  // SNI
        SSL_set1_host(ssl, parsed.host.c_str());             // hostname verification
        SSL_set_verify(ssl, SSL_VERIFY_PEER, nullptr);

        if (SSL_connect(ssl) != 1) {
            throw std::runtime_error(opensslError("TLS handshake with " + parsed.host +
                                                  " failed"));
        }
        if (SSL_get_verify_result(ssl) != X509_V_OK) {
            throw std::runtime_error("Certificate verification failed for " + parsed.host);
        }

        std::string requestText = method + " " + parsed.target + " HTTP/1.1\r\n";
        requestText += "Host: " + parsed.host + "\r\n";
        requestText += "User-Agent: tgbotcpp/0.1\r\n";
        requestText += "Accept: */*\r\n";
        requestText += "Connection: close\r\n";
        if (method == "POST") {
            requestText += "Content-Type: " + contentType + "\r\n";
            requestText += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        }
        requestText += "\r\n";
        requestText += body;

        writeAll(ssl, requestText);
        std::string response = readAll(ssl);
        return extractBody(response);
    }

private:
    static void writeAll(SSL* ssl, const std::string& data) {
        std::size_t sent = 0;
        while (sent < data.size()) {
            int n = SSL_write(ssl, data.data() + sent, static_cast<int>(data.size() - sent));
            if (n <= 0) {
                throw std::runtime_error(opensslError("SSL_write failed"));
            }
            sent += static_cast<std::size_t>(n);
        }
    }

    static std::string readAll(SSL* ssl) {
        std::string response;
        char buf[4096];
        while (true) {
            int n = SSL_read(ssl, buf, sizeof(buf));
            if (n > 0) {
                response.append(buf, static_cast<std::size_t>(n));
                continue;
            }
            int err = SSL_get_error(ssl, n);
            if (err == SSL_ERROR_ZERO_RETURN || err == SSL_ERROR_SYSCALL) {
                break;  
            }
            throw std::runtime_error(opensslError("SSL_read failed"));
        }
        return response;
    }
};

OpenSslHttpClient::OpenSslHttpClient() : impl_(std::make_unique<Impl>()) {}

OpenSslHttpClient::~OpenSslHttpClient() = default;

std::string OpenSslHttpClient::get(const std::string& url) {
    return impl_->request("GET", url, "", "");
}

std::string OpenSslHttpClient::post(const std::string& url, const std::string& body) {
    return impl_->request("POST", url, body, "application/x-www-form-urlencoded");
}

} // namespace tgbotcpp::net
