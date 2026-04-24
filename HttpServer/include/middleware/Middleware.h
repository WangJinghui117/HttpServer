#pragma once
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
using namespace std;

namespace http{

namespace middleware{

class Middleware{

public:
    virtual ~Middleware() = default;

    // 请求前处理
    virtual void before(HttpRequest& request) = 0;

    // 响应后处理
    virtual void after(HttpResponse& response) = 0;

    // 设置下一个中间件
    void setNext(shared_ptr<Middleware> next){
        nextMiddleware_ = next;
    }

protected:
    shared_ptr<Middleware> nextMiddleware_;

};

} // namespace middleware
} // namespace http