/*
* Author : Avikalp Srivastava
*/

/*********************************** C++ Headers ************************************/
#include "Date.h"
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <cmath>
#include <stdexcept>
#include <stdlib.h>

using namespace std;


/***************************** Misc Functions *****************************/

// Initializing static member 'format' of Date class with the default constructor
// dayFormat = "dd", monthFormat = "mmm", yearFormat = "yy"
DateFormat Date::format;


/*    Name: findDashPosition_DateFormat()
      Input: pos[] - integer array with size >= 2, format - const char* depicting DateFormat
      Output: None.
      Description: The pos array is assigned the 0-index positions of the two '-' in format.
                   Assigned -1 if no '-' or less than two '-' are there.
*/
void findDashPosition_DateFormat(int pos[],const char* format)
{
    // Let dash_pos point to first occurence of '-' in format
    char* dash_pos = strchr(const_cast<char *>(format), '-');
    // If no '-' found, return pos[0] and pos[1] as -1
    if(dash_pos == NULL )
    {
        pos[0] = pos[1] = -1;
        return;
    }
    // The index of first '-' is given by dash_pos - format
    pos[0] = dash_pos - format;

    // Repeating for second '-'
    dash_pos = strchr(dash_pos+1, '-');
    // Second '-' must be present, else return pos[0] and pos[1] as -1
    if(dash_pos == NULL )
    {
        pos[0] = pos[1] = -1;
        return;
    }
    pos[1] = dash_pos - format;
}

