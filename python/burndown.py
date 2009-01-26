#!/usr/bin/env python

"""Copyright (C) University of Oxford, 2005-2009

University of Oxford means the Chancellor, Masters and Scholars of the
University of Oxford, having an administrative office at Wellington
Square, Oxford OX1 2JD, UK.

This file is part of Chaste.

Chaste is free software: you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published
by the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Chaste is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details. The offer of Chaste under the terms of the
License is subject to the License being interpreted in accordance with
English Law and subject to any action against the University of Oxford
being under the jurisdiction of the English Courts.

You should have received a copy of the GNU Lesser General Public License
along with Chaste. If not, see <http://www.gnu.org/licenses/>.
"""
 
"""Generate a burndown plot for Chaste.

What we need to extract from Trac's database is, for each milestone,
the set of data points (time, total_effort).  We can then produce a
burndown plot of total_effort against time.

Tickets have an `effort` field which records estimated time remaining.
We need to sum the effort for all tickets in a milestone at each time
point.

The `ticket_change` table records all changes made to ticket fields,
and gives both old and new values.  It does not specify initial values
when the ticket was created, until the value changed, at which point
we can read its old value.

We thus also need to read the `time` column in the `ticket` table to
see when the ticket was opened, in order to add this initial value as
a data point.

An additional complication is that tickets can move between
milestones.  So we need to keep track of which tickets are in each
milestone at each point in time, by watching for when the `milestone`
field changes.
"""

import sqlite
#from pysqlite2 import dbapi2 as sqlite

# Connect to the trac database
con = sqlite.connect("/opt/tracdb/chaste/db/trac.db")

# Get ticket creation times and final milestones
cur = con.cursor()
cur.execute("select id,time,milestone from ticket")
creation_times = {}
milestones = {}
for (ticket, time, milestone) in cur:
    time = int(time)
    creation_times[ticket] = time
    if not ticket in milestones:
        milestones[ticket] = {}
    milestones[ticket][time] = milestone

# Find when tickets move between milestones
cur = con.cursor()
cur.execute("select ticket,time,oldvalue,newvalue from ticket_change "
            "where field=%s order by time asc", ('milestone',))
for (ticket, time, old, new) in cur:
    # If this is the first change, alter the initial milestone
    time = int(time)
    if len(milestones[ticket]) == 1:
        t0 = milestones[ticket].keys()[0]
        milestones[ticket][t0] = old
    # Record the new milestone
    milestones[ticket][time] = new

# milestones[ticket][time] now records the milestone `ticket` entered at `time`
def milestone(ticket, time):
    """Return the milestone `ticket` was in at `time`."""
    global milestones
    change_times = milestones[ticket].keys()
    change_times.sort()
    m = None
    for t in change_times:
        if t <= time:
            m = milestones[ticket][t]
    return m
def milestone_change_time(ticket, milestone):
    """Find when `ticket` entered `milestone`."""
    global milestones
    for time, m in milestones[ticket].iteritems():
        if m == milestone:
            return time

# Find when the effort estimates for tickets change
cur = con.cursor()
cur.execute("select ticket,time,oldvalue,newvalue from ticket_change "
            "where field=%s order by time asc", ('effort',))
effort = {}
last_milestone = {}
milestone_efforts = {}

def add_effort(milestone, time, effort_delta):
    """At the given time, change this milestone's total effort."""
    global milestone_efforts
    if not milestone in milestone_efforts:
        milestone_efforts[milestone] = [(time, effort_delta)]
        print "Starting", milestone, "with", effort_delta
    else:
        prev_effort = milestone_efforts[milestone][-1][1]
        #print "Adding", effort_delta, "to", milestone,"which was", prev_effort
        milestone_efforts[milestone].append((time, prev_effort + effort_delta))
def last_time(milestone):
    """Find the last time (so far) at which something happened in `milestone`."""
    global milestone_efforts
    t = 0
    for (time, effort) in milestone_efforts[milestone]:
        if time > t: t = time
    return t
def eff(effort_str):
    """Convert effort string to float.

    Various 'bad' strings are coped with: n? ?n n+ <n?
    """
    # Munge some bad strings to be good
    if effort_str and effort_str[0] in ['<', '?']:
        effort_str = effort_str[1:]
    if effort_str and effort_str[-1] in ['?', '+']:
        effort_str = effort_str[:-1]
    # Convert to float
    if effort_str:
        return float(effort_str)
    else:
        return 0.0

# Iterate over changes
for (ticket, time, old, new) in cur:
    try:
        old, new = eff(old), eff(new)
    except:
        print "Bad effort:", ticket, time, old, new
        continue
    time = int(time)
    if not ticket in last_milestone:
        # First time we've seen this ticket; record initial effort
        effort[ticket] = old
        t = creation_times[ticket]
        m = milestone(ticket, t)
        last_milestone[ticket] = m
        add_effort(m, t, old)
    # Record new effort
    m = milestone(ticket, time)
    if m != last_milestone[ticket]:
        # Ticket has moved milestone; transfer its effort over
        t = milestone_change_time(ticket, m)
        add_effort(m, t, old)
        add_effort(last_milestone[ticket], last_time(last_milestone[ticket]), -old)
        last_milestone[ticket] = m
    effort[ticket] = new
    add_effort(m, time, new - old)

# Now, for each milestone, milestone_efforts[milestone] is a list of
# the points we wish to plot, but unsorted.
def paircmp(p1, p2):
    """Compare 2 pairs according to their first elements."""
    return cmp(p1[0], p2[0])

# Print out a separate file per milestone, with 2 columns:
#   time_since_milestone_start, total_effort
ms = milestone_efforts.keys()
for m in ms:
    fp = open(m.replace(' ', '_') + '.csv', 'w')
    milestone_efforts[m].sort(paircmp)
    t0 = milestone_efforts[m][0][0]
    for t, e in milestone_efforts[m]:
        fp.write("%f, %f\n" % ((t-t0)/60.0/60/24, e))
    fp.close()
