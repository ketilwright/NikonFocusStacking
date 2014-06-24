/*
 * Handlers.h
 *
 * Created: 4/26/2014 9:09:40 AM
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

#include <stdint.h>
// Globally unique message types. This should
// tell us what 'happened', not where the message
// came from.
enum MsgType
{
    eButtonActionPress		= 0x00,
    eButtonActionRelease	= 0x01,
    eButtonActionHoldShort  = 0x02,
	eButtonActionHoldMedium = 0x03,
	eButtonActionHoldLong   = 0x04,
    eButtonActionNone	    = 0xff
};

// Globally unique message codes. This should
// tell us where a message originated, eg which
// button, not what 'happened'
enum MsgCode
{
    eLeft,
    eRight,
    eUp,
    eDown,
    eSelect,
    eUnknown
};

// Return codes from IMessageHandler::processMessage.
enum MsgResp
{
    eSuccess,
    eFail,
};

// Forward decls for message struct
class MessagePump;
class IMessageHandler;

// Msg is dispatched from the loop function, and possibly
// other places to the MessagePump.
struct Msg
{
    // press, release,
    MsgType m_type;
    // eg, which button was pressed.
    const MsgCode m_code;
    // Deposit the value of next handler here before returning from
    // IMessageHandler::processMessage to set make a new handler.
    // Otherwise, leave it null. Depositing this pointer will force
    // a complete refresh of the LCD.
    IMessageHandler *m_nextHandler;
    Msg(MsgType _type, MsgCode _code) : m_type(_type), m_code(_code), m_nextHandler(0){}
};


enum eCaretMoveDir
{
    eMoveDown = 0x00,
    eMoveUp   = 0x01,
    eMoveLeft = 0x02, // not currently implemented
    eMoveRight= 0x03  // not currently implemented
};


class IMessageHandler
{
    // TODO: consider just using g_pump.
    MessagePump *m_pump;
protected:
    // Derived classes should write menu text strings
    const char *menu[4];
    // The caret is displayed as a > in the first display
    // column beneath a menu item
    unsigned char m_caretCol, m_caretRow;
	void printMenuItem(uint8_t col, uint8_t item);
public:
    // Base class ctor just initializes OK on bottom line of menu[].
    IMessageHandler(MessagePump* _pump);
    // Must be implemented in derived classes.
    virtual MsgResp processMessage(Msg&) = 0;
    // Base class implementation just draws whatever is contained in
    // menu[]. All lines but the 1st are indented by one character.
    // The first column of each row is reserved for the caret.
    // Override this method to write any additional output that is required.
    // Sub classes should call IMessageHandler::show() if they override it,
    // in order for the caret to be displayed in the current location.
    virtual void show();
    // Moves the caret from one line to another in the 1st display column
    //void moveCaret(eCaretMoveDir);
	void moveCaret(uint8_t col, uint8_t row);
    // Hides or shows the caret
    void showCaret(bool);
    // Returns the current col location.
    unsigned char getCaretCol() const { return m_caretCol;}
    // Moves the caret row location.
    void setCaretCol(unsigned char col) { m_caretCol = col;}
		
		
};



