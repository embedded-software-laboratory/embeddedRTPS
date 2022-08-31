#include "rtps/utils/Lock.h"

namespace rtps {

bool createMutex(SemaphoreHandle_t *mutex) {
  *mutex = xSemaphoreCreateRecursiveMutex();
  if (*mutex != NULL) {
    return true;
  } else {
    LWIP_ASSERT("Mutex creation failed", true);
    return false;
  }
}

} // namespace rtps
