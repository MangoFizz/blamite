#ifndef BLAMITE__MEMORY__STRUCT_HPP
#define BLAMITE__MEMORY__STRUCT_HPP

/**
 * Struct packed attribute
 */
#define PACKED __attribute__((__packed__))

/** 
 * Padding in bytes
 */
#define GAP(bytes, line) char gap_##line[bytes]
#define GET_GAP(bytes, line) GAP(bytes, line)
#define PADDING(bytes) GET_GAP(bytes, __LINE__)

/** 
 * Padding in bits
 */
#define BIT_GAP(bits, line) char gap_##line : bits
#define GET_BIT_GAP(bits, line) GAP(bits, line)
#define PADDING_BIT(bits) GET_GAP(bits, __LINE__)

#endif
