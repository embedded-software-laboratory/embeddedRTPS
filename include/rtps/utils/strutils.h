//
// Created by andreas on 13.01.19.
//

#ifndef RTPS_STRUTILS_H
#define RTPS_STRUTILS_H

#include "rtps/common/types.h"

inline int entityId2Str(rtps::EntityId_t id, char* str, size_t size) {
  int bytes = 0;
  for (const auto byte : id.entityKey) {
    bytes += snprintf(&str[bytes], size - bytes, "%x", (int)byte);
  }
  bytes +=
    snprintf(
      &str[bytes],
      size - bytes,
      "%x",
      static_cast<uint8_t>(id.entityKind)
  );
  return bytes;
}

inline int guidPrefix2Str(rtps::GuidPrefix_t prefix, char* str, size_t size) {
  int bytes = 0;
  for (const auto byte : prefix.id) {
    bytes +=  snprintf(&str[bytes], size - bytes, "%x", (int)byte);
  }
  return bytes;
}

inline int guid2Str(rtps::Guid_t guid, char* str, size_t size) {
  int bytes = guidPrefix2Str(guid.prefix, str, size);
  bytes += snprintf(&str[bytes], size - bytes, ":");
  bytes += entityId2Str(guid.entityId, &str[bytes], size - bytes);
  return bytes;
}

#endif // RTPS_STRUTILS_H
