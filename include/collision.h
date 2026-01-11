/* Start Header ************************************************************************/
/*!
\file       collision.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      This file declares the static and dynamic collision functions, as well as
            the main AABB-AABB collision function implementation.
            cirlce-cirlce collision

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#ifndef COLLISION_H
#define COLLISION_H

#include <cstdint>
#include <cstddef>
#include <mathlib.h>
#include "component.h"
#include <vector>
#include "GameObject.h"

namespace Collision {
    using f64 = double;
    extern f64 g_dt;

    //AABB
    class AABB
    {
    private:
        Vector2D min;
        Vector2D max;
    public:
        //constructor ... can add more in future 
        AABB(const Vector2D& _min, const Vector2D& _max) : min(_min), max(_max) {}

        //getters
        const Vector2D& getMin() const { return min; }
        const Vector2D& getMax() const { return max; }

        //setters
        void setMin(const Vector2D& _min) { min = _min; }
        void setMax(const Vector2D& _max) { max = _max; }

        //for imgui
        Vector2D& getMinRef() { return min; }
        Vector2D& getMaxRef() { return max; }
    };

    //circles
    class Circle {
    private:
        Vector2D center;
        float radius;
    public:
        //constructor .. can add more in future 
        Circle() : center(0, 0), radius(1) {}
        Circle(const Vector2D& _center, float _radius) : center(_center), radius(_radius) {}

        //getters
        const Vector2D& getCenter() const { return center; }
        float getRadius() const { return radius; }

        //setters
        void setCenter(const Vector2D& _center) { center = _center; }
        void setRadius(float _radius) { radius = _radius; }

        //for imgui
        Vector2D& getCenterRef() { return center; }
        float& getRadiusRef() { return radius; }
    };

    //Broad phase collision spatial partitioning
    struct Cell {
        std::vector<GameObject*> objects;
        float minX, minY;
        float maxX, maxY;
        Cell() : minX(0), minY(0), maxX(0), maxY(0) {}
    };

    AABB getObjectAABB(const Transform* transform);
    AABB getObjectAABBbyCollider(const Transform* transform, const Vector2D& collider); // use customized collider for AABB
    Circle getObjectCircle(const Transform* transform);
    Circle getObjectCirclebyCollider(const Transform* transform, const Vector2D& collider);

    //AABB and AABB
    bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);
    CollisionInfo CollisionIntersection_RectRect_Dynamic_Info(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2);

    //circle and AABB
    bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb);
    CollisionInfo CollisionIntersection_CircleAABB_Dynamic_Info(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB);

    //circle and circle 
    bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2);
    CollisionInfo CollisionIntersection_CircleCircle_Dynamic_Info(const Circle& c1, const Vector2D& velC1, const Circle& c2, const Vector2D& velC2);

    //cirlce and line
	bool CollisionIntersection_CircleLine_Static(const Circle& c, const Vector2D& lineStart, const Vector2D& lineEnd);
	CollisionInfo CollisionIntersection_CircleLine_Dynamic_Info(const Circle& c, const Vector2D& velC, const Vector2D& lineStart, const Vector2D& lineEnd);
}
#endif
