#pragma once
#include <memory>
#include <string>
#include <mutex>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include "DbException.h"
using namespace std;

namespace http{
namespace db{

class DbConnection{

public:
    DbConnection(const string& host, const string& user, const string& password, const string& database);
    ~DbConnection();

    // 禁止拷贝
    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;

    bool isValid();
    void reconnect();
    void cleanup();

    template<typename... Args>
    sql::ResultSet* executeQuery(const string& sql, Args&&... args){
        lock_guard<mutex> lock(mutex_);
        try
        {
            // 直接创建新的预处理语句， 不使用缓存
            unique_ptr<sql::PreparedStatement> stmt(
                conn_->prepareStatement(sql)
            );

            bindParams(stmt.get(), 1, std::forward<Args>(args)...);
            return stmt->executeQuery();
        }
        catch (const sql::SQLException& e){
            LOG_ERROR << "Query failed: " << e.what() << ", SQL: " << sql;
            throw DbException(e.what());
        }
    } 

    template<typename... Args>
    int executeUpdate(const string& sql, Args&&... args){
        lock_guard<mutex> lock(mutex_);
        try
        {
            unique_ptr<sql::PreparedStatement> stmt(
                conn_->prepareStatement(sql)
            );
            bindParams(stmt.get(), 1, std::forward<Args>(args)...);
            return stmt->executeUpdate();
        }
        catch (const sql::SQLException& e)
        {
            LOG_ERROR << "Update failed: " << e.what() << ", SQL: " << sql;
            throw DbException(e.what());
        }
    }

    bool ping();  // 检测连接是否有效

private:
    // 递归终止条件
    void bindParams(sql::PreparedStatement*, int){}

    // 绑定参数
    template<typename T, typename... Args>
    void bindParams(sql::PreparedStatement* stmt, int index, T&& value, Args&&... args){
        stmt->setString(index, to_string(forward<T>(value)));
        bindParams(stmt, index+1, std::forward<Args>(args)...);
    }

    // 特化 string 类型的参数绑定
    template<typename... Args>
    void bindParams(sql::PreparedStatement* stmt, int index, const string& value, Args&&... args){
        stmt->setString(index, value);
        bindParams(stmt, index + 1, forward<Args>(args)...);
    }


private:
    shared_ptr<sql::Connection> conn_;
    string                      host_;
    string                      user_;
    string                      password_;
    string                      database_;
    mutex                       mutex_;

};

}
}