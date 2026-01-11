/* Start Header ************************************************************************/
/*!
\file       mathlib.cpp
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       September, 11th, 2025
\brief      This file implements basic scalar math functions, utilities for 2D and 3D
            operations, functions for creating and manipulating 3x3 and 4x4 matrices

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "mathlib.h"

float Math_Sqrt(float v) {
    return std::sqrt(v);
}
float Math_Fabs(float v) {
    return std::fabs(v);
}
float Math_Fmax(float a, float b) {
    return std::fmax(a, b);
}
float Math_Min(float a, float b) {
    return (a < b) ? a : b;
}
float Math_Max(float a, float b) {
    return (a > b) ? a : b;
}

// --- Angle conversion ---
float DegToRad(float deg) {
    return deg * (3.14159265358979323846f / 180.0f);
}

float RadToDeg(float rad) {
    return rad * (180.0f / 3.14159265358979323846f);
}

// --- Radian trigonometry ---
float SinRad(float rad) { return std::sin(rad); }
float CosRad(float rad) { return std::cos(rad); }
float TanRad(float rad) { return std::tan(rad); }
float ASinRad(float val) { return std::asin(val); }
float ACosRad(float val) { return std::acos(val); }
float ATanRad(float val) { return std::atan(val); }

// --- Degre trigonometry ---
float SinDeg(float deg) { return std::sin(DegToRad(deg)); }
float CosDeg(float deg) { return std::cos(DegToRad(deg)); }
float TanDeg(float deg) { return std::tan(DegToRad(deg)); }
float ASinDeg(float val) { return RadToDeg(std::asin(val)); }
float ACosDeg(float val) { return RadToDeg(std::acos(val)); }
float ATanDeg(float val) { return RadToDeg(std::atan(val)); }



//----------------------------------------
// Vector Operations for 2
//----------------------------------------

// Vector addition: sum = a + b
// Formula: 
//    sum.x = a.x + b.x
//    sum.y = a.y + b.y
Vector2D Vec_Add(const Vector2D* a, const Vector2D* b) {
    Vector2D sum = { a->x + b->x,
                     a->y + b->y };
    return sum;
}

// Vector subtraction: sub = a - b
// Formula: 
//    sub.x = a.x - b.x
//    sub.y = a.y - b.y
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b) {
    Vector2D sub = { a->x - b->x,
                     a->y - b->y };
    return sub;
}

// Normalize a vector: Norm = v / |v|
// Formula:
//    |v| = sqrt(v.x^2 + v.y^2)
//    Norm.x = v.x / |v|
//    Norm.y = v.y / |v|
Vector2D Vec_Normalize(const Vector2D* v) {
    Vector2D Norm = { 0.0f, 0.0f };
    float dis = sqrtf(v->x * v->x + v->y * v->y);
    if (dis > 0.00001f) { // avoid division by zero
        Norm.x = v->x / dis;
        Norm.y = v->y / dis;
    }
    return Norm;
}

// Length (magnitude) of a vector
float Vec_Length(const Vector2D* v) {
    return sqrtf(v->x * v->x + v->y * v->y);
}

// Distance between two vectors
float Vec_Distance(const Vector2D* a, const Vector2D* b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    return sqrtf(dx * dx + dy * dy);
}

// Dot product: a · b
float Vec_Dot(const Vector2D* a, const Vector2D* b) {
    return a->x * b->x + a->y * b->y;
}

// Cross product (2D scalar version): a × b
float Vec_Cross(const Vector2D* a, const Vector2D* b) {
    return a->x * b->y - a->y * b->x;
}

// Negate a vector: -v
Vector2D Vec_Negate(const Vector2D* v) {
    Vector2D neg = { -v->x, -v->y };
    return neg;
}

// Create a vector from an angle in radians-->unit vector
Vector2D Vec_FromAngle(float rad) {
    Vector2D v = { cosf(rad), sinf(rad) };
    return v;
}

// Set vector values
Vector2D Vec_Set(float x, float y) {
    Vector2D v = { x, y };
    return v;
}

// Zero vector
Vector2D Vec_Zero() {
    Vector2D v = { 0.0f, 0.0f };
    return v;
}

// Scale a vector: scaled = v * scale
// Formula:
//    scaled.x = v.x * scale
//    scaled.y = v.y * scale
Vector2D Vec_Scale(const Vector2D* v, float scale) {
    Vector2D scaled = { v->x * scale, v->y * scale };
    return scaled;
}


// Rotate a vector by rad radians
// Formula (2D rotation matrix):
//    [x']   [ cosθ  -sinθ ] [x]
//    [y'] = [ sinθ   cosθ ] [y]
//    rotated.x = v.x * cos(rad) - v.y * sin(rad)
//    rotated.y = v.x * sin(rad) + v.y * cos(rad)
Vector2D Vec_Rotate(const Vector2D* v, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);
    Vector2D rotated = { v->x * cos_value - v->y * sin_value,
                         v->x * sin_value + v->y * cos_value };
    return rotated;
}

// Transform a vector by a 3x3 matrix (affine transformation)
// Formula:
//    [x']   [ m00 m01 m02 ] [x]
//    [y'] = [ m10 m11 m12 ] [y]
//    x' = x * m00 + y * m01 + m02
//    y' = x * m10 + y * m11 + m12
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat) {
    Vector2D transformed;
    transformed.x = v->x * mat->m[0][0] + v->y * mat->m[0][1] + mat->m[0][2];
    transformed.y = v->x * mat->m[1][0] + v->y * mat->m[1][1] + mat->m[1][2];
    return transformed;
}

Vector2D Vec_Multi(const Vector2D* a, const Vector2D* b) {
    Vector2D result = { a->x * b->x, a->y * b->y };
    return result;
}

Vector2D Vec_Div(const Vector2D* a, const Vector2D* b) {
    Vector2D result = { 0.0f, 0.0f };
    if (b->x != 0.0f && b->y != 0.0f) {
        result.x = a->x / b->x;
        result.y = a->y / b->y;
    }
    return result;
}


//----------------------------------------
// Matrix Operations (3x3)
//----------------------------------------

// Set identity matrix
void Mat33_Identity(Mat33* mat) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mat->m[i][j] = (i == j) ? 1.0f : 0.0f;
}

// Transpose: mat^T
void Mat33_Transpose(Mat33* out, const Mat33* mat) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            out->m[i][j] = mat->m[j][i];
}

// Determinant
float Mat33_Det(const Mat33* mat) {
    return mat->m[0][0] * (mat->m[1][1] * mat->m[2][2] - mat->m[1][2] * mat->m[2][1])
        - mat->m[0][1] * (mat->m[1][0] * mat->m[2][2] - mat->m[1][2] * mat->m[2][0])
        + mat->m[0][2] * (mat->m[1][0] * mat->m[2][1] - mat->m[1][1] * mat->m[2][0]);
}

// Create a scaling matrix
// Formula:
//    [ x  0  0 ]
//    [ 0  y  0 ]
//    [ 0  0  1 ]
void Mat33_Scale(Mat33* mat, float x, float y) {
    mat->m[0][0] = x; mat->m[0][1] = 0; mat->m[0][2] = 0;
    mat->m[1][0] = 0; mat->m[1][1] = y; mat->m[1][2] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Create a rotation matrix (radians)
// Formula:
//    [ cosθ  -sinθ  0 ]
//    [ sinθ   cosθ  0 ]
//    [ 0      0     1 ]
void Mat33_Rot(Mat33* mat, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);

    mat->m[0][0] = cos_value; mat->m[0][1] = -sin_value; mat->m[0][2] = 0;
    mat->m[1][0] = sin_value; mat->m[1][1] = cos_value; mat->m[1][2] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Create a translation matrix of 3by3
// Formula:
//    [ 1  0  x ]
//    [ 0  1  y ]
//    [ 0  0  1 ]
void Mat33_Trans(Mat33* mat, float x, float y) {
    mat->m[0][0] = 1; mat->m[0][1] = 0; mat->m[0][2] = x;
    mat->m[1][0] = 0; mat->m[1][1] = 1; mat->m[1][2] = y;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;
}

// Concatenate two matrices: con_cat = a * b
// Formula: standard matrix multiplication
//    con_cat[i][j] = sum(a[i][k] * b[k][j]) for k = 0..2
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b) {
    Mat33 temp;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            temp.m[i][j] = 0.0f;
            for (int k = 0; k < 3; k++)
            {
                temp.m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
    *con_cat = temp;
}

//----------------------------------------
// Vector3D Operations
//----------------------------------------

// Add, subtract
Vector3D Vec3_Add(const Vector3D* a, const Vector3D* b) {
    Vector3D sum = { a->x + b->x,
                     a->y + b->y,
                     a->z + b->z };
    return sum;
}

Vector3D Vec3_Sub(const Vector3D* a, const Vector3D* b) {
    Vector3D sub = { a->x - b->x,
                     a->y - b->y,
                     a->z - b->z };
    return sub;
}

// Negate a vector: -v
Vector3D Vec3_Negate(const Vector3D* v) {
    Vector3D neg = { -v->x, -v->y, -v->z };
    return neg;
}

// Set vector values
Vector3D Vec3_Set(float x, float y, float z) {
    Vector3D v = { x, y, z };
    return v;
}

// Zero vector
Vector3D Vec3_Zero() {
    Vector3D v = { 0.0f, 0.0f, 0.0f };
    return v;
}

// Length magnitude of a vector
float Vec3_Length(const Vector3D* v) {
    return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

// Distance between two vectors
float Vec3_Distance(const Vector3D* a, const Vector3D* b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    float dz = a->z - b->z;
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

// Dot product: a · b
float Vec3_Dot(const Vector3D* a, const Vector3D* b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

// Cross product: a × b
Vector3D Vec3_Cross(const Vector3D* a, const Vector3D* b) {
    Vector3D cross;
    cross.x = a->y * b->z - a->z * b->y;
    cross.y = a->z * b->x - a->x * b->z;
    cross.z = a->x * b->y - a->y * b->x;
    return cross;
}

//Normalise
Vector3D Vec3_Normalize(const Vector3D* v) {
    Vector3D Norm = { 0.0f, 0.0f, 0.0f };
    float dis = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (dis > 0.00001f) {
        Norm.x = v->x / dis;
        Norm.y = v->y / dis;
        Norm.z = v->z / dis;
    }
    return Norm;
}

//scale
Vector3D Vec3_Scale(const Vector3D* v, float scale) {
    Vector3D scaled = { v->x * scale, v->y * scale , v->z * scale };
    return scaled;
}
//transform
Vector3D Vec3_Transform(const Vector3D* v, const Mat44* mat) {
    Vector3D transformed;
    transformed.x = v->x * mat->m[0][0] + v->y * mat->m[0][1] + v->z * mat->m[0][2] + mat->m[0][3];
    transformed.y = v->x * mat->m[1][0] + v->y * mat->m[1][1] + v->z * mat->m[1][2] + mat->m[1][3];
    transformed.z = v->x * mat->m[2][0] + v->y * mat->m[2][1] + v->z * mat->m[2][2] + mat->m[2][3];
    return transformed;
}

Vector3D Vec3_Multi(const Vector3D* a, const Vector3D* b) {
    Vector3D result = { a->x * b->x, a->y * b->y, a->z * b->z };
    return result;
}

Vector3D Vec3_Div(const Vector3D* a, const Vector3D* b) {
    Vector3D result = { 0.0f, 0.0f, 0.0f };
    if (b->x != 0.0f && b->y != 0.0f && b->z != 0.0f) {
        result.x = a->x / b->x;
        result.y = a->y / b->y;
        result.z = a->z / b->z;
    }
    return result;
}
//----------------------------------------
// Matrix Operations (4x4)
//----------------------------------------
// Set identity matrix
void Mat44_Identity(Mat44* mat) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            mat->m[i][j] = (i == j) ? 1.0f : 0.0f;
}

// Transpose
void Mat44_Transpose(Mat44* out, const Mat44* mat) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            out->m[i][j] = mat->m[j][i];
}
// Scale matrix
void Mat44_Scale(Mat44* mat, float x, float y, float z) {
    mat->m[0][0] = x; mat->m[0][1] = 0; mat->m[0][2] = 0; mat->m[0][3] = 0;
    mat->m[1][0] = 0; mat->m[1][1] = y; mat->m[1][2] = 0; mat->m[1][3] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = z; mat->m[2][3] = 0;
    mat->m[3][0] = 0; mat->m[3][1] = 0; mat->m[3][2] = 0; mat->m[3][3] = 1;
}
// Rotation matrices --> X
void Mat44_RotX(Mat44* mat, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);

    mat->m[0][0] = 1; mat->m[0][1] = 0; mat->m[0][2] = 0; mat->m[0][3] = 0;
    mat->m[1][0] = 0; mat->m[1][1] = cos_value; mat->m[1][2] = -sin_value; mat->m[1][3] = 0;
    mat->m[2][0] = 0; mat->m[2][1] = sin_value; mat->m[2][2] = cos_value; mat->m[2][3] = 0;
    mat->m[3][0] = 0; mat->m[3][1] = 0; mat->m[3][2] = 0; mat->m[3][3] = 1;
}
// Rotation matrices --> Y
void Mat44_RotY(Mat44* mat, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);

    mat->m[0][0] = cos_value; mat->m[0][1] = 0; mat->m[0][2] = sin_value; mat->m[0][3] = 0;
    mat->m[1][0] = 0; mat->m[1][1] = 1; mat->m[1][2] = 0; mat->m[1][3] = 0;
    mat->m[2][0] = -sin_value; mat->m[2][1] = 0; mat->m[2][2] = cos_value; mat->m[2][3] = 0;
    mat->m[3][0] = 0; mat->m[3][1] = 0; mat->m[3][2] = 0; mat->m[3][3] = 1;
}
// Rotation matrices --> Z
void Mat44_RotZ(Mat44* mat, float rad) {
    float cos_value = cosf(rad);
    float sin_value = sinf(rad);

    mat->m[0][0] = cos_value; mat->m[0][1] = -sin_value; mat->m[0][2] = 0; mat->m[0][3] = 0;
    mat->m[1][0] = sin_value; mat->m[1][1] = cos_value; mat->m[1][2] = 0; mat->m[1][3] = 0;
    mat->m[2][0] = 0;         mat->m[2][1] = 0;         mat->m[2][2] = 1; mat->m[2][3] = 0;
    mat->m[3][0] = 0;         mat->m[3][1] = 0;         mat->m[3][2] = 0; mat->m[3][3] = 1;
}

// Create a translation matrix of 4by4
// Formula:
//    [ 1  0  0  x ]
//    [ 0  1  0  y ]
//    [ 0  0  1  0 ]
//    [ 0  0  0  1 ]
void Mat44_Trans(Mat44* mat, float x, float y, float z) {
    mat->m[0][0] = 1; mat->m[0][1] = 0; mat->m[0][2] = 0; mat->m[0][3] = x;
    mat->m[1][0] = 0; mat->m[1][1] = 1; mat->m[1][2] = 0;  mat->m[1][3] = y;
    mat->m[2][0] = 0; mat->m[2][1] = 0; mat->m[2][2] = 1;  mat->m[2][3] = z;
    mat->m[3][0] = 0; mat->m[3][1] = 0; mat->m[3][2] = 0;  mat->m[3][3] = 1;
}
// Concatenate (multiply) two 4x4 matrices
void Mat44_Con_cat(Mat44* con_cat, const Mat44* a, const Mat44* b) {
    Mat44 temp;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++)
            {
                temp.m[i][j] += a->m[i][k] * b->m[k][j];
            }
        }
    }
    *con_cat = temp;
}