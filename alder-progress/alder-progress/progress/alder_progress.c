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

static int tick = 0;
static int tick_frequency = 10;
static int prev_progress = -1;
static time_t start;
static time_t end;

void alder_progress_start(int n_frequency)
{
    start = time(NULL);
    tick_frequency = n_frequency;
}

void alder_progress_frequency(int n_frequency)
{
    tick_frequency = n_frequency;
}

int alder_progress_step(unsigned long cstep, unsigned long tstep, int tostderr)
{
    tick++;
    if (tick % tick_frequency == 0) {
        int progress = (int)((double)(cstep) * 100.0
                             /
                             (double)(tstep));
        if (prev_progress < progress) {
            if (tostderr) {
                fprintf(stderr,"%d\n", progress);
                fflush(stderr);
            } else if (cstep > 0) {
                end = time(NULL);
                double elapsed = difftime(end, start);
                double remaining = elapsed * (tstep / cstep - 1) / 3600.0;
                fprintf(stdout,"\rProgress: %d%% %.1f (hours)", progress, remaining);
                fflush(stdout);
            }
        }
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
