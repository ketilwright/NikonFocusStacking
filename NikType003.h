/*
 * NikType003.h
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

#pragma once
#include <nkeventparser.h>
#include <ptp.h>
#include <Usb.h>
#include <usbhub.h>


class NikType003StateHandler;
class NKEvent;

struct NikStateFlags
{
    uint8_t m_connected         : 1; // 0: Camera disconnected      1: camera connected
    uint8_t m_lvEnabled         : 1; // 0: live view is off,        1: live view is on
    uint8_t m_captureInProgress : 1; // 0: capture is idle,         1: capture is busy
    uint8_t m_stackActive       : 1; // 0: focus stacking inactive  1: focus stack active
    uint8_t m_allFramesOK       : 1; // Cleared when focus stack starts, set when successfully done.
	uint8_t m_preparedNextFrame : 1; // indicates if focus is ready for next frame.
    uint8_t m_reserved          : 2; // for the future
    NikStateFlags()
        :
        m_connected(0),
        m_lvEnabled(0),
        m_captureInProgress(0),
        m_stackActive(0),
        m_allFramesOK(0),
		m_preparedNextFrame(1)
        {}
};

// NikType003 handles focus stack operations for type0003
// Nikon cameras. This is currently known to include the D90,
// D5000 and D7000. Other Nikon DSLRs may or may not work.
class NikType003 : public PTP, public NKEventHandlers
{
    // PTP event check time interval in milliseconds.
    const uint32_t m_eventCheckInterval;
    // The time we will next check for events
    uint32_t m_nextEventCheckTime;
    // Contains flags for live view, capture states.
    NikStateFlags m_stateFlags;
    // Count of remaining frames when focus stacking
    uint16_t m_remainingFrames;
    // The amount of MFDrive to apply in order to restore
    // the original focus either due to cancel, or due
    // to the stack completing.
    int32_t m_restoreFocusDrive;
    // Distinguishes one Nikon product from another.
    uint16_t m_idProduct;
    // Delay in milliseconds between successive NK_OC_DeviceReady
    // transactions when waiting on the camera.
    uint16_t m_checkReadyInternal;
	// time since the last capture 
	uint32_t m_timeLastCapture;
public:
    NikType003(USB *usb, PTPStateHandlers *stateHandler);
	// PTP overrides
    virtual uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);
    virtual uint8_t Poll();
    uint16_t EventCheck(PTPReadParser *parser);

    // NKEventHandlers implementation
    virtual void OnEvent(const NKEvent *evt);
    // TODO: NKEventHandlers appears intended to be
    // a more complete class. See nkeventparser.h

    // NikType003 specific methods

    // Function: waitForReady()
    //      Spins on NK_OC_DeviceReady, until the camera
    //      returns PTP_RC_OK, or until maxAttempts.
    //  returns: a ptp response error code
    uint16_t waitForReady(uint16_t maxAttempts);

    // Function: enableLiveView()
    //      Changes live view state
    //  returns: a ptp response error code
    uint16_t enableLiveView(bool enable);

    // Function: captureToCard
    //      uses PTP_OC_Nikon_CaptureRecInMedia
    //      to capture to the camera memory card.
    //  returns: a ptp response error code
    uint16_t captureToCard();

    // Function: moveFocus
    //      Uses PTP_OC_NIKON_MfDrive to drive the
    //      camera focus. Note that live view must
    //      be enabled for this to work
    //  parameters:
    //      uint32_t direction:
    //          0x00000001: infinity -> near
    //          0x00000002: near -> infinity
    //      uint32_t amount:
    //          An opaque value within the range
    //          of 1 to 32767
    //  returns: a ptp response error code
    uint16_t moveFocus(uint32_t direction, uint32_t amount);

    // Function: focusStack
    //      Initiates a focus stack sequence, from the current
    //      focal distance, moving the focus g_pSetupHandler->getNumFrames()
    //      times, by g_pSetHandler->getDriveAmount().
    //  returns: a ptp response error code indicating what occurred
    //      while initiating the 1st frame.
    uint16_t startFocusStack();

	// Function: prepareNextFrame
	//      Handles moving the focus to the next point, along
	//      with all the LV on/off & wait checks.
	//  returns: a ptp response error code indicating what occurred
	//      
	uint16_t prepareNextFrame();
	
    // Function: focusStackNextFrame
    //      Advances focus by SetupHandler::getDriveAmount
    //      and shoots the next frame.
    //  returns: a ptp response error code indicating what occurred
    //      while shooting a frame
    uint16_t focusStackNextFrame();

    // Function: cancelFocusStack();
    //      Stops the focus stack op
    void cancelFocusStack();

    // Function: restoreOriginalFocus();
    //      attempts to put the focus back where it was when the
    //      focusStack op started
    //  returns: a ptp response error code indicating if the focus
    //      was successfully restored.
    uint16_t restoreOriginalFocus();

    // various accessors
    void setIsConnected(bool connected);
    bool isConnected() const;
    bool isLiveViewEnabled() const;
    bool isCaptureInProgress() const;
    bool isFocusStackActive() const;
	bool isNextFrameFocused() const;
    uint16_t getProductID() const;
	uint32_t getTimeLastCaptureStart() const;
	uint16_t getRemainingFrames() const;

};



