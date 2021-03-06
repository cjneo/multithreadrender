/*
 *   Author: zhaog
 *   Date: 2014-10-8
 */

#ifndef CANVAS_H
#define CANVAS_H
#include <iostream>
#include  <string>
#include "ALLNEED.h"
//#include "util/stdtypes.h"
//#include "Core/ScreenSpaceEventOption.h"
//#include "WorldPrerequisites.h"

//#include "Cartesian2.h"

#define CALLBACK

#define ES_WINDOW_RGB           0
#define ES_WINDOW_ALPHA         1
#define ES_WINDOW_DEPTH         2
#define ES_WINDOW_STENCIL       4
#define ES_WINDOW_MULTISAMPLE   8

namespace World {

struct ClientRect {
	int left;
	int top;
	int right;
	int bottom;
};

typedef void (CALLBACK *WorldKeyFunc)(unsigned char, int, int);
typedef void (CALLBACK *WorldMouseFunc)(int, int, int, int);
typedef void (CALLBACK *WorldDrawFunc)();

class Canvas {

private:
	virtual bool initDisplay(void) {
		return false;
	}

	virtual bool initWindow(void) {
		return false;
	}

	virtual bool initContext(void) {
		return false;
	}

	virtual bool userInterrupt(void) {
		return false;
	}

public:
	Canvas() {
		_width = 0;
		_height = 0;
		_flags = 0;

		_keyFunc = 0;
		_mouseFunc = 0;
		_drawFunc = 0;

		_currFps = 0;
		_frames = 0;
		_totalTime = 0;

		_wndType = 0;
		_eglDisplay = 0;
		_eglContext = 0;
		_eglSurface = 0;
		_eglConfig = 0;

		_clientWidth = 0;
		_clientHeight = 0;
		//_handler.clear();
		disableRootEvents = 0;
	}
	virtual ~Canvas() {

	}
 
	virtual bool createWindow(int winWidth, int winHeight, int winOptions,
			std::string winTitle = "") =0; //{return false;}
	virtual void mainLoop(void) {
	}

	virtual void registerKeyboardFunc(WorldKeyFunc keyFunc) {
	}
	virtual void registerMouseFunc(WorldMouseFunc mouseFunc) {
	}
	virtual void registerDrawFunc(WorldDrawFunc drawFunc) {
	}
	//---
//	virtual void addEventListener(std::string name, ScreenActionHandler action,
//			bool bubble) = 0;

//	virtual void removeEventListener(std::string name,
//			ScreenActionHandler action, bool bubble) = 0;

	virtual void removeEventALLListener() = 0;
	//virtual void fireEvents(const std::string& name,
	//		EventOption& eventOption) = 0;

	virtual bool makeCurrentImplementation() = 0;

	virtual bool releaseContextImplementation() = 0;

	ClientRect getBoundingClientRect() {
		ClientRect rect;
		rect.left = 0;
		rect.bottom = 0;
		rect.right = this->_clientWidth;
		rect.top = this->_clientHeight;
		return rect;
	}
/*
	void dump() {
		std::cout << RED << "Canvas()------------{" << GREEN << std::endl;
		{
			std::cout << "_width:" << _width << std::endl;
			std::cout << "_height:" << _height << std::endl;
			std::cout << "_clientWidth:" << _clientWidth << std::endl;
			std::cout << "_clientHeight:" << _clientHeight << std::endl;

		}
		std::cout << RED << "Canvas()------------}" << RESET << std::endl;
	}
*/
	//---
public:
	int _width;
	int _height;
	int _flags;
	std::string _title;

	WorldKeyFunc _keyFunc;
	WorldMouseFunc _mouseFunc;
	WorldDrawFunc _drawFunc;

	float _currFps;
	int _frames;
	float _totalTime;

	unsigned long _wndType;
	void* _eglDisplay;
	void* _eglContext;
	void* _eglSurface;
	void* _eglConfig;

	int _clientWidth;
	int _clientHeight;

	//std::vector<ScreenSpaceEventHandler*> _handler;
	//--
	//std::map<std::string, ScreenActionHandler> _callbackActionList;
	bool disableRootEvents;

	//--
};
}

#endif // CANVAS_H
