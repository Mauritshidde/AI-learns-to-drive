#include <raylib.h>
#include <cmath>
#include <vector>
#include <iostream>

class Rays {
    public:
        Rays(int newRayAmount = 8, int newRaylenght = 200);
        ~Rays();
        void update(float *x, float *y, double newAngle, Map* map);
        void draw();
        double lerp(double A, double B, double t);
        Vector4 getIntersection(Vector2 A, Vector2 B, Vector2 C, Vector2 D);
        int rayAmountInt;
        std::vector<Vector3> hitCoordVec3; 
    private:
        bool calcRayHits(int endRayLoc, Map* map);
        void castRays();

        Vector2 startRay;
        Vector3 hitCoordVec;
        std::vector<Vector2> eindRay, wallVector, outerWallvector; 

        int mapArraySize, outerMapArraySize, rayAmount;
        double angle, rayLenght, rayAngle;
        const double raySpread = M_PI*2;
        const double halfRaySpread = raySpread/2;

};

Rays::Rays(int newRayAmount, int newRaylenght) {
    rayAmount = newRayAmount;
    rayAmountInt = newRayAmount;
    rayLenght = newRaylenght;
}

Rays::~Rays() {

}

double Rays::lerp(double A, double B, double t) {
    double value = A + (B-A) *t;
    return value;
}

Vector4 Rays::getIntersection(Vector2 A, Vector2 B, Vector2 C, Vector2 D) {
    double tTop = (D.x-C.x) * (A.y-C.y) - (D.y-C.y) * (A.x-C.x);
    double uTop = (C.y-A.y) * (A.x-B.x) - (C.x-A.x) * (A.y-B.y);
    double bottom = (D.y-C.y) * (B.x-A.x) - (D.x-C.x) * (B.y-A.y);
    double retVal[3];
    Vector4 test;
    test.x = 0;
    test.y = 0;
    test.z = 0;
    test.w = 0;
    if (bottom != 0) {
        double t = tTop/bottom;
        double u = uTop/bottom;
        if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
            test.x = lerp(A.x, B.x, t);
            test.y = lerp(A.y, B.y, t);
            test.z = t;
            test.w = 5;
            return test;
        }
    }
    return test;
}

bool Rays::calcRayHits(int endRayLoc, Map* map) {
    std::vector<Vector3> touches;
    Vector2 endRay = eindRay.at(endRayLoc);

    for (int i=0; i < map->arraySize; i++) {
        Vector4 test;
        if (i < map->arraySize-1) {
            test = getIntersection(startRay, endRay, map->wallVectorVec[i], map->wallVectorVec[i+1]);
        } else {
            test = getIntersection(startRay, endRay, map->wallVectorVec[i], map->wallVectorVec[0]);
        }

        if (test.w == 5) {
            Vector3 valueVec;
            valueVec.x = test.x;
            valueVec.y = test.y;
            valueVec.z = test.z;
            touches.push_back(valueVec); 
        }

    }
    for (int i=0; i < map->outerSize; i++) {
        Vector4 test;
        if (i < map->outerSize-1) {
            test = getIntersection(startRay, endRay, map->outerWall[i], map->outerWall[i+1]);
        } else {
            test = getIntersection(startRay, endRay, map->outerWall[i], map->outerWall[0]);
        }

        if (test.w == 5) {
            Vector3 valueVec;
            valueVec.x = test.x;
            valueVec.y = test.y;
            valueVec.z = test.z;
            touches.push_back(valueVec); 
        }

    }
    if (touches.size() > 0) {
        double minOffset = touches.at(0).z;
        int minOffsetLocation = 0;
        for (int i=0; i < touches.size(); i++) {
            if (minOffset > touches.at(i).z) {
                minOffset = touches.at(i).z;
                minOffsetLocation = i;
            }

        }
        hitCoordVec = touches.at(minOffsetLocation);
        return true;
    }

    return false;
}

void Rays::draw() {
    for (int i=0; i < rayAmount; i++) {
        Vector2 start = {startRay.x, startRay.y};
        if (hitCoordVec3.at(i).x == 0 && hitCoordVec3.at(i).y == 0 && hitCoordVec3.at(i).z == 0) {
            Vector2 eind = {eindRay.at(i).x, eindRay.at(i).y};
            DrawLineV(start, eind, WHITE);
        } else {
            Vector2 eind = {hitCoordVec3.at(i).x, hitCoordVec3.at(i).y};
            DrawLineV(start, eind, RED);
        }
    }
}

void Rays::update(float* x, float* y, double newAngle, Map* map) {
    eindRay = {};
    hitCoordVec3 = {};
    angle = newAngle;

    startRay.x = *x;
    startRay.y = *y;

    castRays();
    for (int i=0; i < rayAmount; i++) {
        if (calcRayHits(i, map)) {
            hitCoordVec3.push_back(hitCoordVec);
        } else {
            hitCoordVec3.push_back({});
        }
    }
}

void Rays::castRays() {
    for (int i=0; i < rayAmount; i++) {
        double value;
        if (i != 0) {
            value = i/float(rayAmount);
        } else {
            value = 0;
        }

        double rayAngle = lerp(halfRaySpread, -halfRaySpread, value);
        rayAngle += angle;

        float eindx = (startRay.x  - sin(rayAngle) * rayLenght);
        float eindy = (startRay.y - cos(rayAngle) * rayLenght);

        Vector2 newEindRay = {eindx, eindy};

        eindRay.push_back(newEindRay);
    }
}
