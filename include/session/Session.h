#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
using namespace std;

namespace http{
namespace session{

class SessionManager;

class Session : public enable_shared_from_this<Session>{  // enable_shared_from_this  能够安全指向自己 this 不会导致双重删除

public:
    Session(const string& sessionId, SessionManager* sessionManager, int maxAge = 3600);

    const string& getId() const{
        return sessionId_;
    }

    bool isExpired() const;

    void setManager(SessionManager* sessionManager){
        sessionManager_ = sessionManager;
    }

    SessionManager* getManager() const{
        return sessionManager_;
    }

    // 数据存取
    void setValue(const string& key, const string& value);
    string getValue(const string& key) const;
    void remove(const string& key);
    void clear();



private:
    string                             sessionId_;
    unordered_map<string, string>      data_;
    chrono::system_clock::time_point   expiryTime_;
    int                                maxAge_;   // 过期时间/秒
    SessionManager*                    sessionManager_;
};

}
}