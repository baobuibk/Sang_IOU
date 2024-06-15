/**
  ******************************************************************************
  * @file    Project/Corona/basetype.h 
  * @author  NGC Application Team
  * @version V1.0.0
  * @date    12/7/2012
  * @brief   <update file description>.
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BASETYPE_H
#define __BASETYPE_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
typedef enum t_Status
{
    STS_FIRST = 0,
    STS_DONE = STS_FIRST,
    STS_ERROR,
    STS_ITEMS_NOT_FOUND,
    STS_INVALID_PARAMETERS,
    STS_OUT_OF_HEAP,
    STS_CRC_ERROR,
    STS_OUT_OF_MEMORY,
    STS_INVALID_RESPONSE,
    STS_MEM_WRITE_FAILED,
    STS_MEM_READ_FAILED,
    STS_NOT_ENABLED,
    STS_LAST = STS_NOT_ENABLED
} t_Status;


/* Exported defines ----------------------------------------------------------*/
#define TRUE                        (uint16_t)(1 == 1)
#define FALSE                       (uint16_t)(1 == 0)
#ifndef NULL
#define NULL                        0
#endif
typedef enum {NO = 0, YES = ! NO} YesNoStatus;

typedef enum {STOP = 0, START = ! STOP} StartStopStatus;

typedef enum {CLOSE = 0, OPEN = ! CLOSE} OpenCloseStatus;

typedef enum {RESET = 0, SET = ! RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = ! DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = ! ERROR} ErrorStatus;
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define  MIN(a, b)      (((a) < (b)) ? (a) : (b))
#define  MAX(a, b)      (((a) > (b)) ? (a) : (b))

/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif /* __BASETYPE_H */

/******************* (C) COPYRIGHT 2013 Ecowise LLC *********END OF FILE*******/

