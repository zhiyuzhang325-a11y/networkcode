#pragma once // 防止头文件被重复包含，放在头文件最上面

#include<string>
#include<cstring>
#include<fstream>
#include<mutex>
#include<queue>
#include<thread>
#include<condition_variable>
#include<atomic>
#include<memory>
#include<chrono>
#include<ctime>
#include<iomanip>
#include<sstream>   //string stream

enum class LogLevel {
    DEBUG = 0,
    INFO,       // 信息
    WARN,       // 警告
    ERROR
};

class Logger {
public:
    static Logger &instance();

    void setLogFile(const std::string &filename);

    void log(LogLevel level, const std::string &message);

    ~Logger();
private:
    Logger();
    Logger(const Logger&) = delete;
    Logger &operator=(const Logger&) = delete;

    void processQueue();

    std::string getCurTime();
    std::string levelToString(LogLevel);

    LogLevel level_;
    std::string logFile_;
    std::ofstream ofs_;

    std::mutex mtx_;
    std::queue<std::string> logQueue_;
    std::condition_variable cv_;
    std::atomic<bool> stop;
    std::thread worker_;
};