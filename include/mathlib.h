/* Start Header ************************************************************************/
/*!
\file       mathlib.h
\author     Asha Mathyalakan, asha.m, 2402886
            - 100% of the file
\par        asha.m@digipen.edu
\date       September, 11th, 2025
\brief      This file implements the declarations of basic scalar math functions,
            utilities for 2D and 3D operations, functions for creating and
            manipulating 3x3 and 4x4 matrices.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#ifndef MATHLIB_H
#define MATHLIB_H

#include <cmath>

typedef struct {
    float x;
    float y;
} Vector2D;

typedef struct {
    float m[3][3];
} Mat33;

typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

typedef struct {
    float m[4][4];
} Mat44;

//------------------------------------------
// **GLM Conversions for graphics coders only**
// usage: Before including your mathlib.h, 
// define this #define GLM_CONVERSION
// 
// Normal matrix to GLM 
// eg. Vector2D myVec2{1.0f, 2.0f};
// glm::vec2 glmVec2 = myVec2; 
//
// GLM to Normal Matrix
// eg. glm::vec2 gVec2(5.0f, 6.0f);
//  Vector2D myVec2FromGLM(gVec2);
// 
//----------------------------------------
//#define GLM_CONVERSION
#ifdef GLM_CONVERSION
#include <glm/glm.hpp>

//---------------- Vector2D ----------------
struct Vector2D {
    float x, y;

    operator glm::vec2() const { return glm::vec2(x, y); }
    Vector2D(const glm::vec2& v) : x(v.x), y(v.y) {}
    Vector2D() : x(0), y(0) {}
};

//---------------- Vector3D ----------------
struct Vector3D {
    float x, y, z;

    operator glm::vec3() const { return glm::vec3(x, y, z); }
    Vector3D(const glm::vec3& v) : x(v.x), y(v.y), z(v.z) {}
    Vector3D() : x(0), y(0), z(0) {}
};

//---------------- Mat33 ----------------
struct Mat33 {
    float m[3][3];

    operator glm::mat3() const {
        return glm::mat3(
            m[0][0], m[0][1], m[0][2],
            m[1][0], m[1][1], m[1][2],
            m[2][0], m[2][1], m[2][2]
        );
    }

    Mat33(const glm::mat3& g) {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = g[i][j];
    }
    Mat33() { for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++) m[i][j] = 0; }
};

//---------------- Mat44 ----------------
struct Mat44 {
    float m[4][4];

    operator glm::mat4() const {
        return glm::mat4(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]
        );
    }

    Mat44(const glm::mat4& g) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = g[i][j];
    }
    Mat44() { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = 0; }
};

#endif //GLM_CONVERSION

//----------------------------------------
// Math Functions
//----------------------------------------

// Basic math utilities 
float Math_Sqrt(float v);
float Math_Fabs(float v);
float Math_Fmax(float a, float b);
float Math_Min(float a, float b);
float Math_Max(float a, float b);

// Angle conversion
float DegToRad(float deg);
float RadToDeg(float rad);

// Radian trigonometry
float SinRad(float rad);
float CosRad(float rad);
float TanRad(float rad);
float ASinRad(float val);
float ACosRad(float val);
float ATanRad(float val);

// Degree trigonometry
float SinDeg(float deg);
float CosDeg(float deg);
float TanDeg(float deg);
float ASinDeg(float val);
float ACosDeg(float val);
float ATanDeg(float val);


//------------ Vector2D operations------------------
// 
// Add two 2D vectors
Vector2D Vec_Add(const Vector2D* a, const Vector2D* b);

// Subtract two 2D vectors
Vector2D Vec_Sub(const Vector2D* a, const Vector2D* b);

// Normalize a 2D vector
Vector2D Vec_Normalize(const Vector2D* v);

// Length (magnitude) of a 2D vector
float Vec_Length(const Vector2D* v);

// Distance between two 2D vectors
float Vec_Distance(const Vector2D* a, const Vector2D* b);

// Dot product of two 2D vectors
float Vec_Dot(const Vector2D* a, const Vector2D* b);

// Scalar cross product of two 2D vectors
float Vec_Cross(const Vector2D* a, const Vector2D* b);

// Negate a 2D vector
Vector2D Vec_Negate(const Vector2D* v);

// Create a 2D unit vector from an angle in radians
Vector2D Vec_FromAngle(float rad);

// Set 2D vector values
Vector2D Vec_Set(float x, float y);

// Return zero 2D vector
Vector2D Vec_Zero();

// Scale a 2D vector
Vector2D Vec_Scale(const Vector2D* v, float scale);

// Rotate a 2D vector by radians
Vector2D Vec_Rotate(const Vector2D* v, float rad);

// Transform a 2D vector by a 3x3 matrix
Vector2D Vec_Transform(const Vector2D* v, const Mat33* mat);

// Vector2D element-wise multiplication
Vector2D Vec_Multi(const Vector2D* a, const Vector2D* b);

// Vector2D element-wise division  
Vector2D Vec_Div(const Vector2D* a, const Vector2D* b);



// ------------------- Mat33 operations ------------------
// Set a 3x3 matrix to identity
void Mat33_Identity(Mat33* mat);

// Transpose a 3x3 matrix
void Mat33_Transpose(Mat33* out, const Mat33* mat);

// Determinant of a 3x3 matrix
float Mat33_Det(const Mat33* mat);

// Create a 3x3 scaling matrix
void Mat33_Scale(Mat33* mat, float x, float y);

// Create a 3x3 rotation matrix (radians)
void Mat33_Rot(Mat33* mat, float rad);

// Create a 3x3 translation matrix
void Mat33_Trans(Mat33* mat, float x, float y);

// Concatenate two 3x3 matrices
void Mat33_Con_cat(Mat33* con_cat, const Mat33* a, const Mat33* b);

// ----------------- Vector3D operations -----------------
// Add two 3D vectors
Vector3D Vec3_Add(const Vector3D* a, const Vector3D* b);

// Subtract two 3D vectors
Vector3D Vec3_Sub(const Vector3D* a, const Vector3D* b);

// Negate a 3D vector
Vector3D Vec3_Negate(const Vector3D* v);

// Set 3D vector values
Vector3D Vec3_Set(float x, float y, float z);

// Return zero 3D vector
Vector3D Vec3_Zero();

// Length (magnitude) of a 3D vector
float Vec3_Length(const Vector3D* v);

// Distance between two 3D vectors
float Vec3_Distance(const Vector3D* a, const Vector3D* b);

// Dot product of two 3D vectors
float Vec3_Dot(const Vector3D* a, const Vector3D* b);

// Cross product of two 3D vectors
Vector3D Vec3_Cross(const Vector3D* a, const Vector3D* b);

// Normalize a 3D vector
Vector3D Vec3_Normalize(const Vector3D* v);

// Scale a 3D vector
Vector3D Vec3_Scale(const Vector3D* v, float scale);

// Transform a 3D vector by a 4x4 matrix
Vector3D Vec3_Transform(const Vector3D* v, const Mat44* mat);

// Vector3D element-wise multiplication
Vector3D Vec3_Multi(const Vector3D* a, const Vector3D* b);

// Vector3D element-wise division
Vector3D Vec3_Div(const Vector3D* a, const Vector3D* b);

// -------------- Mat44 operations --------------------- 

// Set a 4x4 matrix to identity
void Mat44_Identity(Mat44* mat);

// Transpose a 4x4 matrix
void Mat44_Transpose(Mat44* out, const Mat44* mat);

// Create a 4x4 scaling matrix
void Mat44_Scale(Mat44* mat, float x, float y, float z);

// Create a 4x4 rotation matrix around X-axis (radians)
void Mat44_RotX(Mat44* mat, float rad);

// Create a 4x4 rotation matrix around Y-axis (radians)
void Mat44_RotY(Mat44* mat, float rad);

// Create a 4x4 rotation matrix around Z-axis (radians)
void Mat44_RotZ(Mat44* mat, float rad);

// Create a 4x4 translation matrix
void Mat44_Trans(Mat44* mat, float x, float y, float z);

// Concatenate two 4x4 matrices
void Mat44_Con_cat(Mat44* con_cat, const Mat44* a, const Mat44* b);

#endif 