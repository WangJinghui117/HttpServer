#pragma once

#include "SessionStorage.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include <memory>
#include <random>
using namespace std;

namespace http{
namespace session{

class SessionManager{

public:
    explicit SessionManager(unique_ptr<SessionStorage> storage);

    // 从请求中获取或创建会话
    shared_ptr<Session> getSession(const HttpRequest& req, HttpResponse* resp);

    // 销毁会话
    void destroySession(const string& sessionId);

    // 清理过期会话
    void cleanExpiredSessions();

    // 更新会话
    void updateSession(shared_ptr<Session> session){
        storage_->save(session);
    }

private:
    string generateSessionId();
    string getSessionIdFromCookie(const HttpRequest& req);
    void setSessionCookie(const string& sessionId, HttpResponse* resp);


private:
    unique_ptr<SessionStorage>     storage_;
    mt19937                        rng_;           //用于生成随机会话id

};


} // namespace session
} // namespace http