#ifndef __PM_H__
#define __PM_H__

/* Power Management */
// OP_PM_POWER_UP              0x60
// OP_PM_POWER_DOWN            0x61
// OP_PM_SPD                   0x62    // Standby Power Down
// OP_PM_DPD                   0x63    // Deep Power Down
// OP_PM_DS                    0x64    // Deep Sleep
// OP_PM_STOP                  0x65    // Stop
// OP_PM_WAKEUP                0x66    // Wakeup

void PM_PowerUp(void);
void PM_PowerDown(void);
void PM_StandbyPowerDown(void);
void PM_DeepPowerDown(void);
void PM_DeepSleep(void);
void PM_Stop(void);
void PM_Wakeup(void);

#endif /* __PM_H__ */

/* vim: set ts=4 sw=4 tw=0 list : */
