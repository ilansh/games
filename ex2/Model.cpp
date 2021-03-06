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

int Model::randIntInRange(int range) const{
	return ((float)rand() / RAND_MAX) * range;
}


//check if c is left of the line defined by a,b.
//if p3 is above, return value is true
bool Model::isLeft(vec2 a, vec2 b, vec2 c) const
{
	 return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

void Model::createFault()
{
	static float faultSize = 1;

	for(int i = 0; i < FAULTS_PER_CALL; i++)
	{
		vec2 p1 = vec2(randIntInRange(GRID_SIZE), randIntInRange(GRID_SIZE));
		vec2 p2 = vec2(randIntInRange(GRID_SIZE), randIntInRange(GRID_SIZE));
		while(p1 == p2)
		{
			p2 = vec2(randIntInRange(GRID_SIZE), randIntInRange(GRID_SIZE));
		}
		for(int i = 0; i < GRID_SIZE; i++)
		{
			for(int j = 0; j < GRID_SIZE; j++)
			{
				if(isLeft(p1,p2,vec2(i,j)))
				{
					_vertices[GRID_COORD(i,j)].y += faultSize;
				}
				else
				{
					_vertices[GRID_COORD(i,j)].y -= faultSize;
				}
			}
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * _vertices.size() , &(_vertices[0]), GL_STATIC_DRAW);

	faultSize /= 1.2;
}

void Model::generateGrid(std::vector<face_indices_t> &triangles)
{
	for (int i = 0; i < GRID_SIZE ; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
			_vertices[GRID_COORD(i, j)] = glm::vec4(j, 0.0, i, 1.0);
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
    glUniform4f(_fillColorUV, 0.3f, 0.9f, 0.3f, 1.0);

    // Draw using the state stored in the Vertex Array object:
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glEnableVertexAttribArray(_posAttrib);
    glVertexAttribPointer(_posAttrib, // attribute handle
			  4,          // number of scalars per vertex
			  GL_FLOAT,   // scalar type
			  GL_FALSE,
			  0,
			  0);

    glDrawElements(GL_TRIANGLES, _nVertices, GL_UNSIGNED_BYTE, (GLvoid*)0);
    glDisableVertexAttribArray(_posAttrib);
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

float Model::getPosHeight(vec3 pos)
{
	if(pos.x < 0.0 || pos.z < 0.0 || pos.x > GRID_SIZE - 1 || pos.z >  GRID_SIZE - 1)
	{
		return 0.0;
	}
	vec3 floorPoint = vec3(_vertices[GRID_COORD(floor(pos.z), floor(pos.x))]);
	return floorPoint.y;
}
