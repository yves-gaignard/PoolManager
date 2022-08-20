# Date-Class
C++ library to help you handle dates from the Gregorian Calendar. Programming with dates should be almost as simple and easy as programming with strings and integers. Applications with lots of date-time related computations can be simplified significantly by having a robust set of operators and calculation capabilities.

## dateformat Functionalities

The `dateformat` class defines the format in which a date is output or input.

### Accepted Date Formats:

#### Date Formats:
* **`"0"`**: No date provided. For output only.
* **`"d"`**: Single digit date in one digit *(9)*, Double Digit date in two digits *(25)*.
* **`"dd"`**: All dates in two digits. Single digit dates with leading 0 *(09, 25)*.

#### Month Format:
* **`"0"`**: Full names of months *(February, November)*. For output only.
* **`"m"`**: Single digit month in one digit *(2)*, Double Digit month in two digits *(11)*.
* **`"mm"`**: All months in two digits. Single digit months with leading 0 *(02, 11)*.
* **`"mmm"`**: First three letters of month name *(Feb, Nov)*. For output only.

#### Year Format:
* **`"0"`**: No year provided. For output only.
* **`"yy"`**: Year in last two digits *(61, 16)*.
* **`"yyyy"`**: Year in four digits *(1961, 2016)*.

### Constructors

* `DateFormat (const char* dateFormat, const char* monthFormat, const char* yearFormat)` - Three C-strings format with Date Format, Month Format, and Year Format provided. `DateFormat format ("d", "mm", "yyyy")`.

* `DateFormat (const char* format)` - Single C-string format where Date Format, Month Format, and Year Format are separated by ’-’. `DateFormat format ("d-mm-yyyy")`.

* `DateFormat ()` - Creates default `DateFormat` - "dd-mmm-yy".

### Methods

* `char* get_dateFormat () const` - Get Date Format.
* `char* get_monthFormat () const` - Get Month Format.
* `char* get_yearFormat () const` - Get Year Format.

### Usage with `date` Class

To define the format in which `Date` objects are input and output, set the value of the static `format` member as follows:

```cpp
DateFormat form ("dd-mm-yyyy");
Date::setFormat (form);
```

## date Functionalities

The `date` class gives functionalities to use dates and perform operation on them, in C++ programs.

### Date Data Types

* `Day` - `DXX` representing the XXth Day of the Month, (`D28`, 28th), (`D01`, 1st).
* `Month` - `MXX` representing the XXth Month of the Year, (`M09`, September).
* `Year` - Unsigned Integer values.
* `WeekDay` - `Mon` representing Monday.
* `WeekNumber` - `WXX` representing the XXth Week of the Year, (`W34`, Week Number 34).

### Constructors

* `Date (Day d, Month m, Year y)` - Construct a Date from (d, m, y) values.
* `Date (const char* date)` - Date in string format - parsed as specified by static `format` member.
* `Date ()` - Default Constructor - construct Today's date.
* `Date (const Date&)` - Copy Constructor.

### Methods

#### Arithmetic Operators

```cpp
Date example_date (D25, M05, 2015);
```

* `Date& operator++ ()` - Next day (`++example_date` gives 26th March, 2015).
* `Date& operator++ (int)` -  Same day next week (`example_date++` gives 1st April, 2015).
* `Date& operator-- ()` - Previous day (`--example_date` gives 24th March, 2015).
* `Date& operator-- (int)` - Same day previous week (`example_date--` gives 18th March, 2015).
* `unsigned int operator- (const Date& otherDate)` - Number of days between `otherDate` and current date. (`Date (D22, M04, 2015) - Date (D22, M05, 2015)` will return 30).
* `Date operator+ (int noOfDays)` - Day `noOfDays` after (before) the current date. Ahead or behind current date depends on sign of `noOfDays`.

#### Relational Operators

The following operators can be used to compare dates:

```cpp
bool operator== (const Date& otherDate);
bool operator!= (const Date& otherDate);
bool operator< (const Date& otherDate);
bool operator<= (const Date& otherDate);
bool operator> (const Date& otherDate);
bool operator>= (const Date& otherDate);
```

For example, `Date () >= Date (D01, M01, 2016)` will return `true`.

#### Casting Operators

* `operator WeekNumber () const` - Cast to the week number of the year in which the current date falls.
* `operator Month () const` - Cast to the month of the year in which the current date falls.
* `operator WeekDay () const` - Cast to the day of the week of the current date.

#### Date Format Setting

* `static void setFormat (DateFormat&)` - Set/Change Input/Output `DateFormat` for dates.
* `static DateFormat& getFormat ()` - Get Input/Output `DateFormat` for dates.

#### Global Methods

* `ostream& operator<< (ostream&, const Date&)` - Output dates as per the `format`.

```cpp
Date test_date (D28, M03, 2005);
std::cout << test_date;
```

* `istream& operator>> (istream&, Date&)` - Input dates as per the `format`.

```cpp
Date test_date (D28, M03, 2005);
std::cin >> test_date;
```

#### Other Methods

* `Date& operator= (const Date&)` - Assignment operator for Date objects.
* `bool leapYear () const` - Check if the year of the current date is a leap year.


