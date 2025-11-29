#include"log.h"
#include<iostream>

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() : level_(LogLevel::DEBUG), stop(false) {
    worker_ = std::thread(&Logger::processQueue, this);
}

Logger::~Logger() {
    stop = true;
    cv_.notify_all();
    if(worker_.joinable()) worker_.join();
    if(ofs_.is_open()) ofs_.close();
}

void Logger::setLogFile(const std::string &filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    logFile_ = filename;
    if(ofs_.is_open()) ofs_.close();
    ofs_.open(logFile_, std::ios::app);
    if(!ofs_.is_open()) {
        std::cerr << "Failed to open log file: " << logFile_ << " error: " << strerror(errno) << '\n';
    }
}

std::string Logger::getCurTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm_time;
    localtime_r(&time, &tm_time);

    std::ostringstream oss;
    oss << std::put_time(&tm_time, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKOWN";
    }
}

void Logger::log(LogLevel level, const std::string &message) {
    if(level < level_) return;

    std::ostringstream oss;
    oss << getCurTime() << " " << levelToString(level) << ": " << message << '\n';
    {
        std::lock_guard<std::mutex> lock(mtx_);
        logQueue_.push(oss.str());
    }
    cv_.notify_one();
}

void Logger::processQueue() {
    while(true) {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return stop || !logQueue_.empty(); });
        if(stop&&logQueue_.empty()) return;

        std::queue<std::string> logQueue;
        logQueue.swap(logQueue_);
        lock.unlock();

        while(!logQueue.empty()) {
            std::string msg = logQueue.front();
            logQueue.pop();

            std::cout << msg;
            std::cout.flush();

            {
                std::lock_guard<std::mutex> lock(mtx_);
                if(ofs_.is_open()) {
                    ofs_ << msg;
                    ofs_.flush();
                }
            }
        }
    }
}