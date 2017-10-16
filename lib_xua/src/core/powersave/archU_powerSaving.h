#ifndef ARCH_U_POWER_SAVING_
#define ARCH_U_POWER_SAVING_

/* Sets the voltage down by VOLTAGE_REDUCTION_mV (voltage is set to 10 * X + 600 mV),
 * and adjusts other features to save power
 */
void archU_powerSaving();

#endif /*ARCH_U_POWER_SAVING_*/
