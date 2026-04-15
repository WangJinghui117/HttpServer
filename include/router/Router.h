#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <regex>
#include <vector>

#include "RouterHandler.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
using namespace std;

namespace http{
namespace router{

// 选择注册对象式的路由处理器还是注册回调函数式的处理器取决于处理器执行的复杂程度
// 如果是简单的处理可以注册回调函数，否则注册对象式路由处理器(对象中可封装多个相关函数)
// 二者注册其一即可
class Router{

public:
    using HandlerPtr = shared_ptr<RouterHandler>;
    using HandlerCallback = function<void(const HttpRequest &, HttpResponse *)>;

    // 路由键（请求方法 + URI）
    struct RouteKey{
        HttpRequest::Method    method;
        string                 path;
        
        bool operator==(const RouteKey &other) const{
            return method == other.method  && path == other.path;
        }
    };

    // 为 RouteKey 定义哈希函数
    struct RouteKeyHash{
        size_t operator()(const RouteKey &key) const{
            size_t methodHash = hash<int>{}(static_cast<int>(key.method));
            size_t pathHash = hash<string>{}(key.path);
            return methodHash * 31 + pathHash;
        }
    };

    // 注册路由处理器
    void registerHandler(HttpRequest::Method method, const string &path, HandlerPtr handler);

    // 注册回调函数形式的处理器
    void registerCallback(HttpRequest::Method method, const string &path, const HandlerCallback &callback);

    // 注册动态路由处理器
    void addRegexHandler(HttpRequest::Method method, const string &path, HandlerPtr handler){
        regex pathRegex = converToRegex(path);
        regexHandlers_.emplace_back(method, pathRegex, handler);
    }

    // 注册动态路由处理函数
    void addRegexCallback(HttpRequest::Method method, const string &path, const HandlerCallback &callback){
        regex pathRegex = converToRegex(path);
        regexHandlers_.emplace_back(method, pathRegex, callback);
    }

    // 处理请求
    bool route(const HttpRequest &req, HttpResponse *resp);

private:
    regex converToRegex(const string &pathPattern){ // 将路径模式转换为正则表达式，支持匹配任意路径参数
        string regexPattern = "^" + regex_replace(pathPattern, regex(R"(/:([^/]+))"), R"(/([^/]+))") + "$";
        return regex(regexPattern);
    }

    // 提取路径参数
    void extractPathParameters(const smatch &match, HttpRequest &request){
        // assume the first match is the full path, parameters start from index 1
        for(size_t i = 1; i < match.size(); i++){
            request.setPathParameter("param" + to_string(i), match[i].str());
        }
    }
    

private:
    struct RouteCallbackObj{
        HttpRequest::Method            method_;
        regex                          pathRegex_;
        HandlerCallback                callback_;
        RouteCallbackObj(HttpRequest::Method method, regex pathRegex, const HandlerCallback &callback)
            : method_(method), pathRegex_(pathRegex), callback_(callback) {}
    };

    struct RouteHandlerObj{
        HttpRequest::Method            method_;
        regex                          pathRegex_;
        HandlerPtr                     handler_;
        RouteHandlerObj(HttpRequest::Method method, regex pathRegex, HandlerPtr handler)
            : method_(method), pathRegex_(pathRegex), handler_(handler) {}
    };

    unordered_map<RouteKey, HandlerPtr, RouteKeyHash>       handlers_;              // 精准匹配
    unordered_map<RouteKey, HandlerCallback, RouteKeyHash>  callbacks_;             // 精准匹配
    vector<RouteHandlerObj>                                 regexHandlers_;         // 正则匹配
    vector<RouteCallbackObj>                                regexCallbacks_;        // 正则匹配


};

}   // namespace router
}   // namespace http