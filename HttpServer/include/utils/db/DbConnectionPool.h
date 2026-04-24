#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include "DbConnection.h"
using namespace std;

namespace http{
namespace db{

class DbConnectionPool{

public:
    // 单例模式
    static DbConnectionPool& getInstance(){
        static DbConnectionPool instance;
        return instance;
    }

    // 初始化连接池
    void init(const string& host, const string& user, const string& password, const string& database, size_t poolSize = 10);

    // 获取连接
    shared_ptr<DbConnection> getConnection();

private:
    DbConnectionPool();
    ~DbConnectionPool();

    // 禁止拷贝
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

    shared_ptr<DbConnection> createConnection();

    void checkConnections();  // 添加 连接检查方法

private:
    string                host_;
    string                user_;
    string                password_;
    string                database_;
    queue<shared_ptr<DbConnection>>    connections_;
    mutex                 mutex_;
    condition_variable    cv_;
    bool                  initialized = false;
    thread                checkThread_;   // 添加检查线程

};

}
}