#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;
private:
	void reset();

	glm::vec2 drawProjection(glm::vec4 point);
	
        void drawCube();
	void drawWorldGnom();
	void drawCubeGnom();
	void drawViewport();

	glm::mat4 createViewMatrix(glm::vec3 lookAt, glm::vec3 lookFrom, glm::vec3 up);	
	void createProj(float fovDegrees, float near, float far, float aspect);

	glm::mat4 translate(float xDiff, float yDiff, float zDiff);
	glm::mat4 rotate(char axis, float degrees);
	glm::mat4 scale(float xScale, float yScale, float zScale);

	bool clipZ(glm::vec4 &point1, glm::vec4 &point2);
	bool clipXY(glm::vec2 &point1, glm::vec2 &point2);

	glm::vec2 orthographicProjection(glm::vec4 point);
	glm::vec2 projection(glm::vec4 point);

	glm::mat4 worldMat;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 model;
	glm::mat4 modelScale;
	float fovDegrees;
	float near;
	float far;
	float aspect;
	bool mouseLeftPressed;
	bool mouseRightPressed;
	bool mouseMiddlePressed;
	int mode;
	double oldX;
	char* modes[7] = {"O", "N", "P", "R", "T", "S", "V"};
	float lowXBoundary;
	float highXBoundary;
	float lowYBoundary;
	float highYBoundary;
};
