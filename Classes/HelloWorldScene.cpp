#include "HelloWorldScene.h"
#include "PhysicsShapeCache.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
    scene->getPhysicsWorld()->setGravity(Vec2(0, -900));
    //scene->getPhysicsWorld()->setDebugDrawMask(0xffff);
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}


void HelloWorld::spawnSprite(const std::string &name, Vec2 pos)
{
    auto sprite = Sprite::create(name);
    shapeCache->setBodyOnSprite(name, sprite);
    sprite->setPosition(pos);
    addChild(sprite);
}


bool HelloWorld::init()
{
    if (!Layer::init())
    {
        return false;
    }
    auto center = Vec2(Director::getInstance()->getWinSize()) / 2 +
                  Director::getInstance()->getVisibleOrigin();
    
    // Load shapes
    shapeCache = PhysicsShapeCache::getInstance();
    shapeCache->addShapesWithFile("Shapes.plist");
    
    // Load background image
    Sprite *background = Sprite::create("background.png");
    background->setPosition(center);
    addChild(background);

    // Add ground sprite and drop a banana
    spawnSprite("ground.png", center);
    spawnSprite("banana.png", center);
    
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchesBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}


bool HelloWorld::onTouchesBegan(Touch *touch, Event *event)
{
    auto touchLoc = touch->getLocation();
    
    static int i = 0;
    std::string sprites[] = { "banana.png", "cherries.png", "crate.png", "orange.png" };
    
    spawnSprite(sprites[i], touchLoc);
    i = (i + 1) % (sizeof(sprites)/sizeof(sprites[0]));
    
    return false;
}
