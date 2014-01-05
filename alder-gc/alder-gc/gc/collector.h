/* Collector.h - A simple garbage collector.
 * Copyright (C) 2011 Rob King <jking@deadpixi.com>
 *
 * This file is part of DPGC.
 *
 * DPGC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * DPGC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DPGC.  If not, see <http://www.gnu.org/licenses/>.
 */

typedef struct Collectee Collectee;
struct Collectee{
    Collectee   *prev;
    Collectee   *next;
    int         color;
    int         tag;
};

typedef struct Root Root;
struct Root{
    Root        *prev;
    Root        *next;
    Collectee   *root;
};

typedef struct Collector Collector;
typedef int    (*Marker)(Collector *, Collectee *);
typedef void   (*Disposer)(Collector *, Collectee *);
struct Collector{
    unsigned int    deferrals;
    unsigned int    pauses;
    unsigned int    count;

    Root            *roots;
    Collectee       *blacks;
    Collectee       *grays;
    Collectee       *whites;

    Marker          touch;
    Disposer        dispose;
};

Collector *
makecollector(Marker touch, Disposer dispose);

int
addroot(Collector *, Collectee *);

void
removeroot(Collector *, Collectee *);

void
addobj(Collector *, Collectee *, Collectee *);

void
touchobj(Collector *, Collectee *);

void
addref(Collector *, Collectee *, Collectee *);

void
stepgc(Collector *, unsigned int);

void *
gcalloc(Collector *, size_t);

void
pausegc(Collector *collector);

void
unpausegc(Collector *collector);
