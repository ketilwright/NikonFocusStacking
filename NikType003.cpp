/*
 * NikType003.cpp
 *
 * Created: 5/7/2014 8:45:25 AM
 * Author: Ketil Wright
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "NikType003.h"
#include "MessagePump.h"
#include "MainMenu.h"
#include "SetupHandler.h"
#include "RunFocusStackHandler.h"
#include "LcdImpl.h"
extern MessagePump g_pump;
extern MainMenuHandler *g_pMain;
extern SetupHandler *g_pSetup;
extern RunFocusStackHandler *g_pRunStack;


NikType003::NikType003(USB *usb, PTPStateHandlers *stateHandler)
    :
    PTP(usb, stateHandler),
    m_eventCheckInterval(1000), // milliseconds in between event checking.
    m_nextEventCheckTime(0),   // marker for the next time we poll for events
    m_remainingFrames(0),      // counts down focus stack frames
    m_restoreFocusDrive(0),    // reverse focus, to restore after stack
    m_idProduct(0),            // distinguishes model of camera
    m_checkReadyInternal(100), // milliseconds delay between NK_OC_DeviceReady transactions.
	m_timeLastCapture(0)
{}


// Verbatim from Nikon.cpp in PTP directory.
// TODO: This (and NikonDSLR) are almost exactly the same
// as the PTP base class method, except this also probes
// for vendor ID (we might like deviceID also). A potentially
// more efficient solution might be for PTP to provide
// vendor/dev id somehow.
uint8_t NikType003::Init(uint8_t parent, uint8_t port, bool lowspeed)
{
    uint8_t		buf[12];
    uint8_t		rcode;
    UsbDevice	*p = NULL;
    EpInfo		*oldep_ptr = NULL;

    PTPTRACE("NK Init\r\n");

    AddressPool	&addrPool = pUsb->GetAddressPool();

    if (devAddress)
    return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;

    // Get pointer to pseudo device with address 0 assigned
    p = addrPool.GetUsbDevicePtr(0);

    if (!p)
    return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

    if (!p->epinfo)
    {
        PTPTRACE("epinfo\r\n");
        return USB_ERROR_EPINFO_IS_NULL;
    }

    // Save old pointer to EP_RECORD of address 0
    oldep_ptr = p->epinfo;

    // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
    p->epinfo = epInfo;

    // Get device descriptor
    rcode = pUsb->getDevDescr( 0, 0, sizeof(buf), (uint8_t*)buf );

    // Restore p->epinfo
    p->epinfo = oldep_ptr;

    if( rcode )
    {
        PTPTRACE2("getDevDesc:", rcode);
        return rcode;
    }
    USB_DEVICE_DESCRIPTOR *devDesc = reinterpret_cast<USB_DEVICE_DESCRIPTOR*>(buf);
    if (devDesc->idVendor == 0x04B0)
    {
        m_idProduct = devDesc->idProduct;
        return PTP::Init(parent, port, lowspeed);
    }
    else
    {
		g_print->clear();
		g_print->print(F("Not nikon"));
        return USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;
    }
}


// Override the PTP::Poll method, to check for events.
// The only event we are handling is PTP_EC_CaptureComplete,
// which is used to kick the next focus stack frame.
uint8_t NikType003::Poll()
{
    PTP::Poll();
    if(isConnected())
    {
        // Check for camera events if m_nextPollTime milliseconds
        // have elapsed since the previous check.
        uint32_t curTime = millis();
        if(curTime >= m_nextEventCheckTime)
        {
            NKEventParser parser(this);
            EventCheck(&parser); // invokes our OnEvent
            m_nextEventCheckTime = curTime + m_eventCheckInterval;
        }
    }
    return 0;
}

// Nikons don't always report events on the interrupt EP.
// When in PC mode, we need to issue the NK_OC_CheckEvent op code
// which will populate the output with pending events.

uint16_t NikType003::EventCheck(PTPReadParser *parser)
{
    OperFlags	flags		= { 0, 0, 0, 1, 1, 0 };
    return Transaction(NK_OC_CheckEvent, &flags, NULL, parser);
}

// Looks for a capture completion event, and queues the
// next focus stack frame if necessary.
void NikType003::OnEvent(const NKEvent* ev)
{
    if(!ev) return;
    // TODO: The NKEventHandler class defined in nkeventparser.h
    // appears to be intender to break events out into separate
    // handlers.
    if(PTP_EC_CaptureComplete == ev->eventCode )
    {
		g_pRunStack->reportStatus(F("Cap OK"));
        m_stateFlags.m_captureInProgress = 0;
		if(0 == m_remainingFrames)
		{
			//restoreOriginalFocus();
			// invoke focusStackNextFrame, which when
			// 0 == m_remaining frames will just restore
			// original focus
			focusStackNextFrame();
		}
    }
}

// Almost all Nikon "activation commands" require a wait on
// NK_OC_DeviceReady. The docs explicitly say that a minimum of
// 2 attempts are required.
uint16_t NikType003::waitForReady(uint16_t maxAttempts)
{
    unsigned long nNotReady = 0;
    // Nikon docs explicitly say 2 attempts on device ready. You'd think we could
    // just start with the while loop.
    uint16_t retDevReady = Operation(NK_OC_DeviceReady, 0, NULL);
    delay(m_checkReadyInternal);
    retDevReady = Operation(NK_OC_DeviceReady, 0, NULL);
    delay(m_checkReadyInternal);
    while((PTP_RC_DeviceBusy == retDevReady) && (++nNotReady < maxAttempts))
    {
        retDevReady = Operation(NK_OC_DeviceReady, 0, NULL);
        delay(m_checkReadyInternal);
    }
    return retDevReady;
}

// Turns on or off live view. Nikons won't handle PTP_OC_NIKON_MfDrive
// unless they are in live view. Also, after initiating capture to
// the card, the camera removes itself from live view, and we must
// restore that for the next focus operation.
uint16_t NikType003::enableLiveView(bool enable)
{

    uint16_t  ret = PTP_RC_GeneralError;
    if(!enable)
    {
        ret  = Operation(PTP_OC_NIKON_EndLiveView, 0, NULL);
    }
    else
    {
        ret  = Operation(PTP_OC_NIKON_StartLiveView, 0, NULL);
    }
    if(PTP_RC_OK == ret)
    {
        m_stateFlags.m_lvEnabled  = enable;
    }
    return ret;
}

// PTP_OC_InitiateCapture will not work for our purposes.
// CaptureRecInSDram (0x90C0) would work if we could fetch and store a
// a huge NEF file. Therefore, the solution is to use PTP_OC_NIKON_CaptureRecInMedia,
// telling the camera to put the data in the SD card
uint16_t NikType003::captureToCard()
{
    uint16_t ret = PTP_RC_GeneralError;
    if(!m_stateFlags.m_captureInProgress)
    {
		g_pRunStack->reportStatus(F("Capture"));
        uint32_t params[2];
        params[0] = 0xffffffff; // Capture Sort = Normal, no AF.
        params[1] = 0;          // Save media = SD card.
		if((ret = Operation(PTP_OC_NIKON_CaptureRecInMedia, 2, params)) == PTP_RC_OK)
		{
			m_stateFlags.m_preparedNextFrame = (m_remainingFrames > 1) ? 0 : 1;
			m_stateFlags.m_captureInProgress = 1;
			m_timeLastCapture = millis();	
		}
    }
    return ret;
}

// Drives focus.
// Parameters:
//      direction 1: infinity ->close, 2: close -> infinity
//      amount     : an apparently dimensionless and undocumented quantity.
uint16_t NikType003::moveFocus(uint32_t direction, uint32_t amount)
{
    OperFlags	flags		= { 2, 0, 0, 0, 0, 0 };
    uint32_t	params[2];
    params[0]	= (uint32_t)direction;
    params[1]	= (uint32_t)amount;
    return Transaction(PTP_OC_NIKON_MfDrive, &flags, params, NULL);
}

// initiates the 1st frame of focus stack operation,
// subsequent frames are kicked off by the capture complete event.
uint16_t NikType003::startFocusStack()
{
    uint16_t ret = PTP_RC_GeneralError;
    m_remainingFrames = g_pSetup->getNumFrames();
	g_pRunStack->reportFrame(m_remainingFrames);
    m_restoreFocusDrive = 0;
    m_stateFlags.m_stackActive = 1;
    m_stateFlags.m_allFramesOK = false;
    ret = captureToCard();
    if(PTP_RC_OK == ret)
    {
        g_pRunStack->reportFrame(m_remainingFrames);
		g_pRunStack->resetLastUpdateTime(m_timeLastCapture);
		m_remainingFrames--;
    }
    else
    {
        m_remainingFrames = 0;
    }
    return ret;
}

// Function: prepareNextFrame
//      Handles moving the focus to the next point, along
//      with all the LV on/off & wait checks.
//  returns: a ptp response error code indicating what occurred
//
uint16_t NikType003::prepareNextFrame()
{
	uint16_t retVal = PTP_RC_GeneralError;
	if(m_remainingFrames > 0)
	{
		// Shooting the previous frame happened with the camera
		// out of live view. We need live view back on, in order for
		// PTP_OC_NIKON_MfDrive to work.
		g_pRunStack->reportStatus(F("LV on"));
		if((retVal = enableLiveView(true)) == PTP_RC_OK)
		{
			if((retVal = waitForReady(1000)) == PTP_RC_OK)
			{
				g_pRunStack->reportStatus(F("Focus"));
				if((retVal = moveFocus(2, g_pSetup->getDriveAmount())) == PTP_RC_OK)
				{
					m_restoreFocusDrive += g_pSetup->getDriveAmount();
					if((retVal = waitForReady(1000)) == PTP_RC_OK)
					{
						g_pRunStack->reportStatus(F("LV off"));
						retVal = enableLiveView(false);
						m_stateFlags.m_preparedNextFrame = 1;
						g_pRunStack->reportFrame(m_remainingFrames);
					} // check ready after focus
				} // move focus
			} // check ready after enable live view
		} // enable live view
		switch(retVal)
		{
			case PTP_RC_OK:
			{
				break;
			}
			case NK_RC_MfDriveStepInsufficiency:
			{
				g_pRunStack->reportStatus(F("Inf"));
				break;
			}
		}
		if(PTP_RC_OK != retVal)
		{
			cancelFocusStack();
		}
		
	}
	return retVal;
}
uint16_t NikType003::focusStackNextFrame()
{
	uint16_t retVal = PTP_RC_GeneralError;
	if(m_remainingFrames > 0)
	{
		if((retVal = captureToCard()) == PTP_RC_OK)
		{
			// following frames, if any are kicked off
			// in the capture complete handling.
			m_remainingFrames--;
			if(0 == m_remainingFrames)
			{
				m_stateFlags.m_allFramesOK = 1;
			}

		} // capture
		switch(retVal)
		{
			case PTP_RC_OK:
			{
				break;
			}
			case PTP_RC_StoreFull:
			{
				g_pRunStack->reportStatus(F("CardFull"));
				break;
			}
		}
		if(PTP_RC_OK != retVal)
		{
			cancelFocusStack();
		}
	}
	else
	{
		m_stateFlags.m_stackActive = false;
		if(m_stateFlags.m_allFramesOK)
		{
			restoreOriginalFocus();
		}
		g_pump.setNextHandler(g_pMain);
	}
	return retVal;
}

// Function: cancelFocusStack();
//      Stops the focus stack op
void NikType003::cancelFocusStack()
{
    m_remainingFrames = 0;
	m_stateFlags.m_stackActive = 0;
}

// Function: restoreOriginalFocus();
//      attempts to put the focus back where it was when the
//      focusStack op started.
//  returns: a ptp response error code indicating if the focus
//      was successfully restored.
uint16_t NikType003::restoreOriginalFocus()
{
	uint16_t retVal = PTP_RC_GeneralError;
	if(g_pSetup->getRestoreFocus())
	{
		g_pRunStack->reportStatus(F("Restore"));
		if((retVal = enableLiveView(true)) == PTP_RC_OK)
		{
			if((retVal = waitForReady(1000)) == PTP_RC_OK)
			{
				if((retVal = moveFocus(1, m_restoreFocusDrive)) == PTP_RC_OK)
				{
					if((retVal = waitForReady(1000)) == PTP_RC_OK)
					{
						retVal = enableLiveView(false);
					} //ready
				}  // moveFocus
			} // ready
		} // live view on	
	}
	else
	{
		retVal = PTP_RC_OK;
	}
    return retVal;
}

// various accessors
void NikType003::setIsConnected(bool connected) { m_stateFlags.m_connected = connected; }
bool NikType003::isConnected() const { return m_stateFlags.m_connected; }
bool NikType003::isLiveViewEnabled() const { return m_stateFlags.m_lvEnabled; }
bool NikType003::isCaptureInProgress() const { return m_stateFlags.m_captureInProgress;}
bool NikType003::isFocusStackActive() const { return m_stateFlags.m_stackActive; }
bool NikType003::isNextFrameFocused() const { return m_stateFlags.m_preparedNextFrame; }
uint16_t NikType003::getProductID() const { return m_idProduct; }
uint32_t NikType003::getTimeLastCaptureStart() const { return m_timeLastCapture; }
uint16_t NikType003::getRemainingFrames() const { return m_remainingFrames; }