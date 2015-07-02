//
//  PhysicsShapeCache.cpp
//
#include "PhysicsShapeCache.h"
#include "CCPhysicsHelper.h"

using namespace cocos2d;
static PhysicsShapeCache *_instance = nullptr;


PhysicsShapeCache::PhysicsShapeCache()
{
}


PhysicsShapeCache::~PhysicsShapeCache()
{
    removeAllShapes();
}


PhysicsShapeCache *PhysicsShapeCache::getInstance()
{
    if (!_instance)
    {
        _instance = new PhysicsShapeCache();
    }
    return _instance;
}


void PhysicsShapeCache::destroyInstance()
{
    CC_SAFE_DELETE(_instance);
}


bool PhysicsShapeCache::addShapesWithFile(const std::string &plist)
{
    ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(plist);
    if (dict.empty())
    {
        // plist file not found
        return false;
    }

    ValueMap &metadata = dict["metadata"].asValueMap();
    int format = metadata["format"].asInt();
    if (format != 1)
    {
        CCASSERT(format == 1, "format not supported!");
        return false;
    }
    float scaleFactor = dict["scale_factor"].asFloat();
    
    ValueMap &bodydict = dict.at("bodies").asValueMap();
    for (auto iter = bodydict.cbegin(); iter != bodydict.cend(); ++iter)
    {
        const ValueMap &bodyData = iter->second.asValueMap();
        std::string bodyName = iter->first;
        BodyDef *bodyDef = new BodyDef();
        bodyDefs.insert(bodyName, bodyDef);
        bodyDef->anchorPoint = PointFromString(bodyData.at("anchorpoint").asString());
        bodyDef->isDynamic = bodyData.at("is_dynamic").asBool();
        bodyDef->affectedByGravity = bodyData.at("affected_by_gravity").asBool();
        bodyDef->allowsRotation = bodyData.at("allows_rotation").asBool();
        bodyDef->linearDamping = bodyData.at("linear_damping").asFloat();
        bodyDef->angularDamping = bodyData.at("angular_damping").asFloat();

        const ValueVector &fixtureList = bodyData.at("fixtures").asValueVector();
        for (auto &fixtureitem : fixtureList)
        {
            FixtureData *fd = new FixtureData();
            bodyDef->fixtures.pushBack(fd);
            auto &fixturedata = fixtureitem.asValueMap();
            fd->density = fixturedata.at("density").asFloat();
            fd->restitution = fixturedata.at("restitution").asFloat();
            fd->friction = fixturedata.at("friction").asFloat();
            fd->tag = fixturedata.at("tag").asInt();
            fd->group = fixturedata.at("group").asInt();
            fd->categoryMask = fixturedata.at("category_mask").asInt();
            fd->collisionMask = fixturedata.at("collision_mask").asInt();
            fd->contactTestMask = fixturedata.at("contact_test_mask").asInt();
            
            std::string fixtureType = fixturedata.at("fixture_type").asString();
            if (fixtureType == "POLYGON")
            {
                const ValueVector &polygonsArray = fixturedata.at("polygons").asValueVector();
                fd->fixtureType = FIXTURE_POLYGON;
                for (auto &polygonitem : polygonsArray)
                {
                    Polygon *poly = new Polygon();
                    fd->polygons.pushBack(poly);
                    auto &polygonArray = polygonitem.asValueVector();
                    poly->numVertices = (int)polygonArray.size();
                    Point *vertices = poly->vertices = new Point[poly->numVertices];
                    int vindex = 0;
                    for (auto &pointString : polygonArray)
                    {
                        Point offsex = PointFromString(pointString.asString());
                        vertices[vindex].x = offsex.x / scaleFactor;
                        vertices[vindex].y = offsex.y / scaleFactor;
                        vindex++;
                    }
                }
            }
            else if (fixtureType == "CIRCLE")
            {
                fd->fixtureType = FIXTURE_CIRCLE;
                const ValueMap &circleData = fixturedata.at("circle").asValueMap();
                fd->radius = circleData.at("radius").asFloat() / scaleFactor;
                fd->center = PointFromString(circleData.at("position").asString()) / scaleFactor;
            }
            else
            {
                // unknown type
                return false;
            }

        }
    }
    return true;
}


