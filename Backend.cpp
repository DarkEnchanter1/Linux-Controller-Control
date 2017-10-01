#include "Backend.h"
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <boost/log/trivial.hpp>
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)

	#include <windows.h>

	inline void delay( unsigned long ms )
	{
		Sleep( ms );
	}
#else  /* presume POSIX */
	inline void delay( unsigned long ms ) {
	usleep( ms * 1000 );
}

#endif


//-----------------------------------------------------------
//|					BEGIN PRINT CLASS					|
//-----------------------------------------------------------
//wstring str2wstr(string in) {
//wstring temp(in.length(), L' '); // Make room for characters
//
//// Copy string to wstring.
//std::copy(in.begin(), in.end(), temp.begin());
//return temp;
//}
string wstr2str(wstring string_to_convert) {
//setup converter
	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	std::string converted_str = converter.to_bytes( string_to_convert );
	return converted_str;
}



//-----------------------------------------------------------
//|							 BEGIN MAIN CODE							  |
//-----------------------------------------------------------

long translateX = 0;
long translateY = 0;
bool run = true;
/*
 * The Timer class - Used for calculating the timing delta in the controller execution.
 */
class Timer
{

private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
public:
	/**
	 * Constructor for creating a new instance of <b>timer</b>.
	 */
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const {
		return std::chrono::duration_cast<second_>
				(clock_::now() - beg_).count(); }

};
Timer deltaXcalc = Timer();
Timer deltaYcalc = Timer();

char * getThreadName() {
	static char temp[16];
	int err = pthread_getname_np(pthread_self(), temp, 16);
	if (err != 0) {
		::err << strerror(err) << flush;
		std::exit( err);
	}
	return temp;
}

void mouseClick(int button, bool press) {
	Display *display = XOpenDisplay(NULL);

	XEvent event;

	if (display == NULL) {
		fprintf(stderr, "Error, no DISPLAY provided!");
		exit(EXIT_FAILURE);
	}

	memset(&event, 0x00, sizeof(event));

	event.type = ButtonPress;
	event.xbutton.button = button;
	event.xbutton.same_screen = True;

	XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window,
				  &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
				  &event.xbutton.state);

	event.xbutton.subwindow = event.xbutton.window;

	while (event.xbutton.subwindow) {
		event.xbutton.window = event.xbutton.subwindow;

		XQueryPointer(display, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow,
					  &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y,
					  &event.xbutton.state);
	}
	if (press) {
		if (XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
			fprintf(stderr, "Error when sending the event!\n");
	} else {
		event.type = ButtonRelease;
		event.xbutton.state = 0x100;

		if (XSendEvent(display, PointerWindow, True, 0xfff, &event) == 0)
			fprintf(stderr, "Error when sending the event!\n");
	}
	XFlush(display);

	XCloseDisplay(display);
}

std::string getStringTime() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time (&rawtime);
	timeinfo = localtime(&rawtime);
	timespec curTimeMA;
	clock_gettime(CLOCK_REALTIME, &curTimeMA);
	long nanos = curTimeMA.tv_nsec;
	strftime(buffer,sizeof(buffer),"%H:%M:%S",timeinfo);
	std::string str(buffer);
	long mil = nanos / 1000000;
	string millistring;
	if (mil < 10) millistring = "00" + to_string(mil);
	else if (mil < 100) millistring = "0" + to_string(mil);
	else millistring = to_string(mil);
	return str + "." + millistring;
}
//static void print(std::string str) {
//	std::cout  << "["<< getStringTime() << "] " << str.c_str() << std::endl; std::flush(std::cout);
//}
//static void print(std::wstring str) {
//	std::cout  << "["<< getStringTime() << "] "; std::cout.flush(); wcout << str.c_str() << std::endl; std::flush(std::cout);std::flush(std::wcout);
//}
double adjust = 0.5;
int deadZone = 2500;
void mouseMove(int x, int y)
{
	std::string str = "xdotool mousemove_relative -- ";
	str.append(std::to_string(x) + " ");
	str.append(std::to_string(y));
	std::system(str.c_str());
}
bool move_out = true, input_out = true;
void move_t() {
	pthread_setname_np(pthread_self(), "Movement");
	double tX = 0;
	double tY = 0;
	while(run) {
		bool xCan = (abs(translateX) > deadZone);
		bool yCan = (abs(translateY) > deadZone);
		bool doneSomething = false;
		if (!xCan) deltaXcalc.reset();
		if (!yCan) deltaYcalc.reset();
		if (xCan) tX = tX + translateX * deltaXcalc.elapsed() * adjust;
		if (yCan) tY = tY + translateY * deltaYcalc.elapsed() * adjust;
		double temptX = tX;
		double temptY = tY;
		double XC = deltaXcalc.elapsed();
		double YC = deltaYcalc.elapsed();
		if (xCan && yCan && (abs(tX) >= 1) && (abs(tY) >= 1)){
			doneSomething = true;
			mouseMove(tX, tY);
			tX = 0; tY = 0;
			deltaXcalc.reset();
			deltaYcalc.reset();
		}
		if (xCan && abs(tX) >= 1 && !doneSomething) {
			doneSomething = true;
			mouseMove(tX, tY);
			tX = 0;
			deltaXcalc.reset();
		}
		if (yCan && abs(tY) >= 1 && !doneSomething)  {
			doneSomething = true;
			mouseMove(tX, tY);
			tY = 0;
			deltaYcalc.reset();
		}
		if (doneSomething && move_out) {
			debug << tX << ", " << tY << " [Originally " << temptX << ", " << temptY << "] || " << translateX << ", " << translateY << " AT " << XC << ", " << YC << flush;
		}// else logger << "Done Nothing, vals " << tX << " and " << tY << flush;
	}
}

