#include"CanvasX11.h"
#include<iostream>
#include "Canvas.h"
#include "CanvasX11.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <pthread.h> 
#include <unistd.h> 
#include<stdio.h>
#include<string.h>
using namespace World;
using namespace std;
typedef struct
{
   GLuint programObject;

} UserData;
UserData  userData;
pthread_t       ntid; 
CanvasX11 *_canvas;

void printids(const char *s) 
{ 
    pid_t   pid; 
    pthread_t   tid; 
 
    pid = getpid(); 
    tid = pthread_self(); 
 
    printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid, 
        (unsigned int)tid, (unsigned int)tid); 
} 


 GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 ){
	cout<<"create sharder error"<<endl;
   	return 0;
	
	}
cout<<"create sharder ok"<<endl;
   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = (char *)malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         cout<<"Error compiling shader:"<< infoLog<<endl;            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

///
// Initialize the shader and program object
//
void * Init (  void * );
void * ThreadInit(void *){
printids("new thread:");

	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE,
	EGL_NONE };


	EGLContext threadContext=eglCreateContext(_canvas->_display,
	_canvas->_config,_canvas->_context,contextAttribs);
	if (threadContext == EGL_NO_CONTEXT) {
		EGLint err=eglGetError();
	cout<<"renderthread makecurrent error:"<<err<<endl;
printf("%x\n",err);
		cout<<"renderthread context error"<<endl;		
		return EGL_FALSE;
	}
	//EGLint err=eglGetError();
	//cout<<" error:"<<err<<endl;
	//printf("%x\n",err);
	//if (!eglMakeCurrent(_canvas->_display, _canvas->_surface, _canvas->_surface, threadContext)) {
	if (!eglMakeCurrent(_canvas->_display, EGL_NO_SURFACE, EGL_NO_SURFACE, threadContext)) {
		EGLint err=eglGetError();
	cout<<"renderthread makecurrent error:"<<err<<endl;
printf("%x\n",err);
	//const GLubyte* errstr=eglErrorString(err);
	//cout<<errstr<<endl;		
 	return EGL_FALSE;
	}
#if 0
CanvasX11 *threadcanvas=new CanvasX11();
	//*threadcanvas=*_canvas;
	if(!threadcanvas->shareContext(_canvas->_context)){
		cout<<"share context error"<<endl;	
	}
	cout<<"what"<<endl;
#endif	
Init(0);

}
void * Init (  void * )
{
   //UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, (const char *)vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, (const char *)fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 ){
	cout<<"can not create program object"<<endl;      
	return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = (char *)malloc (sizeof(char) * infoLen );

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         cout <<"Error linking program:"<< infoLog <<endl;            
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
      return FALSE;
   }

   // Store the program object
   userData.programObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return (void *)TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( CanvasX11 *_canvas  )
{
   //UserData *userData = esContext->userData;
   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };
      
   // Set the viewport
   glViewport ( 0, 0, _canvas->_width, _canvas->_height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData.programObject );

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLES, 0, 3 );

   eglSwapBuffers ( _canvas->_eglDisplay, _canvas->_eglSurface );
}

void renderThread(){
	int err; 
	err = pthread_create(&ntid, NULL, ThreadInit, NULL); 
    if ( 0 != err ) 
    { 
        printf("can't create thread:%s\n", strerror(err)); 
    }
	//return 1;
}

int main(){
	//CanvasX11 *_canvas;
	_canvas=new CanvasX11();
	if(!_canvas->createWindow(250,250,0,"")){
	   cout<<"create window error"<<endl;	
	}
	 
	cout<<"ok"<<endl;
	/* if ( !Init ( ) ){
		cout<<"ERROR"<<endl;            
		return 0;
	}
*/

#if 1
	//create thread
	renderThread();
	
//ThreadInit(0);	 
	int *ret; 
    pthread_join(ntid, (void **)(&ret)); 
   //printf("thr_fn return %d\n", (int)(*ret));
 #endif
//Init(0);
int i=0;
while(i<3)
{
	Draw(_canvas);
	i++;
	sleep(1);
	cout<<i<<endl;
	//EGLint err=eglGetError();
}
cout<<"finish<<"<<endl;
//glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
//glClear ( GL_COLOR_BUFFER_BIT );
//eglSwapBuffers ( _canvas->_eglDisplay, _canvas->_eglSurface );
//glDeleteProgram(userData.programObject);
EGLint err22=eglGetError();
	cout<<"renderthread makecurrent error:"<<err22<<endl;
printf("%x\n",err22);
while(true){
}
}

