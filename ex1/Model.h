//
//  Model.h
//  cg-projects
//
//  Created by HUJI Computer Graphics course staff, 2013.
//

#ifndef __ex0__Model__
#define __ex0__Model__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // for glm::value_ptr

using namespace glm;

#include <iostream>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#else
#include <GL/gl.h>
#endif

#include <vector>

class Model {
	
private:

	//constants:
	static const int GRID_SIZE = 128;

    GLuint _vao, _vbo, _ibo;

    // Attribute handle:
    GLint _posAttrib;
	
    // Uniform handle:
    GLint _fillColorUV, _gpuWVP;
    
    size_t _nVertices;

	//model vertices
	std::vector<glm::vec4> _vertices;


    // View port frame:
    float _offsetX, _offsetY;
	int _width, _height;

	

	void generateGrid();

public:
    Model();
    virtual ~Model();
    void init();
    void draw(mat4 wvp);
    void resize(int width, int height);	
};

#endif /* defined(__ex0__Model__) */