void input_t() {
	pthread_setname_np(pthread_self(), "Input");
	int val = open("/dev/input/js0", O_RDONLY); //Identifier token for the joystick (controller)

	char name[128];
	int error = -2020;
	error = ioctl(val, JSIOCGNAME(sizeof(name)), name);
	if (error < 0) {
		strncpy(name, "Unknown", sizeof(name));
		err << to_string(error) << flush;
	}
	out << "Name: " <<  name << flush;
	bool leftPressed = false;
	struct js_event e;
	while (true) {
		while (run && read(val, &e, sizeof(e))) {
			e.time += 300000;                //Corrects e.time, which for some reason returns the system uptime - ~300000 ms
			if (input_out) debug << "-----RECIEVED EVENT AT: " + to_string(e.time) + "-----" << flush;
			if (input_out)
				debug << (string("Event type: ") +
						  ((e.type == 0x01) ? "Button" : ((e.type == 0x02) ? "Joystick" : "Initial Data"))) << " ("
					  << e.type << ")" << flush;
			if (input_out)
				debug << (string("Event ") + ((e.type == 1) ? "Button: " : "Axis: ") + to_string(e.number)) << flush;
			if (input_out) debug << ("Event Value: " + to_string(e.value)) << flush;
			if (input_out) debug << ("Event Time (In reference to system uptime): " + to_string(e.time)) << flush;
			//---------------------Converts e.time to hour/second/millisecond.--------------------
			//	int millis = e.time % 1000;
			//	int seconds = e.time / 1000;
			//	int hours = seconds / 60;
			//	seconds = seconds % 60;

			//--------------------Input Interpretation-------------------
			/********************
			 * BUTTON INTERPRET *
			 ********************/
			if ((e.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON && e.value == 1) {
				if (e.number == 0) input_out = !input_out;
				if (e.number == 1) move_out = !move_out;
			}
			if (e.type == 2) {
				if (e.number == 3) { //Recieves X Axis input
					translateX = e.value; //Assigns input to translateX for use in daemon thread
				};
				if (e.number == 4) { //Recieves Y axis input
					translateY = e.value; //Assigns input to translateY for use in daemon thread
				};
				if (e.number == 5 && e.value > 28000 && !leftPressed) {
					out << ("Clicking mouse 1") << flush;
					mouseClick(1, true); //Presses left mouse
					leftPressed = true;  //Sets leftPressed flag, so that left does not get set every time the trigger changes position.
				}
				if (e.number == 5 && e.value <= 28000 && leftPressed) {
					out << ("Releasing mouse 1") << flush;
					mouseClick(1, false); //Releases left mouse
					leftPressed = false;  //Sets leftPressed flag, so that left does not get set every time the trigger moves.
				}
			}
		}
		if (errno != EAGAIN) break; else debug << "Everything OK" << flush;
	}
	terminate:
	run = false;
}
string ESC = "\033";
int main(int argCount, char* argv[]) {
	string arguments[argCount - 1];
	for (int i = 1; i < argCount; i++) { //Starts at one to avoid the name of the program
		arguments[i - 1] = argv[i];
	}
	for (string arg : arguments) {
			progLogLevel = 4;
		if (arg == "-info")
			progLogLevel = 5;
		if (arg == "-debug")
			progLogLevel = 6;
		if (arg == "-verbose")
			progLogLevel = 7;
		if (arg == "-trace")
			progLogLevel = 8;
	}
	pthread_setname_np(pthread_self(), "Main");
	debug << string("Launching Debug Client Log... Client started with " + to_string(argCount - 1) + " argument(s)"); //For some reason, it is necessary to print a message out here
	debug << flush;  //To allow the logger to work. IDK, but it's better than nothing.
	out << "Launching Client..."; //For some reason, it is necessary to print a message out here
	out << flush;  //To allow the logger to work. IDK, but it's better than nothing.
	err << "Launching Error Client Log..."; //For some reason, it is necessary to print a message out here
	err << flush;  //To allow the logger to work. IDK, but it's better than nothing.
	boost::thread moveThread = boost::thread(move_t);
	struct sched_param highPri; highPri.__sched_priority = 1;
	pthread_setschedparam(moveThread.native_handle(), SCHED_BATCH, &highPri);
	boost::thread inputThread = boost::thread(input_t);
	struct sched_param lowPri; lowPri.__sched_priority = 73;
	pthread_setschedparam(inputThread.native_handle(), SCHED_BATCH, &lowPri);
	while (run) {} //Forces the main thread to keep running until told to stop. Necessary, because whenever the main thread stops running, all other threads are instantly killed.
	return 0; //Program always exits normally because reasons.
}
