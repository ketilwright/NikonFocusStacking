/* 
* ReadAdaFruitLcdButtonState.h
*
* Created: 5/24/2014 1:29:33 PM
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
//#include "Sketch.h"
#include <stdint.h>
#include "ReadButtonState.h"

class Adafruit_RGBLCDShield;
class ReadAdaFruitLcdButtonState : public ReadButtonState
{
	// using a ref generates a warning about ignoring -fpack-struct for non POD types.
	Adafruit_RGBLCDShield *m_lcdShield;
public:
	ReadAdaFruitLcdButtonState(Adafruit_RGBLCDShield *lcdShield);
	~ReadAdaFruitLcdButtonState();
	// ReadButtonState impl
	virtual bool isPressed(uint8_t button);
	virtual void setPinMode(uint8_t button, uint8_t mode);
	
}; 

