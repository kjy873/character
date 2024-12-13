#define _CRT_SECURE_NO_WARININGS
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <random>
#include <vector>
#include <algorithm>
#include <sstream>
#include <string.h>
#include <string>

using namespace std;

bool w = false;
bool s = false;

//변수
float windowWidth = 800;
float windowHeight = 600;
const float defaultSize = 0.05;
static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(-1.0, 1.0);
static uniform_int_distribution<> distribution_diag(1, 6);
static uniform_real_distribution<> distribution_size(0.05, 0.1);

struct COLOR {
	GLclampf R = 1.0f;
	GLclampf G = 1.0f;
	GLclampf B = 1.0f;
	GLclampf A = 0.0f;
};
GLvoid setColor(COLOR& c, GLclampf r, GLclampf g, GLclampf b, GLclampf a) {
	c.R = r;
	c.G = g;
	c.B = b;
	c.A = a;
}
GLvoid setColorRand(COLOR& c) {
	c.R = (float)(rand() % 256 + 1) / 255;
	c.G = (float)(rand() % 256 + 1) / 255;
	c.B = (float)(rand() % 256 + 1) / 255;
	c.A = 1.0f;

}
glm::vec3* returnColorRand2() {
	glm::vec3 color[2];
	for (int i = 0; i < 2; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
glm::vec3* returnColorRand8() {
	glm::vec3 color[8];
	for(int i = 0;i <8; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
COLOR backgroundColor;

struct mouseLocationGL {
	float x;
	float y;
};
mouseLocationGL transformMouseToGL(int x, int y) {
	mouseLocationGL m;
	m.x = (2.0f * x) / windowWidth - 1.0f;
	m.y = 1.0f - (2.0f * y) / windowHeight;
	return m;
}
mouseLocationGL mgl;
mouseLocationGL preMousePosition;


void idleScene();
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid KeyboardUp(unsigned char key, int x, int y);
void KeyboardSpecial(int key, int x, int y);
void TimerFunction(int value);
void mouse(int button, int state, int x, int y);
void init();
void motion(int x, int y);
GLchar* filetobuf(const char* filepath);
void make_vertexShaders(GLuint& vertexShader, const char* vertexName);
void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName);
void make_shaderProgram(GLuint& shaderProgramID);
void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize,
	const glm::vec3* color, int colorSize);
void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize,
	const glm::vec3* color, int colorSize);
void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize);
inline GLvoid InitShader(GLuint& programID, GLuint& vertex, const char* vertexName, GLuint& fragment, const char* fragmentName);

// 도형 구조체

struct diagram {
	GLuint VAO{ NULL };												// VAO
	glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);					// 중심 좌표	
	float radius = defaultSize;										// 반지름
	glm::vec3 initialPosition = glm::vec3(0.0f, 0.0f, 0.0f);		// 초기 위치(중심)
	vector<glm::vec3> position;										// 정점 좌표 - 정점의 개수만큼 넣어놨음
	vector<glm::vec3> color;										// 정점 색상 - 정점의 개수만큼 넣어놨음
	int vertices = 0;												// 정점 개수
	bool polyhedron = false;										// 입체 도형 여부
	vector<int> index;												// EBO를 위한 인덱스
	glm::mat4 TSR = glm::mat4(1.0f);								// 변환 행렬
	float width{ 0 };
	float height{ 0 };
	float depth{ 0 };
	// width, height, depth는 각 변의 길이, 육면체의 경우 중심부터 각 면까지 길이는 /2로 계산

	//평면 도형 생성자
	diagram(int vertexCount) : vertices(vertexCount) {
		position.resize(vertices);
		color.resize(vertices);
		if (vertices == 4) {
			index.resize(6);
			index = vector<int>{ 0, 1, 3, 1, 2, 3 };
		}
	}
	// 입체 도형 생성자
	diagram(int vertexCount, bool polyhedron) : vertices(vertexCount), polyhedron(polyhedron) {
		position.resize(vertices);
		color.resize(vertices);
		setIndices();
	}

	// 입체 도형 인덱스 설정
	void setIndices() {
		if (vertices == 3) {
			// 정사면체 인덱스
			index = { 0, 1, 2 }; // 삼각형으로 구성된 면을 위한 인덱스
		}
		else if (vertices == 4) {
			index = { 0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 2, 3 };
		}
		else if (vertices == 5) {
			index = { 0, 1, 2, 0, 2, 3, 0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4 };
		}
		else if (vertices == 8) {
			// 정육면체 인덱스
			index = {
				0, 1, 2, 2, 3, 0,  // 앞면
				4, 5, 6, 6, 7, 4,  // 뒷면
				0, 1, 5, 5, 4, 0,  // 왼쪽면
				2, 3, 7, 7, 6, 2,  // 오른쪽면
				0, 3, 7, 7, 4, 0,  // 위쪽면
				1, 2, 6, 6, 5, 1   // 아래쪽면
			};
		}
		else {
			throw std::invalid_argument("Unsupported vertex count for index generation");
		}
	}
};

