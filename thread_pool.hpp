#ifndef SPIDER_THREAD_POOL_HPP
#define SPIDER_THREAD_POOL_HPP

#include <queue>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace spider {

    template <typename TCallable>
    class Consumer {
        std::queue<TCallable> & m_tasks;
        boost::mutex & m_queue_mutex;
        boost::mutex & m_has_tasks_mutex;
        boost::thread m_thread;

        TCallable getTask() {
            using boost::defer_lock;
            using boost::mutex;
            using boost::unique_lock;

            unique_lock<mutex> queue_access_lock(m_queue_mutex, defer_lock);
            lock(queue_access_lock, m_has_tasks_mutex);
            TCallable callable = m_tasks.front();
            m_tasks.pop();
            if (!m_tasks.empty()) {
                m_has_tasks_mutex.unlock();
            }
            return callable;
        }

        void consume() {
            while (true) {
                TCallable callable = getTask();
                try {
                    callable();
                } catch (...) {
                }
            }
        }

    public:
        Consumer(
            std::queue<TCallable> & tasks,
            boost::mutex & queue_mutex,
            boost::mutex & has_tasks_mutex)
        :
            m_tasks(tasks),
            m_queue_mutex(queue_mutex),
            m_has_tasks_mutex(has_tasks_mutex),
            m_thread(boost::bind(&Consumer<TCallable>::consume, this)) {
        }
    };

    template <typename TCallable>
    class ThreadPool {
        std::queue<TCallable> m_tasks;
        std::vector<boost::shared_ptr<Consumer<TCallable> > > m_pool;
        boost::mutex m_queue_mutex;
        boost::mutex m_has_tasks_mutex;

        ThreadPool(ThreadPool const& other);
        ThreadPool& operator=(ThreadPool const& other);

    public:
        ThreadPool(int size) {
            using boost::shared_ptr;

            for (int count = 0; count != size; ++count) {
                shared_ptr<Consumer<TCallable> > consumer(
                    new Consumer<TCallable>(
                        m_tasks, m_queue_mutex, m_has_tasks_mutex));
                m_pool.push_back(consumer);
            }
        }

        void addTask(TCallable callable) {
            using boost::defer_lock;
            using boost::lock_guard;
            using boost::mutex;
            using boost::unique_lock;

            lock_guard<mutex> queue_access_lock(m_queue_mutex);
            m_tasks.push(callable);
            m_has_tasks_mutex.unlock();
        }
    };

}

#endif // SPIDER_THREAD_POOL_HPP