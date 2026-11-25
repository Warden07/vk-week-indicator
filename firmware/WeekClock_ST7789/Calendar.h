#pragma once
struct Study { int semester; int week; };
inline bool leap(int y) { return y%4==0 && (y%100!=0 || y%400==0); }
inline int monthDays(int y,int m) {
  const int d[]={31,28,31,30,31,30,31,31,30,31,30,31};
  return m>=1 && m<=12 ? d[m-1]+(m==2 && leap(y)) : 0;
}
inline int ordinal(int y,int m,int d) {
  int n=d; for(int i=1;i<m;++i) n+=monthDays(y,i); return n;
}
inline Study studyWeek(int y,int m,int d) {
  if(y<1970 || y>2099 || m<1 || m>12 || d<1 || d>monthDays(y,m)) return {0,-2};
  if(m>=9) return {1,(ordinal(y,m,d)-ordinal(y,9,1))/7+1};
  if(m==1 || (m==2 && d<=9))
    return {1,((leap(y-1)?366:365)-ordinal(y-1,9,1)+ordinal(y,m,d))/7+1};
  if(m<7 || (m==7 && d<=20)) return {2,(ordinal(y,m,d)-ordinal(y,2,10))/7+1};
  return {0,-1};
}
