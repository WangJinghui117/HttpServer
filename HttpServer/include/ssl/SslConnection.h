#pragma once
#include "SslContext.h"
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/noncopyable.h>
#include <openssl/ssl.h>
#include <memory>
using namespace std;

namespace ssl{

// 添加消息回调函数类型定义
using MessageCallback = std::function<void(const shared_ptr<muduo::net::TcpConnection>&, muduo::net::Buffer*, muduo::Timestamp)>;

class SslConnection : muduo::noncopyable{

public:
    using TcpConnectionPtr = std::shared_ptr<muduo::net::TcpConnection>;
    using BufferPtr = muduo::net::Buffer*;

    SslConnection(const TcpConnectionPtr& conn, SslContext* ctx);
    ~SslConnection();

    void startHandshake();
    void send(const void* data, size_t len);
    void onRead(const TcpConnectionPtr& conn, BufferPtr buf, muduo::Timestamp time);        // 接收加密数据
    bool isHandshakeCompleted() const {return state_ == SSLState::ESTABLISHED; }
    muduo::net::Buffer* getDecryptedBuffer() {return &decryptedBuffer_; }

    // SSL BIO操作回调
    static int bioWrite(BIO* bio, const char* data, int len);                        // 加密数据写入bid
    static int bioRead(BIO* bio, char* data, int len);                               // 解密数据
    static long bioCtrl(BIO* bio, int cmd, long num, void* ptr);

    // 设置消息回调函数
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

private:
    void handleHandshake();
    void onEncrypted(const char* data, size_t len);                                  // 发送加密数据到网络
    void onDecrypted(const char* data, size_t len);                                  // 存储解密数据
    SSLError getLastError(int ret);
    void handleError(SSLError error);

private:
    SSL*                               ssl_; // SSL连接
    SslContext*                        ctx_; // SSL上下文
    TcpConnectionPtr                   conn_; // TCP连接
    SSLState                           state_; // SSL状态

    // BIO 就像一个"水管"，数据可以流入流出
    // 网络数据 -> BIO -> SSL 解密 -> BIO -> 应用数据
    BIO*                               readBio_; // 网络数据 -> SSL
    BIO*                               writeBio_; // SSL -> 网络数据

    muduo::net::Buffer                 readBuffer_;
    muduo::net::Buffer                 writeBuffer_;
    muduo::net::Buffer                 decryptedBuffer_;   // 解密后的数据
    MessageCallback                    messageCallback_;   // 消息回调

};

}