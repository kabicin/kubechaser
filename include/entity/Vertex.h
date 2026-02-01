#ifndef VERTEX_H
#define VERTEX_H
struct PCVertex
{
	PCVertex() {}
	PCVertex(float x, float y, float z, float cx, float cy, float cz)
		: Pos{ x,y,z }, Color{ cx, cy, cz } {}
	float Pos[3];
	float Color[3];
};

struct UVVertex
{
	UVVertex() {}
	UVVertex(float x, float y, float z, float tx, float ty)
		: Pos{ x,y,z }, TexturePos{ tx, ty } {}
	float Pos[3];
	float TexturePos[2];
};

struct NVertex
{
	NVertex() {}
	NVertex(float x, float y, float z, float nx, float ny, float nz, float tx, float ty)
		: Pos{ x,y,z }, Normal{ nx,ny,nz }, TexturePos{ tx,ty } {}
	float Pos[3];
	float Normal[3];
	float TexturePos[2];
};

struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, float r, float g, float b, float tx, float ty)
		: Pos{ x,y,z }, Color{ r,g,b }, TexturePos{ tx,ty } {}
	float Pos[3];
	float Color[3];
	float TexturePos[2];
};
#endif