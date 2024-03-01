#pragma once
#include "framework.h"
#include "camera.h"
#include "material.h"
#include "renderstate.h"
#include "light.h"
#include "material.h"
#include "object.h"
#include "geometry.h"
#include "terrainshader.h"
#include <iostream>

const int gui_width = 300;
const int gui_height = 100;
const int tesselation = 256;
const int scale = 100;

int fps;
int frameCount = 0;
float previousTime = glfwGetTime();



class Scene {
	Camera camera;
	RenderState state;
	std::vector<Object*> objects;
	std::vector<Light> lights;

	void getFPS(int& fps) {
		float currentTime = glfwGetTime();
		frameCount++;
		if (currentTime - previousTime >= 1.0) {
			fps = frameCount;
			frameCount = 0;
			previousTime = currentTime;
		}
	}

	void updateState(RenderState& state) {
		state.lights = lights;
		state.time = getTime();
		state.wEye = camera.getEyePos();
		state.M = mat4(	1, 0, 0, 0,
						0, 1, 0, 0,
						0, 0, 1, 0,
						0, 0, 0, 1);
		state.V = camera.V();
		state.P = camera.P();
	}

public:
	void Render() {
		glViewport(0, 0, windowWidth, windowHeight);
		updateState(state);
		for (Object* obj : objects) { obj->Draw(state); }
		drawGUI(windowWidth - gui_width, 0, gui_width, gui_height);
	}

	void Build() {
		//Camera
		camera.setEyePos(vec3(0, 1, 0));
		camera.setEyeDir(vec3(-1, -0.5, 0));
		

		// Shaders
		Shader* terrainShader		= new TerrainShader();

		// Materials
		Material* terrainMaterial		= new Material(vec3(0.5f, 0.5f, 0.5f), vec3(0.4f, 0.4f, 0.4f), vec3(0.4f, 0.4f, 0.4f), 1.0f);

		// Geometries
		Geometry* terrainGeometry	= new Geometry();

		// Objects

		Object* terrainObject = new Object(terrainShader, terrainMaterial, terrainGeometry);
		terrainObject->pos = vec3(0, 0, 0);
		objects.push_back(terrainObject);

		// Lights
		lights.resize(1);
		lights[0].wLightPos = vec4(0, 50, 0, 1);
		lights[0].Le = vec3(1.0, 1.0, 1.0);
		lights[0].La = vec3(0.5, 0.5, 0.5);
	}



	void drawGUI(int x, int y, int w, int h) {
		ImGui::SetNextWindowPos(ImVec2(x, y));
		ImGui::SetNextWindowSize(ImVec2(w, h));
		ImGui::Begin("Settings");

		// Display FPS
		getFPS(fps);
		ImGui::Text("FPS: %d", fps);

		ImGui::End();
	}

	void moveCamera(int key) {
		camera.move(key);
	}

	void rotateCamera(float x, float y) {
		camera.rotate(x, y);
	}

	void setCameraFirstMouse() {
		camera.setFirstMouse();
	}
};
