
#ifndef RTPS_THREADSAFECIRCULARBUFFER_TPP
#define RTPS_THREADSAFECIRCULARBUFFER_TPP

#include "rtps/utils/Lock.h"
#include "rtps/utils/Log.h"

#if TSCB_VERBOSE && RTPS_GLOBAL_VERBOSE
#ifndef TSCB_LOG
#define TSCB_LOG(...)                                                          \
  if (true) {                                                                  \
    printf("[TSCircularBuffer] ");                                             \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  }
#endif
#else
#define TSCB_LOG(...) //
#endif

namespace rtps {

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::init() {
  if (m_initialized) {
    return true;
  }
  if (!createMutex(&m_mutex)) {
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
bool ThreadSafeCircularBuffer<T, SIZE>::moveElementIntoBuffer(T &&elem) {
  Lock lock(m_mutex);
  if (!isFull()) {
    m_buffer[m_head] = std::move(elem);
    incrementHead();
    return true;
  } else {
    m_insertion_failures++;
    return false;
  }
}

template <typename T, uint16_t SIZE>
bool ThreadSafeCircularBuffer<T, SIZE>::copyElementIntoBuffer(const T &elem) {
  Lock lock(m_mutex);
  if (!isFull()) {
    m_buffer[m_head] = elem;
    incrementHead();
    return true;
  } else {
    m_insertion_failures++;
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
bool ThreadSafeCircularBuffer<T, SIZE>::peakFirst(T &hull) {
  Lock lock(m_mutex);
  if (m_head != m_tail) {
    hull = m_buffer[m_tail];
    return true;
  } else {
    return false;
  }
}

template <typename T, uint16_t SIZE>
uint32_t ThreadSafeCircularBuffer<T, SIZE>::numElements() {
  return m_num_elements;
}

template <typename T, uint16_t SIZE>
void ThreadSafeCircularBuffer<T, SIZE>::clear() {
  Lock lock(m_mutex);
  m_head = m_tail;
  m_num_elements = 0;
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
  m_num_elements--;
}

template <typename T, uint16_t SIZE>
inline void ThreadSafeCircularBuffer<T, SIZE>::incrementHead() {
  incrementIterator(m_head);
  m_num_elements++;
  if (m_head == m_tail) {
    incrementTail();
  }
}
} // namespace rtps

#endif // RTPS_THREADSAFECIRCULARBUFFER_TPP
