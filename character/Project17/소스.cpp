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
#include "aabb.h"
#include "file_open.h"
#include "map_tile.h"

#define green_color glm::vec3(0.0f, 1.0f, 0.0f)
#define blue_color glm::vec3(0.0f, 0.0f, 1.0f)
#define sky_color glm::vec3(0.0f, 0.5f, 1.0f)
#define purple_color glm::vec3(1.0f, 0.0f, 1.0f)
#define brown_color glm::vec3(0.5f, 0.3f, 0.0f)
#define gray_color glm::vec3(0.5f, 0.5f, 0.5f)
#define white_color glm::vec3(1.0f, 1.0f, 1.0f)
#define red_color glm::vec3(1.0f, 0.0f, 0.0f)
#define yellow_color glm::vec3(1.0f, 1.0f, 0.0f)


using namespace std;

float a = 0.1; // 중력가속도
float v = 0.0;
bool w = false;
bool s = false;
bool turnL = false;
bool turnR = false;
float jumpSpeed = 0.0;

float camera_x;
float camera_y;
float camera_z;
float camera_angle;

//변수
float windowWidth = 800;
float windowHeight = 600;
const float defaultSize = 0.05;
static random_device random;
static mt19937 gen(random());
static uniform_real_distribution<> distribution(-1.0, 1.0);
static uniform_int_distribution<> distribution_diag(1, 6);
static uniform_real_distribution<> distribution_size(0.05, 0.1);

// 복귀용
float save_x;
float save_y{ 0.5 };
float save_z;


int life{ 3 };//라이프


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
	for (int i = 0; i < 8; i++) color[i] = glm::vec3((float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255, (float)(rand() % 256 + 1) / 255);
	return color;
}
COLOR backgroundColor{ 0.0f, 0.0f, 0.0f, 0.0f };

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
void KeyboardSpecialUp(int key, int x, int y);
void TimerFunction(int value);
GLvoid timerMap(int value);
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
	glm::vec3 initialCenter = glm::vec3(0.0f, 0.0f, 0.0f);		// 초기 위치(중심)
	vector<glm::vec3> position;										// 정점 좌표 - 정점의 개수만큼 넣어놨음
	vector<glm::vec3> currentPosition;								// aabb박스 계산용 변환 누적 위치(정점 좌표)
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
		currentPosition.resize(vertices);
		color.resize(vertices);
		if (vertices == 4) {
			index.resize(6);
			index = vector<int>{ 0, 1, 3, 1, 2, 3 };
		}
	}
	// 입체 도형 생성자
	diagram(int vertexCount, bool polyhedron) : vertices(vertexCount), polyhedron(polyhedron) {
		position.resize(vertices);
		currentPosition.resize(vertices);
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
	dst.initialCenter = glm::vec3(center);
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

	for (int i = 0; i < dst.currentPosition.size(); i++) {
		dst.position[i] = glm::vec3(dst.position[i]);
	}

	InitBufferRectangle(dst.VAO, dst.position.data(), dst.position.size(),
		dst.color.data(), dst.color.size(),
		dst.index.data(), dst.index.size());

	return dst;
}

// 이동
inline void move(diagram& dia, glm::vec3 delta) {
	dia.TSR = glm::translate(glm::mat4(1.0f), delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
}
// 자전
inline void rotateByCenter(diagram& dia, glm::vec3 axis, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -dia.center) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::normalize(axis)) * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
}
// 공전
inline void moveAndRotate(diagram& dia, glm::vec3 axis, glm::vec3 delta, const float& degree) {
	dia.TSR = glm::translate(glm::mat4(1.0f), -(dia.center + delta)) * dia.TSR;
	dia.TSR = glm::rotate(glm::mat4(1.0f), glm::radians(degree), glm::vec3(axis)) * dia.TSR;
	dia.TSR = glm::translate(glm::mat4(1.0f), dia.center + delta) * dia.TSR;
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
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
	dia.center = glm::vec3(dia.TSR * glm::vec4(dia.initialCenter, 1.0f));
	for (int i = 0; i < dia.position.size(); i++) dia.currentPosition[i] = glm::vec3(dia.TSR * glm::vec4(dia.position[i], 1.0f));
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
	//cout << target.center.x << ", " << target.center.y << ", " << target.center.z << endl;
	camera[0] = glm::vec3(cameraDirection.x, cameraDirection.y + 0.05, cameraDirection.z);
	camera[1] = target.center;
	view = glm::lookAt(camera[0], camera[1], camera[2]);
}

