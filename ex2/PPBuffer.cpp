//
//  PPBuffer.cpp
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#include "ShaderIO.h"
#include "PPBuffer.h"
#include <cstdio>

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#define SHADERS_DIR "shaders/"
//
PPBuffer::PPBuffer() {

}

PPBuffer::~PPBuffer() {
  glDeleteRenderbuffers(1, &rbo_depth);
  glDeleteTextures(1, &fbo_texture);
  glDeleteFramebuffers(1, &fbo);

  glDeleteBuffers(1, &vbo_fbo_vertices);

  glDeleteProgram(program_postproc); 
} 

void PPBuffer::init(int screen_width, int screen_height)
{
  // buffer
  // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
 
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);


  /* Texture  */
	
  // The texture we're going to render to
  glGenTextures(1, &fbo_texture);
  
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA,screen_width, screen_height, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glBindTexture(GL_TEXTURE_2D, 0);
  
  // depth buffer
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screen_width, screen_height );
  
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  
  /* Framebuffer to link everything together */
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fbo_texture, 0); //use more of these for MRT
  // glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, fbo_texture1, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth);

  // Set the list of draw buffers.
  GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
  
  glDrawBuffers(1,DrawBuffers); 

  // for MRT:  glDrawBuffers(2,{GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1}); 

  GLenum status;
  if ((status = glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "glCheckFramebufferStatus: error %d", status);
    exit(0);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  // vertices 
  GLfloat fbo_vertices[] = {
    -1, -1,
    1, -1,
    -1,  1,
    1,  1,
  };

  // Create and bind the object's Vertex Array Object:
  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glGenBuffers(1, &vbo_fbo_vertices);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
  glBufferData(GL_ARRAY_BUFFER, sizeof(fbo_vertices), fbo_vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);  

  glBindVertexArray(0);
  // shader
  programManager::sharedInstance()
    .createProgram("pp",
		   SHADERS_DIR "postproc.v.glsl",
		   SHADERS_DIR "postproc.f.glsl");
  program_postproc = programManager::sharedInstance().programWithID("pp");

  attribute_v_coord_postproc = glGetAttribLocation(program_postproc, "v_coord");
  uniform_fbo_texture = glGetUniformLocation(program_postproc, "fbo_texture");
}

void PPBuffer::setup() {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

void PPBuffer::render() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glClearColor(0.3, 0.0, 0.0, 1.0);

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  
  glUseProgram(program_postproc);

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glUniform1i(uniform_fbo_texture, /*GL_TEXTURE*/0);
 
  glBindVertexArray(_vao);
  glEnableVertexAttribArray(attribute_v_coord_postproc);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_fbo_vertices);
  glVertexAttribPointer(
			attribute_v_coord_postproc,  // attribute
			2,                  // number of elements per vertex, here (x,y)
			GL_FLOAT,           // the type of each element
			GL_FALSE,           // take our values as-is
			0,                  // no extra data between each position
			0                   // offset of first element
			);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(attribute_v_coord_postproc);
  glBindBuffer(GL_ARRAY_BUFFER,0);
  glBindVertexArray(0);
  glUseProgram(0);
}

void PPBuffer::resize(int screen_width, int screen_height) {
  
  /* onReshape */
  // Rescale FBO and RBO as well
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screen_width, screen_height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  
  _width = screen_width;
  _height = screen_height;
}
