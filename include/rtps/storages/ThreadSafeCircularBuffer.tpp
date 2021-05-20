
#ifndef RTPS_THREADSAFECIRCULARBUFFER_TPP
#define RTPS_THREADSAFECIRCULARBUFFER_TPP

#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#if TSCB_VERBOSE && RTPS_GLOBAL_VERBOSE
#define TSCB_LOG(...)                                                          \
  if (true) {                                                                  \
    printf("[TSCircularBuffer] ");                                             \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#else
#define TSCB_LOG(...) //
#endif

namespace rtps {

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::init() {
  if (m_initialized) {
    return true;
  }
  if (sys_mutex_new(&m_mutex) != ERR_OK) {
    TSCB_LOG("Failed to create mutex \n");
    return false;
  } else {
    TSCB_LOG("Successfully created mutex at %p\n",
             static_cast<void *>(&m_mutex));
    m_initialized = true;
    return true;
  }
}

template <typename T, uint16_t SIZE>
ThreadSafeCircularBuffer<T, SIZE>::~ThreadSafeCircularBuffer() {
  if (m_initialized) {
    sys_mutex_free(&m_mutex);
  }
}

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::moveElementIntoBuffer(T &&elem) {
  Lock lock(m_mutex);
  if (!isFull()) {
    m_buffer[m_head] = std::move(elem);
    incrementHead();
    return true;
  } else {
    return false;
  }
}

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::moveFirstInto(T &hull) {
  Lock lock(m_mutex);
  if (m_head != m_tail) {
    hull = std::move(m_buffer[m_tail]);
    incrementTail();
    return true;
  } else {
    return false;
  }
}

template <typename T, uint16_t SIZE>
void ThreadSafeCircularBuffer<T, SIZE>::clear() {
  Lock lock(m_mutex);
  m_head = m_tail;
}

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::isFull() {
  auto it = m_head;
  incrementIterator(it);
  return it == m_tail;
}

template <typename T, uint16_t SIZE>
inline void
ThreadSafeCircularBuffer<T, SIZE>::incrementIterator(uint16_t &iterator) {
  ++iterator;
  if (iterator >= m_buffer.size()) {
    iterator = 0;
  }
}

template <typename T, uint16_t SIZE>
inline void ThreadSafeCircularBuffer<T, SIZE>::incrementTail() {
  incrementIterator(m_tail);
}

template <typename T, uint16_t SIZE>
inline void ThreadSafeCircularBuffer<T, SIZE>::incrementHead() {
  incrementIterator(m_head);
  if (m_head == m_tail) {
    incrementTail();
  }
}
} // namespace rtps

#endif // RTPS_THREADSAFECIRCULARBUFFER_TPP
