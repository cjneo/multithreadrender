/*
 *  Author: zhaog
 *  Date: 2014-10-8
 */

#include <EGL/egl.h>

#include "CanvasX11.h"
#include<stdio.h>
#include <string.h>
#include <sys/time.h>
#include<iostream>
namespace World {
CanvasX11::CanvasX11() {

	this->_clientHeight = 0;
	this->_rootWin = 0;
	this->_win = 0;
	//this->_handler = 0;
	
	this->_xDisplay = NULL;
	//this->_keyFunc = NULL;
	//this->_mouseFunc = NULL;
	//this->_drawFunc = NULL;
	//-----------------------zhao add-------------
	this->_numRegistered = 0;

	//this->_disableRootEvents = true;

#if 0

	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("keydown", this->_keydown));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("mousemove",
					this->_mousemove));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("mouseup", this->_mouseup));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("mousedown",
					this->_mousedown));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("dblclick", this->_dblclick));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("mousewheel",
					this->_mousewheel));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("touchstart",
					this->_touchstart));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("touchmove",
					this->_touchmove));
	this->_callbacks.insert(
			std::pair<std::string, CallFuncList>("touchend", this->_touchend));
#endif

	//----------------------------
}

CanvasX11::~CanvasX11() {
}

bool CanvasX11::initDisplay(void) {
	this->_xDisplay = XOpenDisplay(NULL);

	if (this->_xDisplay != NULL) {
		return true;
	} else
		return false;
}

bool CanvasX11::initWindow(void) {
	XSetWindowAttributes swa;
	XSetWindowAttributes xattr;
	XWMHints hints;
	Atom wm_state;
	XEvent xev;

	_rootWin = DefaultRootWindow(this->_xDisplay);
	swa.event_mask = ButtonPressMask | ButtonReleaseMask | KeyPressMask
			| KeyReleaseMask | ButtonMotionMask | PointerMotionMask
			| PointerMotionHintMask | StructureNotifyMask; //ExposureMask | PointerMotionMask | KeyPressMask;
	_win = XCreateWindow(this->_xDisplay, _rootWin, 0, 0, this->_width,
			this->_height, 0,
			CopyFromParent, InputOutput,
			CopyFromParent, CWEventMask, &swa);

	xattr.override_redirect = FALSE;
	XChangeWindowAttributes(this->_xDisplay, _win, CWOverrideRedirect, &xattr);

	hints.input = TRUE;
	hints.flags = InputHint;
	XSetWMHints(this->_xDisplay, _win, &hints);

	// make the window visible on the screen
	XMapWindow(this->_xDisplay, _win);
	XStoreName(this->_xDisplay, _win, this->_title.c_str());

	// get identifiers for the provided atom name strings
	wm_state = XInternAtom(this->_xDisplay, "_NET_WM_STATE", FALSE);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = _win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = FALSE;
	XSendEvent(this->_xDisplay, DefaultRootWindow(this->_xDisplay),
	FALSE,
	SubstructureNotifyMask, &xev);

	this->_wndType = (EGLNativeWindowType) _win;
	return true;
}

Display* CanvasX11::get_xDisplay() {
	return _xDisplay;
}

bool CanvasX11::initContext(void) {
	EGLint attribList[] = {
	/*
	 EGL_RED_SIZE,       5,
	 EGL_GREEN_SIZE,     6,
	 EGL_BLUE_SIZE,      5,
	 EGL_ALPHA_SIZE,     (this->_flags & ES_WINDOW_ALPHA) ? 8 : EGL_DONT_CARE,
	 EGL_DEPTH_SIZE,     (this->_flags & ES_WINDOW_DEPTH) ? 8 : EGL_DONT_CARE,
	 EGL_STENCIL_SIZE,   (this->_flags & ES_WINDOW_STENCIL) ? 8 : EGL_DONT_CARE,
	 */
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_DEPTH_SIZE, 24,
	EGL_STENCIL_SIZE, 8,
	EGL_SAMPLE_BUFFERS, (this->_flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
	EGL_NONE };

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
	EGL_NONE };

	// Get Display  链接本地窗口系统，参数窗口类型
	display = eglGetDisplay((EGLNativeDisplayType) this->_xDisplay);
	if (display == EGL_NO_DISPLAY) {
		return false;
	}

	// Initialize EGL 初始化EGL
	if (!eglInitialize(display, &majorVersion, &minorVersion)) {
		return false;
	}

	// Get configs 获取surface 配置
	if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
		return false;
	}

	// Choose config
	if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) {
		return false;
	}
	
	// Create a surface  //创建一块区域
	surface = eglCreateWindowSurface(display, config,
			(EGLNativeWindowType) this->_wndType, NULL);
	if (surface == EGL_NO_SURFACE) {
		return EGL_FALSE;
	}
	
	// Create a GL context //创建一个渲染上下文数据结构，保存渲染状态和相关操作
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT) {
		return EGL_FALSE;
	}
	
	// Make the context current
	if (!eglMakeCurrent(display, surface, surface, context)) {
		return EGL_FALSE;
	}
