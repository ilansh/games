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


#define DEBUG_PRINT(s, x) std::cout << s << ": " << x << std::endl;

#define GRID_COORD(z, x) (z) * GRID_SIZE + (x)

Model::Model() :
    _vao(0), _vbo(0), _ibo(0), _vertices(GRID_SIZE*GRID_SIZE)
{

}

Model::~Model()
{
    if (_vao != 0) glDeleteVertexArrays(1, &_vao);
    if (_vbo != 0) glDeleteBuffers(1, &_vbo);
    if (_ibo != 0) glDeleteBuffers(1, &_ibo);
}


void Model::generateGrid(std::vector<face_indices_t> &triangles)
{
	for (int i = 0; i < GRID_SIZE ; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			_vertices[GRID_COORD(i, j)] = glm::vec4(-j + GRID_SIZE / 2, -1.0, i - GRID_SIZE / 2, 1.0);
		}
	}

	int triCount = 0;
	for (int i = 0; i < GRID_SIZE - 1; i++)
	{
		for(int j = 0; j < GRID_SIZE - 1; j++)
		{
			triangles[triCount].a = GRID_COORD(i, j);
			triangles[triCount].b = GRID_COORD(i, j + 1);
			triangles[triCount + 1].a = GRID_COORD(i + 1, j);
			triangles[triCount + 1].b = GRID_COORD(i + 1, j + 1);
			if((j % 2 == 0 && i % 2 == 0) || (j % 2 != 0 && i % 2 != 0))
			{
				triangles[triCount].c = GRID_COORD(i + 1, j + 1);
				triangles[triCount + 1].c = GRID_COORD(i, j);
			}
			else
			{
				triangles[triCount].c = GRID_COORD(i + 1, j);
				triangles[triCount + 1].c = GRID_COORD(i, j + 1);
			}


			triCount += 2;
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
    	int numTriangles = (GRID_SIZE - 1) * (GRID_SIZE - 1) * 2;
        _nVertices = numTriangles * 3;

        std::vector<face_indices_t> triangles(numTriangles);
        generateGrid(triangles);


        // Create and bind the object's Vertex Array Object:
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
		
        // Create and load vertex data into a Vertex Buffer Object:
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * _vertices.size() , &(_vertices[0]), GL_STATIC_DRAW);

        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_indices_t) * numTriangles, &(triangles[0]), GL_STATIC_DRAW);
        
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

void Model::draw(mat4 world, mat4 view, mat4 projection)
{
    // Set the program to be used in subsequent lines:
    glUseProgram(programManager::sharedInstance().programWithID("default"));

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Also try using GL_FILL and GL_POINT

    // pass the wvp to vertex shader
    glUniformMatrix4fv (_gpuWVP, 1, GL_FALSE, value_ptr(projection * view * world));
    
    // pass the model color to fragment shader   
    glUniform4f(_fillColorUV, 0.3f, 0.5f, 0.3f, 1.0);

    // Draw using the state stored in the Vertex Array object:
    glBindVertexArray(_vao);
	
   
    glDrawElements(GL_TRIANGLES, _nVertices, GL_UNSIGNED_INT, (GLvoid*)0);

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

//void Model::moveForward()
//{
//	pos = pos + vec3(0,0,1);
//	View = lookAt(pos, pos+dir, up);
//}
