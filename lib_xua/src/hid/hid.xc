// Copyright 2019-2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include <stdint.h>
#include <xs1.h>
#include "descriptor_defs.h"
#include "hid.h"
#include "xud.h"
#include "xud_std_requests.h"
#include "xua_hid.h"
#include "xua_hid_report.h"

#define DEBUG_UNIT HID_XC
#define DEBUG_PRINT_ENABLE_HID_XC 0
#include "debug_print.h"

#if XUA_HID_ENABLED
static unsigned     HidCalcNewReportTime( const unsigned currentPeriod, const unsigned reportTime, const unsigned reportToSetIdleInterval, const unsigned newPeriod );
static unsigned     HidCalcReportToSetIdleInterval( const unsigned reportTime );
static unsigned     HidFindSetIdleActivationPoint( const unsigned currentPeriod, const unsigned timeWithinPeriod );
static XUD_Result_t HidProcessSetIdleRequest( XUD_ep c_ep0_out, XUD_ep c_ep0_in, USB_SetupPacket_t &sp );
static unsigned     HidTimeDiff( const unsigned earlierTime, const unsigned laterTime );

XUD_Result_t HidInterfaceClassRequests(
  XUD_ep c_ep0_out,
  XUD_ep c_ep0_in,
  USB_SetupPacket_t &sp )
{
  XUD_Result_t result = XUD_RES_ERR;

  switch ( sp.bRequest ) {
    case HID_SET_IDLE:
      result = HidProcessSetIdleRequest( c_ep0_out, c_ep0_in, sp );
      break;

    default:
      break;
  }

  return result;
}

unsigned HidIsSetIdleSilenced( const unsigned id )
{
 unsigned isSilenced = hidIsIdleActive( id );

 if( !isSilenced ) {
   unsigned currentTime;
   // Use inline assembly to access the time without creating a side-effect.
   // The mapper complains if the time comes from an XC timer because this function is called in the guard of a select case.
   // Appearently the use of a timer creates a side-effect that prohibits the operation of the select functionality.
   asm volatile( "gettime %0" : "=r" ( currentTime ));
   isSilenced = ( 0U == hidGetReportPeriod( id ) || ( timeafter( hidGetNextReportTime( id ), currentTime )));
 }

 return isSilenced;
}

/**
 * \brief Calculate the timer value for sending the next HID Report.
 *
 *  With regard to Section 7.2.4 Set_Idle Request of the USB Device Class Definition for Human
 *    Interface Devices (HID) Version 1.11, 'currently executing period' and 'current period' have
 *    been interpreted to mean the previously established Set Idle duration if one has been
 *    established or the polling interval from the HID Report Descriptor if a Set Idle duration
 *    has not been established.
 *
 * \param[in]  currentPeriod           -- The duration of the current period in timer ticks
 * \param[in]  reportTime              -- The time at which the last HID Report was sent
 * \param[in]  reportToSetIdleInterval -- The time interval between receiving the Set Idle Request
 *                                        and sending the most recent HID Report
 * \param[in]  newPeriod               -- The new period value in timer ticks
 *
 * \return     The time at which the next HID Report should be sent
 */
static unsigned HidCalcNewReportTime( const unsigned currentPeriod, const unsigned reportTime, const unsigned reportToSetIdleInterval, const unsigned newPeriod )
{
  unsigned nextReportTime = 0U;

  if( HidFindSetIdleActivationPoint( currentPeriod, reportToSetIdleInterval )) {
    /* Activate immediately after sending the next HID Report */
    nextReportTime = reportTime + currentPeriod;
  } else {
    /* Activate immediately after sending the most recent HID Report */
    nextReportTime = reportTime + newPeriod;
  }

  return nextReportTime;
}

/**
 * \brief Calculate the time interval between the most recent HID Report and a subsequent Set Idle Request
 *
 * \warning For this function to produce an accurate interval measument, it must be called without delay
 *   upon receiving a Set Idle Request from the USB Host.
 *
 * \param[in]  reportTime -- The time at which the last HID Report was sent
 *
 * \return     The time interval between receiving the Set Idle Request and sending the most recent HID Report
 */
static unsigned HidCalcReportToSetIdleInterval( const unsigned reportTime )
{
  timer tmr;
  unsigned setIdleTime;

  tmr :> setIdleTime;
  unsigned result = HidTimeDiff( reportTime, setIdleTime );
  return result;
}

/**
 * \brief Indicate if activation of the Set Idle Request happens at the previous or next HID Report
 *
 *  Section 7.2.4 Set_Idle Request of the USB Device Class Definition for Human Interface
 *    Devices (HID) Version 1.11 makes two statements about the activation point for starting the
 *    duration of the request:
 *  - 'A new request will be executed as if it were issued immediately after the last report, if
 *     the new request is received at least 4 milliseconds before the end of the currently executing
 *     period.'
 *  - 'If the new request is received within 4 milliseconds of the end of the current period, then
 *     the new request will have no effect until after the report.'
 *
 * \param[in]  currentPeriod    -- The duration of the current period
 * \param[in]  timeWithinPeriod -- The current point in time relative to the current period
 *
 * \return     A Boolean indicating where the activation of the Set Idle Request Duration occurs.
 * \retval     1 -- Activate immediately after the next HID Report
 * \retval     0 -- Activate immediately after the previous HID Report
 */
