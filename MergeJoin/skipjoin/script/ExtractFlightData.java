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
import java.util.Collections;
import java.util.List;
import java.util.Comparator;
import java.util.Calendar;
import java.nio.file.Files;
import java.nio.file.Paths;

class ExtractFlightData
{
    static class Pair
    {
        Pair(String year, String month, String day,
             String time, String duration)
        {
            int yeari = Integer.parseInt(year);
            int monthi = Integer.parseInt(month);
            int dayi = Integer.parseInt(day);
            int houri = Integer.parseInt(time.substring(0, 2));
            int mini = Integer.parseInt(time.substring(2));
            int duri = Integer.parseInt(duration);

            Calendar date = new Calendar.Builder()
                                        .setDate(yeari, monthi - 1, dayi)
                                        .setTimeOfDay(houri, mini, 0)
                                        .build();
            start = date.getTimeInMillis() / (60 * 1000);
            end = start + duri;
        }

        long start;
        long end;
    }
    
    public static void main(String[] args) throws Exception
    {
        if (args.length == 0) {
            System.exit(1);
        }

        ArrayList<Pair> events = new ArrayList<Pair>();
        for (int i = 0; i < args.length; ++i) {
            String fileName = args[i];
            List<String> lines = Files.readAllLines(Paths.get(fileName));

            System.err.println(fileName + "  " + lines.get(1));
            
            /* Skip over first line. */
            for (int j = 1; j < lines.size(); ++j) {
                String[] pieces = lines.get(j).split(",");
                if (pieces.length == 5) {
                    String year = pieces[0];
                    String month = pieces[1];
                    String day = pieces[2];
                    String deptime = pieces[3];
                    String airtime = pieces[4];

                    deptime = deptime.substring(1, deptime.length() - 1);
                    airtime = airtime.substring(0, airtime.length() - 3);
                    events.add(new Pair(year, month, day, deptime, airtime));
                }
            }
        }
        
        Collections.sort(events, new Comparator<Pair>() {
            public int compare(Pair lhs, Pair rhs)
            {
                if (lhs.start < rhs.start) {
                    return -1;
                }
                else if (lhs.start > rhs.start) {
                    return 1;
                }
                else if (lhs.end < rhs.end) {
                    return -1;
                }
                else if (lhs.end > rhs.end) {
                    return 1;
                }
                else {
                    return 0;
                }
            }
        });
        
        for (int i = 0; i < events.size(); ++i) {
            Pair event = events.get(i);
            System.out.println(event.start + " " + event.end);
        }
        
        System.err.println(events.size());
    }
}