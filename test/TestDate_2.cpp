#include "Date.h"

#include <iostream>
#include <string>
using namespace std;

void dateFormatConstructors()
{
    cout << "\n*************************************************************";
    cout << "\nChecking DateFormat(const char* dateFormat, const char* monthFormat, const char* yearFormat)\n";
    cout << "HAPPY PATHS:\n";
    const char* dateF[] = {NULL, "d", "dd"};
    const char* monthF[] = {NULL, "m", "mm", "mmm"};
    const char* yearF[] =  {NULL, "yy", "yyyy"};

    int exception_counter = 0;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                try
                {
                    string nul = "NULL";
                    DateFormat df(dateF[i], monthF[j], yearF[k]);
                    cout << "(" << ((dateF[i] == NULL)? nul:dateF[i]) << ", " << ((monthF[j] == NULL)? nul:monthF[j]) << ", " << ((yearF[k] == NULL)? nul:yearF[k]) << ")" << endl;
                }
                catch(invalid_argument error)
                {
                    string nul = "NULL";
                    cout << "Invalid Argument : " << ((dateF[i] == NULL)? nul:dateF[i]) << ", " << ((monthF[j] == NULL)? nul:monthF[j]) << ", " << ((yearF[k] == NULL)? nul:yearF[k]) << endl;
                    exception_counter++;
                }
            }
        }
    }
    cout << "Exceptions Caught : " << exception_counter << endl;

    cout << "\nEXCEPTION PATHS :\n";
    exception_counter = 0;
    const char* dateFex[]  = {"", "ddd", "ab", "1-t", "  @#$ 1920 "};
    const char* monthFex[] = {"", "mmmm", "mm1ty", " !@//", "0e1"};
    const char* yearFex[]  =  {"", "y", "yyy", "yyyyy", "12-yt"};

    const char* validDate = "dd";
    const char* validMonth = "mm";
    const char* validYear = "yyyy";

    cout << "Date exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df(dateFex[i], validMonth, validYear);
        }
        catch(invalid_argument error)
        {
            string nul = "NULL";
            cout << "Invalid Argument : (" << (dateFex[i]) << ", mm, yyyy)"  << endl;
        }
    }
    cout << "Month exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df(validDate, monthFex[i], validYear);
        }
        catch(invalid_argument error)
        {
            string nul = "NULL";
            cout << "Invalid Argument : " << "(dd, " << monthFex[i] << ", yyyy)"  << endl;
        }
    }
    cout << "Year exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df(validDate, validMonth, yearFex[i]);
        }
        catch(invalid_argument error)
        {
            string nul = "NULL";
            cout << "Invalid Argument : " << "(dd, mm, " << yearFex[i] << ")"  << endl;
        }
    }

    cout << "\n*************************************************************";
    cout << "\nChecking DateFormat(const char* format)\n";
    cout << "It requires date month and year format to be separated by -, even if they are NULL\n";
    cout << "NULL is printed only for easy reading and understanding.\n";
    cout << "HAPPY PATHS :\n";
    string date[] = {"-", "d-", "dd-"};
    string month[] = {"-", "m-", "mm-", "mmm-"};
    string year[] = {"yy", "yyyy"};
    exception_counter = 0;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            for(int k = 0; k < 2; k++)
            {
                try
                {
                    DateFormat df((date[i]+month[j]+year[k]).c_str());
                    cout << "\"" << (date[i]+month[j]+year[k]) << "\"\n";
                }
                catch(invalid_argument error)
                {
                    cout << "Invalid Argument : \"" << (date[i]+month[j]+year[k]) << "\"\n";
                    exception_counter++;
                }
                catch(exception e)
                {
                    cout << "Unknown exception\n";
                }
            }
        }
    }
    cout << "Exceptions Caught : " << exception_counter << endl;

    cout << "\nEXCEPTION PATHS\n";
    string datex[] = {"", "--", "ddd-", "dd" , "12d3"};
    string monthex[] = {"", "mm", "mmmm-", "momq-", "!@3-4-"};
    string yearex[] = {"--", "yyy", "y", "!2s", "yy-"};
    string valDate = "dd-", valMonth = "mm-", valYear = "yy";
    cout << "Date exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df((datex[i]+valMonth+valYear).c_str());
        }
        catch(invalid_argument error)
        {
            cout << "Invalid Argument : \"" << (datex[i]+valMonth+valYear) << "\"\n";
        }
    }
    cout << "Month exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df((valDate+monthex[i]+valYear).c_str());
        }
        catch(invalid_argument error)
        {
            cout << "Invalid Argument : \"" << (valDate+monthex[i]+valYear) << "\"\n";
        }
    }
    cout << "Year exceptions: \n";
    for(int i = 0; i < 5; i++)
    {
        try
        {
            DateFormat df((valDate+valMonth+yearex[i]).c_str());
        }
        catch(invalid_argument error)
        {
            cout << "Invalid Argument : \"" << (valDate+valMonth+yearex[i]) << "\"\n";
        }
    }

    cout << "\n*************************************************************";
    cout << "\nChecking DateFormat::DateFormat()\n";
    DateFormat df;
    cout << "Default DateFormat created ";
    cout << endl;

}

