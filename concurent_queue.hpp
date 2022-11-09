#pragma once

#include <mutex>
#include <queue>

template <typename T>
class ConcurrentQueue {
    std::mutex m_mtx;
    std::queue<T> m_q;

public:
    using value_type = T;

    ConcurrentQueue() = default;
    ConcurrentQueue(const ConcurrentQueue& other) : m_mtx{}, m_q{other.m_q} {}

    void push(T el) {
        std::lock_guard lk{m_mtx};
        m_q.push(el);
    }

    T pop() {
        std::lock_guard lk{m_mtx};
        if (m_q.size() > 0) {
            auto ret = m_q.front();
            m_q.pop();
            return ret;
        }
        else {
            return T{};
        }
    }
};
