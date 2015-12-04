#ifndef CANVASX11_H
#define CANVASX11_H

#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
#include <vector>
#include <map>
#include "Canvas.h"
#include <EGL/egl.h>
//#include "ScreenSpaceEventHandler.h"

namespace World {

class CanvasX11: public Canvas {

public:
	Window _win;

private:
	virtual bool initDisplay(void);

	virtual bool initWindow(void);

	virtual bool initContext(void);

	virtual bool userInterrupt(void);

public:

	CanvasX11();

	virtual ~CanvasX11();

	virtual bool createWindow(int winWidth, int winHeight, int winOptions,
			std::string winTitle = "");

	virtual void mainLoop(void);

	//virtual void registerKeyboardFunc(WorldKeyFunc keyFun);

	//virtual void registerMouseFunc(WorldMouseFunc mouseFunc);

	//virtual void registerDrawFunc(WorldDrawFunc drawFunc);

	//virtual void fireEvents(const std::string& name, EventOption& eventOption);

	//virtual void addEventListener(std::string name, ScreenActionHandler action,
	//		bool bubble);

	virtual void removeEventALLListener(){

	}

	virtual bool makeCurrentImplementation() ;

	virtual bool releaseContextImplementation();

	//void removeEventListener(std::string name, ScreenActionHandler callback,
	//		bool bubble);

	//static void moveMouse(Canvas& canvas, MouseButtons button,
	//			const Cartesian2& startPosition, const Cartesian2& endPosition,
	//			bool shiftKey = false);

	Display* get_xDisplay();

public:

	Display* _xDisplay;
	
	Window _rootWin;
public:
	EGLContext _context;
	EGLConfig _config;
	EGLDisplay _display;
	EGLSurface _surface;
bool  initShareContext(EGLContext shareContext);
bool shareContext(EGLContext shareContext);
private:

	//typedef ScreenActionHandler CallFunc;
typedef void * CallFunc;
	typedef std::vector<CallFunc> CallFuncList;

	typedef std::map<std::string, CallFuncList> CallBacks;

	CallBacks _callbacks;

#if 0  //no use
	std::vector<CallFunc> _keydown;
	std::vector<CallFunc> _mousemove;
	std::vector<CallFunc> _mouseup;
	std::vector<CallFunc> _mousedown;
	std::vector<CallFunc> _dblclick;
	std::vector<CallFunc> _mousewheel;
	std::vector<CallFunc> _touchstart;
	std::vector<CallFunc> _touchmove;
	std::vector<CallFunc> _touchend;
#endif

	int _numRegistered;



	bool _disableRootEvents;

};
}

#endif // CANVASX11_H
