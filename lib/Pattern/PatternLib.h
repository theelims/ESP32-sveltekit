/**
 *
 * Copyright (C) 2025 theelims <elims@gmx.net>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef PATTERN_LIB_H
#define PATTERN_LIB_H

#include <Pattern.h>
#include <TeasingPounding.h>
#include <RoboStroke.h>
#include <HalfnHalf.h>
#include <Deeper.h>
#include <StopNGo.h>
#include <Insist.h>
#include <RandomDepth.h>
// <-- include your new pattern class here!

extern "C"
{
    static TeasingPounding teasingPounding("PoundingTeasing");
    static RoboStroke roboStroke("RoboStroke");
    static HalfnHalf halfnHalf("Half'n'Half");
    static Deeper deeper("Deeper");
    static StopNGo stopNGo("Stop'n'Go");
    static Insist insist("Insist");
    static RandomDepth randomDepth("RandomDepth");
    // <-- instantiate your new pattern class here!

    /**************************************************************************/
    /*
      Array holding all different patterns. Please include any custom pattern here.
    */
    /**************************************************************************/
    static Pattern *patternTable[] = {
        &teasingPounding,
        &roboStroke,
        &halfnHalf,
        &deeper,
        &stopNGo,
        &insist,
        &randomDepth
        // <-- insert your new pattern class here!
    };

    static const unsigned int patternTableSize = sizeof(patternTable) / sizeof(patternTable[0]);
}

#endif // PATTERN_LIB_H