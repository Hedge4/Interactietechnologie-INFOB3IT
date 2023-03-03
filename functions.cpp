#include "functions.h"


/////////////////////////////////////////
// TIMESTAMP/ROLLOVER CHECKER FUNCTION //
/////////////////////////////////////////

// 1 day in milliseconds, UL to prevent int overflow
unsigned long maxDifference = 1UL * 24 * 3600 * 1000;

// return if timestampA is later in time than timestampB, taking rollover of millis() into account
bool compareTimestamps(unsigned long timestampA, unsigned long timestampB) {
  /*
    If both or neither values rolled over, we have no problem comparing them. This means that we
    only have to check if one of the two values rolled over.

    To check if either value rolled over, we'll assume the delay will always be less than one day.
    We assume value X rolled over if "X < Y" and "X + D > Y + D" (D = a day in milliseconds)
  */

  // A only needs to be larger or equal to return true, B needs to be larger
  if (timestampA >= timestampB) {
    // A was larger/equal, check if B rolled over
    if (timestampB + maxDifference > timestampA + maxDifference) {
      // B rolled over, so A isn't actually later in time than B
      return false;
    } else {
      // B didn't roll over, so A is indeed later in time than B
      return true;
    }
  } else {
    // B was larger, check if A rolled over
    if (timestampA + maxDifference >= timestampB + maxDifference) {
      // A rolled over, so A is actually later in time than B
      return true;
    } else {
      // A didn't roll over, so A indeed isn't later in time than B
      return false;
    }
  }
}
