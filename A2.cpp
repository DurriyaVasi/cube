#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)), worldMat(mat4(1.0f)), view(mat4(1.0f)), proj(mat4(1.0f)), model(mat4(1.0f)), modelScale(mat4(1.0f)), fovDegrees(60.0f), near(-2.0f), far(8.0f), aspect(1.0f), mouseLeftPressed(false), mouseRightPressed(false), mouseMiddlePressed(false), mode(0), oldX(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

	createProj(fovDegrees, near, far, aspect);
	view = createViewMatrix(vec3(0.0f, 0.0f, 0.0f), vec3(-2.0f, -4.0f, -4.0f), vec3(0.0f, 1.0f, 0.0f) );
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

mat4 A2::createViewMatrix(vec3 lookAt, vec3 lookFrom, vec3 up) {
	vec3 vz = normalize(lookAt - lookFrom);
	vec3 vx = normalize(cross(up, vz));
	vec3 vy = cross(vz, vx);
	mat4 r(0.0f);
	r[0][0] = vx[0]; r[1][0] = vx[1]; r[2][0] = vx[2];
	r[0][1] = vy[0]; r[1][1] = vy[1]; r[2][1] = vy[2];
	r[0][2] = vz[0]; r[1][2] = vz[1]; r[2][2] = vz[2];
	mat4 t(1.0f);
	t[3][0] = lookFrom[0] * (-1);
	t[3][1] = lookFrom[1] * (-1);
	t[3][2] = lookFrom[2] * (-1);

	//cout << "vx " << vx << endl;
	//cout << "vy " << vy << endl;
	//cout << "vz " << vz << endl;
	//cout << r << endl;
	//cout << t << endl;

	return (r * t);
}			

void A2::createProj(float fovDegrees, float near, float far, float aspect) {
	proj = mat4(0.0f);
	float cottheta = (1/(tan(radians(fovDegrees/2))));
	proj[0][0] = cottheta/aspect;
	proj[1][1] = cottheta;
	proj[2][2] = 1 * (far + near)/(far - near);
	proj[3][2] = (-2 * far * near)/(far - near);
	proj[2][3] = 1;
}

vec2 A2::orthographicProjection(vec4 point) {
	vec2 orthProj = vec2(point[0]/3, point[1]/3);
	return orthProj;
}

vec2 A2::projection(vec4 point) {
	vec4 point2 = proj * view * worldMat * modelScale * model * point;
	point2 = normalize(point2);
	vec2 ans(point2[0], point2[1]);
	return ans;
}

mat4 A2::translate(float xDiff, float yDiff, float zDiff) {
	mat4 trans(1.0f);
	trans[3][0] = xDiff;
	trans[3][1] = yDiff;
	trans[3][2] = zDiff;
	return trans;
}

mat4 A2::rotate(char axis, float degrees) {
	mat4 rot(1.0f);
	if (axis == 'x') {
		rot[1][1] = cos(radians(degrees));
		rot[1][2] = sin(radians(degrees));
		rot[2][1] = -1 * sin(radians(degrees));
		rot[2][2] = cos(radians(degrees));
	}
	if (axis == 'y') {
		rot[0][0] = cos(radians(degrees));
		rot[2][0] = sin(radians(degrees));
		rot[0][2] = -1 * sin(radians(degrees));
		rot[2][2] = cos(radians(degrees));
	}
	if (axis == 'z') {
		rot[0][0] = cos(radians(degrees));
		rot[1][0] = -1 * sin(radians(degrees));
		rot[0][1] = sin(radians(degrees));
		rot[1][1] = cos(radians(degrees));
	}
	return rot;
}
	
mat4 A2::scale(float xScale, float yScale, float zScale) {
	mat4 sc(1.0f);
	sc[0][0] = xScale;
	sc[1][1] = yScale;
	sc[2][2] = zScale;
	return sc;
}		

