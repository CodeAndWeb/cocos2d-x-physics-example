//
//  PhysicsShapeCache.cpp
//
#include "PhysicsShapeCache.h"
#include "CCPhysicsHelper.h"

using namespace cocos2d;
static PhysicsShapeCache *_instance = nullptr;

static float area(cocos2d::Point *vertices, int numVertices)
{
    float area = 0.0f;
    int r = (numVertices - 1);
    area += vertices[0].x * vertices[r].y - vertices[r].x * vertices[0].y;
    for (int i = 0; i < numVertices - 1; ++i)
    {
        area += vertices[r - i].x * vertices[r - (i + 1)].y - vertices[r - (i + 1)].x * vertices[r - i].y;
    }
    area *= .5f;
    return area;
}


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
        const ValueVector &fixtureList = bodyData.at("fixtures").asValueVector();
        float totalMass = 0.0f;
        float totalBodyMomentum = 0.0f;
        for (auto &fixtureitem : fixtureList)
        {
            FixtureData *fd = new FixtureData();
            bodyDef->fixtures.pushBack(fd);
            auto &fixturedata = fixtureitem.asValueMap();
            fd->friction = fixturedata.at("friction").asFloat();
            fd->elasticity = fixturedata.at("elasticity").asFloat();
            fd->mass = fixturedata.at("mass").asFloat();
            fd->group = fixturedata.at("group").asInt();
            fd->categoryMask = fixturedata.at("category_mask").asInt();
            fd->collisionMask = fixturedata.at("collision_mask").asInt();
            fd->isSensor = fixturedata.at("is_sensor").asBool();
            std::string fixtureType = fixturedata.at("fixture_type").asString();
            float totalArea = 0.0f;
            totalMass += fd->mass;
            if (strcmp("POLYGON", fixtureType.c_str()) == 0)
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
                        vertices[vindex].x = offsex.x;
                        vertices[vindex].y = offsex.y;
                        vindex++;
                    }
                    poly->area = area(vertices, poly->numVertices);
                    totalArea += poly->area;
                }
            }
            else if (strcmp("CIRCLE", fixtureType.c_str()) == 0)
            {
                fd->fixtureType = FIXTURE_CIRCLE;
                const ValueMap &circleData = fixturedata.at("circle").asValueMap();
                fd->radius = circleData.at("radius").asFloat();
                fd->center = PointFromString(circleData.at("position").asString());
                totalArea += 3.1415927 * fd->radius * fd->radius;
            }
            else // todo polyline
            {
                // unknown type
                return false;
            }
            fd->area = totalArea;
            // update sub polygon's masses and momentum
            cpFloat totalFixtureMomentum = 0.0f;
            if (totalArea)
            {
                if (fd->fixtureType == FIXTURE_CIRCLE)
                {
                    totalFixtureMomentum += cpMomentForCircle(PhysicsHelper::float2cpfloat(fd->mass), PhysicsHelper::float2cpfloat(fd->radius), PhysicsHelper::float2cpfloat(fd->radius), PhysicsHelper::point2cpv(fd->center));
                }
                else
                {
                    for (auto *p : fd->polygons)
                    {
                        // update mass
                        p->mass = (p->area * fd->mass) / fd->area;
                        cpVect *cpvs = new cpVect[p->numVertices];
                        // calculate momentum
                        p->momentum = cpMomentForPoly(PhysicsHelper::float2cpfloat(p->mass), p->numVertices, PhysicsHelper::points2cpvs(p->vertices, cpvs, p->numVertices), PhysicsHelper::point2cpv(Point::ZERO));
                        delete[] cpvs;
                        // calculate total momentum
                        totalFixtureMomentum += p->momentum;
                    }
                }
            }
            fd->momentum = PhysicsHelper::cpfloat2float(totalFixtureMomentum);
            totalBodyMomentum = PhysicsHelper::cpfloat2float(totalFixtureMomentum);
        }
        // set bodies total mass
        bodyDef->mass = totalMass;
        bodyDef->momentum = totalBodyMomentum;
    }
    return true;
}


PhysicsBody *PhysicsShapeCache::createBodyWithName(const std::string &name)
{
    BodyDef *bd = bodyDefs.at(name);
    if (!bd)
    {
        bd = bodyDefs.at(name.substr(0, name.rfind('.'))); // try without file suffix
    }
    if (!bd)
    {
        return 0; // body not found
    }
    PhysicsBody *body = PhysicsBody::create(bd->mass, bd->momentum);
    body->setGravityEnable(bd->affectedByGravity);
    body->setDynamic(bd->isDynamic);
    body->setRotationEnable(bd->allowsRotation);
    
    for (auto fd : bd->fixtures)
    {
        if (fd->fixtureType == FIXTURE_CIRCLE)
        {
            auto shape = PhysicsShapeCircle::create(fd->radius, PhysicsMaterial(0.0f, fd->elasticity, fd->friction), fd->center);
            shape->setGroup(fd->group);
            shape->setCategoryBitmask(fd->categoryMask);
            shape->setCollisionBitmask(fd->collisionMask);
            body->addShape(shape);
        }
        else if (fd->fixtureType == FIXTURE_POLYGON)
        {
            for (auto polygon : fd->polygons)
            {
                auto shape = PhysicsShapePolygon::create(polygon->vertices, polygon->numVertices, PhysicsMaterial(0.0f, fd->elasticity, fd->friction), fd->center);
                shape->setGroup(fd->group);
                shape->setCategoryBitmask(fd->categoryMask);
                shape->setCollisionBitmask(fd->collisionMask);
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