static unsigned HidFindSetIdleActivationPoint( const unsigned currentPeriod, const unsigned timeWithinPeriod )
{
  unsigned result = (( currentPeriod - timeWithinPeriod ) < ( 4U * MS_IN_TICKS )) ? 1 : 0;

  return result;
}

/**
 *  \brief Configure a hid report's next report time and idle status based on a setidle request
 *
 *  \param[in]  reportId -- The report ID to modify
 *  \param[in]  reportDuration  -- The duration of the setidle request
 *
 */
static void HidUpdateReportPeriod( unsigned reportId, unsigned reportDuration ) {
  unsigned currentPeriod = hidGetReportPeriod( reportId );

  hidSetIdle( reportId, ( 0U == reportDuration ) || ( ENDPOINT_INT_INTERVAL_IN_HID < reportDuration ));

  if( hidIsIdleActive( reportId )) {
    unsigned reportTime = hidGetReportTime( reportId );
    unsigned reportToSetIdleInterval = HidCalcReportToSetIdleInterval( reportTime );
    unsigned nextReportTime = HidCalcNewReportTime( currentPeriod, reportTime, reportToSetIdleInterval, reportDuration * MS_IN_TICKS );
    hidSetNextReportTime( reportId, nextReportTime );
    currentPeriod = reportDuration * MS_IN_TICKS;
  }

  hidSetReportPeriod( reportId, currentPeriod );
}

/**
 *  \brief Process a Set Idle request
 *
 *  \param[in]  c_ep0_out -- the channel that carries data from Endpoint 0
 *  \param[in]  c_ep0_in  -- the channel that carries data for Endpoint 0
 *  \param[in]  sp        -- a structure containing the Set Idle data
 *
 *  \return     An XUD status value
 */
static XUD_Result_t HidProcessSetIdleRequest( XUD_ep c_ep0_out, XUD_ep c_ep0_in, USB_SetupPacket_t &sp )
{
  XUD_Result_t result = XUD_RES_ERR;

    /*
      The Set Idle request wValue field contains two sub-fields:
      - Duration in the MSB; and
      - Report ID in the LSB.

      The Duration field specifies how long the USB Device responds with NAK provided the HID data hasn't changed.
      Zero means indefinitely.
      The value is in units of 4ms.

      The Report ID identifies the HID report that the USB Host wishes to silence.

      The Set Idle request xIndex field contains the interface number.
   */
  uint16_t duration     = ( sp.wValue & 0xFF00 ) >> 6; // Transform from units of 4ms into units of 1ms.
  uint8_t  reportId     =   sp.wValue & 0x00FF;
  uint16_t interfaceNum =   sp.wIndex;

  /*
      As long as our HID Report Descriptor does not include a Report ID, any Report ID value other than zero
        indicates an error by the USB Host (see xua_ep0_descriptors.h for the definition of the HID
        Report Descriptor).

      Any Interface value other than INTERFACE_NUMBER_HID indicates an error by the USB Host.
   */
  if( INTERFACE_NUMBER_HID == interfaceNum ) {
    if( hidIsReportIdValid( reportId ) ) {
      HidUpdateReportPeriod( reportId, duration );

      result = XUD_DoSetRequestStatus( c_ep0_in );
    }
    else if ( reportId == 0U ) {
      // Wildcard request - set all report IDs to idle
      unsigned startReportId = hidGetNextValidReportId(reportId);

      reportId = startReportId;
      do {
        HidUpdateReportPeriod( reportId, duration );
        reportId = hidGetNextValidReportId( reportId );
      } while( reportId != startReportId);

      result = XUD_DoSetRequestStatus( c_ep0_in );
    }
  }

  return result;
}

/**
 * \brief Calculate the difference between two points in time
 *
 * This function calculates the difference between two two points in time.
 * It always returns a positive value even if the timer used to obtain the two
 *   time measurements has wrapped around.
 *
 * \warning If time values have been obtained from a timer that has wrapped
 *   more than once in between the two measurements, this function returns an
 *   incorrect value.
 *
 * \param[in]  earlierTime -- A value from a timer
 * \param[in]  laterTime   -- A value from a timer taken after \a earlierTime
 *
 * \return     The interval between the two points in time
 */
static unsigned HidTimeDiff( const unsigned earlierTime, const unsigned laterTime )
{
  return ( earlierTime < laterTime ) ? laterTime - earlierTime : UINT_MAX - earlierTime + laterTime;
}

#endif /* ( 0 < HID_CONTROLS ) */
