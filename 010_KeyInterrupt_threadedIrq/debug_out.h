#ifndef _DEBUG_OUT_H_
#define _DEBUG_OUT_H_

#define INFO 1
#define ERROR 1

#if DEBUG
    #define DBG(fmt, args...) \
    {\
        printk("[DBG]<< File:%s Line:%d， Function:%s>> ", __FILE__, __LINE__, __FUNCTION__);\
        printk(fmt, ##args);\
    }
#endif

#if INFO
    #define INF(fmt, args...) \
    {\
        printk("[INF]<< File:%s Line:%d， Function:%s>> ", __FILE__, __LINE__, __FUNCTION__);\
        printk(fmt, ##args);\
    }
#endif

#if ERROR
    #define ERR(fmt, args...) \
    {\
        printk("[ERR]<< File:%s Line:%d， Function:%s>> ", __FILE__, __LINE__, __FUNCTION__);\
        printk(fmt, ##args);\
    }
#endif

#endif /* _DEBUG_OUT_H_ */
