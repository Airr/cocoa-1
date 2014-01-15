/**
 * This file, alder_progress.c, is part of alder-progress.
 *
 * Copyright 2013 by Sang Chul Choi
 *
 * alder-progress is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * alder-progress is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with alder-progress.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  See the followings:
 *  http://stackoverflow.com/questions/1637587/c-libcurl-console-progress-bar
 *  http://stackoverflow.com/questions/3440101/c-libcurl-measure-download-speed-and-time-remaining
 *  http://www.chemie.fu-berlin.de/chemnet/use/info/libc/libc_17.html
 */

#include <stdio.h>
#include <time.h>

int alder_progress_step(unsigned long cstep, unsigned long tstep, int tostderr)
{
    static int tick = 0;
    static time_t start;
    static time_t end;
    if (tick == 0) {
        start = time(NULL);
    }
    tick++;
    int progress = (int)((double)(cstep) * 100
                         /
                         (double)(tstep));
    if (tostderr) {
        fprintf(stderr,"%d\n", progress);
        fflush(stderr);
    } else {
        end = time(NULL);
        double elapsed = difftime(end, start);
        double remaining = elapsed * (tstep / cstep - 1) / 3600.0;
        fprintf(stdout,"\rProgress: %d%% %.1f (hours)", progress, remaining);
        fflush(stdout);
    }
    return 0;
}

int alder_progress_end(int tostderr)
{
    if (tostderr) {
        fprintf(stderr, "100\n");
        fflush(stderr);
    } else {
        fprintf(stdout, "\r                                                 ");
        fflush(stdout);
        fprintf(stdout, "\rDone!\n");
    }
    return 0;
}