/************************************ DATE CONSTRUCTORS *****************************************************/

void dateConstructors()
{
    cout << "\n*************************************************************";
    cout << "\nChecking Date(Day d, Month m, Year y)\n";
    try{
        DateFormat df("dd-mm-yyyy");
        Date::setFormat(df);
    }
    catch(invalid_argument error){
        cout << error.what() << endl;
    }
    Day day[]={D01, D15, D28, D29, D30, D31};
	Month month[]={Jan, Feb, May, Nov, Dec};
	Year year[]={1949, 1950, 2000, 2049, 2050};

	cout << "HAPPY AND EXCEPTION PATHS : \n";
	for(int i = 0; i < 6; i++){
		for(int j = 0; j < 5; j++){
			for(int k = 0; k < 5; k++){
				try{
					Date date(day[i],month[j],year[k]);
					cout<<"Date type constructed : (" << day[i] << ", " << month[j] << ", " << year[k] << ")" << endl;
				}
				catch(invalid_argument error){
                    cout<<"Invalid Argument      : (" << day[i] << ", " << month[j] << ", " << year[k] << ")" << endl;
				}
				catch(domain_error error){
					cout<<"Domain Error          : (" << day[i] << ", " << month[j] << ", " << year[k] << ")" << endl;
				}
				catch(out_of_range error){
					cout<<"Out of Range          : (" << day[i] << ", " << month[j] << ", " << year[k] << ")" << endl;
				}
			}
		}
	}

	cout << "\nAdditional Exception paths\n";
	cout << "The user can static cast invalid values to enum.\n";
	cout << "Checking that Date(day, month, year) doesn't crash\n";
	Day valDay = D04;
	Month valMonth = Jun;
	Year valYear = 2014;

	Day dayex[] = {static_cast<Day>(32), static_cast<Day>(-13), static_cast<Day>(1111)};
    Month monthex[] = {static_cast<Month>(0), static_cast<Month>(13), static_cast<Month>(-10)};
    Year yearex[] = {static_cast<Year>(-2014), static_cast<Year>(-20)};

    cout << "Checking day exceptions\n";
    for(int i = 0; i < 3; i++)
    {
        try{
            Date d(dayex[i],valMonth, valYear);
            cout << "Date type constructed : (" << dayex[i] << ", " << valMonth << ", " << valYear << ")" << endl;
        }
        catch(invalid_argument error){
            cout<<"Invalid Argument      : (" << dayex[i] << ", " << valMonth << ", " << valYear << ")" << endl;
        }
        catch(domain_error error){
            cout<<"Domain Error          : (" << dayex[i] << ", " << valMonth << ", " << valYear << ")" << endl;
        }
        catch(out_of_range error){
            cout<<"Out of Range          : (" << dayex[i] << ", " << valMonth << ", " << valYear << ")" << endl;
        }
    }
    cout << "Checking month exceptions\n";
    for(int i = 0; i < 3; i++)
    {
        try{
            Date d(valDay,monthex[i], valYear);
            cout << "Date type constructed : (" << valDay << ", " << monthex[i] << ", " << valYear << ")" << endl;
        }
        catch(invalid_argument error){
            cout<<"Invalid Argument      : (" << valDay << ", " << monthex[i] << ", " << valYear << ")" << endl;
        }
        catch(domain_error error){
            cout<<"Domain Error          : (" << valDay << ", " << monthex[i] << ", " << valYear << ")" << endl;
        }
        catch(out_of_range error){
            cout<<"Out of Range          : (" << valDay << ", " << monthex[i] << ", " << valYear << ")" << endl;
        }
    }

    cout << "Checking year exceptions\n";
    for(int i = 0; i < 2; i++)
    {
        try{
            Date d(valDay,valMonth, yearex[i]);
            cout << "Date type constructed : (" << valDay << ", " << valMonth << ", " << yearex[i] << ")" << endl;
        }
        catch(invalid_argument error){
            cout<<"Invalid Argument      : (" << valDay << ", " << valMonth << ", " << yearex[i] << ")" << endl;
        }
        catch(domain_error error){
            cout<<"Domain Error          : (" << valDay << ", " << valMonth << ", " << yearex[i] << ")" << endl;
        }
        catch(out_of_range error){
            cout<<"Out of Range          : (" << valDay << ", " << valMonth << ", " << yearex[i] << ")" << endl;
        }
    }

    cout << "\n*************************************************************";
    cout << "\nChecking explicit Date(const char* date)\n";
    cout << "Checking for formats not allowed for input.\n";
    DateFormat formats[]={DateFormat(NULL,"mm","yy"),DateFormat("dd",NULL,"yy"),DateFormat("dd","mmm","yy"),
                            DateFormat("dd","mm",NULL)};
    for(int i = 0; i < 4; i++)
    {
		Date::setFormat(formats[i]);
		try{
            Date d("12-May-99");
		}
		catch(invalid_argument error){
		    formats[i].printDateFormat();
			cout << " : " << error.what() << endl;
		}
		catch(domain_error error){
			cout << error.what() << endl;
		}
		catch(out_of_range error){
			cout << error.what() << endl;
		}
	}

	cout << "\nChecking for input-acceptable date arguments\n";
	cout << "Setting format as dd-mm-yyyy" << endl;

	DateFormat df("dd-mm-yyyy");
	Date::setFormat(df);
	//string days[]={"31-"};

	cout << "\nChecking for non 31 / 30 days months\n";
	string cornerDay = "31-";
	string months[]={"01-", "02-", "03-", "04-", "05-", "06-", "07-", "08-", "09-", "10-", "11-", "12-", "02-"};

	string validYear = "1980";
	//string years[]={"1949", "1950", "2049", "2050"};

	for(int i = 0; i < 12; i++)
    {
        try{
            Date d((cornerDay+months[i]+validYear).c_str());
            cout << "Successfully constructed : " << "\"" << (cornerDay+months[i]+validYear) << "\"\n";
            if(i==11)
            {
                cornerDay = "30-";
                i++;
                Date dd((cornerDay+months[i]+validYear).c_str());
            }
        }
        catch(invalid_argument error){
            cout << "Invalid Argument         : " << "\"" << (cornerDay+months[i]+validYear) << "\"\n";
        }
        catch(domain_error error){
            cout << "Domain Error             : " << "\"" << (cornerDay+months[i]+validYear) << "\"\n";
        }
        catch(out_of_range error){
            cout << "Out of Range             : " << "\"" << (cornerDay+months[i]+validYear) << "\"\n";
        }
    }

    cout << "\nChecking for 29th Feb cases\n";
    string lYears[] = {"1950", "1972", "2000", "2014", "2048", "2100"};
    cornerDay = "29-";
    string cornerMonth = "02-";
    for(int i = 0; i < 6; i++)
    {
        try{
            Date d((cornerDay+cornerMonth+lYears[i]).c_str());
            cout << "Successfully constructed : " << "\"" << (cornerDay + cornerMonth + lYears[i]) << "\"\n";
        }
        catch(invalid_argument error){
            cout << "Invalid Argument         : " << "\"" << (cornerDay + cornerMonth + lYears[i]) << "\"\n";
        }
        catch(domain_error error){
            cout << "Domain Error             : " << "\"" << (cornerDay + cornerMonth + lYears[i]) << "\"\n";
        }
        catch(out_of_range error){
            cout << "Out of Range             : " << "\"" << (cornerDay + cornerMonth + lYears[i]) << "\"\n";
        }
    }

    cout << "\nChecking for invalid day parameters.\n";
    cout << "NOTE : If date is valid but not matching dd-mm-yyyy, Invalid argument will be thrown \n";
    string invalDays[] = {"1-", "d", "-1", "-12-", "32-", "0-", "1p!!@}&-", ""};
    cornerMonth = "07-";
    string cornerYear = "2014";
    for(int i = 0; i < 8; i++)
    {
        try{
            Date d((invalDays[i]+cornerMonth+cornerYear).c_str());
            cout << "Successfully constructed : " << "\"" << (invalDays[i] + cornerMonth + cornerYear) << "\"\n";
        }
        catch(invalid_argument error){
            cout << "Invalid Argument         : " << "\"" << (invalDays[i] + cornerMonth + cornerYear) << "\"\n";
        }
        catch(domain_error error){
            cout << "Domain Error             : " << "\"" << (invalDays[i] + cornerMonth + cornerYear) << "\"\n";
        }
        catch(out_of_range error){
            cout << "Out of Range             : " << "\"" << (invalDays[i] + cornerMonth + cornerYear) << "\"\n";
        }
    }

    cout << "\nChecking for invalid month parameters.\n";
    cout << "NOTE : If date is valid but not matching dd-mm-yyyy, Invalid argument will be thrown \n";
    string invalMonths[] = {"1-", "Mar-", "m", "-1", "-12-", "13-", "0-", "1p!!@}&-", ""};
    cornerDay = "07-";
    for(int i = 0; i < 9; i++)
    {
        try{
            Date d((cornerDay+invalMonths[i]+cornerYear).c_str());
            cout << "Successfully constructed : " << "\"" << (cornerDay + invalMonths[i] + cornerYear) << "\"\n";
        }
        catch(invalid_argument error){
            cout << "Invalid Argument         : " << "\"" << (cornerDay + invalMonths[i] + cornerYear) << "\"\n";
        }
        catch(domain_error error){
            cout << "Domain Error             : " << "\"" << (cornerDay + invalMonths[i] + cornerYear) << "\"\n";
        }
        catch(out_of_range error){
            cout << "Out of Range             : " << "\"" << (cornerDay + invalMonths[i] + cornerYear) << "\"\n";
        }
    }

    cout << "\nChecking for invalid year parameters.\n";
    cout << "NOTE : If date is valid but not matching dd-mm-yyyy, Invalid argument will be thrown \n";
    string invalYears[] = {"96", "1a98", "-2014", "", "20000", "1p!!@}&-"};
    cornerDay = "07-";
    cornerMonth = "12-";
    for(int i = 0; i < 6; i++)
    {
        try{
            Date d((cornerDay+cornerMonth+invalYears[i]).c_str());
            cout << "Successfully constructed : " << "\"" << (cornerDay + cornerMonth + invalYears[i]) << "\"\n";
        }
        catch(invalid_argument error){
            cout << "Invalid Argument         : " << "\"" << (cornerDay + cornerMonth + invalYears[i]) << "\"\n";
        }
        catch(domain_error error){
            cout << "Domain Error             : " << "\"" << (cornerDay + cornerMonth + invalYears[i]) << "\"\n";
        }
        catch(out_of_range error){
            cout << "Out of Range             : " << "\"" << (cornerDay + cornerMonth + invalYears[i]) << "\"\n";
        }
    }
}

