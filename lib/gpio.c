
#include "gpio.h"

#ifdef GPIO_MUTEX
#ifdef CPU_ALLWINNER_A20
//#include "allwinner_4.c"
#include  "gpio/mutex/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H3
//#include "allwinner_2.c"
#include "gpio/mutex/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H2P
//#include "allwinner_2.c"
#include "gpio/mutex/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H5
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_R40
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A33
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A31s
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A64
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A83T
#include "gpio/mutex/allwinner_2.c"
#else
#ifdef CPU_ROCKCHIP_RK3288
#include "gpio/mutex/rk3288.c"
#else
//debugging mode (for machine with no GPIO)
#include "gpio/all.c"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#else
#ifdef CPU_ALLWINNER_A20
//#include "allwinner_4.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H3
//#include "allwinner_2.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H2P
//#include "allwinner_2.c"
#include "gpio/allwinner_5.c"
#else
#ifdef CPU_ALLWINNER_H5
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_R40
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A33
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A31s
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A64
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ALLWINNER_A83T
#include "gpio/allwinner_2.c"
#else
#ifdef CPU_ROCKCHIP_RK3288
#include "gpio/rk3288.c"
#else
//debugging mode (for machine with no GPIO)
#include "gpio/all.c"
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
