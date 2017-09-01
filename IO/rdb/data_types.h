/*
 * data types defines
 * zhouxf, 2017.6.23
 */

#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

typedef char Int8;
typedef short Int16;
typedef int Int32;
typedef long long Int64;

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;
typedef unsigned long long Uint64;

typedef float float32;
typedef double float64;
/*


typedef struct {
	unsigned int seconds;
	unsigned int msec;
} time_type;*/

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef MSB
#define MSB(X) (((X) >> 8) & 0xff)
#endif /* MSB */

#ifndef LSB
#define LSB(X) ((X) & 0xff)
#endif /* LSB */

#ifndef MSW
#define MSW(X) (((X) >> 16) & 0xffff)
#endif /* MSW */

#ifndef LSW
#define LSW(X) ((X) & 0xffff)
#endif /* LSW */

#endif /* _DATA_TYPES_H_ */

