
#ifndef RTPS_THREADSAFECIRCULARBUFFER_TPP
#define RTPS_THREADSAFECIRCULARBUFFER_TPP

namespace rtps {


    template<typename T, uint16_t SIZE>
    bool ThreadSafeCircularBuffer<T, SIZE>::init(){
        if (initialized) {
            return true;
        }
        if (sys_mutex_new(&mutex) != ERR_OK) {
            printf("Failed to create mutex \n");
            return false;
        } else {
            printf("Successfully created mutex at %p\n", &mutex);
            initialized = true;
            return true;
        }
    }

    template<typename T, uint16_t SIZE>
    ThreadSafeCircularBuffer<T, SIZE>::~ThreadSafeCircularBuffer(){
        if (mutex.mut != nullptr) {
            sys_mutex_free(&mutex);
        }
    }


    template<typename T, uint16_t SIZE>
    void ThreadSafeCircularBuffer<T, SIZE>::moveElementIntoBuffer(T &&elem) {
        Lock lock(mutex);
        buffer[head] = std::move(elem);
        incrementHead();
    }

    template<typename T, uint16_t SIZE>
    bool ThreadSafeCircularBuffer<T, SIZE>::moveFirstInto(T &hull) {
        Lock lock(mutex);
        if (head != tail) {
            hull = std::move(buffer[tail]);
            incrementTail();
            return true;
        } else {
            return false;
        }
    }

    template<typename T, uint16_t SIZE>
    void ThreadSafeCircularBuffer<T, SIZE>::clear() {
        Lock lock(mutex);
        head = tail;
    }

    template<typename T, uint16_t SIZE>
    inline void ThreadSafeCircularBuffer<T,SIZE>::incrementIterator(uint16_t& iterator) {
        ++iterator;
        if (iterator >= buffer.size()) {
            iterator = 0;
        }
    }

    template<typename T, uint16_t SIZE>
    inline void ThreadSafeCircularBuffer<T,SIZE>::incrementTail() {
        incrementIterator(tail);
    }

    template<typename T, uint16_t SIZE>
    inline void ThreadSafeCircularBuffer<T,SIZE>::incrementHead() {
        incrementIterator(head);
        if (head == tail) {
            incrementTail();
        }
    }
}

#endif //RTPS_THREADSAFECIRCULARBUFFER_TPP