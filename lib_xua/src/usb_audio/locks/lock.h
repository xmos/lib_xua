/** @file       lock.h
  * @brief      Functions for using hardware locks
  * @author     Ross Owen, XMOS Limited
  */

#ifndef _LOCK_H_
#define _LOCK_H_ 1

typedef unsigned lock;

/* Allocates and returns a lock resource - returns 0 if out of lock */
lock GetLockResource();

/* Claims the passed lock, this is a blocking call */
void ClaimLock(lock l);

/* Frees the passed lock */
void FreeLock(lock l);

/* De-allocated the passed lock resource */
void FreeLockResource(lock l);



#endif
