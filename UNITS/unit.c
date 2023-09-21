/**
 * @file   unit.c 
 * @brief     API for level sensor
 * @ingroup   level_sensor  
 * 
*  @section   check_level 
 */


#include "unit.h"
#include "string.h"

/**
 * @brief instance of settings
 */
sens_sett default_sett ={
    .dScale = 0, // at start - 1m
    .dSPS = 1    //at start - 0.1Hz - one sample per 10 sec
};
 
/**
 * @brief read one register from sensor
 * @param p_sens instance of level sensor
 * @param Indx index of register
 * @return 0 if susseful
 */
static int Read_reg(lvl_sens * p_sens, enmRegs Indx);
/**
 * @brief  read all registers from sensor
 * @param p_sens instance of level sensor
 * @return 0 if susseful
 */
static int Read_All_regs(lvl_sens * p_sens); 
/**
 * @brief reset level sensor 
 * @param p_sens instance of level sensor
 * @return 0 if susseful 
 */
static int Reset_Lvl_sens(lvl_sens * p_sens);
/**
 * @brief Compare settings insight sensor and given instance of settings
 * @param p_sens  instance of level sensor
 * @param sett  instance of settings
 * @return 0 if equally 
 */
static int CheckSettings(lvl_sens * p_sens, sens_sett* sett);
/**
 * @brief write settings to sensor 
 * @param p_sens instance of level sensor 
 * @param sett  instance of settings
 * @return  0 if susseful 
 */
static int Write_Settings(lvl_sens * p_sens, sens_sett* sett);
/**
 * @brief get instance of given index register
 * @param p_sens  instance of level sensor 
 * @param indx index register @enmRegs
 * @return instance of register @Regs
 */
static Regs* GetRegs(lvl_sens * p_sens, enmRegs  indx);
/**
 * @brief get instance of named register  
 * @param p_sens  instance of level sensor  
 * @param Name  name of register
 * @return instance of register @Regs
 */
static Regs* GetRegByName(lvl_sens * p_sens, const char* Name);

/**
 * @brief initialization of sensor 
 * @details Dynamic create  instance of sensor
 * @param dev[in] I2C hardware device
 * @param adr[in] adress of  I2C  device
 * @param flags[in]  addressing format
 * @return if susseful instance of level sensor 
 * else NULL
 */
lvl_sens *  Ini_Lvl_sens(i2c_t dev, u16 adr, u8 flags){ 
    // create dynamic instance
  lvl_sens * t = calloc(1,sizeof(lvl_sens));
  if(t != NULL){
    // fill feld of struct of level sensor
    for (int i = 0; i < 7; i++) {
        t->RegsSens[i].RegAddr = i;
        char *tNameReg = "Status";
        switch (i){
            case 0:  break;
            case 1:tNameReg = "SPS_H";  break;
            case 2:tNameReg = "SPS_L";  break;
            case 3:tNameReg = "Reset";  break;
            case 4:tNameReg = "Scale";  break;
            case 5:tNameReg = "ValueH";  break;
            case 6:tNameReg = "ValueL";  break;
        }
        t->RegsSens[i].pNameReg = tNameReg;
    }
    t->i2c_dev = dev;
    t->adr_dev = adr;
    t->mflags = flags;

   
    sleep(300);                 // wait for ready after power on
    if(Reset_Lvl_sens(t))return  NULL;                  // reset sensor
    if(Write_Settings(t, &default_sett))return  NULL;  // write settings
    if(CheckSettings (t, &default_sett))return  NULL;  // check corect writing
    return t;
    }
    return  NULL;
}

static int CheckSettings(lvl_sens * p_sens, sens_sett* sett){
    if (Read_All_regs(p_sens))
        return (-1);
    if (p_sens->RegsSens[Scale].bits.raw != default_sett.dScale)
        return (-1);
    if ((((u16)p_sens->RegsSens[SPS_H].bits.raw << 8) | (p_sens->RegsSens[SPS_L].bits.raw)) != default_sett.dSPS)
        return (-1);
    return 0;
}

static Regs* GetRegByName(lvl_sens * p_sens, const char* Name){ 
    for (int i = 0; i < 7; i++) {
      if(strncmp (p_sens->RegsSens[i].pNameReg, Name,5) == 0){
        return &p_sens->RegsSens[i] ;
      }
    }
    return NULL;
}

static Regs* GetRegs(lvl_sens * p_sens, enmRegs indx){  
      if(IsInRange(indx,0,6)){
        return &p_sens->RegsSens[indx] ;
      } 
    return NULL;
}

