#pragma once
#include <string>
#include <vector>
using namespace std;

namespace http{

namespace middleware{

struct CorsConfig{
    vector<string> allowedOrigins;
    vector<string> allowedMethods;
    vector<string> allowedHeaders;
    bool allowCredentials = false;
    int maxAge = 3600;

    static CorsConfig defaultConfig(){
        CorsConfig config;
        config.allowedOrigins = {"*"};
        config.allowedMethods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
        config.allowedHeaders = {"Content-Type", "Authorization"};
        return config;
    }
};

}
}