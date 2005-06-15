// @(#)root/gl:$Name:  $:$Id: TGLStopwatch.cxx,v 1.3 2005/05/26 12:29:50 rdm Exp $
// Author:  Richard Maunder  25/05/2005

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

// TODO: Function descriptions
// TODO: Class def - same as header!!!

#include "TGLStopwatch.h"
#include "TGLIncludes.h"

#ifdef R__WIN32
#include <Windows.h>  // For GetSystemTimeAsFileTime()
#else
#include <sys/time.h> // For gettimeofday()
#endif

ClassImp(TGLStopwatch)

Bool_t   TGLStopwatch::fgInitOverhead = kFALSE;
Double_t TGLStopwatch::fgOverhead = 0.0;

//______________________________________________________________________________
TGLStopwatch::TGLStopwatch()
{
   if (!fgInitOverhead)
   {
      InitOverhead();
   }
}

//______________________________________________________________________________
TGLStopwatch::~TGLStopwatch()
{
}

//______________________________________________________________________________
void TGLStopwatch::Start()
{
   FinishDrawing();
   fStart = WaitForTick();
}

// In milliseconds
//______________________________________________________________________________
Double_t TGLStopwatch::Lap() const
{
   Double_t elapsed = GetClock() - fStart - fgOverhead;
   return elapsed > 0.0 ? elapsed : 0.0;
}

// In milliseconds
//______________________________________________________________________________
Double_t TGLStopwatch::End()
{
   FinishDrawing();
   return Lap();
}

// In milliseconds
//______________________________________________________________________________
Double_t TGLStopwatch::GetClock(void) const
{
#ifdef R__WIN32
   // TODO: Portability - check with Rene
   FILETIME        ft;
   ULARGE_INTEGER  uli;
   __int64         t;

   GetSystemTimeAsFileTime(&ft);
   uli.LowPart  = ft.dwLowDateTime;
   uli.HighPart = ft.dwHighDateTime;
   t  = uli.QuadPart;                        // 100-nanosecond resolution
   return static_cast<Double_t>(t /= 1E4);   // Milliseconds
#else
   struct timeval tv;
   gettimeofday(&tv, 0);
   return static_cast<Double_t>(tv.tv_sec*1E3) + static_cast<Double_t>(tv.tv_usec) / 1E3;
#endif
}

//______________________________________________________________________________
void TGLStopwatch::FinishDrawing(void) const
{
   glFinish();
}

//______________________________________________________________________________
Double_t TGLStopwatch::WaitForTick(void)  const
{
   Double_t start;
   Double_t current;

   start = GetClock();

   // Next tick
   while ((current = GetClock()) == start);

   return current;
}

//______________________________________________________________________________
void TGLStopwatch::InitOverhead(void) const
{
   Double_t runTime;
   Long_t   reps;
   Double_t start;
   Double_t finish;
   Double_t current;

   start = GetClock();

   // Next tick
   while ((finish = GetClock()) == start);

   // Test on 100 ticks range to 0.1 sec - 0.5 sec
   runTime = 100.0 * (finish - start);
   if (runTime < 100)
      runTime = 100;
   else if (runTime > 500)
      runTime = 1000;

   // Clear GL pipe
   FinishDrawing();

   // Overhead for finalization and timing routines
   reps = 0;
   start = WaitForTick();
   finish = start + runTime;
   do {
      FinishDrawing();
      ++reps;
   } while ((current = GetClock()) < finish);

   fgOverhead = (current - start) / (Double_t) reps;
   fgInitOverhead = true;
}
