#pragma once

struct Mat4 {
public:
    float data[4][4];
};

struct vec4f {
    float x, y, z, w;

    vec4f() = default;
    vec4f(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};

vec4f transformVertexScalar(const Mat4& mat, vec4f& v) {
    float nx = mat.data[0][0] * v.x + mat.data[0][1] * v.y + mat.data[0][2] * v.z + mat.data[0][3] * v.w;
    float ny = mat.data[1][0] * v.x + mat.data[1][1] * v.y + mat.data[1][2] * v.z + mat.data[1][3] * v.w;
    float nz = mat.data[2][0] * v.x + mat.data[2][1] * v.y + mat.data[2][2] * v.z + mat.data[2][3] * v.w;
    float nw = mat.data[3][0] * v.x + mat.data[3][1] * v.y + mat.data[3][2] * v.z + mat.data[3][3] * v.w;
    return { nx, ny, nz, nw };
}
