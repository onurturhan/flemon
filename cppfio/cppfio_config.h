#ifdef ENABLE_COMPRESS
 #define ZLIB_SUPPORT   1
 #define BZLIB_SUPPORT  1
#else
 // Disable zlib and bzlib support, not really useful for this project
 #define ZLIB_SUPPORT   0
 #define BZLIB_SUPPORT  0
#endif
#define PCRE_SUPPORT   0
#define LIBUSB_SUPPORT 0
