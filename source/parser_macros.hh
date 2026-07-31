    // big endian read macros
#define READ_BE32(ptr) \
    ( (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[0])) << 24) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[1])) << 16) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[2])) <<  8) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[3]))      ) )

#define READ_BE16(ptr) \
    ( (static_cast<uint16_t>(static_cast<uint8_t>((ptr)[0])) << 8) | \
      (static_cast<uint16_t>(static_cast<uint8_t>((ptr)[1]))     ) )

#define READ_BE8(ptr) \
    ( static_cast<uint8_t>((ptr)[0]) )

    // little endian read macros
#define READ_LE32(ptr) \
    ( (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[3])) << 24) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[2])) << 16) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[1])) <<  8) | \
      (static_cast<uint32_t>(static_cast<uint8_t>((ptr)[0]))      ) )

#define READ_LE16(ptr) \
    ( (static_cast<uint16_t>(static_cast<uint8_t>((ptr)[1])) << 8) | \
      (static_cast<uint16_t>(static_cast<uint8_t>((ptr)[0]))     ) )

#define READ_LE8(ptr) \
    ( static_cast<uint8_t>((ptr)[0]) )

