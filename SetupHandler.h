/*
 * SetupHandler.h
 *
 * Created: 4/29/2014 11:38:34 AM
 *  Author: Ketil Wright
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

#include "MessageHandler.h"
#include "DisplayableParameter.h"
#include "Sketch.h"

// menu layout for setup
// Amt: MF drive amoumt
// Frm: number of frames to shoot
// Dl:  delay between frames (for flash recharge)
// Lf:  last fstop ( to support smoother DOF fall off in the focus stack)
// R:   return MF drive to initial position when done (not currently implented)

// 0 1 2 3 4 5 6 7 8 9 a b c d e f
// A m t     F r m   D l   L f   R
// 9 9 9 9   9 9 9   9 9   16    N

// column start/end for setup field
#define AmountFieldStartCol       0
#define AmountFieldEndCol         3
#define AmountFieldRow            1

#define FrameFieldStartCol        5
#define FrameFieldEndCol          7
#define FrameFieldRow	          1

#define DelayFieldStartCol        9
#define DelayFieldEndCol          10
#define DelayFieldRow	          1

#define LastFstopFieldStartCol    12
#define LastFstopFieldEndCol      13
#define LastFstopFieldRow         1

#define RestoreFocusFieldStartCol 15
#define RestoreFocusFieldEndCol   15
#define RestoreFocusFieldRow      1


#define maxApertures 13

class SetupHandler : public IMessageHandler
{
    static uint16_t m_apertures[maxApertures];
	DisplayableParameter m_driveAmount;
	DisplayableParameter m_numFrames;
	DisplayableParameter m_frameDelaySeconds;
    DisplayableParameter m_lastFStop;
	bool    m_restoreFocus;
	void updateRestoreFocusUI(int change = 0);
	void advanceCaret(uint8_t dir); // -1 = left, 1 right. All other values ignored
    uint16_t m_lastFrameFstopInx;
 public:
    SetupHandler(MessagePump *_pump, uint32_t driveAmount, uint32_t frames);
    uint32_t getDriveAmount() const { return m_driveAmount.getVal();}
    void setDriveAmount(uint32_t driveAmount) { m_driveAmount.setVal(driveAmount);}
    uint32_t getNumFrames() const { return m_numFrames.getVal(); }
    void setNumFrames(uint32_t numFrames) { m_numFrames.setVal(numFrames);}
	uint32_t getFrameDelayMilliseconds() const { return m_frameDelaySeconds.getVal() * 1000; }
	void setFrameDelaySeconds(uint32_t delay) { m_frameDelaySeconds.setVal(delay); }
    // bookend the stack with a smaller aperture for
    // more natural looking DOF falloff at the front
    // and back of the stack
    uint16_t getBookEndFstop() const 
    {
        uint16_t retVal = 1600;
        if(m_lastFrameFstopInx < maxApertures) 
            retVal = m_apertures[m_lastFrameFstopInx]; 
        return retVal;            
    }
	
	bool getRestoreFocus() const { return m_restoreFocus; }
	void setRestoreFocus(bool restore) { m_restoreFocus = restore; }
	
	
    ~SetupHandler();
    MsgResp processMessage(Msg& msg);
    virtual void show();
	friend class ManualHandler; // allow access to updateDriveAmountUI
};