bool A2::clipZ(vec4 &point1, vec4 &point2) {
	return true;
/*	bool point1In;
	bool point2In;
	if ((point1[3] < near) || (point1[3] > far)) {
		point1In = false;
	}
	if ((point2[3] < near) || (point2[3] > far)) {
		point2In = false;
	}
	if (point1In && point2In) {
		return true;
	}
	else if (!point1In && !point2In) {
		return false;
	}
	else if (!point1In) {
		vec3 normal;
		vec3 P;
		if (point1[3] < near) {
			normal = vec3(0, 0, 1);
			P = vec3(0, 0, near);
		}
		else if (point1[3] > far) {
			normal = vec3(0, 0, -1);
			P = vec3(0, 0, near);
		}
		float t = (dot( (vec3(point1) - P), normal)) / (dot( (vec3(point1) - vec3(point2)), normal));
		vec3 newPoint = vec3(point1) + (t * (vec3(point2) - vec3(point1) ) );
		point1[0] = newPoint[0];
		point1[1] = newPoint[1];
		point1[2] = newPoint[2];
		return true;			
	}
	else if (!point2In) {
		vec3 normal;
		vec3 P;
		if (point2[3] < near) { 
                        normal = vec3(0, 0, 1);
                        P = vec3(0, 0, near);
                }
                else if (point2[3] > far) {
                        normal = vec3(0, 0, -1); 
                        P = vec3(0, 0, near);
                }
                float t = (dot( (vec3(point2) - P), normal)) / (dot( (vec3(point2) - vec3(point1)), normal));
                vec3 newPoint = vec3(point2) + (t * (vec3(point2) - vec3(point1) ) );
                point2[0] = newPoint[0];
                point2[1] = newPoint[1];
                point2[2] = newPoint[2];
                return true;  
	}   */
}
	
		
		

void A2::drawCube()
{
	vec4 line1a(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 line1b(-1.0f, 1.0f, 1.0f, 1.0f);

 	vec4 line2a(-1.0f, 1.0f, 1.0f, 1.0f);
	vec4 line2b(-1.0f, -1.0f, 1.0f, 1.0f);

	vec4 line3a(-1.0f, -1.0f, 1.0f, 1.0f);
	vec4 line3b(1.0f, -1.0f, 1.0f, 1.0f);

	vec4 line4a(1.0f, -1.0f, 1.0f, 1.0f);
	vec4 line4b(1.0f, 1.0f, 1.0f, 1.0f);

	vec4 line5a(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 line5b(1.0f, 1.0f, -1.0f, 1.0f);

	vec4 line6a(-1.0f, 1.0f, 1.0f, 1.0f);
        vec4 line6b(-1.0f, 1.0f, -1.0f, 1.0f);

	vec4 line7a(-1.0f, -1.0f, 1.0f, 1.0f);
        vec4 line7b(-1.0f, -1.0f, -1.0f, 1.0f);

	vec4 line8a(1.0f, -1.0f, 1.0f, 1.0f);
        vec4 line8b(1.0f, -1.0f, -1.0f, 1.0f);

	vec4 line9a(1.0f, 1.0f, -1.0f, 1.0f);
        vec4 line9b(-1.0f, 1.0f, -1.0f, 1.0f);

	vec4 line10a(-1.0f, 1.0f, -1.0f, 1.0f);
        vec4 line10b(-1.0f, -1.0f, -1.0f, 1.0f);

	vec4 line11a(-1.0f, -1.0f, -1.0f, 1.0f);
        vec4 line11b(1.0f, -1.0f, -1.0f, 1.0f);

	vec4 line12a(1.0f, -1.0f, -1.0f, 1.0f);
        vec4 line12b(1.0f, 1.0f, -1.0f, 1.0f);

	vec4 cubeLines[12][2];
	
	cubeLines[0][0] = line1a;
	cubeLines[0][1] = line1b;
	cubeLines[1][0] = line2a;
	cubeLines[1][1] = line2b;
	cubeLines[2][0] = line3a;
        cubeLines[2][1] = line3b;
	cubeLines[3][0] = line4a;
        cubeLines[3][1] = line4b;
	cubeLines[4][0] = line5a;
        cubeLines[4][1] = line5b;
	cubeLines[5][0] = line6a;
        cubeLines[5][1] = line6b;
	cubeLines[6][0] = line7a;
        cubeLines[6][1] = line7b;
	cubeLines[7][0] = line8a;
        cubeLines[7][1] = line8b;
	cubeLines[8][0] = line9a;
        cubeLines[8][1] = line9b;
	cubeLines[9][0] = line10a;
        cubeLines[9][1] = line10b;
	cubeLines[10][0] = line11a;
        cubeLines[10][1] = line11b;
	cubeLines[11][0] = line12a;
        cubeLines[11][1] = line12b;

	vec2 cubeLinesProj[12][2];

	bool keepLine[12];

	for(int i = 0; i < 12; i++) {
		vec4 point1 = view * worldMat * modelScale * model * cubeLines[i][0];
      		vec4 point2 = view * worldMat * modelScale * model * cubeLines[i][1];
		keepLine[i] = clipZ(point1, point2);
		point1 = proj * point1;
		point2 = proj * point2;	
                point1 = normalize(point1 * (1/point1[3]));
		point2 = normalize(point2 * (1/point2[3]));
		cubeLinesProj[i][0] = vec2(point1[0], point1[1]);
		cubeLinesProj[i][1] = vec2(point2[0], point2[1]);
	}
	
	setLineColour(vec3(0.0f, 0.0f, 0.0f));
	for (int i = 0; i < 12; i++) {
		drawLine(cubeLinesProj[i][0], cubeLinesProj[i][1]);
	}
}

void A2::drawCubeGnom() {
	vec4 lines[3][2] = {
                {vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.25f, 0.0f, 0.0f, 1.0f)},
                {vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.25f, 0.0f, 1.0f)},
                {vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.25f, 1.0f)},
        };

        vec2 linesProj[3][2];

        for (int i = 0; i < 3; i++) {
                vec4 point1 = proj * view * modelScale * model * lines[i][0];
                point1 = normalize(point1 * (1/point1[3]));
                vec4 point2 = proj * view * modelScale * model * lines[i][1];
                point2 = normalize(point2 * (1/point2[3]));
                linesProj[i][0] = vec2(point1[0], point1[1]);
                linesProj[i][1] = vec2(point2[0], point2[1]);
        }

        setLineColour(vec3(1.0f, 1.0f, 0.0f));
        drawLine(linesProj[0][0], linesProj[0][1]);
        setLineColour(vec3(1.0f, 0.0f, 1.0f));
        drawLine(linesProj[1][0], linesProj[1][1]);
        setLineColour(vec3(0.0f, 1.0f, 1.0f));
        drawLine(linesProj[2][0], linesProj[2][1]);
}