// 캐릭터의 aabb박스를 계산해 반환
aabb make_aabb_charactor(const vector<glm::vec3>& vertices) {
	aabb box;
	float min_x = std::numeric_limits<float>::max();
	float max_x = std::numeric_limits<float>::min();
	float min_y = std::numeric_limits<float>::max();
	float max_y = std::numeric_limits<float>::min();
	float min_z = std::numeric_limits<float>::max();
	float max_z = std::numeric_limits<float>::min();

	min_x = vertices[0].x;
	max_x = vertices[0].x;
	min_y = vertices[0].y;
	max_y = vertices[0].y;
	min_z = vertices[0].z;
	max_z = vertices[0].z;

	for (const auto& v : vertices) {
		min_x = std::min(min_x, v.x);
		max_x = std::max(max_x, v.x);
		min_y = std::min(min_y, v.y);
		max_y = std::max(max_y, v.y);
		min_z = std::min(min_z, v.z);
		max_z = std::max(max_z, v.z);
	}

	aabb temp = {
		min_x,
		max_x,

		min_y = vertices.begin()->y - 0.3, // 다리가 회전하면서 y값이 변함 -> 충돌 이상, y값 고정
		max_y,

		min_z,
		max_z,
	};

	return temp;
}
bool aabb_collision(const aabb& a, const aabb& b) {
	if (a.max_x >= b.min_x && a.min_x <= b.max_x &&
		a.max_y >= b.min_y && a.min_y <= b.max_y &&
		a.max_z >= b.min_z && a.min_z <= b.max_z) return true;
	return false;
}

GLint width, height;
GLuint shaderProgramID;
GLuint vertexShader;
GLuint fragmentShader;
vector <diagram> character;
diagram boxForCollision(8);
vector<glm::vec3> verticesForAABB(0);
vector <diagram> axes;
glm::vec3 camera[3];
aabb aabbCharacter;
glm::vec3 headDirection;
std::string mapType;

