#include <cstdio>
#include "../firmware/WeekClock_ST7789/Calendar.h"
int main() {
 int y,m,d;
 while(std::scanf("%d %d %d",&y,&m,&d)==3) {
 Study s=studyWeek(y,m,d);
 std::printf("%d %d\n",s.semester,s.week);
 }
}
