#pragma once
#include "db/DbConnectionPool.h"
#include <string>

using namespace std;

namespace http{

class MysqlUtil{

public:
    static void init(const string& host, const string& user, const string& password, const string& database, size_t poolSize = 10){

        http::db::DbConnectionPool::getInstance().init(host, user, password, database, poolSize);

    }

    template<typename... Args>
    sql::ResultSet* executeQuery(const string& sql, Args&&... args){
        auto conn = http::db::DbConnectionPool::getInstance().getConnection();
        return conn->executeQuery(sql, std::forward<Args>(args)...);
    }

    template<typename... Args>
    int executeUpdate(const string& sql, Args&&... args){
        auto conn = http::db::DbConnectionPool::getInstance().getConnection();
        return conn->executeUpdate(sql, forward<Args>(args)...);
    }

};


} // namespace http