#pragma once

#include <cstddef>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>

// Logger class helper to log nowhere
struct void_ostream {};

void_ostream& operator<<(void_ostream& l, auto) {
    return l;
}


template <typename Stream1 = std::ostream&, typename Stream2 = void_ostream, size_t BUFFSIZE = 2048>
class Logger {
    Stream1 m_stream1;
    Stream2 m_stream2;

public:
    Logger(Stream1 console = std::clog, Stream2 stream = void_ostream{}) : m_stream1{console}, m_stream2{stream} {}
    Logger(std::ofstream& fileStream) : m_stream1{void_ostream{}}, m_stream2{fileStream} {}

    template <typename ...T>
    void log(T... args) {
        (m_stream1 << ... << args);
        (m_stream2 << ... << args);
        m_stream1 << "\n\n";
        m_stream2 << "\n\n";
    }

    template <typename ...T>
    void log_fmt(const char* format, T... args) {
        static char buf[BUFFSIZE];
        snprintf(buf, BUFFSIZE, format, args...);
        log(buf);
    }
};