PhysicsBody *PhysicsShapeCache::createBodyWithName(const std::string &name)
{
    BodyDef *bd = bodyDefs.at(name);
    if (!bd)
    {
        bd = bodyDefs.at(name.substr(0, name.rfind('.')));
    }
    if (!bd)
    {
        return 0; // body not found
    }
    PhysicsBody *body = PhysicsBody::create();
    body->setGravityEnable(bd->affectedByGravity);
    body->setDynamic(bd->isDynamic);
    body->setRotationEnable(bd->allowsRotation);
    body->setLinearDamping(bd->linearDamping);
    body->setAngularDamping(bd->angularDamping);
    
    for (auto fd : bd->fixtures)
    {
        PhysicsMaterial material(fd->density, fd->restitution, fd->friction);
        if (fd->fixtureType == FIXTURE_CIRCLE)
        {
            auto shape = PhysicsShapeCircle::create(fd->radius, material, fd->center);
            shape->setGroup(fd->group);
            shape->setCategoryBitmask(fd->categoryMask);
            shape->setCollisionBitmask(fd->collisionMask);
            shape->setContactTestBitmask(fd->contactTestMask);
            shape->setTag(fd->tag);
            body->addShape(shape);
        }
        else if (fd->fixtureType == FIXTURE_POLYGON)
        {
            for (auto polygon : fd->polygons)
            {
                auto shape = PhysicsShapePolygon::create(polygon->vertices, polygon->numVertices, material, fd->center);
                shape->setGroup(fd->group);
                shape->setCategoryBitmask(fd->categoryMask);
                shape->setCollisionBitmask(fd->collisionMask);
                shape->setContactTestBitmask(fd->contactTestMask);
                shape->setTag(fd->tag);
                body->addShape(shape);
            }
        }
    }
    return body;
}


bool PhysicsShapeCache::setBodyOnSprite(const std::string &name, Sprite *sprite)
{
    PhysicsBody *body = createBodyWithName(name);
    if (body)
    {
        sprite->setPhysicsBody(body);
        // Cocos2d-x 3.6 does not support custsom anchor points when using physics
        //sprite->setAnchorPoint(bodyDefs.at(name)->anchorPoint);
    }
    return body != 0;
}


bool PhysicsShapeCache::removeShapesWithFile(const std::string &plist)
{
    ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(plist);
    if (dict.empty())
    {
        // plist file not found
        return false;
    }
    
    ValueMap &metadata = dict["metadata"].asValueMap();
    int format = metadata["format"].asInt();
    if (format != 1)
    {
        CCASSERT(format == 1, "format not supported!");
        return false;
    }
    
    ValueMap &bodydict = dict.at("bodies").asValueMap();
    for (auto iter = bodydict.cbegin(); iter != bodydict.cend(); ++iter)
    {
        std::string bodyName = iter->first;
        BodyDef *bd = bodyDefs.at(bodyName);
        if (bd != nullptr)
        {
            safeDeleteBodyDef(bd);
            bodyDefs.erase(bodyName);
        }
       
    }
    return true;
}


bool PhysicsShapeCache::removeAllShapes()
{
    CCLOG("%s"," PEShapeCache removeAllbodys");
    for (auto iter = bodyDefs.cbegin(); iter != bodyDefs.cend(); ++iter)
    {
        safeDeleteBodyDef(iter->second);
    }
    bodyDefs.clear();
    return true;
}


bool PhysicsShapeCache::safeDeleteBodyDef(BodyDef *bodyDef)
{
    for (auto fixturedate : bodyDef->fixtures)
    {
        for (auto polygon : fixturedate->polygons)
        {
            CC_SAFE_DELETE_ARRAY(polygon->vertices);
        }
        fixturedate->polygons.clear();
    }
    bodyDef->fixtures.clear();
    return true;
}
