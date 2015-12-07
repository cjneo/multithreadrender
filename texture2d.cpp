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
#include "cjesUtil.h"
using namespace World;
using namespace std;
typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Texture handle
   GLuint textureId;

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


 GLuint CreateSimpleTexture2D( )
{
   // Texture object handle
   GLuint textureId;
   
   // 2x2 Image, 3 bytes per pixel (R, G, B)
   GLubyte pixels[4 * 3] =
   {  
      255,   0,   0, // Red
        0, 255,   0, // Green
        0,   0, 255, // Blue
      255, 255,   0  // Yellow
   };

   // Use tightly packed data
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );

   // Generate a texture object
   glGenTextures ( 1, &textureId );

   // Bind the texture object
   glBindTexture ( GL_TEXTURE_2D, textureId );

   // Load the texture
   glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels );

   // Set the filtering mode
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

   return textureId;

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
	 	
 	return EGL_FALSE;
	}
 
userData.textureId=CreateSimpleTexture2D();
}
bool  Init (   )
{
//UserData *userData = esContext->userData;
   GLbyte vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";

   // Load the shaders and get a linked program object
   userData.programObject = esLoadProgram ( vShaderStr, fShaderStr );

   // Get the attribute locations
   userData.positionLoc = glGetAttribLocation ( userData.programObject, "a_position" );
   userData.texCoordLoc = glGetAttribLocation ( userData.programObject, "a_texCoord" );
   
   // Get the sampler location
   userData.samplerLoc = glGetUniformLocation ( userData.programObject, "s_texture" );

   // Load the texture
  // userData.textureId = CreateSimpleTexture2D ();

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw (   )
{
    
   GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
      
   // Set the viewport
   //glViewport ( 0, 0, esContext.width, esContext.height );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData.programObject );

   // Load the vertex position
   glVertexAttribPointer ( userData.positionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData.texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( userData.positionLoc );
   glEnableVertexAttribArray ( userData.texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData.textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData.samplerLoc, 0 );

   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

   //eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
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
	_canvas=new CanvasX11();
	if(!_canvas->createWindow(250,250,0,"")){
	   cout<<"create window error"<<endl;	
	}
	 glViewport ( 0, 0, 250, 250 );
	cout<<"ok"<<endl;
	 Init ();

#if 1
	//create thread
	renderThread();	 
	int *ret; 
    pthread_join(ntid, (void **)(&ret)); 
//glDeleteProgram(userData.programObject);
   //printf("thr_fn return %d\n", (int)(*ret));
 #endif
//Init(0);
int i=0;
while(i<3)
{
   //renderThread();	 
//	int *ret; 
   // pthread_join(ntid, (void **)(&ret)); 
	Draw( );
	i++;
	sleep(1);
	cout<<i<<endl;
	//EGLint err=eglGetError();
}
cout<<"finish<<"<<endl;
glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
glClear ( GL_COLOR_BUFFER_BIT );
eglSwapBuffers ( _canvas->_eglDisplay, _canvas->_eglSurface );
//glDeleteProgram(userData.programObject);
EGLint err22=eglGetError();
	cout<<"renderthread makecurrent error:"<<err22<<endl;
printf("%x\n",err22);
while(true){
}
}

