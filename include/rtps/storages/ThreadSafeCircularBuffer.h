/*
 *
 * Author: Andreas Wüstenberg (andreas.wuestenberg@rwth-aachen.de)
 */

#ifndef RTPS_THREADSAFEQUEUE_H
#define RTPS_THREADSAFEQUEUE_H

#include "lwip/sys.h"
#include <array>


namespace rtps {

    namespace {
        class Lock {
        public:
            Lock(sys_mutex_t &passedMutex) : mutex(passedMutex) {
                sys_mutex_lock(&mutex);
            };

            ~Lock() {
                sys_mutex_unlock(&mutex);
            };
        private:
            sys_mutex_t &mutex;
        };
    }

    template<typename T, uint16_t SIZE>
    class ThreadSafeCircularBuffer {

    private:
        std::array<T, SIZE> queue{};
        sys_mutex_t mutex;
        uint32_t head = 0;
        uint32_t tail = 0;
        bool initialized = false;

    public:

        bool init() {
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

        ~ThreadSafeCircularBuffer() {
            if (mutex.mut != nullptr) {
                sys_mutex_free(&mutex);
            }
        }

        void moveElementIntoBuffer(T &&elem) {
            Lock lock(mutex);

            queue[head] = std::move(elem);
            head = (head + 1) % SIZE;
            if (head == tail) { // drop one element
                tail = (tail + 1) % SIZE;
            }
        }

        /**
         * Removes the first into the given hull. Also moves responsibility for resources.
         *
         * @return true if element was injected. False if no element was present.
         */
        bool moveFirstInto(T &hull) {
            Lock lock(mutex);
            if (head != tail) {
                hull = std::move(queue[tail]);
                tail = (tail + 1) % SIZE;
                return true;
            } else {
                return false;
            }
        }

        void clear(){
            Lock lock(mutex);
            head = tail;
        }
    };

}

#endif //RTPS_THREADSAFEQUEUE_H
