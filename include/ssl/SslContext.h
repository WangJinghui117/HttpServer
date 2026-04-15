#pragma once
#include "SslConfig.h"
#include <openssl/ssl.h>
#include <memory>
#include <muduo/base/noncopyable.h>
using namespace std;

namespace ssl{

class SslContext : muduo::noncopyable{    // 禁止对象拷贝（可以移动）

public:
    explicit SslContext(const SslConfig& config);
    ~SslContext();

    bool initialize();
    SSL_CTX* getNativeHandle() { return ctx_; }

private:
    bool loadCertificates();
    bool setupProtocol();
    void setupSessionCache();
    static void handleSslError(const char* msg);

private:
    SSL_CTX*    ctx_;    // SSL 上下文
    SslConfig   config_; // SSL 配置

};


}