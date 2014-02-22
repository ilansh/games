//
//  Model.cpp
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#include "ShaderIO.h"
#include "Model.h"

#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtc/matrix_transform.hpp"

#define SHADERS_DIR "shaders/"
//
Model::Model() :
    _vao(0), _vbo(0), _ibo(0)
{

}

Model::~Model()
{
    if (_vao != 0) glDeleteVertexArrays(1, &_vao);
    if (_vbo != 0) glDeleteBuffers(1, &_vbo);
    if (_ibo != 0) glDeleteBuffers(1, &_ibo);
}

void Model::generateGrid()
{
	_nVertices = GRID_SIZE * GRID_SIZE;
	for (int i = 0; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			_vertices[i * GRID_SIZE + j] = glm::vec4(j, 0.0, i, 1.0);
		}
	}

	std::vector<glm::vec3> indices;
	for (int i = 1; i < GRID_SIZE; i++)
	{
		for(int j = 0; j < GRID_SIZE; j+=2)
		{
			indices[(i - 1) * GRID_SIZE + j] = glm::vec3(j, j + 1, i * GRID_SIZE);
			indices[(i - 1) * GRID_SIZE + j] = glm::vec3(j, j + 1, i * GRID_SIZE + 1);
		}
	}
	
}

void Model::init()
{
    programManager::sharedInstance()
	.createProgram("default",
                       SHADERS_DIR "SimpleShader.vert",
                       SHADERS_DIR "SimpleShader.frag");

    GLuint program = programManager::sharedInstance().programWithID("default");
		
    // Obtain uniform variable handles:
    _fillColorUV  = glGetUniformLocation(program, "fillColor");
    _gpuWVP  = glGetUniformLocation(program, "wvp");

    // Initialize vertices buffer and transfer it to OpenGL
    {
        // positioning vertices
        static const GLfloat vertices[] = {
            0.0f,  -1.0f, 0.0f, 1.0f,
            1.0f,  -1.0f, 0.0f, 1.0f,
            0.0f,  -1.0f, 1.0f, 1.0f,
            1.0f,  -1.0f, 1.0f, 1.0f,
        };
        
        // connecting vertices into triangles
        static const GLubyte indices[] = {
            0,1,3,
            0,2,3
        };      	
        _nVertices = 6;

        // Create and bind the object's Vertex Array Object:
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
		
        // Create and load vertex data into a Vertex Buffer Object:
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        // Tells OpenGL that there is vertex data in this buffer object and what form that vertex data takes:
        // Obtain attribute handles:
        _posAttrib = glGetAttribLocation(program, "position");
        glEnableVertexAttribArray(_posAttrib);
        glVertexAttribPointer(_posAttrib, // attribute handle
                              4,          // number of scalars per vertex
                              GL_FLOAT,   // scalar type
                              GL_FALSE,
                              0,
                              0);
		
        // Unbind vertex array:
        glBindVertexArray(0);
    }
}

void Model::draw(mat4 wvp)
{
    // Set the program to be used in subsequent lines:
    glUseProgram(programManager::sharedInstance().programWithID("default"));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Also try using GL_FILL and GL_POINT
  
    // pass the wvp to vertex shader
    glUniformMatrix4fv (_gpuWVP, 1, GL_FALSE, value_ptr(wvp));
    
    // pass the model color to fragment shader   
    glUniform4f(_fillColorUV, 0.3f, 0.5f, 0.3f, 1.0);

    // Draw using the state stored in the Vertex Array object:
    glBindVertexArray(_vao);
	
   
    glDrawElements(GL_TRIANGLES, _nVertices, GL_UNSIGNED_BYTE, (GLvoid*)0);

    // Unbind the Vertex Array object
    glBindVertexArray(0);
	
    // Cleanup, not strictly necessary
    glUseProgram(0);
}

void Model::resize(int width, int height)
{
    _width = width;
    _height = height;
    _offsetX = 0;
    _offsetY = 0;
}