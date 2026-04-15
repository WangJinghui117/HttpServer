#pragma once
#include <iostream>
#include <muduo/net/TcpServer.h>
#include "HttpRequest.h"
using namespace std;

namespace http{

class HttpContext{

public:
    enum HttpRequestParseState{
        kExpectRequestLine,       // 期望请求行
        kExpectHeaders,           // 解析请求头
        kExpectBody,              // 解析请求体
        kGotAll,                  // 解析完成
    };

    HttpContext(): state_(kExpectRequestLine) {}

    bool parseRequest(muduo::net::Buffer* buf, muduo::Timestamp receiveTime); // 解析HTTP请求
    bool gotAll() const{
        return state_ == kGotAll;
    }

    void reset(){
        state_ = kExpectRequestLine;
        HttpRequest dummyData;
        request_.swap(dummyData);
    }

    const HttpRequest& request() const{
        return request_;
    }

    HttpRequest& request(){
        return request_;
    }

private:
    bool processRequestLine(const char* begin, const char* end); // 处理请求行

private:
    HttpRequestParseState state_;             // HTTP请求解析状态
    HttpRequest           request_;           // HTTP请求对象
};


}