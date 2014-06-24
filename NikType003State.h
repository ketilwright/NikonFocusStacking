/*
 * NikType003State.h
 *
 * Created: 5/7/2014 8:56:43 AM
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

#include <ptp.h>
// Lets the rest of the program know whether a Nikon
// is connected and on.
class NikType003StateHandler : public PTPStateHandlers
{
    enum CamStates { stInitial, stDisconnected, stConnected };
    CamStates stateConnected;
public:
    NikType003StateHandler();
    virtual void OnDeviceDisconnectedState(PTP *nk3);
    virtual void OnDeviceInitializedState(PTP *nk3);
};