static int Reset_Lvl_sens(lvl_sens * p_sens){ 
    i2c_acquire(p_sens->i2c_dev); 

    Regs*pReset =
    // GetRegByName(p_sens,"Reset"); ///< get by name
     GetRegs (p_sens,Reset);         ///<  or get by index
    if (pReset != NULL)  { 
        if (i2c_write_reg(p_sens->i2c_dev, p_sens->adr_dev, pReset->RegAddr, 0xA8,p_sens->mflags) == 0)  {
            i2c_write_reg(p_sens->i2c_dev, p_sens->adr_dev, pReset->RegAddr, 0x53, p_sens->mflags);
        }else return -1;
    }else return -1;
    i2c_release(p_sens->i2c_dev ); 
    sleep(300);  
    return 0;
}

static int Read_reg(lvl_sens * p_sens, enmRegs Indx){
    u8 regPend;
    i2c_acquire(p_sens->i2c_dev); 
    int res =
     i2c_read_reg(p_sens->i2c_dev, p_sens->adr_dev , p_sens->RegsSens[Indx].RegAddr,
     p_sens->RegsSens[Indx].bits.raw,p_sens->mflags);
    i2c_release(p_sens->i2c_dev ); 
    return res;
}

static int Read_All_regs(lvl_sens * p_sens){
    u8 regPend[10];
    i2c_acquire(p_sens->i2c_dev); 
    int res =
     i2c_read_regs(p_sens->i2c_dev, p_sens->adr_dev , p_sens->RegsSens[0].RegAddr,regPend, 7, p_sens->mflags);
    if(res == 0){
        for (int i = 0; i < 7; i++) {
            p_sens->RegsSens[i].bits.raw = regPend[i];
        }
    }
    i2c_release(p_sens->i2c_dev ); 
    return res;
}

static int Write_Settings(lvl_sens * p_sens, sens_sett* sett){
    FillOnBand(sett->dScale,0,3);
    FillOnBand(sett->dSPS,0,1023);
    i2c_acquire(p_sens->i2c_dev); 

    Regs*pReg =   GetRegs (p_sens,SPS_H);            ///< get by index
    if ( pReg != NULL)  { 
        if (i2c_write_reg(p_sens->i2c_dev, p_sens->adr_dev, pReg->RegAddr,((sett->dSPS>>8)&0xFF) , p_sens->mflags) == 0)  {
            i2c_write_reg(p_sens->i2c_dev, p_sens->adr_dev, pReg->RegAddr+1,((sett->dSPS)&0xFF)  , p_sens->mflags);
        }else return -1;

    }else return -1;
    pReg =   GetRegs (p_sens,Scale);      
    if ( pReg != NULL)  { 
        if (i2c_write_reg(p_sens->i2c_dev, p_sens->adr_dev, pReg->RegAddr,(sett->dScale, p_sens->mflags) != 0))  
         return -1;
    }else return -1;
    
    i2c_release(p_sens->i2c_dev ); 
    p_sens->range = CALC_RESOLUTION(sett->dSPS);  
    return 0;
}


/**
 * @brief runtime get level
 * @details dynamic calc period ms of scan and call sleep func
 * @param p_sens instance of level sensor
 * @return if susseful - value (float) of Level in meter, 
 * else if wiat for RDY - (-1)
 * if have a ERR  - (-2)
 * if have a OVF  - (-3)
 * if Unsuccessful write settings  - (-4)
 */
float runtimeCycle(lvl_sens * p_sens ){

    sleep(CALC_DELAY_MS(default_sett.dSPS));
    static int now = 0;
    float Value = (-1.0f);
        Read_reg(p_sens ,Status);  

        if(p_sens-> ERR){
            Value = (-2.0f);
            if(Write_Settings(p_sens,&default_sett)==0)
                if(CheckSettings (p_sens, &default_sett)) Value = (-4.0f);  ///< check corect writing
            return Value;
        }
        if(p_sens-> OVF){
            Value = (-3.0f);
            default_sett.dScale++; 
            if(Write_Settings(p_sens,&default_sett)==0)
                if(CheckSettings (p_sens, &default_sett)) Value = (-4.0f);  ///< check corect writing
            return Value;
        }        
        if(p_sens-> RDY){
            Read_reg(p_sens ,ValueH); 
            Read_reg(p_sens ,ValueL); 
            u16 val_raw = VAL_SENS(p_sens );
            Value = PARS_VAL(p_sens, val_raw);
        }
    return Value;
}

