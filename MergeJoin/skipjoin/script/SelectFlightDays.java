/**
 *
 * Copyright (c) 2017 Jelle Hellings.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY JELLE HELLINGS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
import java.util.ArrayList;
import java.util.Calendar;

class SelectFlightDays
{
    static class Pair
    {
        Pair(int year, int month)
        {
            Calendar date = new Calendar.Builder()
                                        .setDate(year, month - 1, 7)
                                        .build();
            start = date.getTimeInMillis() / (1000 * 60);
            end = start + (24 * 60);
        }

        long start;
        long end;
    }

    public static void main(String[] args) throws Exception
    {
        if (args.length != 0) {
            System.exit(1);
        }

        ArrayList<Pair> events = new ArrayList<Pair>();
        for (int month = 7; month <= 12; ++month) {
            events.add(new Pair(2007, month));
        }
        for (int year = 2008; year <= 2016; ++year) {
            for (int month = 1; month <= 12; ++month) {
                events.add(new Pair(year, month));
            }
        }
        for (int month = 1; month <= 6; ++month) {
            events.add(new Pair(2017, month));
        }
        
        for (int i = 0; i < events.size(); ++i) {
            Pair event = events.get(i);
            System.out.println(event.start + " " + event.end);
        }
        
        System.err.println(events.size());
    }
}