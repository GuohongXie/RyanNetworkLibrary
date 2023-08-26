// This is a public header file, it must only include public header files.

#ifndef RYANLIB_NET_ENDIAN_H_
#define RYANLIB_NET_ENDIAN_H_

#include <endian.h>
#include <stdint.h>


// the inline assembler code makes type blur,
// so we disable warnings for a while.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
inline uint64_t HostToNetwork64(uint64_t host64) { return htobe64(host64); }

inline uint32_t HostToNetwork32(uint32_t host32) { return htobe32(host32); }

inline uint16_t HostToNetwork16(uint16_t host16) { return htobe16(host16); }

inline uint64_t NetworkToHost64(uint64_t net64) { return be64toh(net64); }

inline uint32_t NetworkToHost32(uint32_t net32) { return be32toh(net32); }

inline uint16_t NetworkToHost16(uint16_t net16) { return be16toh(net16); }

#pragma GCC diagnostic pop


#endif  // RYANLIB_NET_ENDIAN_H_
