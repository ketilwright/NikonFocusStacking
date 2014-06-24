/* 
* ReadAdaFruitLcdButtonState.cpp
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


#include "ReadAdaFruitLcdButtonState.h"
#include "Adafruit_RGBLCDShield.h"

ReadAdaFruitLcdButtonState::ReadAdaFruitLcdButtonState(Adafruit_RGBLCDShield *lcdShield)
	:
	m_lcdShield(lcdShield)
{}
ReadAdaFruitLcdButtonState::~ReadAdaFruitLcdButtonState()
{}
// ReadButtonState impl
bool ReadAdaFruitLcdButtonState::isPressed(uint8_t button)
{
	return 0 != (m_lcdShield->readButtons() & button);
}
void ReadAdaFruitLcdButtonState::setPinMode(uint8_t button, uint8_t mode)
{
	// nothing todo, the shield library handles this.
}