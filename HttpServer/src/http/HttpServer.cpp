#include "../../include/http/HttpServer.h"

#include <any>
#include <functional>
#include <memory>

namespace http{

// 默认http回调函数
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp){
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
}

HttpServer::HttpServer(int port, const string& name, 
                       bool useSSL, muduo::net::TcpServer::Option option)
                       : listenAddr_(port), server_(&mainLoop_, listenAddr_, name, option), useSSL_(useSSL),
                       httpCallback_(bind(&HttpServer::handleRequest, this, placeholders::_1, placeholders::_2))
                       {
                        initialize();
                       }

void HttpServer::start(){
    LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on" << server_.ipPort();
    server_.start();
    mainLoop_.loop();
}

void HttpServer::initialize(){
    // set http Callback
    server_.setConnectionCallback(bind(&HttpServer::onConnection, this, placeholders::_1));
    server_.setMessageCallback(
        bind(&HttpServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));
}

void HttpServer::setSslConfig(const ssl::SslConfig& config){
    if(useSSL_){
        sslCtx_ = make_unique<ssl::SslContext>(config);
        if(!sslCtx_->initialize()){
            LOG_ERROR << "Failed to initialize SSL context";
            abort();
        }
    }
}

void HttpServer::onConnection(const muduo::net::TcpConnectionPtr& conn){
    if(conn->connected()){
        if(useSSL_){
            auto sslConn = make_unique<ssl::SslConnection>(conn, sslCtx_.get());
            sslConn->setMessageCallback(bind(&HttpServer::onMessage, this, placeholders::_1, placeholders::_2, placeholders::_3));
            sslConns_[conn] = move(sslConn);
            sslConns_[conn]->startHandshake();
        }
        conn->setContext(HttpContext());
    }
    else{
        if(useSSL_){
            sslConns_.erase(conn);
        }
    }
}

void HttpServer::onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp receiveTime){
    try
    {
        if(useSSL_){
            LOG_INFO << "onMessage useSSL is true";
            // 查找对应的SSL连接
            auto it = sslConns_.find(conn);
            if(it != sslConns_.end()){
                LOG_INFO << "onMessage sslConns_ is not empty";
                // ssl 连接处理数据
                it->second->onRead(conn, buf, receiveTime);

                // 如果ssl握手还未完成，直接返回
                if(!it->second->isHandshakeCompleted()){
                    LOG_INFO << "onMessage sslConns_ is not empty";
                    return;
                }
                
                // 从ssl连接的解密缓冲区获取数据
                muduo::net::Buffer* decryptedBuf = it->second->getDecryptedBuffer();
                if(decryptedBuf->readableBytes() == 0){
                    return;
                }

                // 缓冲区buf指向解密后的数据
                buf = decryptedBuf;
                LOG_INFO << "onMessage decryptedBuf is not empty";
            }
        }

        //HttpContext 对象用于解析出buf中的请求报文，并把报文的关键信息封装到HttpRequest对象中
        HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());
        if(!context->parseRequest(buf, receiveTime)){  // 解析一个http请求
            // 如果解析http报文过程中出错
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        //如果buf缓冲区中解析出一个完整的数据包才封装响应报文
        if(context->gotAll()){
            onRequest(conn, context->request());
            context->reset();
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERROR << "Exception in onMessage: " << e.what();
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}

void HttpServer::onRequest(const muduo::net::TcpConnectionPtr& conn, const HttpRequest& req){
    const string& connection = req.getHeader("Connection");
    bool close = ((connection == "close") || (req.getVersion() == "HTTP/1.0" && connection != "Keep-Alive"));
    HttpResponse response(close);

    // 根据请求报文信息来封装响应报文对象
    httpCallback_(req, &response);    // onHttpCallback
    
    muduo::net::Buffer buf;
    response.appendToBuffer(&buf);
    // 打印完整的响应内容用于调试
    LOG_INFO << "Sending response:\n" << buf.toStringPiece().as_string();

    conn->send(&buf);
    // 如果是短链接的话，返回响应报文后断开连接
    if(response.closeConnection()){
        conn->shutdown();
    }
}

// 执行请求对应的路由处理函数
void HttpServer::handleRequest(const HttpRequest& req, HttpResponse* resp){
    try
    {
        // 处理请求前的中间件
        HttpRequest mutableReq = req;
        middlewareChain_.processBefore(mutableReq);

        // router
        if(!router_.route(mutableReq, resp)){
            LOG_INFO << "未知请求， url: " << req.method() << " " << req.path();
            LOG_INFO << "未找到路由， 返回404";
            resp->setStatusCode(HttpResponse::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }

        // 处理响应后的中间件
        middlewareChain_.processAfter(*resp);
    }
    catch(const HttpResponse& res)
    {
        // 处理中间件抛出的响应（如CORS预检请求）
        *resp = res;
    }
    catch(const std::exception& e)
    {
        resp->setStatusCode(HttpResponse::k500InternalServerError);
        resp->setBody(e.what());
    }
}

}