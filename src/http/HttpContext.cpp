#include "../../include/http/HttpContext.h"

using namespace muduo;
using namespace muduo::net;

namespace http{

// 将报文解析出来将关键信息封装到HttpRequest对象里面去
bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime){
    bool ok = true; // 解析每行请求格式是否正确
    bool hasMore = true;
    while(hasMore){
        if(state_ == kExpectRequestLine){
            const char* crlf = buf->findCRLF();
            if(crlf){
                ok = processRequestLine(buf->peek(), crlf);
                if(ok){
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                }else{
                    hasMore = false;
                }
            }else{
                hasMore = false;
            }
        }else if(state_ == kExpectHeaders){
            const char* crlf = buf->findCRLF();
            if(crlf){
                const char* colon = find(buf->peek(), crlf, ':');
                if(colon < crlf){
                    request_.addHeader(buf->peek(), colon, crlf);
                }
                else if(buf->peek() == crlf){
                    // 空行， 结束header
                    // 根据请求方法和Content-Length判断是否继续读取body
                    if(request_.method() == HttpRequest::kPost || request_.method() == HttpRequest::kPut){
                        string contentLength = request_.getHeader("Content-Length");
                        if(!contentLength.empty()){
                            request_.setContentLength(stoi(contentLength));
                            if(request_.contentLength() > 0){
                                state_ = kExpectBody;
                            }
                            else{
                                state_ = kGotAll;
                                hasMore = false;
                            }
                        }
                        else{
                            // POST/PUT 请求没有 Content-Length，是HTTP语法错误
                            ok = false;
                            hasMore = false;
                        }
                    }else{
                        // GET/HEAD/DELETE 等方法直接完成（没有请求体）
                        state_ = kGotAll;
                        hasMore = false;
                    }
                }
                else{
                    ok = false; // header行格式错误
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2); // 开始读ptr 指向 下一行数据
            }else{
                hasMore = false;
            }
        }
        else if(state_ == kExpectBody){
            // 检查缓冲区是否有足够的数据
            if(buf->readableBytes() < request_.contentLength()){
                hasMore = false;
                return true;
            }

            // 只读取Content-Length指定的长度
            string body(buf->peek(), buf->peek() + request_.contentLength());
            request_.setBody(body);

            // 准确移动读指针
            buf->retrieve(request_.contentLength());

            state_ = kGotAll;
            hasMore = false;
        }
    }
    return ok; // ok为false代表报文语法解析错误
}

bool HttpContext::processRequestLine(const char* begin, const char* end){
    bool succeed = false;
    const char* start = begin;
    const char* space = find(start, end, ' ');
    if(space != end && request_.setMethod(start, space)){
        start = space + 1;
        space = find(start, end, ' ');
        if(space != end){
            const char* argumentStart = find(start, space, '?');
            if(argumentStart != space){     // 请求带参数
                request_.setPath(start, argumentStart);
                request_.setQueryParameter(argumentStart+1, space);
            }else{
                request_.setPath(start, space);
            }

            start = space + 1;
            succeed = ((end - start == 8) && equal(start, end - 1, "HTTP/1."));
            if(succeed){
                if(*(end - 1)=='1'){
                    request_.setVersion("HTTP/1.1");
                }
                else if(*(end - 1) == '0'){
                    request_.setVersion("HTTP/1.0");
                }
                else{
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

}