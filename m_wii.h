bplist00�_WebMainResource�	
_WebResourceData_WebResourceMIMEType_WebResourceTextEncodingName_WebResourceFrameName^WebResourceURLO�<html class="gr__medesign_seas_upenn_edu"><head><style></style></head><body><pre style="word-wrap: break-word; white-space: pre-wrap;">// -----------------------------------------------------------------------------
// M2 Wiimote Pixart Sensor interface
// version: 1.1
// date: Nov 11, 2011
// author: J. Fiene
// -----------------------------------------------------------------------------

#ifndef m_wii__
#define m_wii__

#include "m_general.h"
#include "m_bus.h"

// -----------------------------------------------------------------------------
// Public functions:
// -----------------------------------------------------------------------------

char m_wii_open(void);
// FUNCTIONALITY:
// configure the mWii Pixart sensor.
// (note:  the mWii does not generate interrupts)
//
// TAKES:
// nothing
//
// RETURNS:
// 1 : success
// 0 : failure


char m_wii_read(unsigned int* blobs);
// FUNCTIONALITY:
// read blob data
//
// TAKES:
// blobs : pointer to a 12-element unsigned int buffer
//
// RETURNS:
// 1 : success
// 0 : failure

#endif</pre><div class="grammarly-disable-indicator"></div></body><span class="gr__tooltip"><span class="gr__tooltip-content"></span><i class="gr__tooltip-logo"></i><span class="gr__triangle"></span></span></html>[text/x-chdrUUTF-8P_5http://medesign.seas.upenn.edu/uploads/Guides/m_wii.h    ( : P n � �~���                           