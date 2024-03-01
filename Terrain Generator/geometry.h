#pragma once
#include "framework.h"
#include "lut.h"
#include "FastNoiseLite.h"


class Geometry {
protected:
	unsigned int vao, vbo;
	unsigned int totalVertices;
    FastNoiseLite noise;
    FastNoiseLite caveNoise;

	struct VertexData {
		vec3 pos;
	};

    struct Triangle {
        vec3 p[3];
    };

    typedef struct GridCell {
        vec3 p[8];
        double val[8];
    };

    float boundary = 50;
    float isolevel = 0.4;     // Threshold for the isosurface
    float stepSize = 0.5;   // Resolution

public:
	Geometry() {
        noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
        noise.SetFrequency(0.05);

        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFractalOctaves(5);
        noise.SetFractalLacunarity(2.0);
        noise.SetFractalGain(0.5);

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		create();
	}

    virtual ~Geometry() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    // Scalar field
    float f(float x, float y, float z) {
        
        float density = (noise.GetNoise(x, y, z) + 1) / 2;

        return density;
    }

    int Polygonise(GridCell grid, double isolevel, Triangle* triangles)
    {
        int i, nTriangle;
        int cubeindex;
        vec3 vertlist[12];

        cubeindex = 0;
        if (grid.val[0] < isolevel) cubeindex |= 1;
        if (grid.val[1] < isolevel) cubeindex |= 2;
        if (grid.val[2] < isolevel) cubeindex |= 4;
        if (grid.val[3] < isolevel) cubeindex |= 8;
        if (grid.val[4] < isolevel) cubeindex |= 16;
        if (grid.val[5] < isolevel) cubeindex |= 32;
        if (grid.val[6] < isolevel) cubeindex |= 64;
        if (grid.val[7] < isolevel) cubeindex |= 128;

        // Cube is entirely in/out of the surface
        if (edgeTable[cubeindex] == 0)
            return(0);

        // Find the vertices where the surface intersects the cube
        if (edgeTable[cubeindex] & 1)       vertlist[0] = lerp(isolevel, grid.p[0], grid.p[1], grid.val[0], grid.val[1]);
        if (edgeTable[cubeindex] & 2)       vertlist[1] = lerp(isolevel, grid.p[1], grid.p[2], grid.val[1], grid.val[2]);
        if (edgeTable[cubeindex] & 4)       vertlist[2] = lerp(isolevel, grid.p[2], grid.p[3], grid.val[2], grid.val[3]);
        if (edgeTable[cubeindex] & 8)       vertlist[3] = lerp(isolevel, grid.p[3], grid.p[0], grid.val[3], grid.val[0]);
        if (edgeTable[cubeindex] & 16)      vertlist[4] = lerp(isolevel, grid.p[4], grid.p[5], grid.val[4], grid.val[5]);
        if (edgeTable[cubeindex] & 32)      vertlist[5] = lerp(isolevel, grid.p[5], grid.p[6], grid.val[5], grid.val[6]);
        if (edgeTable[cubeindex] & 64)      vertlist[6] = lerp(isolevel, grid.p[6], grid.p[7], grid.val[6], grid.val[7]);
        if (edgeTable[cubeindex] & 128)     vertlist[7] = lerp(isolevel, grid.p[7], grid.p[4], grid.val[7], grid.val[4]);
        if (edgeTable[cubeindex] & 256)     vertlist[8] = lerp(isolevel, grid.p[0], grid.p[4], grid.val[0], grid.val[4]);
        if (edgeTable[cubeindex] & 512)     vertlist[9] = lerp(isolevel, grid.p[1], grid.p[5], grid.val[1], grid.val[5]);
        if (edgeTable[cubeindex] & 1024)    vertlist[10] = lerp(isolevel, grid.p[2], grid.p[6], grid.val[2], grid.val[6]);
        if (edgeTable[cubeindex] & 2048)    vertlist[11] = lerp(isolevel, grid.p[3], grid.p[7], grid.val[3], grid.val[7]);

        // Create the triangle
        nTriangle = 0;
        for (i = 0; triTable[cubeindex][i] != -1; i += 3) {
            triangles[nTriangle].p[0] = vertlist[triTable[cubeindex][i]];
            triangles[nTriangle].p[1] = vertlist[triTable[cubeindex][i + 1]];
            triangles[nTriangle].p[2] = vertlist[triTable[cubeindex][i + 2]];
            nTriangle++;
        }

        return(nTriangle);
    }


    vec3 lerp(float isolevel, vec3 p1, vec3 p2, float valp1, float valp2) {
        if (abs(valp1 - valp2) < 0.00001)       return p1;
        if (abs(isolevel - valp1) < 0.00001)    return p1;
        if (abs(isolevel - valp2) < 0.00001)    return p2;

        float mu = (isolevel - valp1) / (valp2 - valp1);
        return { p1.x + mu * (p2.x - p1.x), p1.y + mu * (p2.y - p1.y), p1.z + mu * (p2.z - p1.z) };
    }

    void create() {
        std::vector<VertexData> vtxData;

        totalVertices = 0;

        for (float x = 0; x < boundary; x += stepSize) {
            for (float y = 0; y < boundary; y += stepSize) {
                for (float z = 0; z < boundary; z += stepSize) {
                    GridCell grid;
                    Triangle triangles[5];

                    // Define grid points and their scalar values
                    grid.p[0] = vec3(x,             y,              z);
                    grid.p[1] = vec3(x + stepSize,  y,              z);
                    grid.p[2] = vec3(x + stepSize,  y,              z + stepSize);
                    grid.p[3] = vec3(x,             y,              z + stepSize);
                    grid.p[4] = vec3(x,             y + stepSize,   z);
                    grid.p[5] = vec3(x + stepSize,  y + stepSize,   z);
                    grid.p[6] = vec3(x + stepSize,  y + stepSize,   z + stepSize);
                    grid.p[7] = vec3(x,             y + stepSize,   z + stepSize);

                    for (int i = 0; i < 8; i++) {
                        grid.val[i] = f(grid.p[i].x, grid.p[i].y, grid.p[i].z);
                    }

                    // Polygonise the grid
                    int numTriangles = Polygonise(grid, isolevel, triangles);

                    // Store the vertices in the vertex buffer
                    for (int tri = 0; tri < numTriangles; tri++) {
                        for (int vert = 0; vert < 3; vert++) { // Each triangle has 3 vertices
                            VertexData v;
                            v.pos = triangles[tri].p[vert];
                            vtxData.push_back(v);
                            totalVertices++;
                        }
                    }
                }
            }
        }

        // Upload vertex data to the GPU
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vtxData.size() * sizeof(VertexData), vtxData.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); // Attribute Array 0 = POSITION
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, pos));
    }


	void Draw() {
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, totalVertices);
	}
};
