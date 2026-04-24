#include "../../include/router/Router.h"
#include <muduo/base/Logging.h>

namespace http{
namespace router{

void Router::registerHandler(HttpRequest::Method method, const string& path, HandlerPtr handler){
    RouteKey key{method, path};
    handlers_[key] = move(handler);
}

void Router::registerCallback(HttpRequest::Method method, const string& path, const HandlerCallback& callback){
    RouteKey key{method, path};
    callbacks_[key] = move(callback);
}

bool Router::route(const HttpRequest& req, HttpResponse* resp){
    RouteKey key{req.method(), req.path()};

    auto handlerIt = handlers_.find(key);
    if(handlerIt != handlers_.end()){
        handlerIt->second->handle(req, resp);
        return true;
    }

    auto callbackIt = callbacks_.find(key);
    if(callbackIt != callbacks_.end()){
        callbackIt->second(req, resp);
        return true;
    }

    for(const auto& [method, pathRegex, handler] : regexHandlers_){
        smatch match;
        string pathStr(req.path());
        //如果方法匹配并且动态路由匹配，则执行处理器
        if(method == req.method() && regex_match(pathStr, match, pathRegex)){
            // Extract path parameters and add them to the request
            HttpRequest newReq(req);
            extractPathParameters(match, newReq);

            handler->handle(newReq, resp);
            return true;
        }
    }

    // 查找动态路由回调函数
    for(const auto& [method, pathRegex, callback] : regexCallbacks_){
        smatch match;
        string pathStr(req.path());
        if(method == req.method() && regex_match(pathStr, match, pathRegex)){
            HttpRequest newReq(req);
            extractPathParameters(match, newReq);
            callback(newReq, resp);
            return true;
        }
    }
    return false;
}



}
}