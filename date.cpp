//小细节：修改Date类
//需要对现有Date类作如下改动： 为了将Date类型的数据做为键，需要为Date重载“ < ”运算符，以支持键值的比较。
//bool operator < (const Date& date) const {
//}
#include"date.h"
#include<string>
#include<iostream>
using namespace std;
Date::Date(int pyear, int pmonth, int pday) :year(pyear), month(pmonth), day(pday)
{
    //完善日期错误解决方案
    if (month < 1 || month>12)
    {
        //抛出异常
        throw std::runtime_error("Invalid month: " + std::to_string(month));
    }

    if (day <= 0 || day > getMaxDay())
    {
        throw std::runtime_error("Invalid day: " + std::to_string(day) + " month" + std::to_string(month));
    }


    int years = year - 1;//排除当前年
    //假设每一年都是365天  总天数=年*365+每四年一个闰年 加闰日 世纪年不是闰年要减去 但是能被400整除的世纪年是闰年 要加回来
    //加上当前年1月1日到month年1月1日的天数 再加上当前月已经过了的天数
    totalDays = years * 365 + years / 4 - years / 100 + years / 400 + getDays(year, month - 1) + day;
    if (isLeapYear(year) && month > 2)//如果当前年是闰年并且已经过了2月 要加上1天
    {
        totalDays++;
    }
    /*if (month > 12 )
    {
        throw month;
    }
    if (day > 31)
    {
        throw day;
    }*/
}
void Date::show()
{
    cout << year << "-" << month << "-" << day;
}
int Date::getDay()
{
    return day;
}
int Date::getMonth()
{
    return month;
}
int Date::getYear()
{
    return year;
}
Date Date::read()
{
    //读取账目时间
    int year, month, day;
    char c1, c2;
    cin >> year >> c1 >> month >> c2 >> day;
    return Date(year, month, day);
}


//运算符重载 使得date可以用> < =这些运算符直接比较
bool Date::operator<(const Date& other) const
{
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
}

//比较date是否相等
bool Date::operator==(const Date& other) const
{
    return year == other.year && month == other.month && day == other.day;
}

//date之间的<=情况
bool Date::operator<=(const Date& other) const
{
    return *this < other || *this == other;
}

//>情况
bool Date::operator>(const Date& other) const
{
    return !(*this <= other);
}

//>=情况
bool Date::operator>=(const Date& other) const
{
    return !(*this < other);
}

//date的不相等!=情况
bool Date::operator!=(const Date& other) const
{
    return !(*this == other);
}


