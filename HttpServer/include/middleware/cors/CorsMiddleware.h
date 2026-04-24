#pragma once

#include "../Middleware.h"
#include "../../http/HttpRequest.h"
#include "../../http/HttpResponse.h"
#include "CorsConfig.h"
using namespace std;

namespace http{

namespace middleware{

class CorsMiddleware : public Middleware{

public:
    explicit CorsMiddleware(const CorsConfig& config = CorsConfig::defaultConfig());

    void before(HttpRequest& request) override;
    void after(HttpResponse& response) override;

    string join(const vector<string>& strings, const string& delimiter);

private:
    bool isOriginAllowed(const string& origin) const;
    void handlePreflightRequest(const HttpRequest& request, HttpResponse& response);
    void addCorsHeaders(HttpResponse& response, const string& origin);


private:
    CorsConfig config_;

};

}
}