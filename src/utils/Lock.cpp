#include "rtps/utils/Lock.h"

#if defined(POSIX)
#include <iostream>
#endif

namespace rtps {

bool createMutex(Lock_t *mutex) {
#if defined(POSIX)
  *mutex = new std::recursive_mutex();
#else
  *mutex = xSemaphoreCreateRecursiveMutex();
#endif
  if (*mutex != NULL) {
    return true;
  } else {
#if !defined(POSIX)
    LWIP_ASSERT("Mutex creation failed", true);
#else
    std::cout << "Failed to create mutex" << std::endl;
    std::terminate();
#endif
    return false;
  }
}

} // namespace rtps
