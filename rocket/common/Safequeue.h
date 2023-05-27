#include <mutex>
#include <queue>

namespace rocket
{

    template <typename T>
    class SafeQueue
    {
    private:
        std::queue<T> m_q;
        std::mutex m_mutex;

    public:
        int size()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_q.size();
        }

        bool empty()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_q.empty();
        }

        void push(T &t)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_q.push(t);
        }

        bool dequeue(T &t)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (empty())
            {
                return false;
            }
            t = std::move(m_q.front());
            m_q.pop();

            return true;
        }

        void swap(SafeQueue &other)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            this->m_q.swap(other.m_q);
        }

        T &front()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_q.front();
        }

        void pop()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_q.pop();
        }
    };

} // namespace  rocket