// 도형: 라인 설정
void setLine(diagram& dst, const glm::vec3 vertex1, const glm::vec3 vertex2, const glm::vec3* c) {
	for (int i = 0; i < 2; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	dst.position[0] = glm::vec3(vertex1);
	dst.position[1] = glm::vec3(vertex2);

	InitBufferLine(dst.VAO, dst.position.data(), dst.position.size(), dst.color.data(), dst.color.size());
}
// 도형: 육면체 만들어서 리턴, 중심 좌표, 너비(x길이), 높이(ㅛ길이), 깊이(z길이)
diagram returnHexahedron(const glm::vec3 center, const float& width, const float& height, const float& depth, const glm::vec3* c) {
	diagram dst(8, true);
	dst.center = glm::vec3(center);
	dst.width = width;
	dst.height = height;
	dst.depth = depth;
	dst.initialPosition = glm::vec3(center);
	for (int i = 0; i < 8; i++) {
		dst.color[i] = glm::vec3(c[i]);
	}
	dst.position[0] = glm::vec3(dst.center.x - width / 2.0, dst.center.y - height / 2.0, dst.center.z + depth / 2.0);
	dst.position[1] = glm::vec3(dst.center.x - width / 2.0, dst.center.y + height / 2.0, dst.center.z + depth / 2.0);
	dst.position[2] = glm::vec3(dst.center.x + width / 2.0, dst.center.y + height / 2.0, dst.center.z + depth / 2.0);
	dst.position[3] = glm::vec3(dst.center.x + width / 2.0, dst.center.y - height / 2.0, dst.center.z + depth / 2.0);
	dst.position[4] = glm::vec3(dst.center.x - width / 2.0, dst.center.y - height / 2.0, dst.center.z - depth / 2.0);
	dst.position[5] = glm::vec3(dst.center.x - width / 2.0, dst.center.y + height / 2.0, dst.center.z - depth / 2.0);
	dst.position[6] = glm::vec3(dst.center.x + width / 2.0, dst.center.y + height / 2.0, dst.center.z - depth / 2.0);
	dst.position[7] = glm::vec3(dst.center.x + width / 2.0, dst.center.y - height / 2.0, dst.center.z - depth / 2.0);

	InitBufferRectangle(dst.VAO, dst.position.data(), dst.position.size(),
		dst.color.data(), dst.color.size(),
		dst.index.data(), dst.index.size());

	return dst;
}

// 이동
inline void move(diagram& dia, glm::vec3 delta) {
	dia.TSR = glm::translate(glm::mat4(1.0f), delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialPosition, 1.0f));
}
// 자전
inline void rotateByCenter(diagram& dia, glm::vec3 axis, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -dia.center) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::normalize(axis)) * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialPosition, 1.0f));
}
// 공전
inline void moveAndRotate(diagram& dia, glm::vec3 axis, glm::vec3 delta, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -(dia.center + delta)) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(axis))  * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center + delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR *glm::vec4(dia.initialPosition, 1.0f));
}
// 카메라 공전
inline void moveAndRotateByMatrix(glm::mat4& TSR, glm::vec3 axis, glm::vec3 center, glm::vec3 moving, const float& degree) {
	TSR = glm::translate(TSR, -(center + moving)) * TSR;
	TSR = glm::rotate(TSR, glm::radians(degree), glm::vec3(axis)) * TSR;
	TSR = glm::translate(TSR, (center + moving)) * TSR;
}
inline void scaleByCenter(diagram& dia, glm::vec3 size) {
	glm::vec3 preLocation = glm::vec3(dia.center);
	move(dia, -dia.center);
	dia.TSR = glm::scale(glm::mat4(1.0f), size) * dia.TSR;
	move(dia, preLocation);
}

