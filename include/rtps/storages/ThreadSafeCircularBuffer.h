/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADSAFEQUEUE_H
#define RTPS_THREADSAFEQUEUE_H

#include "lwip/sys.h"

#include <array>
#include <limits>

namespace rtps {

    template<typename T, uint16_t SIZE>
    class ThreadSafeCircularBuffer {


    public:

        bool init();

        ~ThreadSafeCircularBuffer();

        bool moveElementIntoBuffer(T&& elem);

        /**
         * Removes the first into the given hull. Also moves responsibility for resources.
         * @return true if element was injected. False if no element was present.
         */
        bool moveFirstInto(T& hull);

        void clear();

    private:
        std::array<T, SIZE + 1> m_buffer{};
        uint16_t m_head = 0;
        uint16_t m_tail = 0;
        static_assert(SIZE + 1 < std::numeric_limits<decltype(m_head)>::max(), "Iterator is large enough for given size");

        sys_mutex_t m_mutex;
        bool m_initialized = false;

        inline bool isFull();
        inline void incrementIterator(uint16_t& iterator);
        inline void incrementTail();
        inline void incrementHead();
    };

}

#include "ThreadSafeCircularBuffer.tpp"

#endif //RTPS_THREADSAFEQUEUE_H
