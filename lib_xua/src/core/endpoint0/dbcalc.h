// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#ifndef __dbcalc_h__
#define __dbcalc_h__

/* Function: db_to_mult

     This function converts decibels into a volume multiplier. It uses a fixed-point polynomial approximation
     to 10^(db/10).

   Parameters:
       db               - The db value to convert.
       db_frac_bits     - The number of binary fractional bits in the supplied decibel value
       result_frac_bits - The number of required fractional bits in the result.

   Returns:
       The multiplier value as a fixed point value with the number of fractional bits as specified by
       the result_frac_bits parameter.
*/
unsigned db_to_mult(int db, int db_frac_bits, int result_frac_bits);

#endif // __dbcalc_h__
