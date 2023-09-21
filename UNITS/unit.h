/**
 * @defgroup    level_sensor
 * @ingroup     drivers_periph 
 * @details functons for handling level sensor

 */

#ifndef UNIT_H
#define UNIT_H

#include <stdint.h>
#include <stddef.h> 
//#include "periph_conf.h" ///< I2C device specified in the boards periph_conf.h 

typedef uint8_t u8;
typedef uint16_t u16;

 
#define IsInRange(x,min,max) x< min ? 0 : x > max ? 0: 1  
#define FillOnBand(x,min,max)  x < min ?  (x = min )  :  x > max ? (x = max)  : (x = x)


//extern struct *i2c_t;

/**  @brief bitmap of registers */
typedef union _sensReg_bitmap{

	struct{
		u8 b0:1;
		u8 b1:1; 
		u8 b2:1;
		u8 b3:1; 
		u8 b4:1;
		u8 b5:1; 
		u8 b6:1;
		u8 b7:1; 
	}b;
	u8 raw;
}Reg_bitmap ;

/**  @brief struct registers of sensor */
typedef struct _Regs{
    u8 RegAddr;
    char* pNameReg;
    Reg_bitmap bits;    
}Regs;

/**  @brief indexes of registers */
typedef enum {
	Status = 0,
	SPS_H,
	SPS_L,
	Reset,
	Scale,
	ValueH,
	ValueL 
}enmRegs;

/**  @brief Get time of period from frequensy SPS */
#define CALC_DELAY_MS(x) x > 0 ? (int)(10000.0f/(float)x + 1) : 10000

/**  @brief Get resolution of sampling */
#define CALC_RESOLUTION(x)  x <  2 ? 32767 : \
		x <  11 ? 4095 :\
		x <  101 ? 1023 : 255


/**  @brief Get 0 bit from register Status */
#define RDY RegsSens[Status].bits.b.b0
/**  @brief Get 1 bit from register Status */
#define OVF RegsSens[Status].bits.b.b1
/**  @brief Get 2 bit from register Status */
#define ERR RegsSens[Status].bits.b.b2
/**  @brief Get two bits from register SCALE */
#define SCALE RegsSens[Scale].bits.raw & 0x03
/**  @brief Concat value H and L byte of registers ValueH and ValueL*/
#define VAL_SENS(x) ((u16)x->RegsSens[ValueH].bits.raw << 8)| \
	(x->RegsSens[ValueH].bits.raw)
/**  @brief Calc level vs SCALE and resolution*/
#define CALC_DEPTH(x,v,r) ((float)x/((float)r) * v)
/**  @brief Convert value of ValueH, ValueL regs to meners*/
#define PARS_VAL(x,v) (x->SCALE) == 0 ?CALC_DEPTH(1,v,x->range): \
		(x->SCALE)  == 1 ?CALC_DEPTH(3,v,x->range): \
		(x->SCALE)  == 2 ?CALC_DEPTH(5,v,x->range): \
		CALC_DEPTH(10,v,x->range)
	
/**  @brief struct of level sensor  */
typedef struct _lvl_sens{
 	Regs RegsSens[7]; 	///< mirror of registers inside sensor
	i2c_t i2c_dev; 		///< perifery of i2c
	u16 adr_dev;		///< I2C adress of  sensor
	u8 mflags;			///<  addressing format
	u16 range;			///<  sampling resolution
}lvl_sens;
	
/**  @brief struct of settings sensor  */
typedef struct _sens_sett{
	u16 dSPS; ///< value of freq sampling
	u8 dScale; ///< scale depth
} sens_sett;

#ifdef __cplusplus
extern "C" {
#endif

lvl_sens *  Ini_Lvl_sens(i2c_t dev, u16 adr, u8 flags);

float runtimeCycle(lvl_sens * p_sens );

#ifdef __cplusplus
}
#endif
#endif /* UNIT_H  */
