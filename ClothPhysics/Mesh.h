#pragma once
#ifndef MESH_H
#define MESH_H

#include "glm\glm.hpp"
#include <GL\glew.h>
#include "obj_loader.h"
#include "Vertex.h"

class Mesh
{
public:
	Mesh() {};
	Mesh(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices, GLenum DRAWTYPE = GL_STATIC_DRAW);
	Mesh(const std::string& fileName, GLenum DRAWTYPE = GL_STATIC_DRAW);
	~Mesh();

	void Draw(GLenum DRAWTYPE = GL_TRIANGLES);
	void Draw(GLenum DRAWTYPE = GL_TRIANGLES) const;

protected:
	IndexedModel CreateIndexedModel(Vertex* vertices, unsigned int numVertices, unsigned int* indices, unsigned int numIndices);

	void StandardDraw();

	void UpdateModel(GLenum DRAWTYPE = GL_STATIC_DRAW);

	void UploadToGPU();

	void InitMesh(const IndexedModel& model, bool updateModel = false, GLenum DRAWTYPE = GL_STATIC_DRAW);

	IndexedModel m_model;

private:
	enum
	{
		POSITION_VB,
		TEXCOORD_VB,
		INDEX_VB,
		NORMAL_VB,

		NUM_BUFFERS
	};

	GLenum m_DRAWTYPE;
	void GenerateBufferAndVertexArray();
	GLuint m_vertexArrayObject;
	GLuint m_vertexArrayBuffers[NUM_BUFFERS];
	unsigned int m_drawCount;
};

#endif //MESH_H