void TestDate()
{
    cout << "Test Date Data Type" << endl;
    dateFormatConstructors();
    dateConstructors();

    DateFormat df("dd-mm-yyyy");
    Date::setFormat(df);
    Date d1(D31, May, 1996);
    Date d2("24-04-2001");
    Date d3;
    cout << "\n*************************************************************";
    cout << "\nChecking functionalities of Date through : \n";
    cout << "Date d1 (D31, May, 1996) = " << d1 << endl;
    cout << "Date d2 (\"24-04-2001\") = " << d2 << endl;
    cout << "Date d3 = " << d3 << endl;
    // BASIC ASSIGNMENT OPERATOR
    // Date& operator=(const Date&)
    cout << "\n*************************************************************";
    cout << "\nBASIC ASSIGNMENT OPERATOR" << endl;
    cout << "Before Assignment : d3 = " << d3 << ", d1 = " << d1 << endl;
    d3 = d1;
    cout << "Assigning d3 = d1..." << endl;
    cout << "After Assignment  : d3 = " << d3 << ", d1 = " << d1 << endl;
    cout << "Testing SELF ASSIGNMENT\n";
    cout << "Before Assignment : d3 = " << d3 << endl;
    d3 = d3;
    cout << "Assigning d3 = d3..." << endl;
    cout << "After Assignment  : d3 = " << d3 << endl;

    // UNARY ARITHMETIC OPERATORS
    // --------------------------
    // Date& operator++() Next day
    cout << "\n*************************************************************";
    cout << "\nUNARY ARITHMETIC OPERATORS" << endl;
    d1 = Date(D30, Jun, 1999);
    d2 = Date("31-12-2040");
    cout << "operator ++()\n";
    cout << "Before ++ pre-operator : d1 = " << d1 << endl;
    ++d1;
    cout << "After  ++ pre-operator : d1 = " << d1 << endl;
    cout << "Before ++ pre-operator : d2 = " << d2 << endl;
    ++d2;
    cout << "After  ++ pre-operator : d2 = " << d2 << endl;
    cout << "EXCEPTION PATH: \n";
    d3 = Date("31-12-2049");
    cout << "Before ++ pre-operator : d3 = " << d3 << endl;
    try{
        cout << "After  ++ pre-operator : d3 = ";
        cout << (++d3) << endl;
    }
    catch(out_of_range error)
    {
        cout << error.what() << endl;
    }

    cout << "\noperator ++(int)\nChecking the post-fix property of operator++(int) :\n";
    d1 = Date(D27, Dec, 1999);
    cout << "d1 = " << d1 << endl;
    cout << "Assigning d3 = d1++ \nd3 = " << (d3 = d1++) << endl;
    cout << "d1 = " << d1 << endl;
    cout << "EXCEPTION PATH :\n";
    try
    {
        Date d4(D27, Dec, 2049);
        cout << "Operator ++ (post) on d4 = " << d4 << " : ";
        cout << (d4++) << endl;
    }
    catch(domain_error error)
    {
        cout << error.what() << endl;
    }
    catch(out_of_range error)
    {
        cout << error.what() << endl;
    }

    cout << "\n*************************************************************";
    d1 = Date(D01, Mar, 2016);
    d2 = Date(D01, Jan, 1960);
    cout << "\noperator --()\nBefore -- pre-operator : d1 = " << d1 << endl;
    --d1;
    cout << "After  -- pre-operator : d1 = " << d1 << endl;
    cout << "Before -- pre-operator : d2 = " << d2 << endl;
    --d2;
    cout << "After  -- pre-operator : d2 = " << d2 << endl;
    cout << "EXCEPTION PATH : \n";
    d2 = Date(D01, Jan, 1950);
    cout << "Before -- pre-operator : d2 = " << d2 << endl;
    try{
        cout << "After  -- pre-operator : d2 = ";
        cout << (--d2) << endl;
    }
    catch(out_of_range error){
        cout << error.what() << endl;
    }

    d1 = Date(D07, Aug, 2033);
    d2 = Date(D03, Jan, 2000);
    cout << "operator --(int)\nBefore -- post-operator : d1 = " << d1 << endl;
    d1--;
    cout << "After  -- post-operator : d1 = " << d1 << endl;
    cout << "Before -- post-operator : d2 = " << d2 << endl;
    d2--;
    cout << "After  -- post-operator : d2 = " << d2 << endl;
    cout << "EXCEPTION PATH : \n";
    d2 = Date(D04, Jan, 1950);
    cout << "Before -- post-operator : d2 = " << d2 << endl;
    try{
        cout << "After  -- pre-operator : d2 = ";
        cout << (d2--) << endl;
    }
    catch(out_of_range error){
        cout << error.what() << endl;
    }

    // BINARY ARITHMETIC OPERATORS
    // ---------------------------
    cout << "\n*************************************************************";
    cout << "\nBINARY ARITHMETIC OPERATIONS" << endl;
    // unsigned int operator-(const Date& otherDate)  Number of days between otherDate and current date
    cout << "operator -\n";
    d1 = Date(D01, Jan, 1960);
    d2 = Date("29-02-1960");
    cout << "Days b/w d1 = " << d1 << " and d2 = " << d2 << endl << "d1 - d2 = " << (d1 - d2) << endl;

    cout << "\noperator +\n";
    try
    {
        int days = 400;
        cout << "Adding days = " << days << " to d1 = " << d1 << endl;
        cout << d1 << " + " << days << " = " << (d1 + 400)  << endl;
    }
    catch(domain_error error)
    {
        cout << "Exception on binary operator + : " << error.what() << endl;
    }
    catch(out_of_range error)
    {
        cout << "Exception on binary operator + : " << error.what() << endl;
    }
    cout << "Exception Path : \n";
    try
    {
        Date d4(D23, Jan, 1950);
        int days = -30;
        cout << "Adding days = " << days << " to d4 = " << d4 << "  : ";
        cout << (d4 + days) << endl;
    }
    catch(domain_error error)
    {
        cout << error.what() << endl;
    }
    catch(out_of_range error)
    {
        cout << error.what() << endl;
    }
    try
    {
        Date d4(D03, Jan, 2044);
        int days = 3000;
        cout << "Adding days = " << days << " to d4 = " << d4 << " : ";
        cout << (d4 + days) << endl;
    }
    catch(domain_error error)
    {
        cout << error.what() << endl;
    }
    catch(out_of_range error)
    {
        cout << error.what() << endl;
    }
    // CAST OPERATORS
    // --------------
    cout << "\n*************************************************************";
    cout << "\nCAST OPERATORS" << endl;
    // operator WeekNumber() const    Cast to the week number of the year in which the current date falls
    cout << "Week Number for date d1 = " << d1 << " : " << WeekNumber(d1) << endl;
    cout << "Week Number for date d2 = " << d2 << " : " << WeekNumber(d2) << endl;

    // operator Month() const         Cast to the month of the year in which the current date falls
    cout << "\nMonth for date d1 = " << d1 << " : " << Month(d1) << endl;
    cout << "Month for date d2 = " << d2 << " : " << Month(d2) << endl;

    // operator WeekDay() const       Cast to the day of the week of the current date
    cout << "\nWeek Day for date d1 = " << d1 << " : " << WeekDay(d1) << endl;
    cout << "Week Day for date d2 = " << d2 << " : " << WeekDay(d2) << endl;

    // bool leapYear() const
    bool test = d1.leapYear();
    cout << "\nIs the year of date d1 = " << d1 << " leap year? -> " << ((test)? "True" : "False") << endl;
    test = d2.leapYear();
    cout << "Is the year of date d2 = " << d2 << " leap year? -> " << ((test)? "True" : "False") << endl;


    // BINARY RELATIONAL OPERATORS
    cout << "\n*************************************************************";
    cout << "\nBINARY RELATIONAL OPERATORS" << endl;
    d1 = Date(D20, Nov, 2033);
    d2 = Date("20-12-2033");
    d3 = d1;

    // bool operator==(const Date& otherDate)
    test = (d1 == d2);
    cout << "Is date d2 = " << d2 << " == date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d1 == d3);
    cout << "Is date d3 = " << d3 << " == date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;

    // bool operator!=(const Date& otherDate)
    test = (d1 != d2);
    cout << "\nIs date d2 = " << d2 << " != date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d1 != d3);
    cout << "Is date d3 = " << d3 << " != date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;

    //bool operator<(const Date& otherDate)
    test = (d2 < d1);
    cout << "\nIs date d2 = " << d2 << " < date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d3 < d1);
    cout << "Is date d3 = " << d3 << " < date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;

    //bool operator<=(const Date& otherDate)
    test = (d2 <= d1);
    cout << "\nIs date d2 = " << d2 << " <= date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d3 <= d1);
    cout << "Is date d3 = " << d3 << " <= date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;

    //bool operator>(const Date& otherDate)
    test = (d2 > d1);
    cout << "\nIs date d2 = " << d2 << " > date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d3 > d1);
    cout << "Is date d3 = " << d3 << " > date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;

    //bool operator>=(const Date& otherDate)
    test = (d2 >= d1);
    cout << "\nIs date d2 = " << d2 << " >= date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;
    test = (d3 >= d1);
    cout << "Is date d3 = " << d3 << " >= date d1 = " << d1 << " ? -> " << ((test)? "True" : "False") << endl;


    // Format Function
    cout << "\n*************************************************************";
    cout << "\nFORMAT FUNCTIONS AND OSTREAM\n" << endl;
    d1 = Date(D04, Jun, 2010);
    d2 = Date("30-11-1979");
    cout << "For d1 = 04/06/2010 and d2 = 30/11/1979\n";
    string datef[] = {"-", "d-", "dd-"};
    string monthf[] = {"-", "m-", "mm-", "mmm-"};
    string yearf[] = {"", "yy", "yyyy"};
    DateFormat df2;
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                cout << "Setting date format as : " << (datef[i] + monthf[j] + yearf[k]) << endl;
                df2 = DateFormat((datef[i] + monthf[j] + yearf[k]).c_str());
                Date::setFormat(df2);
                cout << "d1                     : " << d1 << endl;
                cout << "d2                     : " << d2 << endl;
            }
        }
    }

    cout << "\n*************************************************************";
    cout << "\nCHECKING ISTREAM\n" << endl;
    cout << "Enter a valid date format for defining input format\n";
    string format_input;
    cin >> format_input;
    try{
        DateFormat df_input((format_input).c_str());
        Date::setFormat(df_input);
        cout << "Enter date in " << format_input << " format\n";
        Date in_date;
        cin >> in_date;
        cout << "Enter format for output\n";
        string format_output;
        cin >> format_output;
        DateFormat df_output((format_output).c_str());
        Date::setFormat(df_output);
        cout << "Input Date : " << in_date << endl;
    }
    catch(invalid_argument e)
    {
        cout << e.what() << endl;
    }
    catch(domain_error e)
    {
        cout << e.what() << endl;
    }
    catch(out_of_range e)
    {
        cout << e.what() << endl;
    }
}

int main()
{
    TestDate();
}