MapTile map1[] = {
	//stage 0
	MapTile(0.0f, 0.25f, -22.5f, "cube1.obj", "goal", blue_color),//골
	MapTile(0.0f, 0.2f, -18.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(0.2f, 0.2f, -18.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(0.4f, 0.2f, -18.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(0.6f, 0.2f, -18.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(0.8f, 0.2f, -18.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(-0.2f, 0.2f, -16.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(-0.4f, 0.2f, -16.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(-0.6f, 0.2f, -16.6f, "niddle.obj", "niddle", red_color),//가시
	MapTile(-0.8f, 0.2f, -16.6f, "niddle.obj", "niddle", red_color),//가시	
	MapTile(0.0f, 0.0f, 0.0f, "platform.obj", "floor", green_color),//바닥
	MapTile(0.0f, 0.0f, -4.0f, "platform.obj", "platform_x", red_color),//x축으로 움직이는 발판
	MapTile(0.0f, 0.0f, -8.0f, "platform.obj", "platform_y", yellow_color),//y축으로 움직이는 발판
	MapTile(0.0f, 0.0f, -12.0f, "platform.obj", "platform_z", brown_color),//z축으로 움직이는 발판
	MapTile(0.0f, 0.0f, -18.0f, "platform.obj", "floor", green_color),//바닥
	MapTile(0.0f, 0.0f, -22.0f, "platform.obj", "floor", green_color),//바닥

	//stage1

	//stage2

};

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
	glutSpecialUpFunc(KeyboardSpecialUp);
	glutMouseFunc(mouse);
	glutTimerFunc(100, TimerFunction, 0);
	glutTimerFunc(100, timerMap, 0);

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

	for (int i = 0; i < character.size(); i++) {
		move(character[i], glm::vec3(0.0, 2.0, 0.0));
	}

	boxForCollision = returnHexahedron(glm::vec3(0.0, -0.1, 0.0), 0.4, 0.7, 0.1, returnColorRand8());

	delete(part);

	camera[0] = glm::vec3(character[1].center.x, character[1].center.y + 0.2, character[1].center.z + 0.3);
	camera[1] = character[1].center;
	camera[2] = glm::vec3(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera[0], camera[1], camera[2]);

	for (MapTile& map : map1) map.gen_buffer();

	//for (auto& map : map1) {
	//	cout << "aabb: " << map.get_aabb() <<endl;
	//}
	
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw(axes);
	setCameraToHead(view, camera, character[1]);

	/*glm::vec3 camera_pos(0.0f, 0.0f, 0.0f);
	glm::vec3 camera_target(0.0, 0.0, -1.0);
	glm::vec3 camera_up(0.0f, 1.0f, 0.0f);
	view = glm::lookAt(camera_pos, camera_target, camera_up);

	view = glm::translate(view, glm::vec3(camera_x, camera_y, camera_z - 25.0f));
	view = glm::rotate(view, glm::radians(camera_angle), glm::vec3(0.0f, 1.0f, 0.0f));*/
	unsigned int viewLocation = glGetUniformLocation(shaderProgramID, "viewTransform");
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -2.0));
	unsigned int projectionLocation = glGetUniformLocation(shaderProgramID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	//캐릭터
	draw(character);
	//aabb박스 그리기, 디버그용
	drawWireframe(boxForCollision);

	GLuint trans_mat = glGetUniformLocation(shaderProgramID, "modelTransform");
	GLuint color = glGetUniformLocation(shaderProgramID, "vColor");

	
	for (MapTile w : map1) {
		glBindVertexArray(w.VAO);
		w.update_position();
		glUniform3fv(color, 1, glm::value_ptr(w.color));
		glUniformMatrix4fv(trans_mat, 1, GL_FALSE, glm::value_ptr(w.trans));
		glDrawArrays(GL_TRIANGLES, 0, w.model.vertices.size());
	}

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
	case 's':
		s = true;
		break;
	case ' ':  //점프
		jumpSpeed = 0.5;
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
	case 's':
		s = false;
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
		turnL = true;

		break;
	case GLUT_KEY_RIGHT:
		turnR = true;
		break;
	}
	glutPostRedisplay();
}

void KeyboardSpecialUp(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	case GLUT_KEY_LEFT:
		turnL = false;
		break;
	case GLUT_KEY_RIGHT:
		turnR = false;
		break;
	}
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	mgl = transformMouseToGL(x, y);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && state) {
		glutMotionFunc(motion);
	}
	glutPostRedisplay();
}

void motion(int x, int y) {
	mgl = transformMouseToGL(x, y);


	glutPostRedisplay();
}

void TimerFunction(int value) {
	headDirection = -getHeadDirection(character[1]);
	bool collision = false;
	// 캐릭터의 64개 정점을 모두 모아서 aabb박스를 계산
	
	//if (w == true || s == true || turnL == true || turnR == true) {
	verticesForAABB.clear();
	for (const auto& d : character) {
			verticesForAABB.insert(verticesForAABB.end(), d.currentPosition.begin(), d.currentPosition.end());
		}
	aabbCharacter = make_aabb_charactor(verticesForAABB);

	// aabb박스 그리기용 정육면체
	boxForCollision = returnHexahedron(glm::vec3(aabbCharacter.min_x + (aabbCharacter.max_x - aabbCharacter.min_x) / 2.0,
		aabbCharacter.min_y + (aabbCharacter.max_y - aabbCharacter.min_y) / 2.0,
		aabbCharacter.min_z + (aabbCharacter.max_z - aabbCharacter.min_z) / 2.0),
		aabbCharacter.max_x - aabbCharacter.min_x,
		aabbCharacter.max_y - aabbCharacter.min_y,
		aabbCharacter.max_z - aabbCharacter.min_z,
		returnColorRand8());

	// 중력
	move(boxForCollision, glm::vec3(0.0, -a, 0.0));
	
	aabbCharacter = make_aabb_charactor(vector<glm::vec3>(boxForCollision.currentPosition.data(),
										boxForCollision.currentPosition.data() + boxForCollision.currentPosition.size()));
	// make_aabb_character을 두 번 호출해서 y값 보정이 두 번 적용되는거 수정
	aabbCharacter.min_y += 0.3;
	
	for (auto& m : map1) {
			if (aabb_collision(aabbCharacter, m.get_aabb())) {
				collision = true;
				mapType = m.type;
				break;
			}
			else mapType = "\0";
		}
	//cout << "collision: " << collision << endl;
	if (!collision && jumpSpeed <= 0) {
			for (auto& d : character) {
				move(d, glm::vec3(0.0, -a, 0.0));
			}
		}
	else move(boxForCollision, glm::vec3(0.0, a, 0.0));
	// 중력
	if (w == true) {
		move(boxForCollision, headDirection * 0.02f);

		aabbCharacter = make_aabb_charactor(vector<glm::vec3>(boxForCollision.currentPosition.data(), 
			boxForCollision.currentPosition.data() + boxForCollision.currentPosition.size()));
		aabbCharacter.min_y += 0.3;
		if (!aabb_collision(aabbCharacter, map1->get_aabb())) {

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
		else move(boxForCollision, -headDirection * 0.02f);
	}
	else if (s == true) {
		move(boxForCollision, -headDirection * 0.02f);
		aabbCharacter = make_aabb_charactor(vector<glm::vec3>(boxForCollision.currentPosition.data(),
			boxForCollision.currentPosition.data() + boxForCollision.currentPosition.size()));
		aabbCharacter.min_y += 0.3;
		if (!aabb_collision(aabbCharacter, map1->get_aabb())) {

			for (auto& d : character) {
				move(d, -headDirection * 0.02f);
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
		else move(boxForCollision, headDirection * 0.02f);
	}
	
	if (turnL == true) {
		rotateByCenter(boxForCollision, glm::vec3(0.0, 1.0, 0.0), 3.0);
		aabbCharacter = make_aabb_charactor(vector<glm::vec3>(boxForCollision.currentPosition.data(),
			boxForCollision.currentPosition.data() + boxForCollision.currentPosition.size()));
		aabbCharacter.min_y += 0.3;
		if (!aabb_collision(aabbCharacter, map1->get_aabb())) {
			/*glutTimerFunc(10, TimerFunction, 0);
			return;*/
			rotateByCenter(character[1], glm::vec3(0.0, 1.0, 0.0), 3.0);
			rotateByCenter(character[0], glm::vec3(0.0, 1.0, 0.0), 3.0);
			moveAndRotate(character[2], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[2].center, 3.0);
			moveAndRotate(character[3], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[3].center, 3.0);
			moveAndRotate(character[4], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[4].center, 3.0);
			moveAndRotate(character[5], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[5].center, 3.0);
			moveAndRotate(character[6], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[6].center, 3.0);
			moveAndRotate(character[7], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[7].center, 3.0);
		}
		else rotateByCenter(boxForCollision, glm::vec3(0.0, 1.0, 0.0), -3.0);
	}
	if (turnR == true) {
		rotateByCenter(boxForCollision, glm::vec3(0.0, 1.0, 0.0), -3.0);
		aabbCharacter = make_aabb_charactor(vector<glm::vec3>(boxForCollision.currentPosition.data(),
			boxForCollision.currentPosition.data() + boxForCollision.currentPosition.size()));
		aabbCharacter.min_y += 0.3;
		if (!aabb_collision(aabbCharacter, map1->get_aabb())) {
			/*glutTimerFunc(10, TimerFunction, 0);
			return;*/

			rotateByCenter(character[1], glm::vec3(0.0, 1.0, 0.0), -3.0);
			rotateByCenter(character[0], glm::vec3(0.0, 1.0, 0.0), -3.0);
			moveAndRotate(character[2], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[2].center, -3.0);
			moveAndRotate(character[3], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[3].center, -3.0);
			moveAndRotate(character[4], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[4].center, -3.0);
			moveAndRotate(character[5], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[5].center, -3.0);
			moveAndRotate(character[6], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[6].center, -3.0);
			moveAndRotate(character[7], glm::vec3(0.0, 1.0, 0.0), character[0].center - character[7].center, -3.0);
		}
		else rotateByCenter(boxForCollision, glm::vec3(0.0, 1.0, 0.0), 3.0);
	}

	// 점프
	if (jumpSpeed > 0) {
		for (auto& d : character) {
			move(d, glm::vec3(0.0, jumpSpeed / 10.0, 0.0));
		}
	}
	// 중력 적용
	jumpSpeed -= a / 10.0;

	cout << "mapType: " << mapType << endl;
	if (mapType == "niddle") {
		for (auto& d : character) {
			d.TSR = glm::mat4(1.0f);
			move(d, glm::vec3(save_x, save_y + 0.5, save_z));
		}
		//if (not niddle_hit)
			--life;
	}
	else if (mapType == "goal") {
		
		save_x = map1[0].init_x;
		save_y = map1[0].init_y + 0.5;
		save_z = map1[0].init_z;
		std::cout << save_x << ", " << save_y << ", " << save_z << std::endl;
	}
	if(life<=0)
		glutLeaveMainLoop();
		
	glutTimerFunc(15, TimerFunction, 0);
	glutPostRedisplay();
}

GLvoid timerMap(int value) {

	for (MapTile& map : map1) {
		if (map.type == "platform_x") {
			map.move_x();

		}
		else if (map.type == "platform_y") {
			map.move_y();
			if (mapType == "platform_y") {
				for (auto& d : character) move(d, glm::vec3(0.0, map.dy, 0.0));
			}

		}
		else if (map.type == "platform_z") {
			map.move_z();

		}

	}

	glutPostRedisplay();
	glutTimerFunc(10, timerMap, 0);
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
	strcpy_s(source, contents.size() + 1, contents.c_str());
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
	//cout << "EBO 초기화" << endl;
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