//
// Created by greg on 9/15/17.
//

#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H

#endif //UNTITLED_MAIN_H
#include <string>
#include <iostream>
#include <fcntl.h>
#include <chrono>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <boost/thread.hpp>
#include <fstream>
#include <ctime>
#include <locale>
#include <codecvt>
using std::string;
using std::wstring;
using std::cout;
using std::to_string;

/*
 *  Function that clicks the given mouse button.
 *  Params:
 *      int button - The button to press.
 *          button can be a value from this list:
 *          1 - Left Button
 *          2 - Middle Button
 *          3 - Right Button
 *          4 - Scroll Wheel Up   (???) Who designed this?
 *          5 - Scroll Wheel Down (??????) Seriously, What?
 *          6 - Push scroll wheel left
 *          7 - Push scroll wheel right
 *          8 - 4th Button
 *          9 - 5th Button.
 *      boolean press - Whether the button should be pressed or released. TRUE is press, FALSE is release.
 *
 */
void mouseClick(int button, bool press);
/*
 * Function that returns the current time as a formatted string.
 */
std::string getStringTime();
char * getThreadName();
/*
 * Function that delays execution for the given duration. Cross Platform, but maybe not for long as nothing else here is.
 */
inline void delay( unsigned long ms );
/*
 * The movement thread. Interprets the stored data about the controller, and reacts accordingly.
 * See Also: input_t
 */
void move_t();
/*
 * The input thread. Recieves data from the controller and stores the important data.
 * See Also: move_t
 */
void input_t();
string wstr2str(wstring string_to_convert);
struct END {

};

END* flush;
int progLogLevel = 4;
class Logger
{
    friend std::ostream& operator<<(std::ostream& target, const Logger& source);

public:
    operator string() {
        return this->val;
    }
	int priority = 0;
	string type;
	string mod;
	string val;
    void flush() {
		if (this-> priority <= progLogLevel) {
			if (this->mod.length() > 0) cout << "\033" << mod;
			cout << "[" << getStringTime() << "] [" << this->type << "/" << getThreadName() << "]   " << this->val
				 << std::endl;
			if (this->mod.length() > 0) cout << "\033[0m";
			this->val = string();
		}
    }
	/*
	 * The priority of this Logger. Allows for the filtering of logger data.
	 * 0 - FATAL
	 * 1 - SEVERE
	 * 2 - ERROR
	 * 3 - WARN
	 * 4 - OUTPUT
	 * 5 - INFO
	 * 6 - DEBUG
	 * 7 - VERBOSE
	 * 8 - TRACE
	 */
	Logger(string loggerType)	{
		type = loggerType;
	}
	Logger(string loggerType, string modifier, int logPriority) {
		type = loggerType;
		mod = modifier;
		priority = logPriority;
	}
	Logger(string loggerType, int logPriority) {
		type = loggerType;
		priority = logPriority;
	}
};

Logger debug("DEBUG", 6);
Logger out("OUT", 4);
Logger warn("WARN", "[38;5;208m\033[4m", 3);
Logger err("ERROR", "[38;5;196m", 2);

Logger& operator+(Logger& target, wstring toAdd)
{
    return (target.val += wstr2str(toAdd)), target;
}

Logger& operator+(Logger& target, string toAdd)
{
    return (target.val += toAdd), target;
}
Logger& operator<<(Logger& target, const int in) {
    target.val += to_string(in);
    return target;
}
Logger& operator<<(Logger& target, const long in) {
    target.val += to_string(in);
    return target;
}
Logger& operator<<(Logger& target, const double in) {
    target.val += to_string(in);
    return target;
}
Logger& operator<<(Logger& target, const short in) {
    target.val += to_string(in);
    return target;
}

Logger& operator<<(Logger& target, const char in) {
    target.val += to_string(in);
    return target;
}

Logger& operator<<(Logger& target, const char& in) {
    target.val += string(&in);
    return target;
}

Logger& operator<<(Logger& target, const string in) {
    target.val += in;
    return target;
}

Logger& operator<<(Logger& target, const wstring in) {
    target.val += wstr2str(in);
    return target;
}

Logger& operator<<(Logger& target, END* in) {
    target.flush();
    return target;
}