// head가 바라보는 방향, 법선벡터 구하기
glm::vec3 getHeadDirection(const diagram& head) {
	glm::vec3 vertex1 = glm::vec3(head.TSR * glm::vec4(head.position[4], 1.0f));
	glm::vec3 vertex2 = glm::vec3(head.TSR * glm::vec4(head.position[6], 1.0f));

	glm::vec3 vertex3 = glm::vec3(head.TSR * glm::vec4(head.position[7], 1.0f));
	glm::vec3 vertex4 = glm::vec3(head.TSR * glm::vec4(head.position[5], 1.0f));

	glm::vec3 direction = glm::normalize(glm::cross(vertex2 - vertex1, vertex4 - vertex3));
	
	return direction;
}

// head가 바라보는 반대 방향(머리 뒤)에 카메라 고정
inline void setCameraToHead(glm::mat4& view, glm::vec3 camera[3], const diagram& target) {
	glm::vec3 cameraDirection = target.center + getHeadDirection(target) / 10.0f;
	cout << target.center.x << ", " << target.center.y << ", " << target.center.z << endl;
	camera[0] = glm::vec3(cameraDirection.x, cameraDirection.y + 0.05, cameraDirection.z);
	camera[1] = target.center;
	view = glm::lookAt(camera[0], camera[1], camera[2]);
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
vector <diagram> character;
vector <diagram> axes;
glm::vec3 camera[3];

void main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Example1");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cerr << "unable to initialize GLEW" << endl;
		exit(EXIT_FAILURE);
	}
	else cout << "GLEW initialized" << endl;

	InitShader(shaderProgramID, vertexShader, "vertex.glsl", fragmentShader, "fragment.glsl");
	glUseProgram(shaderProgramID);

	init();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(KeyboardUp);
	glutSpecialFunc(KeyboardSpecial);
	glutMouseFunc(mouse);
	glutTimerFunc(100, TimerFunction, 0);
	glutMainLoop();
}

// 캐릭터 움직임 관리
static bool swingLimbs = true;
static int swingAngle = 0;
// 카메라
glm::mat4 view = glm::mat4(1.0f);



void init() {
	glClearColor(backgroundColor.R, backgroundColor.G, backgroundColor.B, backgroundColor.A);

	diagram* temp = new diagram(2);
	setLine(*temp, glm::vec3(-4.0, 0.0, 0.0), glm::vec3(4.0, 0.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, -4.0, 0.0), glm::vec3(0.0, 4.0, 0.0), returnColorRand2());
	axes.push_back(*temp);
	setLine(*temp, glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 0.0, 4.0), returnColorRand2());
	axes.push_back(*temp);
	delete(temp);

	diagram* part = new diagram(8);
	*part = returnHexahedron(glm::vec3(0.0, 0.0, 0.0), 0.2, 0.3, 0.1, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(0.0, 0.2, 0.0), 0.1, 0.1, 0.1, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(-0.15, 0.1, 0.0), 0.1, 0.1, 0.1, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(0.15, 0.1, 0.0), 0.1, 0.1, 0.1, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(-0.15, -0.05, 0.0), 0.05, 0.2, 0.05, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(0.15, -0.05, 0.0), 0.05, 0.2, 0.05, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(-0.075, -0.3, 0.0), 0.05, 0.3, 0.05, returnColorRand8());
	character.push_back(*part);
	*part = returnHexahedron(glm::vec3(0.075, -0.3, 0.0), 0.05, 0.3, 0.05, returnColorRand8());
	character.push_back(*part);

	delete(part);

	camera[0] = glm::vec3(character[1].center.x, character[1].center.y + 0.2, character[1].center.z + 0.3);
	camera[1] = character[1].center;
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera[0], camera[1], camera[2]);

	glEnable(GL_DEPTH_TEST);
}

void idleScene() {
	glutPostRedisplay();
}

inline void draw(const vector<diagram> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
		else if (d.vertices == 4) glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 5 && d.polyhedron) glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 8 && d.polyhedron) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}
