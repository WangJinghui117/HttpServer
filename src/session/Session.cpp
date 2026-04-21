#include "../../include/session/Session.h"
#include "../../include/session/SessionManager.h"

namespace http{
namespace session{

Session::Session(const string& sessionId, SessionManager* sessionManager, int maxAge)
    : sessionId_(sessionId), maxAge_(maxAge), sessionManager_(sessionManager){
        refresh(); // 初始化时设置过期时间
    }

bool Session::isExpired() const{
    return std::chrono::system_clock::now() > expiryTime_;
}

void Session::refresh(){
    expiryTime_ = chrono::system_clock::now() + chrono::seconds(maxAge_);
}

void Session::setValue(const string& key, const string& value){
    data_[key] = value;
    if(sessionManager_){
        sessionManager_->updateSession(shared_from_this());
    }
}

string Session::getValue(const string& key) const{
    auto it = data_.find(key);
    if(it != data_.end()){
        return it->second;
    }
    return string();
}

// 删除session 数据
void Session::remove(const string& key){
    data_.erase(key);
}

// 清空会话数据
void Session::clear(){
    data_.clear();
}

}
}