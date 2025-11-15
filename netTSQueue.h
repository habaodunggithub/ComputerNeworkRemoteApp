#pragma once

#include "netCommon.h"

namespace net {
    template <typename T> 
    class TSQueue {
    public:
        TSQueue() = default;
        TSQueue(const TSQueue<T>&) = delete; // Hỗ trợ cơ chế mutex(khóa) để đảm bảo thead safe
        virtual ~TSQueue() { clear(); };

        const T& front() {
            std::scoped_lock lock(mutexQueue);
            return deQueue.front();
        }

        const T& back() {
            std::scoped_lock lock(mutexQueue);
            return deQueue.back();
        }

        void push_front(const T& item) {
            std::scoped_lock lock(mutexQueue);
            deQueue.emplace_front(std::move(item));
        }

        void push_back(const T& item) {
            std::scoped_lock lock(mutexQueue);
            deQueue.emplace_back(std::move(item));
        }

        bool empty() {
            std::scoped_lock lock(mutexQueue);
            return deQueue.empty();
        }

        void clear() {
            std::scoped_lock lock(mutexQueue);
            deQueue.clear();
        }

        T pop_front() {
            std::scoped_lock lock(mutexQueue);
            auto temp = std::move(deQueue.front());
            deQueue.pop_front();
            
            return temp;
        }

        T pop_back() {
            std::scoped_lock lock(mutexQueue);
            auto temp = std::move(deQueue.back());
            deQueue.pop_back();
            
            return temp;
        }

        

    protected:
        std::mutex mutexQueue;
        std::deque<T> deQueue;
    };
}