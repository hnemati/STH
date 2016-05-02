#ifndef _UC_TYPES_H_
#define _UC_TYPES_H_

#include <stdint.h>

typedef __SIZE_TYPE__ size_t;
typedef size_t addr_t; /* XXX */

#define BOOL int
#define FALSE 0
#define TRUE 1

#ifndef NULL
 #define NULL 0
#endif

#define BASE_REG volatile uint32_t *



typedef enum return_value_ {
    RV_OK = 0,
    RV_BAD_ARG = -127,
    RV_BAD_PERM,
    RV_BAD_OTHER
} return_value;


/* data types */
typedef return_value (*cpu_callback)(uint32_t, uint32_t, uint32_t);


#endif /* _UC_TYPES_H_ */
