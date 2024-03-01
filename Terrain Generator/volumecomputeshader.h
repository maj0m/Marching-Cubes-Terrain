#pragma once
#include "framework.h"
#include "computeshader.h"

int dim = 32;

class VolumeComputeShader : ComputeShader {
    const char* computeShaderSource = R"(
		#version 450 core

        layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
        layout(rgba32f, binding = 0) uniform image2D volume;

        void main() {
            ivec2 dimensions = imageSize(heightMap);
            ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

            imageStore(volume, pixelCoords, vec4(finalHeight));

        }
		)";

public:
    VolumeComputeShader() { create(computeShaderSource); }

    void Bind() {
        glUseProgram(getId());	// make this program run

        glDispatchCompute(ceil(dim / 8), ceil(dim / 4), 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
};