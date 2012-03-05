/** lock.S
  * @brief     Functions for using hardware locks
  * @author    Ross Owen, XMOS Limited
  */

#include <xs1_user.h>

/////////////////////////////////////////////////////////////////////////
// void GetLockResource()
.extern GetLockResource
.globl GetLockResource.nstackwords
.linkset GetLockResource.nstackwords, 0
.globl GetLockResource
.text 
GetLockResource:
  getr		r0, XS1_RES_TYPE_LOCK
  retsp 	0


/////////////////////////////////////////////////////////////////////////
// void ClaimLock(lock l)
.extern ClaimLock
.globl ClaimLock.nstackwords
.linkset ClaimLock.nstackwords, 0
.globl ClaimLock
.text 
ClaimLock:
  in		r0, res[r0]
  retsp 	0


/////////////////////////////////////////////////////////////////////////
// void FreeLock(lock l)
.extern FreeLock
.globl FreeLock.nstackwords
.linkset FreeLock.nstackwords, 0
.globl FreeLock
.text 
FreeLock:
  out		res[r0], r0
  retsp 	0


/////////////////////////////////////////////////////////////////////////
// void FreeLockResource(lock l)
.extern FreeLockResource
.globl FreeLockResource.nstackwords
.linkset FreeLockResource.nstackwords, 0
.globl FreeLockResource
.text 
FreeLockResource:
  freer		res[r0]
  retsp 	0