/*    Name: isLeapYear()
      Input: enum Year / unsigned int year
      Output: True - if year is leap year. False if not.
      Description: Returns False if year is not divisible by 4 or is divisible by 100 but not 400.
*/
bool isLeapYear(Year year)
{
    int y = year;

    if( (y%4 != 0) || (y%100 == 0 && y%400 != 0) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*    Name: dateHasInvalidArgs
      Input: enum Day, Month and Year
      Output: True - if day and month parameters are within valid range. False if not.
      Description: Returns True if given date has valid args for month and Day.
*/
bool dateHasInvalidArgs(Day day, Month month, Year year)
{
    int d = day, m = month;

    // For easy changes, expansion, re-usability
    int minDay = 1, maxDay = 31;
    int minMonth = 1, maxMonth = 12;

    if(d < minDay || d > maxDay)
    {
        return true;
    }
    else if(m < minMonth || m > maxMonth)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*    Name: isDateValid
      Input: enum Day, Month, Year
      Output: True - if the give date is valid. False if not.
      Description: Checks that for a given Month, Day < max days in that Month.
                   Robust check for leap year condition.
*/
bool isDateValid(Day day, Month month, Year year)
{
    int d = day, m = month, y = year;
    int monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    // If given year a leap year, change number of days in Feb to 29
    if(isLeapYear(y))
    {
        monthDays[2] = 29;
    }
    // Checking that Day > max days in that Month should return false.
    // d < 1 for unseen future expansion - robustness
    if(d > monthDays[m] || d < 1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*    Name: isDateInRange
      Input: enum Day, Month, Year
      Output: True - if the give date is in given range. False if not.
      Description: Checks whether the given date lies in range or not
*/
bool isDateInRange(Day day, Month month, Year year)
{

    int d = day, m = month, y = year;

    // For easy changes, expansion, re-usability
    int minDay =  1, minMonth =  1, minYear = 1950;
    int maxDay = 31, maxMonth = 12, maxYear = 2049;

    // Checking for lower range.
    if( (y < minYear) || (y == minYear && m < minMonth) || (y == minYear && m == minMonth && d < minDay) )
    {
        return false;
    }
    // Checking for upper range
    else if( (y > maxYear) || (y == maxYear && m > maxMonth) || (y == maxYear && m == maxMonth && d > maxDay) )
    {
        return false;
    }
    // Else, Date is with the range, return true
    else
    {
        return true;
    }
}

/*    Name: dayOfWeek
      Input: int Day, Month, Year
      Output: An integer in the range 1 to 7. 1 -> Monday, 7 -> Sunday
      Description: Uses standard week day finding algorithm
      Source : GeeksForGeeks. Code ReUse
*/
int dayOfWeek(int d, int m, int y)
{
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
    y -= m < 3;
    int ans = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
    return (ans == 0) ? ans + 7 : ans;
}

/*    Name: countLeapDays
      Input: enum Month, Year
      Output: A positive integer
      Description: Returns number of leap / extra days passed since beginning of time 0/0/00
*/
int countLeapDays(Month m,Year y)
{
    int month = m, year = y;
    // If the year is leap year but month is <= 2, leap day has not passed.
    if (month <= 2)
    {
        year--;
    }
    // Num of leap days = Years divisible by 4 or 400 but not 100
    return  year / 4 - year / 100 + year / 400;
}

/*    Name: getNumberDaysPassed
      Input: enum Day, Month, Year
      Output: A positive long long integer
      Description: Returns number of days passed since beginning of time 0/0/00 till given Date
*/
long long int getNumberDaysPassed(Day d, Month m, Year y)
{
    int day = d, month = m, year = y;
    int monthDays[13] = {0, 31, 28, 31, 30, 31, 30,31, 31, 30, 31, 30, 31};
    // Adding days contributed by y and d
	long long int days = (static_cast<long long int>(year))*365 + static_cast<long long int>(day);
    // Adding days contributed by m
    while(--month)
    {
        days = days + monthDays[month];
    }
    // Adding days contributed by leap / extra days
    days = days + countLeapDays(m, y);
 	return days;
}

/*    Name: addDays
      Input: int numDays <= 365 for normal years and 33 for leap, address of day and month; and year of date.
      Output: None
      Description: For a year, assigns the day and month that occur after numDays
*/
void addDays(int numDays, int& day, int& month, int year)
{
	int monthDays[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	// If year is leap, changing monthDays of Feb to 29
	if(isLeapYear(static_cast<Year>(year)))
    {
        monthDays[2] = 29;
    }

	for(int i = 1; i <= 12; ++i)
    {
		if(numDays > monthDays[i])
		{
			numDays = numDays - monthDays[i];
		}
        else
        {
            day = numDays;
			month = i;
			return;
		}
	}
}

/*    Name: getDateFromDays
      Input: long long int numDays, and an integer array infoArr of size >= 3
      Output: None
      Description: Given number of days passed since 0/0/00, finds the date
                   and stores day in infoArr[0], month in infoArr[1], year in infoArr[2]
                   Used by : + and - operators
*/
void getDateFromDays(long long int numDays, int infoArr[])
{
    // Gets a rough approximate of the year
	int tempYear = numDays / 365;
	// Additional number of days
	numDays = numDays % 365;

    // Subtracting leap days (due to years with 366 days)
	int numLeapDays = countLeapDays(Jan,tempYear);
	numDays = numDays - numLeapDays;

    // It may be the case that numDays = numDays - numLeapDays is negative
    // Then, the actual year is lesser than our rough approximate
    while(numDays <= 0)
    {
        // Since numDays is -ve, we will do tempYear-- and add 365 / 366 to numDays.
        numDays = numDays + 365;
        if(isLeapYear(tempYear-1))
        {
            numDays+=1;
        }
        // Going one year back
        --tempYear;
    }
    // Now, numDays is a number in range 1 to 366 and tempYear is the exact year
    // So, assigning infoArr[2] as tempYear.
    infoArr[2] = tempYear;

    // Getting day and month by passing infoArr[0] & infoArr[1] as references
	addDays(numDays,infoArr[0], infoArr[1], infoArr[2]);
}

/*    Name: getFirstMondayDate
      Input: enum year
      Output: A Date type
      Description: Takes an year input and outputs a date corresponding to the Monday of its Week Number 1
                   Uses the fact that this 'first Monday' can only lie b/w 29 Dec and 4 Jan
                   Used by : Week Number cast operator
*/
Date getFirstMondayDate(Year year)
{
    int y = year;

    // finding b/w 29 Dec and 31 Dec
    for(int day = 29; day <= 31; day++)
    {
        // Check if its Monday
        if(dayOfWeek(day, 12, y-1) == 1)
        {
            Date firstMondayDate(static_cast<Day>(day), static_cast<Month>(12), static_cast<Year>(y-1));
            return firstMondayDate;
        }
    }
    // finding b/w 1 Jan and 4 Jan
    for(int day = 1; day <= 4; day++)
    {
        if(dayOfWeek(day, 1, y) == 1)
        {
            Date firstMondayDate(static_cast<Day>(day), static_cast<Month>(1), static_cast<Year>(y));
            return firstMondayDate;
        }
    }

    // For robustness
    Date default_date;
    return (default_date);
}


/***************************** CLASS Members Declarations *****************************/

// CONSTRUCTORS
DateFormat::DateFormat(const char* dateFormat,
                       const char* monthFormat,
                       const char* yearFormat )
{
    // Checking dateFormat
    if(dateFormat == NULL)
    {
        this->dateFormat = NULL;
    }
    else if(strcmp(dateFormat, "d") == 0 || strcmp(dateFormat,"dd") == 0)
    {
        this->dateFormat = new char[strlen(dateFormat)+1];
        strcpy(this->dateFormat, dateFormat);
    }
    // Argument should be 'd' or 'dd' , otherwise throw invalid argument
    else
    {
        throw invalid_argument("Exception Raised. The argument day provided for Date Format is wrong.");
    }

    // Checking monthFormat
    if(monthFormat == NULL)
    {
        this->monthFormat = NULL;
    }
    else if(strcmp(monthFormat, "m") == 0 || strcmp(monthFormat, "mm") == 0 || strcmp(monthFormat, "mmm") == 0)
    {
        this->monthFormat = new char[strlen(monthFormat) + 1];
        strcpy(this->monthFormat, monthFormat);
    }
    else
    {
       throw invalid_argument("Exception Raised. The argument month provided for Date Format is wrong.");
    }

    // Checking yearFormat
    if(yearFormat == NULL)
    {
        this->yearFormat = NULL;
    }
    else if(strcmp(yearFormat, "yy") == 0 || strcmp(yearFormat, "yyyy") == 0)
    {
        this->yearFormat = new char[strlen(yearFormat) + 1];
        strcpy(this->yearFormat, yearFormat);
    }
    else
    {
       throw invalid_argument("Exception Raised. The argument year provided for Date Format is wrong.");
    }
}

DateFormat::DateFormat(const char* format)
{
    // Array to store position of the 2 '-' in format
    // IMPORTANT : A valid format must have 2 '-' . For null format, use 0-0-0
    int dash_pos[2];
    // Function to aptly populate dash_pos[]
    findDashPosition_DateFormat(dash_pos, format);

    // If - is the first character
    // "-mm-yyyy" etc indicate that dateFormat is to be NULL
    if(dash_pos[0] == 0)
    {
        this->dateFormat = NULL;
    }
    // Else if there is one character before '-' , it should be '0' or 'd'
    else if(dash_pos[0] == 1)
    {
        if(format[0] == 'd')
        {
            dateFormat = new char[2];
            strcpy(dateFormat, "d");
        }
        else
        {
            throw invalid_argument("Exception Raised. The argument day provided for Date Format is wrong.");
        }
    }
    // Else if 2 chars before '-', they should be "dd"
    else if(dash_pos[0] == 2)
    {
        if(format[0] == 'd' && format[1] == 'd')
        {
            dateFormat = new char[3];
            strcpy(dateFormat, "dd");
        }
        else
        {
            throw invalid_argument("Exception Raised. The argument day provided for Date Format is wrong.");
        }
    }
    // dash_pos[0] other than 1 and 2 are invalid
    else
    {
        throw invalid_argument("Exception Raised. The argument day provided for Date Format is wrong.");
    }

    // Again, dd--yy etc indicate that monthFormat is invalid
    if(dash_pos[1] - dash_pos[0] - 1 == 0)
    {
        this->monthFormat = NULL;
    }
    // For 1 char b/w first and second '-', it should be '0' or 'm'
    else if (dash_pos[1] - dash_pos[0] - 1 == 1)
    {
        if(format[dash_pos[0]+1] == 'm')
        {
            monthFormat = new char[2];
            strcpy(monthFormat, "m");
        }
        else
        {
           // throw invalid_argument("The argument " + format + " provided for Date Format is wrong.");
            throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
        }
    }
    // For 2 chars, should be 'mm'
    else if (dash_pos[1] - dash_pos[0] - 1 == 2)
    {
        if(format[dash_pos[0]+1] == 'm' && format[dash_pos[0]+2] == 'm')
        {
            monthFormat = new char[3];
            strcpy(monthFormat, "mm");
        }
        else
        {
           // throw invalid_argument("The argument " + format + " provided for Date Format is wrong.");
            throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
        }
    }
    // for three, should be 'mmm'
    else if (dash_pos[1] - dash_pos[0] - 1 == 3)
    {
        if(format[dash_pos[0]+1] == 'm' && format[dash_pos[0]+2] == 'm' && format[dash_pos[0]+3] == 'm')
        {
            monthFormat = new char[4];
            strcpy(monthFormat, "mmm");
        }
        else
        {
            throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
        }
    }
    else
    {
        throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
    }

    // Checking yearFormat the same way. dd-mm- indicate null yearFormat
    if(strlen(format) - dash_pos[1] - 1 == 0)
    {
        this->yearFormat = NULL;
    }
    else if (strlen(format) - dash_pos[1] - 1 == 2)
    {
        if(strcmp("yy", format + dash_pos[1] + 1) == 0)
        {
            yearFormat = new char[3];
            strcpy(yearFormat, "yy");
        }
        else
        {
            throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
        }
    }
    else if (strlen(format) - dash_pos[1] - 1 == 4)
    {
        if(strcmp("yyyy", format + dash_pos[1] + 1) == 0)
        {
            yearFormat = new char[5];
            strcpy(yearFormat, "yyyy");
        }
        else
        {
            throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
        }
    }
    else
    {
        throw invalid_argument("Exception Raised. The argument provided for Date Format is wrong.");
    }

}

// Default DateFormat CONSTRUCTOR
// Uses initializer list to assign the format "dd-mmm-yy"
DateFormat::DateFormat()
{
    dateFormat = new char[3];
    strcpy(dateFormat, "dd");
    monthFormat = new char[4];
    strcpy(monthFormat, "mmm");
    yearFormat = new char[3];
    strcpy(yearFormat, "yy");
}

// DateFormat DESTRUCTOR : deletes the memory allocated to the char* attributes
DateFormat::~DateFormat()
{
    delete[] dateFormat;
    delete[] monthFormat;
    delete[] yearFormat;
}

// ASSIGNMENT OPERATOR
// Till none of the format is null, assign apt memory to this->dateFormat, monthFormat, yearFormat
DateFormat& DateFormat::operator=(const DateFormat& newDF)
{

    if(newDF.dateFormat != NULL)
    {
        dateFormat = new char[strlen(newDF.dateFormat)+1];
        strcpy(dateFormat, newDF.dateFormat);
    }
    else
    {
        dateFormat = NULL;
    }
    if(newDF.monthFormat != NULL)
    {
        monthFormat = new char[strlen(newDF.monthFormat)+1];
        strcpy(monthFormat, newDF.monthFormat);
    }
    else
    {
        monthFormat = NULL;
    }
    if(newDF.yearFormat != NULL)
    {
        yearFormat = new char[strlen(newDF.yearFormat)+1];
        strcpy(yearFormat, newDF.yearFormat);
    }
    else
    {
        yearFormat = NULL;
    }
    return *this;
}

// Date CONSTRUCTOR
Date::Date(Day d, Month m, Year y)
            throw (invalid_argument,
                   domain_error,
                   out_of_range)
{
    // Checking for all exceptions
    if(dateHasInvalidArgs(d, m, y))
    {
        // Constructing Error Message string for Exception
        string errorMessage = "Exception Raised. Invalid Argument error on Date";
        throw invalid_argument(errorMessage);
    }

    if(!isDateValid(d, m, y))
    {
        string errorMessage = "Exception Raised. Domain Error on Date";
        throw domain_error(errorMessage);
    }

    if(!isDateInRange(d, m, y))
    {
        string errorMessage = "Exception Raised. Out of Range Error on Date" ;
        throw out_of_range(errorMessage);
    }

    // If none of the exception is thrown, assigning attributes to data members
    this->d = d;
    this->m = m;
    this->y = y;

}

// Date CONSTRUCTOR
Date::Date(const char* date)
        throw(invalid_argument, domain_error, out_of_range)
{
    if(Date::getDateFormat() == NULL || Date::getMonthFormat() == NULL || strcmp(Date::getMonthFormat(), "mmm") == 0 || Date::getYearFormat() == NULL)
    {
        throw invalid_argument("Exception Raised. The current format does not support input.");
    }

    char *dayFormat;
    char *monthFormat;
    char* yearFormat;
    // Variable defining maximum year in the 2000-2100 range
    // Made for future changes / expansion / easy re-usability
    int maxIn20 = 49;

    int day, month, year;
    // Finding '-' positions, same as in DateFormat constructor
    int pos[2];
    findDashPosition_DateFormat(pos, date);

    // Checking for invalid positions of '-'
    if( pos[0] == -1 || pos[0] == 0 || pos[1] - pos[0] - 1 <= 0 || !(strlen(date) - pos[1] - 1 == 2 || strlen(date) - pos[1] - 1 == 4) )
    {
        throw invalid_argument("Exception Raised. The format of input date string is invalid");
    }

    // Fine checking on valid positions of '-'
    if(pos[0] == 1)
    {
        if(isdigit(date[0]))
        {
            day = date[0] - '0';
            dayFormat = new char[2];
            strcpy(dayFormat, "d");
        }
        else
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    }
    if(pos[0] == 2)
    {
        if(isdigit(date[0]) && isdigit(date[1]))
        {
            day = (date[0] - '0')*10 + date[1] - '0';
            if(date[0] == '0' || strcmp(Date::getDateFormat(), "dd") == 0)
            {
                dayFormat = new char[3];
                strcpy(dayFormat, "dd");
            }
            else
            {
                dayFormat = new char[2];
                strcpy(dayFormat, "d");
            }
        }
        else
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    }
    else
    {
        throw invalid_argument("Exception Raised. Invalid Argument error on Date");
    }

    if(pos[1] - pos[0] - 1 == 1)
    {
        if(isdigit(date[pos[0]+1]))
        {
            month = date[pos[0]+1] - '0';
            monthFormat = new char[2];
            strcpy(monthFormat, "m");
        }
    }
    else if(pos[1] - pos[0] - 1 == 2)
    {
        if(isdigit(date[pos[0]+1]) && isdigit(date[pos[0]+2]))
        {
            month = (date[pos[0]+1] - '0')*10 + date[pos[0]+2] - '0';
            if(date[pos[0]+1] == '0' || strcmp(Date::getMonthFormat(), "mm") == 0)
            {
                monthFormat = new char[3];
                strcpy(monthFormat, "mm");
            }
            else
            {
                monthFormat = new char[2];
                strcpy(monthFormat, "m");
            }

        }
        else
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    }
    // Below commented out code can help expand class to accept inputs with mmm as well.
  /*  else if(pos[1] - pos[0] - 1 == 3)
    {
        string monthShortName[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
        // Flag to be set true if one the short month names is found in the string
        bool flag = false;

        for(int i = 0; i < 12; i++)
        {
            if(strncmp((monthShortName[i]).c_str(), date + pos[0] + 1, 3) == 0)
            {
                month = i+1;
                flag = true;
                monthFormat = new char[4];
                strcpy(monthFormat, "mmm");
                break;
            }
        }
        if(flag == false)
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    } */

    else
    {
        throw invalid_argument("Exception Raised. Invalid Argument error on Date");
    }

    if(strlen(date) - pos[1] - 1 == 2)
    {
        if( isdigit(date[pos[1]+1]) && isdigit(date[pos[1]+2]) )
        {
            year =  (date[pos[1]+1] - '0')*10 + date[pos[1]+2] - '0';
            // Deciding b/w 21st and 20th century year
            if(year <= maxIn20)
                year += 2000;
            else
                year += 1900;

            yearFormat = new char[3];
            strcpy(yearFormat, "yy");
        }
        else
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    }

    else if(strlen(date) - pos[1] - 1 == 4)
    {
        if(isdigit(date[pos[1]+1]) && isdigit(date[pos[1]+2]) && isdigit(date[pos[1]+3]) && isdigit(date[pos[1]+4]))
        {
            year =  (date[pos[1]+1] - '0')*1000 + (date[pos[1]+2] - '0')*100 + (date[pos[1]+3] - '0')*10 + (date[pos[1]+4] - '0') ;
            yearFormat = new char[5];
            strcpy(yearFormat, "yyyy");
        }
        else
        {
            throw invalid_argument("Exception Raised. Invalid Argument error on Date");
        }
    }

    else
    {
        throw invalid_argument("Exception Raised. Invalid Argument error on Date");
    }

    // If the input doesn't match the static member format, raise an invalid argument exception
    if(!(strcmp(dayFormat, Date::getDateFormat())==0 && strcmp(monthFormat, Date::getMonthFormat()) == 0 && strcmp(yearFormat, Date::getYearFormat()) == 0))
    {
        throw invalid_argument("Exception Raised. Format doesn't match the static member format.");
        cout << "Entered Format : " << dayFormat << " " << monthFormat << " " << yearFormat << endl;
        cout << "Expected Format : " << Date::getDateFormat() << " " << Date::getMonthFormat() << " " << Date::getYearFormat() << endl;
    }
    // If exception not thrown till now, we have valid date format
    // Converting to enums.
    Day eday = static_cast<Day>(day);
    Month emonth = static_cast<Month>(month);
    Year eyear = static_cast<Year>(year);

    // Checking for exception in the 'to-be-formed' date
    if(dateHasInvalidArgs(eday, emonth, eyear))
    {
        throw invalid_argument("Exception Raised. Invalid Argument error on Date");
    }
    if(!isDateValid(eday, emonth, eyear))
    {
        throw domain_error("Exception Raised. Domain Error on Date");
    }
    if(!isDateInRange(eday, emonth, eyear))
    {
        throw out_of_range("Exception Raised. Out of Range error on Date");
    }
    // No Exception. Assigning attributes
    this->d = eday;
    this->m = emonth;
    this->y = eyear;
}


// Default Constructor
Date::Date()
    throw(domain_error, out_of_range)
{
    // Getting today's date from system
    // Code ReUse from GeeksForGeeks
    time_t t = time(0);
    struct tm * now = localtime( & t );
    int year  = now->tm_year + 1900,
        month = now->tm_mon + 1,
        day  = now->tm_mday;

    Day eDay = static_cast<Day>(day);
    Month eMonth = static_cast<Month>(month);
    Year eYear = static_cast<Year>(year);

    if(!isDateValid(eDay, eMonth, eYear))
    {
        // string errorMessage = "Exception Raised. Domain Error on date : " + to_string(d) + "-" + to_string(m) + "-" + to_string(y);
        string errorMessage = "Exception Raised. Domain Error on Today's Date (Def Constructor)";
        throw domain_error(errorMessage);
    }

    else if(!isDateInRange(eDay, eMonth, eYear))
    {
        // string errorMessage = "Exception Raised. Out of Range Error on date : " + to_string(d) + "-" + to_string(m) + "-" + to_string(y);
        string errorMessage = "Exception Raised. Out of Range Error on Today's Date (Def Constructor)";
        throw out_of_range(errorMessage);
    }

    // No exception thrown. Setting data members
    else
    {
        this->d = eDay;
        this->m = eMonth;
        this->y = eYear;
    }
}

// COPY CONSTRUCTOR
Date::Date(const Date& date) : d(date.d), m(date.m), y(date.y)
{
    // Empty Body
}

// DESTRUCTOR
Date::~Date()
{
    // Empty body
}


// ASSIGNMENT OPERATOR
Date& Date::operator=(const Date& date)
{
    if((*this) != date)
    {
        this->d = date.d;
        this->m = date.m;
        this->y = date.y;
    }
    return (*this);
}


// UNARY ARITHMETIC OPERATORS
Date& Date::operator++()                                                // Next day
{
    // Uses the overloaded '+' operator on Date
    (*this) = (*this) + 1;
    return *this;
}

Date Date::operator++(int)                                             // Next Week, same day
{
    // Saving the current date, as the postfix operator has to return the old value.
    Date currDate(this->d, this->m, this->y);
    // Uses the overloaded '+' operator on Date
    (*this) = (*this) + 7;
    return currDate;
}

Date& Date::operator--()                                               // Previous day
{
    // Uses the overloaded '+' operator on Date
    (*this) = (*this) + (-1);
    return *this;
}

Date& Date::operator--(int)                                           // Same day previous week
{
    // Uses the overloaded '+' operator on Date
    (*this) = (*this) + (-7);
    return *this;
}


// BINARY ARITHMETIC OPERATORS
unsigned int Date::operator-(const Date& otherDate)                    // Number of days between otherDate and current date
{
    // Getting no of days passed from 0/0/00 to this date and other date
    long long int currDateNoOfDaysPassed =  getNumberDaysPassed(this->d, this->m , this->y);
    long long int otherDateNoOfDaysPassed = getNumberDaysPassed(otherDate.d, otherDate.m, otherDate.y);
    // Taking abs val of difference
    long long int daysBetween = labs(currDateNoOfDaysPassed - otherDateNoOfDaysPassed);
    return static_cast<unsigned int>(daysBetween);
}

Date Date::operator+(int noOfDays)                                        // Day noOfDays after (before) the current date
    throw(domain_error, out_of_range)
{
    // Getting no of days passed from 0/0/00 to this date
    long long int currDateDaysPassed = getNumberDaysPassed(this->d, this->m, this->y);
    // Adding noOfDays to it
    long long int newDateDaysPassed  = currDateDaysPassed + static_cast<long long int>(noOfDays);

    // A negative val of noOfDays might have led to a <0 condition
    if(newDateDaysPassed < 0)
    {
        throw out_of_range("Exception raised. Out of range error on Date.");
    }

    // An array infoArr[3] to store the day, month and year of the new date
    int* infoArr = new int[3];
    // infoArr[0] -> day, infoArr[1] -> month, infoArr[2] -> year of New Date
    getDateFromDays(newDateDaysPassed, infoArr);

    if(!isDateInRange(static_cast<Day>(infoArr[0]), static_cast<Month>(infoArr[1]), static_cast<Year>(infoArr[2])))
    {
        throw out_of_range("Exception raised. Out of range error on Date.");
    }

    Date newDate(static_cast<Day>(infoArr[0]), static_cast<Month>(infoArr[1]), static_cast<Year>(infoArr[2]));
    return newDate;
}


Date::operator WeekNumber() const
{
    int weekNum;
    // Getting the date corresponding to Monday of the week number 1 of the year this->y
    Date firstMondayDate = getFirstMondayDate(this->y);

    // If this Date is lesser than firstMondayDate, then it lies in week number of previous year
    if(*(const_cast<Date *>(this)) < firstMondayDate)
    {
        int year = this->y;
        Date firstMondayDate_prevYear = getFirstMondayDate(static_cast<Year>(year-1));
        weekNum = (*(const_cast<Date *>(this)) - firstMondayDate_prevYear) / 7;
    }
    else
    {
        weekNum = (*(const_cast<Date *>(this)) - firstMondayDate) / 7;
    }
    return static_cast<WeekNumber>(weekNum + 1);
}


Date::operator Month() const                                             // Cast to the month of the year in which the current date falls
{
    return this->m;
}


Date::operator WeekDay() const                                          // Cast to the day of the week of the current date
{
    int day = this->d, month = this->m, year = this->y;
    int weekday = dayOfWeek(day, month, year);
    return static_cast<WeekDay>(weekday);
}

bool Date::leapYear() const                                                  // True if the year of the current date is a leap year
{
    return isLeapYear(this->y);
}


// BINARY RELATIONAL OPERATORS
bool Date::operator==(const Date& otherDate)
{
    int thisday  = this->d, thismonth = this->m, thisyear = this->y;
    int otherday = otherDate.d, othermonth = otherDate.m, otheryear = otherDate.y;
    if(thisday == otherday && thismonth == othermonth && thisyear == otheryear)
        return true;
    else
        return false;
}

bool Date::operator!=(const Date& otherDate)
{
    if((*this) == otherDate)
        return false;
    else
        return true;
}

bool Date::operator<(const Date& otherDate)
{
    int thisday  = this->d, thismonth = this->m, thisyear = this->y;
    int otherday = otherDate.d, othermonth = otherDate.m, otheryear = otherDate.y;

    if(thisyear<otheryear || (thisyear == otheryear && thismonth < othermonth) || (thisyear == otheryear && thismonth == othermonth && thisday < otherday))
        return true;
    else
        return false;
}

bool Date::operator<=(const Date& otherDate)
{
    if(*this == otherDate || *this < otherDate)
        return true;
    else
        return false;
}

bool Date::operator>(const Date& otherDate)
{
    if(!(*this <= otherDate))
        return true;
    else
        return false;
}

bool Date::operator>=(const Date& otherDate)
{
    if(!(*this < otherDate))
        return true;
    else
        return false;
}



ostream& operator<<(ostream& os, const Date& date)
{
    int day = date.d, month = date.m, year = date.y;

    // Flush to os according to the static member format
    if(date.getDateFormat() != NULL)
    {
        if(strcmp(date.getDateFormat(),"d") == 0)
        {
            os<<day<<" ";
        }
        else if(strcmp(date.getDateFormat(),"dd") == 0)
        {
            if(day < 10)
                os<<"0"<<day<<" ";
            else
                os<<day<<" ";
        }
    }

    if(date.getMonthFormat() != NULL)
    {
        if(strcmp(date.getMonthFormat(),"m") == 0)
        {
            os<<month<<" ";
        }
        else if(strcmp(date.getMonthFormat(),"mm") == 0)
        {
            if(month < 10)
                os<<"0"<<month<<" ";
            else
                os<<month<<" ";
        }
        else if(strcmp(date.getMonthFormat(),"mmm") == 0)
        {
            string monthShortName[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
            os<<monthShortName[month-1]<<" ";
        }
    }
    else
    {
        string monthFullName[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
        os<<monthFullName[month-1]<<" ";
    }

    if(date.getYearFormat() != NULL)
    {
        if(strcmp(date.getYearFormat(),"yy") == 0)
        {
            if(year<2000)
                os<<(year%1900);
            else
            {
                if(year%2000 < 10)
                    os<<"0"<<(year%2000);
                else
                   os<<(year%2000);
            }

        }
        else if(strcmp(date.getYearFormat(),"yyyy") == 0)
        {
            os<<year;
        }
    }
    return os;
}


istream& operator>>(istream& is, Date& date)
{
    string input;
    cin >> input;
    date = Date(input.c_str());
    return is;
}


void Date::setFormat(DateFormat& form)
{
    Date::format = form;
}

const DateFormat& Date::getFormat()
{
    return Date::format;
}


/****************************************** END OF Date.h IMPLEMENTATIONS ******************************************/

// Added public methods
unsigned int Date::getYear() {
  return this->y;
}
unsigned int Date::getMonth() {
  return this->m;
}
unsigned int Date::getDay() {
  return this->d;
 }
