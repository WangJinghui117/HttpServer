#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <cassert>

#include <muduo/base/Logging.h>
using namespace std;

class FileUtil{

public:
    explicit FileUtil(string filepath)
        : filePath_(filepath), file_(filePath_, ios::binary), fileSize_(0){
            if(isValid()){
                updateFileSize();
            }
        }

    ~FileUtil(){
        file_.close();
    }

    // 判断是否有效路径
    bool isValid() const{
        return file_.is_open();
    }

    uint64_t size(){
        return fileSize_;
    }

    void updateFileSize(){
        if(!isValid()) return;
        auto curPos = file_.tellg(); //保存当前位置
        file_.seekg(0, ios::end);
        fileSize_ = file_.tellg();
        file_.seekg(curPos);         //恢复位置
    }

        // 重置打开默认文件
    void resetDefaultFile(){
        file_.close();
        file_.open("/Gomoku/GomokuServer/resource/NotFound.html", ios::binary);
    }

    void readFile(vector<char>& buffer){
        uint64_t filesize = size();
        if(filesize == 0){
            LOG_WARN << "File is empty: " << filePath_;
            buffer.clear();
        }

        buffer.resize(filesize);

        auto curPos = file_.tellg();
        file_.seekg(0, ios::beg);

        if(file_.read(buffer.data(), size())){
            LOG_INFO << "File content load to memory (" << size() << "bytes)";
            file_.seekg(curPos);
        }else{
            LOG_ERROR << "File read failed";
            buffer.clear();
            file_.seekg(curPos);
        }
    }

private:
    string filePath_;
    ifstream file_;
    uint64_t fileSize_; 

};