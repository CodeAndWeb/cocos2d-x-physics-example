#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "PhysicsShapeCache.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    // create the scene with physics enabled
    auto scene = Scene::createWithPhysics();
    
    // set gravity
    scene->getPhysicsWorld()->setGravity(Vec2(0, -500));

    // optional: set debug draw
    // scene->getPhysicsWorld()->setDebugDrawMask(0xffff);
    
    auto layer = HelloWorld::create();
    scene->addChild(layer);

    return scene;
}


bool HelloWorld::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto pos = Vec2(Director::getInstance()->getVisibleSize()) / 2 +
    Director::getInstance()->getVisibleOrigin();
    
    // Load shapes
    shapeCache = PhysicsShapeCache::getInstance();
    shapeCache->addShapesWithFile("Shapes.plist");
    
    // Load background image
    Sprite *background = Sprite::create("background.png");
    background->setPosition(pos);
    addChild(background);

    // Add ground sprite and drop a banana
    spawnSprite("ground", pos);
    spawnSprite("banana", pos);
    
    // Add touch listener
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchesBegan, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
    
    return true;
}


bool HelloWorld::onTouchesBegan(Touch *touch, Event *event)
{
    auto touchLoc = touch->getLocation();
    
    static int i = 0;
    static std::string sprites[] = { "banana", "cherries", "crate", "orange" };
    
    spawnSprite(sprites[i], touchLoc);
    i = (i + 1) % (sizeof(sprites)/sizeof(sprites[0]));
    
    return false;
}


void HelloWorld::spawnSprite(const std::string &name, Vec2 pos)
{
    // create a sprite with the given image name
    auto sprite = Sprite::create(name+".png");
    
    // attach physics body
    shapeCache->setBodyOnSprite(name, sprite);

    // set position and add it to the scene
    sprite->setPosition(pos);
    addChild(sprite);
}

