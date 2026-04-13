#pragma once
class Date
{
public:
    int year;
    int month;
    int day;
    //参加一个获取总天数的变量，来处理q之后输入日期显示一段时间内账目的情况
    int totalDays;
    Date() {};
    Date(int year, int month, int day);
    void show();
    int getDay();
    int getMonth();
    int getYear();
    static Date read();
    bool operator<(const Date& other) const;
    bool operator==(const Date& other) const;
    bool operator<=(const Date& other) const;
    bool operator>(const Date& other) const;
    bool operator>=(const Date& other) const;
    bool operator!=(const Date& other) const;

    //判断是否是闰年
    static bool isLeapYear(int year)
    {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            return true;
        }
        return false;
    }

    //获取天数
    static int getDays(int year, int month)
    {
        if (month == 2)
        {
            return isLeapYear(year) ? 29 : 28;
        }
        if (month == 4 || month == 6 || month == 9 || month == 11)
        {
            return 30;
        }
        return 31;
    }

    //计算两个日期之间的天数差
    static int dateDif(const Date& start, const Date& end,bool skip=false)
    {
        int days = 0;

        //如果是同一年
        if (start.year == end.year)
        {
            if (start.month == end.month)
            {
                days = end.day - start.day;
            }
            else
            {
                //起始月剩余的天数
                days += Date::getDays(start.year, start.month) - start.day;
                //中间月的天数
                for (int month = start.month + 1; month < end.month; ++month)
                {
                    days += Date::getDays(start.year, month);
                }
                //结束月的天数
                days += end.day;
            }
        }
        else//如果不是
        {
            days += Date::getDays(start.year, start.month) - start.day;
            for (int month = start.month + 1; month <= 12; ++month)
            {
                days += Date::getDays(start.year, month);
            }
            for (int year = start.year + 1; year < end.year; ++year)
            {
                days += isLeapYear(year) ? 366 : 365;
            }
            for (int month = 1; month < end.month; ++month)
            {
                days += Date::getDays(end.year, month);
            }
            days += end.day;
        }

        if (skip)//存入当天不计利息
        {
            days -= 1;
        }
        return days;
    }

    int getMaxDay()
    {
        if (isLeapYear(year) && month == 2)
        {
            return 29;
        }
        else
        {
            return getDays(year, month);
        }
    }
};


