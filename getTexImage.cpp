#include<stdio.h>
#include<vector>
#include<iostream>
#include <GLES2/gl2.h>
using namespace std;

/*
we can not use glGetTexImage in gles ,what a pity.
*/
/*
void readTexture2D(GLenum target){
	GLubyte pixel[4 * 20]={0};
	glBindTexture ( GL_TEXTURE_2D, target ); 
  GLint level=0;
  GLenum format=GL_RGB ;
  GLenum type=GL_UNSIGNED_BYTE; 
  GLvoid *pixels= (GLvoid*)pixel;
int width=0,height=0;
//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &internalFormat); // get internal format type of GL texture
//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
//glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture
//  glGetTexImage(target,level,format,type,&pixels);

  unsigned char * str=(unsigned char*)pixels;
printf("output texture:    ");
  for(int i=0;i<12;i++){
	printf(" %d ",str[i]);	
}
printf("\n");
}
*/
#if 0
int main(){
}
#endif
void readTexture2D(int width,int height){
	vector< unsigned char > pixels( width * height * 4 ,0);
    glReadPixels
        ( 
        0, 0, 
        width, height, 
        GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0] 
        );
GLenum error = glGetError();
	printf("error: %x\n",error);
for(int i=0;i<height;i++){
	for(int j=0;j<width;j++){
	int offset=(i*width+j)*4;
    cout << "|" << (int)pixels[0+offset];
    //cout << " " << (int)pixels[1+offset] ;
    //cout << " " << (int)pixels[2+offset] ;
    //cout << " " << (int)pixels[3+offset] ;
	cout<<"|";
	}
    cout << endl;
}
}

void GetTexFromFrameBuffer(GLuint fboname){
	//Bind the FBO
glBindFramebuffer(GL_FRAMEBUFFER, fboname);
// set the viewport as the FBO won't be the same dimension as the screen
int width=2,height=2;
glViewport(0, 0, width, height);
vector< unsigned char > pixels ( width * height * 4 ,0);
 
glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
GLenum error = glGetError();
	printf("error: %x\n",error);
//Bind your main FBO again
glBindFramebuffer(GL_FRAMEBUFFER,0);
// set the viewport as the FBO won't be the same dimension as the screen
glViewport(0, 0, 250, 250);
for(int i=0;i<height;i++){
	for(int j=0;j<width;j++){
	int offset=(i*width+j)*4;
    cout << "|" << (int)pixels[0+offset];
    cout << " " << (int)pixels[1+offset] ;
    cout << " " << (int)pixels[2+offset] ;
    cout << " " << (int)pixels[3+offset] ;
	cout<<"|";
	}
    cout << endl;
}
}
 GLuint Texture2DUseFramBuffer( )
{
   GLuint fboname;
   glGenFramebuffers(1,&fboname);
   glBindFramebuffer(GL_FRAMEBUFFER,fboname);
   // Texture object handle
   GLuint textureId;
   
   // 2x2 Image, 3 bytes per pixel (R, G, B)
   GLubyte pixels[4 * 3] =
   {  
      255,   123,   0, // Red
        0, 255,   123, // Green
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

   glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,textureId
,0);
GLenum status=glCheckFramebufferStatus(GL_FRAMEBUFFER);
if(status!=GL_FRAMEBUFFER_COMPLETE){
   printf("failed to make complete framebuffer %x\n",status);
}
else{
	std::cout<<"it is ok"<<std::endl;
}
glBindFramebuffer(GL_FRAMEBUFFER,0);
GetTexFromFrameBuffer(fboname);
glFinish();
 for(int i=0;i<12;i++){
	printf(" %d ",pixels[i]);	
}
   return textureId;

}





