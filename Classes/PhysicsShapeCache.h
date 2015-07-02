

#ifndef __PhysicsShapeCache_h__
#define __PhysicsShapeCache_h__
#include <iostream>
#include "cocos2d.h"
#include <unordered_map>

NS_CC_BEGIN
class PhysicsBody;
class BodyDef;

class PhysicsShapeCache
{
  public:
    static PhysicsShapeCache *getInstance();
    static void destroyInstance();

    bool addShapesWithFile(const std::string &plist);
    bool removeShapesWithFile(const std::string &plist);
    bool removeAllShapes();

    PhysicsBody *createBodyWithName(const std::string &name);
    bool setBodyOnSprite(const std::string &name, Sprite *sprite);

private:
    PhysicsShapeCache();
    ~PhysicsShapeCache();
    bool safeDeleteBodyDef(BodyDef *bodyDef);

    
    Map<std::string, BodyDef *> bodyDefs;
};

typedef enum
{
    FIXTURE_POLYGON,
    FIXTURE_POLYLINE,
    FIXTURE_CIRCLE
} FixtureType;


class Polygon : public Ref
{
  public:
    Point* vertices;
    int numVertices;
    float area;
    float mass;
    float momentum;
};


class FixtureData : public Ref
{
  public:
    FixtureType fixtureType;
    
    float mass;
    float elasticity;
    float friction;
    
    int group;
    int categoryMask;
    int collisionMask;

    bool isSensor;
    int layers;
    float area;            
    float momentum;        
    
    // for circles
    Point center;          
    float radius;          
    
    // for polygons / polyline
    Vector<Polygon *> polygons;
};


class BodyDef : public Ref
{
public:
    Point anchorPoint;
    Vector<FixtureData *> fixtures;
    bool isDynamic;
    bool affectedByGravity;
    bool allowsRotation;
    
    float mass;
    float momentum;
};
NS_CC_END

#endif /* defined(__PhysicsShapeCache_h__) */