void A2::drawWorldGnom() {
	vec4 lines[3][2] = {
		{vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.5f, 0.0f, 0.0f, 1.0f)},
		{vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.5f, 0.0f, 1.0f)},		
		{vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.5f, 1.0f)},
	};
	
	vec2 linesProj[3][2];
	
	for (int i = 0; i < 3; i++) {
		vec4 point1 = proj * view * lines[i][0];
		point1 = normalize(point1 * (1/point1[3]));
		vec4 point2 = proj * view * lines[i][1];
		point2 = normalize(point2 * (1/point2[3]));
                linesProj[i][0] = vec2(point1[0], point1[1]);
                linesProj[i][1] = vec2(point2[0], point2[1]);
	}

	setLineColour(vec3(1.0f, 0.0f, 0.0f));
	drawLine(linesProj[0][0], linesProj[0][1]);
	setLineColour(vec3(0.0f, 1.0f, 0.0f));
	drawLine(linesProj[1][0], linesProj[1][1]);
	setLineColour(vec3(0.0f, 0.0f, 1.0f));  
        drawLine(linesProj[2][0], linesProj[2][1]);	
}


//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

/*	// Draw outer square:
	setLineColour(vec3(1.0f, 0.7f, 0.8f));
	drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));


	// Draw inner square:
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));*/

	drawCube();
	drawWorldGnom();
	drawCubeGnom();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "Near Plane: %.1f", near);
		ImGui::Text( "Far Plane: %.1f", far);
		ImGui::Text( "Mode: %.1d", mode);

		for (int i = 0; i < 7; i++) {
			ImGui::PushID( i );
                        if( ImGui::RadioButton( modes[i], &mode, i ) ) {
                                
                        }
                        ImGui::PopID();
                }


	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	double xDiff = xPos - oldX;
	oldX = xPos;

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (mode == 0) {
			float angle = (float)(xDiff * 5 * -1); //inverse so -1
			if (mouseLeftPressed) {
				mat4 rot = rotate('x', angle);
				view = rot * view;
			}
			if (mouseMiddlePressed) {
				mat4 rot = rotate('y', angle);
				view = rot * view;
			}
			if (mouseRightPressed) {
				mat4 rot = rotate('z', angle);
				view = rot * view;
			}
			eventHandled = true;
		}
		else if (mode == 1) {
			float amount = (float) (xDiff * -1); // inverse so -1
			if (mouseLeftPressed) {
				mat4 trans = translate(amount, 0.0f, 0.0f);
				view = trans * view;
			}
			if (mouseMiddlePressed) {
				mat4 trans = translate(0.0f, amount, 0.0f);
				view = trans * view;
			}
			if (mouseRightPressed) {
				mat4 trans = translate(0.0f, 0.0f, amount);
				view = trans * view;
			}
			eventHandled = true;
		}
		else if (mode == 2) {
			float angle = (float)(xDiff * 5);
			float amount = (float) (xDiff);
			if (mouseLeftPressed) {
				fovDegrees = fovDegrees + angle;
				if (fovDegrees > 160) {
					fovDegrees = 160;
				}
				else if (fovDegrees < 5) {
					fovDegrees = 5;
				}
				createProj(fovDegrees, near, far, aspect);
			}
			if (mouseMiddlePressed) {
				near = near + amount;
				createProj(fovDegrees, near, far, aspect);
			}
			if (mouseRightPressed) {
				far = far + amount;
				createProj(fovDegrees, near, far, aspect);
			}
			eventHandled = true;
		}
		else if (mode == 3) {
			float angle = (float)(xDiff * 5);
                        if (mouseLeftPressed) {
                                mat4 rot = rotate('x', angle);
                                model = rot * model;
                        }
                        if (mouseMiddlePressed) {
                                mat4 rot = rotate('y', angle);
                                model = rot * model;
                        }
                        if (mouseRightPressed) {
                                mat4 rot = rotate('z', angle);
                                model = rot * model;
                        }
                        eventHandled = true;
		}
		else if (mode == 4) {
			float amount = (float) (xDiff);
                        if (mouseLeftPressed) {
                                mat4 trans = translate(amount, 0.0f, 0.0f);
                                model = trans * model;
                        }
                        if (mouseMiddlePressed) {
                                mat4 trans = translate(0.0f, amount, 0.0f);
                                model = trans * model;
                        }
                        if (mouseRightPressed) {
                                mat4 trans = translate(0.0f, 0.0f, amount);
                                model = trans * model;
                        }
                        eventHandled = true;
                }
		else if (mode == 5) {
			float amount = ((float)xDiff)/8.0f;
			if (mouseLeftPressed) {
				mat4 sc = scale(amount, 1.0f, 1.0f);
				modelScale = sc * modelScale;
			}
			if (mouseMiddlePressed) {
				mat4 sc = scale(1.0f, amount, 1.0f);
				modelScale = sc * modelScale;
			}
			if (mouseRightPressed) {
				mat4 sc = scale(1.0f, 1.0f, amount);
				modelScale = sc * modelScale;
			}
			eventHandled = true;
		}
		else if (mode == 6) {
			
			//TODO
		}
	}											

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (actions == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
                	mouseLeftPressed = false;
                }
                if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                	mouseRightPressed = false;
                }
                if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                	mouseMiddlePressed = false;
                }

		eventHandled = true;
	}

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if (actions == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				mouseLeftPressed = true;
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				mouseRightPressed = true;
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
				mouseMiddlePressed = true;
			}
			eventHandled = true;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);
	
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_O) {
			mode = 0;
		}
		else if (key == GLFW_KEY_N) {
			mode = 1;
		}
		else if (key == GLFW_KEY_P) {
			mode = 2;
		}
		else if (key == GLFW_KEY_R) {
			mode = 3;
		}
		else if (key == GLFW_KEY_T) {
			mode = 4;
		}
		else if (key == GLFW_KEY_S) {
			mode = 5;
		}
		else if (key == GLFW_KEY_V) {
			mode = 6;
		}
		eventHandled = true;
	}

	// Fill in with event handling code...

	return eventHandled;
}
