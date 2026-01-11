/* Start Header ************************************************************************/
/*!
\file       collision.h
\author     Pearly Lin Lee Ying, p.lin, 2401591
            - 100% of the file
\par        p.lin@digipen.edu
\date       September, 17th, 2025
\brief      This file defines the static and dynamic collision functions, as well as
            the main AABB-AABB collision function implementation.
            circle-circle collision 

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "collision.h"

#include <iostream>

namespace Collision {
    //all can change to fit our game later
    f64 g_dt = 0.016;
    const float EPSILON = 1e-5f;

    //helper function to get an AABB from a renderer object
    AABB getObjectAABB(const Transform* transform) {
        float halfWidth = transform->scaleX * 0.5f;
        float halfHeight = transform->scaleY * 0.5f;

        Vector2D center(transform->x, transform->y);
        Vector2D min(center.x - halfWidth, center.y - halfHeight);
        Vector2D max(center.x + halfWidth, center.y + halfHeight);
        return Collision::AABB(min, max);
    }

    AABB getObjectAABBbyCollider(const Transform* transform, const Vector2D& collider) {
        float halfWidth = collider.x * 0.5f;
        float halfHeight = collider.y * 0.5f;

        Vector2D center(transform->x, transform->y);
        Vector2D min(center.x - halfWidth, center.y - halfHeight);
        Vector2D max(center.x + halfWidth, center.y + halfHeight);
        return Collision::AABB(min, max);                         
    }                                                             

	//helper function to get a Circle from a renderer object
    Circle getObjectCircle(const Transform* transform) {
        Vector2D center(transform->x, transform->y);
        float radius = std::max(transform->scaleX, transform->scaleY) * 0.5f;
        return Collision::Circle(center, radius);
    }

    Circle getObjectCirclebyCollider(const Transform* transform, const Vector2D& collider) {
        Vector2D center(transform->x, transform->y);
        float radius = std::max(collider.x, collider.y) * 0.5f; // x and y will be the same actually
        return Collision::Circle(center, radius);
    }

    //AABB
    bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
    {
        const Vector2D& min1 = aabb1.getMin();
        const Vector2D& max1 = aabb1.getMax();
        const Vector2D& min2 = aabb2.getMin();
        const Vector2D& max2 = aabb2.getMax();

        if (max1.x < min2.x || min1.x > max2.x)
            return false;
        if (max1.y < min2.y || min1.y > max2.y)
            return false;

        return true;
    }
    CollisionInfo CollisionIntersection_RectRect_Dynamic_Info(const AABB& aabb1, const Vector2D& vel1, const AABB& aabb2, const Vector2D& vel2)
    {
        CollisionInfo info;
        info.collided = false;
        info.timeOfImpact = static_cast<float>(g_dt);
        info.normal = { 0, 0 };
        info.contactPoint = { 0, 0 };
        info.penetration = 0.0f;

        //relative velocity
        Vector2D Vb{ vel1.x - vel2.x, vel1.y - vel2.y };

        float tFirst = 0.0f;
        float tLast = static_cast<float>(g_dt);

        //x
        float x1 = aabb1.getMin().x;
        float x2 = aabb2.getMin().x;
        float dx1 = aabb1.getMax().x - aabb1.getMin().x;
        float dx2 = aabb2.getMax().x - aabb2.getMin().x;
        float tXFirst, tXLast;

        if (Vb.x < 0) {
            tXFirst = (x2 + dx2 - x1) / Vb.x;
            tXLast = (x2 - (x1 + dx1)) / Vb.x;
        }
        else if (Vb.x > 0) {
            tXFirst = (x2 - (x1 + dx1)) / Vb.x;
            tXLast = (x2 + dx2 - x1) / Vb.x;
        }
        else {
            //no relative motion in X must already overlap
            if (x1 + dx1 <= x2 || x2 + dx2 <= x1) return info;
            tXFirst = 0.0f;
            tXLast = static_cast<float>(g_dt);
        }

        if (tXFirst > tXLast || tXLast < 0.0f || tXFirst > g_dt)
            return info;

        tFirst = std::max(tFirst, tXFirst);
        tLast = std::min(tLast, tXLast);

        //y
        float y1 = aabb1.getMin().y;
        float y2 = aabb2.getMin().y;
        float dy1 = aabb1.getMax().y - aabb1.getMin().y;
        float dy2 = aabb2.getMax().y - aabb2.getMin().y;
        float tYFirst, tYLast;

        if (Vb.y < 0) {
            tYFirst = (y2 + dy2 - y1) / Vb.y;
            tYLast = (y2 - (y1 + dy1)) / Vb.y;
        }
        else if (Vb.y > 0) {
            tYFirst = (y2 - (y1 + dy1)) / Vb.y;
            tYLast = (y2 + dy2 - y1) / Vb.y;
        }
        else {
            //no relative motion in Y must already overlap
            if (y1 + dy1 <= y2 || y2 + dy2 <= y1) return info;
            tYFirst = 0.0f;
            tYLast = static_cast<float>(g_dt);
        }

        if (tYFirst > tYLast || tYLast < 0.0f || tYFirst > g_dt)
            return info;

        tFirst = std::max(tFirst, tYFirst);
        tLast = std::min(tLast, tYLast);

        //final check
        if (tFirst < tLast && tFirst >= 0.0f && tFirst <= g_dt) {
            info.collided = true;
            info.timeOfImpact = tFirst;

            //determine collision normal based on which axis constrained first
            if (tXFirst > tYFirst) {
                info.normal = (Vb.x > 0) ? Vector2D{ -1, 0 } : Vector2D{ 1, 0 };
            }
            else {
                info.normal = (Vb.y > 0) ? Vector2D{ 0, -1 } : Vector2D{ 0, 1 };
            }

            //compute contact point
            Vector2D cp = aabb1.getMin();
            cp.x += vel1.x * tFirst;
            cp.y += vel1.y * tFirst;
            info.contactPoint = cp;
            //return info;
        }

        //testing
        if (aabb1.getMax().x > aabb2.getMin().x && aabb1.getMin().x < aabb2.getMax().x &&
            aabb1.getMax().y > aabb2.getMin().y && aabb1.getMin().y < aabb2.getMax().y)
        {
            info.collided = true;

            float overlapX = std::min(aabb1.getMax().x - aabb2.getMin().x,
                aabb2.getMax().x - aabb1.getMin().x);
            float overlapY = std::min(aabb1.getMax().y - aabb2.getMin().y,
                aabb2.getMax().y - aabb1.getMin().y);

            if (overlapX < overlapY) {
                info.penetration = overlapX;
                info.normal = (aabb1.getMax().x > aabb2.getMax().x)
                    ? Vector2D(1, 0)
                    : Vector2D(-1, 0);
            }
            else {
                info.penetration = overlapY;
                info.normal = (aabb1.getMax().y > aabb2.getMax().y)
                    ? Vector2D(0, 1)
                    : Vector2D(0, -1);
            }
        }
        return info;
    }

    //AABB with circles 
    bool CollisionIntersection_CircleAABB_Static(const Circle& c, const AABB& aabb)
    {
        const Vector2D& center = c.getCenter();
        float radius = c.getRadius();

        //find closest point on AABB to circle center
        float closestX = Math_Max(aabb.getMin().x, Math_Min(center.x, aabb.getMax().x));
        float closestY = Math_Max(aabb.getMin().y, Math_Min(center.y, aabb.getMax().y));

        //distance between center to closest point
        float dx = center.x - closestX;
        float dy = center.y - closestY;

        return (dx * dx + dy * dy) <= (radius * radius + EPSILON);
    }
    CollisionInfo CollisionIntersection_CircleAABB_Dynamic_Info(const Circle& c, const Vector2D& velC, const AABB& aabb, const Vector2D& velAABB)
    {
        CollisionInfo info;
        Vector2D relVel = { velC.x - velAABB.x, velC.y - velAABB.y };

        float closestX = Math_Max(aabb.getMin().x, Math_Min(c.getCenter().x, aabb.getMax().x));
        float closestY = Math_Max(aabb.getMin().y, Math_Min(c.getCenter().y, aabb.getMax().y));
        float dx = c.getCenter().x - closestX;
        float dy = c.getCenter().y - closestY;
        float distSq = dx * dx + dy * dy;
        if (distSq <= c.getRadius() * c.getRadius() + EPSILON) {
            info.collided = true;
            info.timeOfImpact = 0.0f;
            Vector2D temp = { dx, dy };
            Vector2D n = Vec_Normalize(&temp);
            if (Vec_Length(&n) <= EPSILON) {
                //if center inside pick nearest axis normal by distance to edges
                float left = std::abs(c.getCenter().x - aabb.getMin().x);
                float right = std::abs(aabb.getMax().x - c.getCenter().x);
                float bottom = std::abs(c.getCenter().y - aabb.getMin().y);
                float top = std::abs(aabb.getMax().y - c.getCenter().y);
                float minv = std::min(std::min(left, right), std::min(bottom, top));
                if (minv == left) n = Vector2D{ -1,0 };
                else if (minv == right) n = Vector2D{ 1,0 };
                else if (minv == bottom) n = Vector2D{ 0,-1 };
                else n = Vector2D{ 0,1 };
            }
            info.normal = n;
            return info;
        }

        float minX = aabb.getMin().x - c.getRadius();
        float maxX = aabb.getMax().x + c.getRadius();
        float minY = aabb.getMin().y - c.getRadius();
        float maxY = aabb.getMax().y + c.getRadius();

        float txEntry, txExit, tyEntry, tyExit;
        if (relVel.x > 0.0f) {
            txEntry = (minX - c.getCenter().x) / relVel.x;
            txExit = (maxX - c.getCenter().x) / relVel.x;
        }
        else if (relVel.x < 0.0f) {
            txEntry = (maxX - c.getCenter().x) / relVel.x;
            txExit = (minX - c.getCenter().x) / relVel.x;
        }
        else {
            txEntry = -INFINITY; txExit = INFINITY;
        }

        if (relVel.y > 0.0f) {
            tyEntry = (minY - c.getCenter().y) / relVel.y;
            tyExit = (maxY - c.getCenter().y) / relVel.y;
        }
        else if (relVel.y < 0.0f) {
            tyEntry = (maxY - c.getCenter().y) / relVel.y;
            tyExit = (minY - c.getCenter().y) / relVel.y;
        }
        else {
            tyEntry = -INFINITY; tyExit = INFINITY;
        }

        float entryTime = std::max(txEntry, tyEntry);
        float exitTime = std::min(txExit, tyExit);

        if (entryTime > exitTime || entryTime < 0.0f || entryTime > 1.0f) {
            info.collided = false;
            return info;
        }

        Vector2D normal{ 0,0 };
        if (txEntry > tyEntry) {
            //x axis
            normal = (relVel.x > 0.0f) ? Vector2D{ -1,0 } : Vector2D{ 1,0 };
        }
        else {
            normal = (relVel.y > 0.0f) ? Vector2D{ 0,-1 } : Vector2D{ 0,1 };
        }

        info.collided = true;
        info.timeOfImpact = entryTime;
        info.normal = normal;
        return info;
    }

    //circle to circle
    bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2) {
        //get radius
        float radius1 = c1.getRadius();
        float radius2 = c2.getRadius();
        //get center
		const Vector2D& center1 = c1.getCenter();
		const Vector2D& center2 = c2.getCenter();

		//distance between centers x and y
		float dx = center2.x - center1.x;
		float dy = center2.y - center1.y;
		//distance squared so dont need to square root
        float distSq = dx * dx + dy * dy;
		float radiusSum = radius1 + radius2;
        
        return distSq <= radiusSum * radiusSum + EPSILON;
    }

    //circle to circle dynamic 
    CollisionInfo CollisionIntersection_CircleCircle_Dynamic_Info(const Circle& c1, const Vector2D& velC1, const Circle& c2, const Vector2D& velC2) {
        CollisionInfo info;
        info.collided = false;
        info.timeOfImpact = static_cast<float>(g_dt);
        info.normal = { 0, 0 };
        info.contactPoint = { 0, 0 };
        info.penetration = 0.0f;

		//relative positions and velocities
        Vector2D s = Vec_Sub(&c1.getCenter(), &c2.getCenter()); //vector from c2 to c1
        Vector2D v = Vec_Sub(&velC1, &velC2);                   //relative velocity from c2 to c1
        float rSum = c1.getRadius() + c2.getRadius();
        
        float sDotS = Vec_Dot(&s, &s);  //squared distance between centers
        float rSumSq = rSum * rSum;     //squared sum of r

        if (sDotS <= rSumSq + EPSILON) {
            info.collided = true;
            info.timeOfImpact = 0.0f;
            Vector2D n = s;
            if (Vec_Length(&n) <= EPSILON) {
                n = Vector2D{ 1.0f, 0.0f }; // fallback if centers coincide
            }
            else {
                n = Vec_Normalize(&n); // normalize to unit vector
            }
            info.normal = n;
            Vector2D scaledNormal = Vec_Scale(&n, c1.getRadius());
            info.contactPoint = Vec_Sub(&c1.getCenter(), &scaledNormal);
            return info;
        }

        //to get the quadratic equation
        float a = Vec_Dot(&v, &v); 
        if (a <= EPSILON) {
            info.collided = false; //no collide
            return info;
        }
        
        float b = 2.0f * Vec_Dot(&s, &v);
        float c = sDotS - rSumSq;

        float discriminant = b * b - 4.0f * a * c;
        if (discriminant < 0.0f) {
            info.collided = false; //no collide
            return info;
        }
        float sqrtD = std::sqrt(discriminant);
        float t1 = (-b - sqrtD) / (2.0f * a);
        float t2 = (-b + sqrtD) / (2.0f * a);
        float toi = INFINITY;
        if (t1 >= 0.0f && t1 <= static_cast<float>(g_dt)) toi = t1;
        else if (t2 >= 0.0f && t2 <= static_cast<float>(g_dt)) toi = t2;

        if (!std::isfinite(toi)) {
            info.collided = false;
            return info;
        }
		//normal at contact
		Vector2D vScale1 = Vec_Scale(&velC1, toi);
		Vector2D vScale2 = Vec_Scale(&velC2, toi);
        Vector2D c1AtT = Vec_Add(&c1.getCenter(), &vScale1);
        Vector2D c2AtT = Vec_Add(&c2.getCenter(), &vScale2);

        Vector2D normal = Vec_Sub(&c1AtT, &c2AtT);
        if (Vec_Length(&normal) <= EPSILON) {
            normal = Vector2D{ 1.0f, 0.0f }; //fall back
        }
        else {
            normal = Vec_Normalize(&normal);
        }
        info.normal = normal;

        //contact point 
		Vector2D cpScale = Vec_Scale(&normal, c1.getRadius());
        info.contactPoint = Vec_Sub(&c1AtT, &cpScale);

        info.collided = true;
        info.timeOfImpact = toi;
        info.penetration = 0.0f;
        return info;
    }

    //cirlce and line
    bool CollisionIntersection_CircleLine_Static(const Circle& c, const Vector2D& lineStart, const Vector2D& lineEnd) {
        float radius = c.getRadius(); //get radius
        const Vector2D& center = c.getCenter(); //get center

        //line vector
        Vector2D lineVec = Vec_Sub(&lineEnd, &lineStart);
        Vector2D toCenter = Vec_Sub(&center,&lineStart);

        //project center onto line
		float projection = Vec_Dot(&toCenter, &lineVec) / Vec_Dot(&lineVec, &lineVec);

		//clamp projection to line segment
        if (projection < 0.0f) projection = 0.0f;
		if (projection > 1.0f) projection = 1.0f;

		//closest point on line segment to circle center
        Vector2D scaling = Vec_Scale(&lineVec, projection); 
        Vector2D closestPoint = Vec_Add(&lineStart, &scaling);
		Vector2D diff = Vec_Sub(&center, &closestPoint); //distances between closest point and center
		float distSq = Vec_Dot(&diff, &diff); //distance squared
		return distSq <= radius * radius + EPSILON; 
    }
	//circle to line dynamic
    CollisionInfo CollisionIntersection_CircleLine_Dynamic_Info(const Circle& c, const Vector2D& velC, const Vector2D& lineStart, const Vector2D& lineEnd) {
        CollisionInfo info;
		info.collided = false;

        float radius = c.getRadius(); //get radius
        const Vector2D& center = c.getCenter(); //get center

        //line vector
        Vector2D lineVec = Vec_Sub(&lineEnd, &lineStart);
        Vector2D toCenter = Vec_Sub(&center, &lineStart);

        //normal
		Vector2D lineNormal = Vector2D{ -lineVec.y, lineVec.x };
		float len = Vec_Length(&lineNormal);
		if (len > 0.0f) lineNormal = Vec_Scale(&lineNormal, 1.0f / len);

		//relative velocity along normal
		float relVelAlongNormal = Vec_Dot(&velC, &lineNormal); //circle velocity
		//this is for earliest contact
		float dist = Vec_Dot(&toCenter, &lineNormal); //perpendicular distance from circle edge to line
        if (relVelAlongNormal != 0.0f) {
            float toi = (radius - dist) / relVelAlongNormal; //time of impact
			if (toi >= 0.0f && toi <= static_cast<float>(g_dt)) { //within time frame
                info.collided = true;
                info.timeOfImpact = toi;
                info.normal = (relVelAlongNormal < 0.0f) ? Vec_Scale(&lineNormal, -1.0f) : lineNormal;
            }
        }
        return info;
    }

    //remove normal ... when object hit the wall
    static inline Vector2D RemoveNormalComponent(const Vector2D& v, const Vector2D& n) {
        float d = Vec_Dot(&v, &n);
        Vector2D scaledN = Vec_Scale(&n, d);
        return Vec_Sub(&v, &scaledN);
    }
}
