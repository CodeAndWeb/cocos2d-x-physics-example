#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

#include "PhysicsShapeCache.h"

class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(HelloWorld);
    
private:
    void spawnSprite(const std::string &name, Vec2 pos);
    bool onTouchesBegan(cocos2d::Touch *touch, cocos2d::Event *unused_event);

    PhysicsShapeCache *shapeCache;
};

#endif // __HELLOWORLD_SCENE_H__
