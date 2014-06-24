/*
 * Main.cpp
 *
 * Created: 6/20/2014 9:22:00 AM
 *  Author: Ketil
 */


#ifndef SKETCH_H_
#define SKETCH_H_

#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef UTILITY_MACROS_H_
#include "Utility\UtilityMacros.h"
#endif

#ifndef BUILDINFO_H_
#include "Utility/Buildinfo.h"
#endif

/* Serial Speed */
const unsigned long SERIAL_SPEED = 9600;

void setup();
void loop();



#endif /* SKETCH_H_ */