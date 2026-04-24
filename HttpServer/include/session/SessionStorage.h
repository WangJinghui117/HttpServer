#pragma once
#include "Session.h"
#include <memory>
using namespace std;

namespace http{
namespace session{

class SessionStorage{

public:
    virtual ~SessionStorage() = default;
    virtual void save(shared_ptr<Session> session) = 0;
    virtual shared_ptr<Session> load(const string& sessionId) = 0;
    virtual void remove(const string& sessionId) = 0;

};

// 基于内存的会话存储
class MemorySessionStorage : public SessionStorage{

public:
    void save(shared_ptr<Session>) override;
    shared_ptr<Session> load(const string& sessionId) override;
    void remove(const string& sessionId) override;

private:
    unordered_map<string, shared_ptr<Session>> sessions_;

};


}
}