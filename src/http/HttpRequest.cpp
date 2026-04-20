#include "../../include/http/HttpRequest.h"

namespace http{

void HttpRequest::setReceiveTime(muduo::Timestamp t){
    receiveTime_ = t;
}

bool HttpRequest::setMethod(const char* start, const char* end){
    assert(method_ == kInvalid);
    string m(start, end);
    if(m == "GET"){
        method_ = kGet;
    }
    else if(m == "POST"){
        method_ = kPost;
    }
    else if(m == "PUT"){
        method_ = kPut;
    }
    else if(m == "DELETE"){
        method_ = kDelete;
    }
    else if(m == "OPTIONS"){
        method_ = kOptions;
    }
    else{
        method_ = kInvalid;
    }
    return method_ != kInvalid;
}

void HttpRequest::setPath(const char* start, const char* end){
    path_.assign(start, end);
}

void HttpRequest::setPathParameter(const string& key, const string& value){
    pathParameters_[key] = value;
}

string HttpRequest::getPathParameter(const string& key) const{
    auto it = pathParameters_.find(key);
    if(it != pathParameters_.end()){
        return it->second;
    }
    return "";
}

void HttpRequest::setQueryParameter(const char* start, const char* end){
    string argumentStr(start, end);
    string::size_type pos = 0;
    string::size_type prev = 0;

    // 按照 & 分割多个参数
    while((pos = argumentStr.find('&', prev)) != string::npos){
        string pair = argumentStr.substr(prev, pos - prev);
        string::size_type equalPos = pair.find('=');
        if(equalPos != string::npos){
            string key = pair.substr(0, equalPos);
            string value = pair.substr(equalPos+1);
            queryParameters_[key] = value;
        }
        prev = pos + 1;
    }

    // 处理最后一个参数
    string lastpair = argumentStr.substr(prev);
    string::size_type equalPos = lastpair.find('=');
    if(equalPos != string::npos){
        string key = lastpair.substr(0, equalPos);
        string value = lastpair.substr(equalPos + 1);
        queryParameters_[key] = value;
    }
}

string HttpRequest::getQueryParameter(const string& key)const{
    auto it = queryParameters_.find(key);
    if(it != queryParameters_.end()){
        return it->second;
    }
    return "";
}

void HttpRequest::addHeader(const char* start, const char* colon, const char* end){
    string key(start, colon);
    colon++;
    while(colon < end && isspace(*colon)){
        colon++;
    }
    string value(colon, end);
    while(!value.empty() && isspace(value[value.size() - 1])) //消除尾部空格
    {
        value.resize(value.size() - 1);
    }
    headers_[key] = value;
}

string HttpRequest::getHeader(const string& field) const {
    string result;
    auto it = headers_.find(field);
    if(it != headers_.end()){
        result = it->second;
    }
    return result;
}

void HttpRequest::swap(HttpRequest& that){
    std::swap(method_, that.method_);
    std::swap(path_, that.path_);
    std::swap(version_, that.version_);
    std::swap(pathParameters_, that.pathParameters_);
    std::swap(queryParameters_, that.queryParameters_);
    std::swap(headers_, that.headers_);
    std::swap(receiveTime_, that.receiveTime_);
}

}