inline void drawWireframe(const vector<diagram> dia) {
	for (const auto& d : dia) {
		glBindVertexArray(d.VAO);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(d.TSR));
		if (d.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
		else if (d.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
		else if (d.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 5 && d.polyhedron) glDrawElements(GL_LINE_LOOP, 18, GL_UNSIGNED_INT, 0);
		else if (d.vertices == 8 && d.polyhedron) glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_INT, 0);
	}
}
inline void draw(const diagram& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_TRIANGLES, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 5 && dia.polyhedron) glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 8 && dia.polyhedron) glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
inline void drawWireframe(const diagram& dia) {
	glBindVertexArray(dia.VAO);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgramID, "modelTransform"), 1, GL_FALSE, glm::value_ptr(dia.TSR));
	if (dia.vertices == 2) glDrawArrays(GL_LINES, 0, 2);
	else if (dia.vertices == 3) glDrawArrays(GL_LINE_LOOP, 0, 3);
	else if (dia.vertices == 4) glDrawElements(GL_LINE_LOOP, 6, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 5 && dia.polyhedron) glDrawElements(GL_LINE_LOOP, 18, GL_UNSIGNED_INT, 0);
	else if (dia.vertices == 8 && dia.polyhedron) glDrawElements(GL_LINE_LOOP, 36, GL_UNSIGNED_INT, 0);
}

GLvoid drawScene() {
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	draw(axes);
	setCameraToHead(view, camera, character[1]);
	
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0));
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
  
	draw(character);

	glutSwapBuffers();
}

GLvoid Reshape(int w, int h) {
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		w = true;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

GLvoid KeyboardUp(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		w = false;
		break;
	}
	glutPostRedisplay();
}

void KeyboardSpecial(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	case GLUT_KEY_LEFT:
		// 몸 회전
		rotateByCenter(character[1], glm::vec3(0.0, 1.0, 0.0), 3.0);
		rotateByCenter(character[0], glm::vec3(0.0, 1.0, 0.0), 3.0);
		moveAndRotate(character[2], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[2].center, 3.0);
		moveAndRotate(character[3], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[3].center, 3.0);
		moveAndRotate(character[4], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[4].center, 3.0);
		moveAndRotate(character[5], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[5].center, 3.0);
		moveAndRotate(character[6], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[6].center, 3.0);
		moveAndRotate(character[7], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[7].center, 3.0);
		break;
	case GLUT_KEY_RIGHT:
		rotateByCenter(character[1], glm::vec3(0.0, 1.0, 0.0), -3.0);
		rotateByCenter(character[0], glm::vec3(0.0, 1.0, 0.0), -3.0);
		moveAndRotate(character[2], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[2].center, -3.0);
		moveAndRotate(character[3], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[3].center, -3.0);
		moveAndRotate(character[4], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[4].center, -3.0);
		moveAndRotate(character[5], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[5].center, -3.0);
		moveAndRotate(character[6], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[6].center, -3.0);
		moveAndRotate(character[7], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[7].center, -3.0);
		break;
	}
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	mgl = transformMouseToGL(x, y);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && state) {
		
	}
	glutPostRedisplay();
}

void motion(int x, int y) {


	glutPostRedisplay();
}

