#pragma once
#include <vector>
#include <memory>
#include "Middleware.h"
using namespace std;

namespace http{

namespace middleware{

class MiddlewareChain{
public:
    void addMiddleware(shared_ptr<Middleware> middleware);
    void processBefore(HttpRequest& request);
    void processAfter(HttpResponse& response);

private:
    vector<shared_ptr<Middleware>> middlewares_;
};

}
}