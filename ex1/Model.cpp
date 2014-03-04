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

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Model::Model() :
    _vao(0), _vbo(0), _ibo(0), _vertices(GRID_SIZE*GRID_SIZE), _faults(GRID_SIZE*GRID_SIZE)
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
}

void Model::generateGrid(std::vector<face_indices_t> &triangles)
{
	for (int i = 0; i < GRID_SIZE ; i++)
	{
		for(int j = 0; j < GRID_SIZE; j++)
		{
//			_vertices[GRID_COORD(i, j)] = glm::vec4(-j + GRID_SIZE / 2, 0.0, i - GRID_SIZE / 2, 1.0);
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

		
        // Create and load vertex data into a Vertex Buffer Object:
        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        //&(_vertices[0])
        glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(GLfloat)) * _vertices.size() , NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0 , sizeof(vec4) * _vertices.size(), &(_vertices[0]));
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _vertices.size() , sizeof(GLfloat) * _faults.size(), &(_faults[0]));

        glGenBuffers(1, &_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_indices_t) * numTriangles, &(triangles[0]), GL_STATIC_DRAW);
        


        // Create and bind the object's Vertex Array Object:
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

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

        _faultAttrib = glGetAttribLocation(program, "faultVal");
        glEnableVertexAttribArray(_faultAttrib);
        glVertexAttribPointer(_faultAttrib, // attribute handle
                              1,          // number of scalars per vertex
                              GL_FLOAT,   // scalar type
                              GL_FALSE,
                              0,
                              BUFFER_OFFSET(sizeof(vec4) * _vertices.size()));
		
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

float distanceXZ(vec3 a, vec3 b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
}

float Model::getPosHeight(vec3 pos)
{
	if(pos.x < 0.0 || pos.z < 0.0 || pos.x > GRID_SIZE - 1 || pos.z >  GRID_SIZE - 1)
	{
		return 0.0;
	}

//	vec3 lowerLeft = vec3(_vertices[GRID_COORD(floor(pos.z), floor(pos.x))]);
//	vec3 upperRight = vec3(_vertices[GRID_COORD(ceil(pos.z), ceil(pos.x))]);
//	vec3 lowerRight = vec3(_vertices[GRID_COORD(floor(pos.z), ceil(pos.x))]);
//	vec3 upperLeft = vec3(_vertices[GRID_COORD(ceil(pos.z), floor(pos.x))]);

	vec3 floorPoint = vec3(_vertices[GRID_COORD(floor(pos.z), floor(pos.x))]);
//	float dist = (0 - 0) * (0 - 0) + (0 - 0) * (0 - 0) + (pos.z - closestPoint.z) * (pos.z - closestPoint.z);
//	DEBUG_VECTOR("pos: ", pos);
//	DEBUG_VECTOR("closestPoint: ", closestPoint);
//	if(dotProd > EPSILON)
//	{
//		dist = sqrt(dotProd);
//	}


//	DEBUG_VECTOR("lower left:  ", lowerLeft);
//	DEBUG_VECTOR("upper right: ",upperRight);

//	float h1 = lowerLeft.y;
//	float h2 = upperRight.y;
//	float h3 = lowerRight.y;
//	float h4 = upperLeft.y;
//
//	float dist1W = (M_SQRT2 - distanceXZ(pos, lowerLeft)) / M_SQRT2;
//	float dist2W = (M_SQRT2 - distanceXZ(pos, upperRight)) / M_SQRT2;
//	float dist3W = (M_SQRT2 - distanceXZ(pos, lowerRight)) / M_SQRT2;
//	float dist4W = (M_SQRT2 - distanceXZ(pos, upperLeft)) / M_SQRT2;
//
//	float weight = distanceXZ(pos, floorPoint) / M_SQRT2;
//	DEBUG_PRINT("weight: ",weight);
//	float h = ;
	return floorPoint.y;

//	return (h1 + h2 + h3 + h4) / 4;

//	DEBUG_PRINT("dist",dist);
//	return closestPoint.y;
}
