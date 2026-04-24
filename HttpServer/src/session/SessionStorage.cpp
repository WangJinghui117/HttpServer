#include "../../include/session/SessionStorage.h"
#include <iostream>
#include <memory>

namespace http{
namespace session{

void MemorySessionStorage::save(shared_ptr<Session> session){
    sessions_[session->getId()] = session;
}

shared_ptr<Session> MemorySessionStorage::load(const string& sessionId){
    auto it = sessions_.find(sessionId);
    if(it != sessions_.end()){
        if(!it->second->isExpired()){
            return it->second;
        }else{
            sessions_.erase(it);
        }
    }
    return nullptr;
}

void MemorySessionStorage::remove(const string& sessionId){
    sessions_.erase(sessionId);
}

}
}