void TimerFunction(int value) {
	if (w == true) {
		glm::vec3 headDirection = -getHeadDirection(character[1]);

		for (auto& d : character) {
			move(d, headDirection * 0.02f);
		}

		if (swingLimbs) {
			// 왼팔 흔들기
			rotateByCenter(character[2], glm::vec3(headDirection.z, 0.0, -headDirection.x), -3.0);
			moveAndRotate(character[4], glm::vec3(headDirection.z, 0.0, -headDirection.x), character[2].center - character[4].center, -3.0f);

			// 오른팔 흔들기
			rotateByCenter(character[3], glm::vec3(headDirection.z, 0.0, -headDirection.x), 3.0f);
			moveAndRotate(character[5], glm::vec3(headDirection.z, 0.0, -headDirection.x), character[3].center - character[5].center, 3.0f);

			// 왼다리 흔들기
			glm::vec3 legRotationAxis = glm::vec3(character[0].center.x, character[0].center.y - character[0].height / 2.0, character[0].center.z);
			moveAndRotate(character[6], glm::vec3(headDirection.z, 0.0, -headDirection.x), legRotationAxis - character[6].center, 3.0f);

			// 오른다리 흔들기
			moveAndRotate(character[7], glm::vec3(headDirection.z, 0.0, -headDirection.x), legRotationAxis - character[7].center, -3.0f);

			if (++swingAngle > 15) swingLimbs = false;
		}
		// 반대로 흔들기
		else {
			// 왼팔 흔들기
			rotateByCenter(character[2], glm::vec3(headDirection.z, 0.0, -headDirection.x), 3.0f);
			moveAndRotate(character[4], glm::vec3(headDirection.z, 0.0, -headDirection.x), character[2].center - character[4].center, 3.0f);

			// 오른팔 흔들기
			rotateByCenter(character[3], glm::vec3(headDirection.z, 0.0, -headDirection.x), -3.0f);
			moveAndRotate(character[5], glm::vec3(headDirection.z, 0.0, -headDirection.x), character[3].center - character[5].center, -3.0f);

			// 왼다리 흔들기
			glm::vec3 legRotationAxis = glm::vec3(character[0].center.x, character[0].center.y - character[0].height / 2.0, character[0].center.z);
			moveAndRotate(character[6], glm::vec3(headDirection.z, 0.0, -headDirection.x), legRotationAxis - character[6].center, -3.0f);

			// 오른다리 흔들기
			moveAndRotate(character[7], glm::vec3(headDirection.z, 0.0, -headDirection.x), legRotationAxis - character[7].center, 3.0f);

			if (--swingAngle < -15) swingLimbs = true;
		}
	}
	
	glutTimerFunc(10, TimerFunction, 0);
	glutPostRedisplay();
}




// 쉐이더 초기화
GLchar* filetobuf(const char* filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filepath << std::endl;
		return nullptr;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	std::string contents = buffer.str();
	char* source = new char[contents.size() + 1];
	strcpy_s(source, contents.size()+1, contents.c_str());
	return source;
}
void make_vertexShaders(GLuint& vertexShader, const char* vertexName) {
	GLchar* vertexSource;

	vertexSource = filetobuf(vertexName);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		cerr << "ERROR: vertex shader error\n" << errorLog << endl;
		return;
	}
}
void make_fragmentShaders(GLuint& fragmentShader, const char* fragmentName) {
	GLchar* fragmentSource;

	fragmentSource = filetobuf(fragmentName);

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "ERROR: fragment shader error\n" << errorLog << endl;
		return;
	}
}
void make_shaderProgram(GLuint& shaderProgramID) {
	shaderProgramID = glCreateProgram();

	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);

	glLinkProgram(shaderProgramID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderProgramID, 512, NULL, errorLog);
		cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
		return;
	}
}
inline GLvoid InitShader(GLuint& programID, GLuint& vertex, const char* vertexName, GLuint& fragment, const char* fragmentName) {
	make_vertexShaders(vertexShader, vertexName);
	make_fragmentShaders(fragmentShader, fragmentName);
	make_shaderProgram(shaderProgramID);
}
// 버퍼 초기화, 각 도형 만드는 함수에서 호출함
void InitBufferLine(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}
void InitBufferTriangle(GLuint& VAO, const glm::vec3* position, int positionSize, const glm::vec3* color, int colorSize) {
	cout << "버퍼 초기화" << endl;
	GLuint VBO_position, VBO_color;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);

	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);
	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);
}
void InitBufferRectangle(GLuint& VAO, const glm::vec3* position, const int positionSize,
	const glm::vec3* color, const int colorSize,
	const int* index, const int indexSize) {
	cout << "EBO 초기화" << endl;
	GLuint VBO_position, VBO_color, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO_position);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_position);
	glBufferData(GL_ARRAY_BUFFER, 3 * positionSize * sizeof(float), position, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexSize * sizeof(int), index, GL_STATIC_DRAW);

	GLint pAttribute = glGetAttribLocation(shaderProgramID, "vPos");
	if (pAttribute < 0) cout << "pAttribute < 0" << endl;
	glVertexAttribPointer(pAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(pAttribute);

	glGenBuffers(1, &VBO_color);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_color);

	glBufferData(GL_ARRAY_BUFFER, 3 * colorSize * sizeof(float), color, GL_STATIC_DRAW);
	GLint cAttribute = glGetAttribLocation(shaderProgramID, "vColor");
	if (cAttribute < 0) cout << "cAttribute < 0" << endl;
	glVertexAttribPointer(cAttribute, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(cAttribute);

}