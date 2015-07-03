//
// PhysicsShapeCache.h
//

#ifndef __PhysicsShapeCache_h__
#define __PhysicsShapeCache_h__
#include "cocos2d.h"

USING_NS_CC;
class BodyDef;
class FixtureData;


class PhysicsShapeCache
{
  public:
    static PhysicsShapeCache *getInstance();
    static void destroyInstance();

    bool addShapesWithFile(const std::string &plist);
    bool addShapesWithFile(const std::string &plist, float scaleFactor);
    bool removeShapesWithFile(const std::string &plist);
    bool removeAllShapes();

    PhysicsBody *createBodyWithName(const std::string &name);
    bool setBodyOnSprite(const std::string &name, Sprite *sprite);

private:
    PhysicsShapeCache();
    ~PhysicsShapeCache();
    bool safeDeleteBodyDef(BodyDef *bodyDef);
    BodyDef *getBodyDef(const std::string &name);
    void setBodyProperties(PhysicsBody *body, BodyDef *bd);
    void setShapeProperties(PhysicsShape *shape, FixtureData *fd);

    Map<std::string, BodyDef *> bodyDefs;
};


typedef enum
{
    FIXTURE_POLYGON,
    FIXTURE_CIRCLE
} FixtureType;


class Polygon : public Ref
{
  public:
    Point* vertices;
    int numVertices;
};


class FixtureData : public Ref
{
  public:
    FixtureType fixtureType;
    
    float density;
    float restitution;
    float friction;
    
    int tag;
    int group;
    int categoryMask;
    int collisionMask;
    int contactTestMask;

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
    
    float linearDamping;
    float angularDamping;
    float velocityLimit;
    float angularVelocityLimit;
};


#endif // __PhysicsShapeCache_h
