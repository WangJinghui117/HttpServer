#include "../../include/session/SessionManager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

namespace http{
namespace session{

SessionManager::SessionManager(unique_ptr<SessionStorage> storage)
    : storage_(move(storage)), rng_(random_device{}()) {}

// 从请求中获取或创建会话，如果请求中包含会话id，则从存储中加载会话，否则创建一个新的会话
shared_ptr<Session> SessionManager::getSession(const HttpRequest& req, HttpResponse* resp){
    string sessionId = getSessionIdFromCookie(req);
    shared_ptr<Session> session;
    if(!sessionId.empty()){
        session = storage_->load(sessionId);
    }

    if(!session || session->isExpired()){
        sessionId = generateSessionId();
        session = make_shared<Session>(sessionId, this);
        setSessionCookie(sessionId, resp);
    }else{
        session->setManager(this); // 为当前会话设置管理器
    }

    session->refresh();
    storage_->save(session);
    return session;
}

// 生成唯一的会话标识符，确保会话的唯一性和安全性
string SessionManager::generateSessionId(){
    stringstream ss;
    uniform_int_distribution<> dist(0, 15);

    // 生成32个字符的会话id，每个字符是一个十六进制的数字
    for(int i = 0; i < 32; i++){
        ss << hex << dist(rng_);
    }
    return ss.str();
}

void SessionManager::destroySession(const string& sessionId){
    storage_->remove(sessionId);
}

void SessionManager::cleanExpiredSessions(){
    auto now = std::chrono::steady_clock::now();
    
    // 注意：这个实现依赖于具体的存储实现
    // 对于内存存储，可以在加载时检查是否过期
    // 对于其他存储的实现，可能需要定期清理过期会话
}

string SessionManager::getSessionIdFromCookie(const HttpRequest& req){
    string sessionId;
    string cookie = req.getHeader("Cookie");
    if(!cookie.empty()){
        size_t pos = cookie.find("sessionId=");
        if(pos != string::npos){
            pos += 10;
            size_t end = cookie.find(';', pos);
            if(end != string::npos){
                sessionId = cookie.substr(pos, end - pos);
            }else{
                sessionId = cookie.substr(pos);
            }
        }
    }
    return sessionId;
}

void SessionManager::setSessionCookie(const string& sessionId, HttpResponse* resp){
    string cookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";
    resp->addHeader("Set-Cookie", cookie);
}

}
}