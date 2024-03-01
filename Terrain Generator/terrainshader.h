#pragma once
#include "framework.h"
#include "shader.h"
#include "renderstate.h"


class TerrainShader : public Shader {
	const char* vertexSource = R"(
		#version 450 core
		precision highp float;

		struct Light {
			vec3 La, Le;
			vec4 wLightPos;
		};
		
		uniform mat4  MVP, M;					// MVP, Model
		uniform Light[8] lights;				// Light sources 
		uniform int   nLights;
		uniform vec3  wEye;						// Eye position

		layout(location = 0) in vec3  vtxPos;   // pos in modeling space
		layout(location = 1) in vec2  vtxUV;

		out vec3 wView;							// view in world space
		out vec3 wLight[8];						// light dir in world space
		out vec2 texcoord;
		
		void main() {
			vec3 vertexPos = vtxPos;
			gl_Position = vec4(vertexPos, 1) * MVP; // to NDC
			vec4 wPos = vec4(vertexPos, 1) * M;
			for(int i = 0; i < nLights; i++) {
				wLight[i] = lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w;
			}
		    wView  = wEye * wPos.w - wPos.xyz;
		    texcoord = vtxUV;
		}
	)";

	const char* fragmentSource = R"(
	#version 450 core
	precision highp float;

	struct Light {
		vec3 La, Le;
		vec4 wLightPos;
	};

	struct Material {
		vec3 kd, ks, ka;
		float shininess;
	};

	uniform Material material;
	uniform Light[8] lights;
	uniform int   nLights;

	in  vec3 wView;						// interpolated world sp view
	in  vec3 wLight[8];					// interpolated world sp illum dir
	in  vec2 texcoord;				
	
	out vec4 fragmentColor;				// output goes to frame buffer

	vec3 texColor = vec3(0.0, 0.5, 1.0);
	
	void main() {
		vec3 xTangent = dFdx(wView);
		vec3 yTangent = dFdy(wView);
		vec3 N = normalize(cross(xTangent, yTangent));
		vec3 V = normalize(wView); 
		
		vec3 ka = material.ka * texColor;
		vec3 kd = material.kd * texColor;
		vec3 ks = material.ks;

		vec3 radiance = vec3(0, 0, 0);
		for(int i = 0; i < nLights; i++) {
			vec3 L = normalize(wLight[i]);
			vec3 H = normalize(L + V);
			float cost = max(dot(N,L), 0);
			float cosd = max(dot(N,H), 0);
			radiance += ka * lights[i].La + (kd * cost + ks * lights[i].Le * pow(cosd, material.shininess)) * lights[i].Le;
		}
		
		fragmentColor = vec4(radiance, 1.0);
	}
)";

public:
	TerrainShader() {
		create(vertexSource, fragmentSource, "fragmentColor");
	}

	void Bind(RenderState state) {
		Use();

		setUniform(state.MVP, "MVP");
		setUniform(state.M, "M");
		setUniform(state.wEye, "wEye");
		setUniformMaterial(*state.material, "material");
		setUniform((int)state.lights.size(), "nLights");
		for (unsigned int i = 0; i < state.lights.size(); i++) {
			setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
		}
	}
};