this->_config=config;
this->_context=context;
this->_surface=surface;
this->_display=display;
	this->_eglDisplay = display;
	this->_eglSurface = surface;
	this->_eglContext = context;
	return true;
}

bool CanvasX11::userInterrupt(void) {
	XEvent xev;
	KeySym key;

	bool userinterrupt = false;

	char ch;

	while (XPending(this->_xDisplay)) {
		XNextEvent(this->_xDisplay, &xev);
		if (xev.type == KeyPress) {
			if (XLookupString(&xev.xkey, &ch, 1, &key, 0) == 1) {
				if (this->_keyFunc) {
					this->_keyFunc(ch, 0, 0);
				}
			}
		}
		if (xev.type == DestroyNotify) {
			userinterrupt = true;
		}
	}
	return userinterrupt;
}

void CanvasX11::mainLoop() {
	struct timeval t1, t2;
	struct timezone tz;
	float deltatime;
	float totaltime = 0.0f;
	unsigned int frames = 0;

	gettimeofday(&t1, &tz);

	while (!userInterrupt()) {
		gettimeofday(&t2, &tz);
		deltatime = (float) (t2.tv_sec - t1.tv_sec
				+ (t2.tv_usec - t1.tv_usec) * 1e-6);
		t1 = t2;

		if (this->_drawFunc != NULL)
			this->_drawFunc();

		eglSwapBuffers(this->_eglDisplay, this->_eglSurface);

		totaltime += deltatime;
		frames++;
		if (totaltime > 2.0f) {
			this->_frames = frames;
			this->_totalTime = totaltime;
			this->_currFps = frames / totaltime;
			//测试条件下输出：
			//printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
			//end
			totaltime -= 2.0f;
			frames = 0;
		}
	}

}
/*
void CanvasX11::registerKeyboardFunc(WorldKeyFunc keyFunc) {
	this->_keyFunc = keyFunc;
}

void CanvasX11::registerMouseFunc(WorldMouseFunc mouseFunc) {
	this->_mouseFunc = mouseFunc;
}

void CanvasX11::registerDrawFunc(WorldDrawFunc drawFunc) {
	this->_drawFunc = drawFunc;
}
*/
bool CanvasX11::createWindow(int winWidth, int winHeight, int winOptions,
		std::string winTitle) {
	this->_clientWidth = this->_width = winWidth;
	this->_clientHeight = this->_height = winHeight;

	this->_flags = winOptions;
	this->_title = winTitle.substr(0);

	bool succ = initDisplay();

	if (!succ)
		return succ;

	succ = initWindow();

	if (!succ)
		return succ;

	succ = initContext();

	if (!succ) {
		return succ;
	}
	return true;
}
bool CanvasX11::initShareContext(EGLContext shareContext) {
	EGLint attribList[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_ALPHA_SIZE, 8,
	EGL_DEPTH_SIZE, 24,
	EGL_STENCIL_SIZE, 8,
	EGL_SAMPLE_BUFFERS, (this->_flags & ES_WINDOW_MULTISAMPLE) ? 1 : 0,
	EGL_NONE };

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
	EGL_NONE };

	// Get Display  链接本地窗口系统，参数窗口类型
	display = eglGetDisplay((EGLNativeDisplayType) this->_xDisplay);
	if (display == EGL_NO_DISPLAY) {
		return false;
	}

	// Initialize EGL 初始化EGL
	//if (!eglInitialize(display, &majorVersion, &minorVersion)) {
	//	return false;
	//}

	// Get configs 获取surface 配置
	if (!eglGetConfigs(display, NULL, 0, &numConfigs)) {
		return false;
	}

	// Choose config
	if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs)) {
		return false;
	}
	
	// Create a surface  //创建一块区域
	surface = eglCreateWindowSurface(display, config,
			(EGLNativeWindowType) this->_wndType, NULL);
	if (surface == EGL_NO_SURFACE) {
		return EGL_FALSE;
	}
	
	// Create a GL context //创建一个渲染上下文数据结构，保存渲染状态和相关操作
	context = eglCreateContext(display, config, shareContext, contextAttribs);
	if (context == EGL_NO_CONTEXT) {
		std::cout<<"make context error"<<std::endl;
		EGLint err=eglGetError();
	std::cout<<"renderthread makecurrent error:"<<err<<std::endl;
	printf("%x\n",err);
		
		return EGL_FALSE;
	}
	
	// Make the context current
	if (!eglMakeCurrent(display, surface, surface, context)) {
		std::cout<<"makecurrent error"<<std::endl;
	EGLint err=eglGetError();
	std::cout<<"renderthread   error:"<<err<<std::endl;
	printf("%x\n",err);
		return EGL_FALSE;
	}
 
		EGLint err=eglGetError();
	std::cout<<"renderthread   error:"<<err<<std::endl;
	printf("%x\n",err);

