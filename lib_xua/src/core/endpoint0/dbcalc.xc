// Copyright 2011-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <xs1.h>
#include <print.h>
/* The coefficients of the chebychev polynomial to approximate 10^x in the interval [-1,1].
   This polynomial was calculated using the mpmath library in Python:

   from mpmath import *
   mp.dps = 15
   mp.pretty = True
   poly, err = chebyfit(lambda x: pow(10,x), [-1,1], 14, error=True)
   nprint([int(x * pow(2,28)) for x in poly])
 */
#define COEF_PREC 28
static unsigned coef[14] = {2407, 13778, 64588, 308051, 1346110, 5261991, 18277531, 55564576, 144789513, 314406484, 546179875, 711608713, 618095479, 268435456};

#define DB_CALC_PREC 28

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
unsigned db_to_mult(int db, int db_frac_bits, int result_frac_bits)
{
  int intpart;
  int val = 0;
  int val0=0;
  unsigned ret;
  unsigned mask = ~((1<<DB_CALC_PREC)-1);

  /* Make sure we get 0db bang on */
  if (db == 0)
    return (1 << result_frac_bits);


  /* First scale the decibal value to the required precision and divide by 10
     We scale to DB_CALC_PREC - 4 before the division with to make sure we don't overflow */
  db = db << (DB_CALC_PREC - 4 - 1 - db_frac_bits);
  db = db / 10;
  db = db << 4;


  /* Extract the integer part of the exponent and calculate the integer power */
  /* This could have been done a bit more efficiently by extracting the largest multiple log_10(2)
     and then calculating a power of 2 (with the polynomial calc in the range [-log_10(2),log_10(2)].
     But we have something that works here and ultra-fast performance is not a requirement */
  if (db < 0) {
    intpart = ((-db) & mask);
    db = db + intpart;
    intpart = intpart >> DB_CALC_PREC;

    if (intpart) {
      val0 = 1 << DB_CALC_PREC;
      for (int i=0;i<intpart;i++)
        val0 = val0/10;
    }
  }
  else {
    intpart = (db & mask);
    db = db - intpart;
    intpart = intpart >> DB_CALC_PREC;
    if (intpart) {
      val0 = 1 << DB_CALC_PREC;
      for (int i=0;i<intpart;i++)
        val0 = val0*10;
    }
  }

  /* db is now just the fractional part in the interval [-1,1] so we can approximate using
     the chebychev polynomial */
  for (int i=0;i<14;i++)
    {
      int hi=0;
      unsigned lo=0;

      {hi, lo} = macs(db,val,hi,lo);

      val = (hi << (32-DB_CALC_PREC)) | (lo >> DB_CALC_PREC);
      val += coef[i] >> (COEF_PREC - DB_CALC_PREC);
    }

  /* Finally multiply by the integer power (if there was an integer part) */
  if (val0) {
      int hi=0;
      unsigned lo=0;

      {hi, lo} = macs(val0,val,hi,lo);
      val = (hi << (32-DB_CALC_PREC)) | (lo >> DB_CALC_PREC);
  }


  /* We now have the result, just need to scale it to the required precision */
  ret = val;

  if (result_frac_bits > DB_CALC_PREC) {
    return ret<<(result_frac_bits-DB_CALC_PREC);
  }
  else {
    return ret>>(DB_CALC_PREC-result_frac_bits);
  }
}

#ifdef TEST_DBCALC
#include <print.h>

int main() {

  /* Check that we don't overflow up to 9db
     Should give a value just under 0x80000 */
  printhexln(db_to_mult(9,0,16));

  /* This test recreates the old db lookup table */
  printuintln(0xffffffff);
  for (int i=1;i<128;i++) {
    printuintln(db_to_mult(-i,0,32));
  }
  return 0;
}
#endif
