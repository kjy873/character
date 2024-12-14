#pragma once

#include"aabb.h";
#include<string>
#include "read_obj.h"


struct MapTile {
	//-------------------------------------------------
	// 멤버 변수
	//-------------------------------------------------
	float x;
	float y;
	float z;

	float init_x;
	float init_y;
	float init_z;

	float max_dx{ 2.0f };//최대 이동 범위(움직이는 발판일 시)
	float max_dy{ 2.0f };
	float max_dz{ 2.0f };

	float speed{ 0.005f }; //발판 이동 속도


	float dx; // 실제 변화량
	float dy;
	float dz;

	Model model;

	aabb box;

	GLuint VAO, VBO, NBO;

	std::string type; //floor, platform, niddle, coin, niddle과 충돌시 구 하나 감소, coin과 접촉 시엔 스코어 증가

	glm::mat4 trans;

	glm::vec3 color; // 색상, 텍스처 등은 이후 gl수업 진도에 따라 로직 추가하겠습니다.




	//-------------------------------------------------
	// 멤버 함수
	//-------------------------------------------------

	MapTile() = default;
	MapTile(float, float, float, const char*, const char*, glm::vec3); // x,y,z좌표, 모델, 해당 물체의 타입, 색

	aabb get_aabb(); //aabb박스 얻어오기

	void update_position(); // 위치 변환을 위한 업데이트, 프레임마다 호출?
	void gen_buffer();
	void move_x();
	void move_y();
	void move_z();
	void handle_collision(); // 인자로 플레이어 클래스 받아오기 충돌시 이 함수 호출
	void print() const;
};