this->_config=config;
this->_context=context;
this->_surface=surface;
this->_display=display;
	this->_eglDisplay = display;
	this->_eglSurface = surface;
	this->_eglContext = context;
	return true;
}
bool CanvasX11::shareContext(EGLContext shareContext){
	//this->_clientWidth = this->_width = winWidth;
	//this->_clientHeight = this->_height = winHeight;

	//this->_flags = winOptions;
	//this->_title = winTitle.substr(0);

	bool succ = initDisplay();

	if (!succ)
		return succ;

	//succ = initWindow();

	if (!succ)
		return succ;

	succ = initShareContext(shareContext);

	if (!succ) {
		return succ;
	}
	return true;
	
}
//--------------------zhaog------------------------
/*
void CanvasX11::addEventListener(std::string name, ScreenActionHandler action,
		bool bubble) {
	if (name == "DOMMouseScroll") {
		name = "mousewheel";
	}

	//CallFuncList callbacks = ;
	this->_callbacks[name].push_back(action);
	++this->_numRegistered;
}

void CanvasX11::removeEventListener(std::string name,
		ScreenActionHandler callback, bool bubble) {
	if (name == "DOMMouseScroll") {
		name = "mousewheel";
	}

	CallFunc callfun;
	CallFuncList &callbacks = this->_callbacks[name];
	int index = -1;
	for (unsigned int i = 0; i < callbacks.size(); i++) {
		callfun = (CallFunc) callbacks[i];
		if (callfun == callback) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		callbacks.erase(callbacks.begin() + index);
		--this->_numRegistered;
	}
}



void CanvasX11::fireEvents(const std::string& name, EventOption& eventOption) {
	CallFuncList funlist = this->_callbacks[name];
	for (unsigned int i = 0; i < funlist.size(); i++) {
		CallFunc func = funlist[i];
		if (func) {
			for (int j = 0; j < _handler.size(); j++) {
				func(_handler[j], &eventOption);
			}

		}
	}
}

void CanvasX11::moveMouse(Canvas& canvas, MouseButtons button,
		const Cartesian2& startPosition, const Cartesian2& endPosition,
		bool shiftKey) {
	EventOption eventOption;
	eventOption._button = button;
	eventOption._clientX = startPosition._x;
	eventOption._clientY = startPosition._y;
	eventOption._shiftKey = shiftKey;
	//void fireEvent(std::string eventName, EventOption* event)

	canvas.fireEvents("mousedown", eventOption);
	eventOption._clientX = endPosition._x;
	eventOption._clientY = endPosition._y;
	canvas.fireEvents("mousemove", eventOption);
	canvas.fireEvents("mouseup", eventOption);

}
*/
bool CanvasX11::makeCurrentImplementation() {
	bool result = eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface,
			_eglContext) == EGL_TRUE;
	//  checkEGLError("after eglMakeCurrent()");
	return result;
}

bool CanvasX11::releaseContextImplementation() {
	bool result = eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE,
	EGL_NO_CONTEXT) == EGL_TRUE;
	// checkEGLError("after eglMakeCurrent() release");
	return result;
}

}
