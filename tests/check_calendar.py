"""Compare the production C++ calendar with Python's independent date arithmetic.
Usage: python tests/check_calendar.py path/to/calendar_probe.exe
"""
from datetime import date, timedelta
from pathlib import Path
import subprocess, sys
dates=[]
day=date(1970,1,1)
while day<date(2100,1,1):
    dates.append(day)
    day+=timedelta(days=1)
raw=''.join(f'{d.year} {d.month} {d.day}\n' for d in dates)
invalid=[(2027,2,29),(2026,0,1),(2026,13,1),(2026,4,31),(2026,1,0)]
raw+=''.join(f'{y} {m} {d}\n' for y,m,d in invalid)
out=subprocess.run([str(Path(sys.argv[1]).resolve())],input=raw,text=True,capture_output=True,check=True).stdout.splitlines()
assert len(out)==len(dates)+len(invalid)
for day,result in zip(dates,out):
    semesters=[(1,date(day.year-1,9,1),date(day.year,2,9)),
               (2,date(day.year,2,10),date(day.year,7,20)),
               (1,date(day.year,9,1),date(day.year+1,2,9))]
    expected=(0,-1)
    for term,start,end in semesters:
        if start<=day<=end: expected=(term,(day-start).days//7+1)
    assert tuple(map(int,result.split()))==expected,(day,result,expected)
for result in out[len(dates):]: assert result=='0 -2'
print(f'PASS: {len(dates)} dates and {len(invalid)} invalid dates; production C++ calendar')
