#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"

class Level;
class Player;
class StudentWorld;

//=================================================
//Some useful constants
const int NUM_MAX_SPRAYERS_ALLOWED = 2;
const int DEFAULT_GOODIE_POINTS = 1000;
const int SIMPLE_ZUMI_SCORE = 100;
const int COMPLEX_ZUMI_SCORE = 500;
const int BUGSPRAYER_TICK = 40;
const int BUGSPRAY_TICK = 3;

//=================================================
//Basic Actor class
class Actor : public GraphObject{
public:
	Actor(int graphID, int x, int y, StudentWorld* world);
	virtual ~Actor() = 0{ };
	virtual void doSomething() = 0;
	virtual void setDead();
	bool isAlive() const;

protected:
	bool tryMoveIn(int dir);
	virtual bool canMove(int x, int y) const;
	bool samePos(const Actor* other) const;
	StudentWorld* getWorld() const;
	Level* getLevel() const;
	Player* getPlayer() const;

private:
	StudentWorld* m_world;
	bool m_alive;
};

//=================================================
//Brick classes
class Brick : public Actor{
public:
	Brick(int graphID, int x, int y, StudentWorld* world);
	virtual ~Brick() = 0 {};
	void doSomething();
};

class PermaBrick : public Brick{
public:
	PermaBrick(int x, int y, StudentWorld* world);
};

class DestroyableBrick : public Brick{
public:
	DestroyableBrick(int x, int y, StudentWorld* world);
};

//=================================================
//Player class
class Player : public Actor{
public:
	Player(int x, int y, StudentWorld* world);
	virtual void doSomething();
	virtual void setDead();
	void setWalkThruTick(int tick);
	void setExtraSprayerTick(int tick);

protected:
	virtual bool canMove(int x, int y) const;

private:
	int m_walkThruTick;
	int m_extraSprayerTick;
	int m_maxNumSprayers;
};

//==================================================
//Exit Class
class Exit : public Actor{
public:
	Exit(int x, int y, StudentWorld* world);
	virtual void doSomething();
	void activate();

private:
	bool m_activated;
};

//==================================================
//Item Class: this will serve as the base class for objects with limited life time
class Item : public Actor{
public:
	Item(int graphID, int x, int y, StudentWorld* world);
	virtual ~Item() = 0 {};
	virtual void doSomething();
	void setLifeTime(int lifeTime);

protected:
	virtual void useItemEffect()=0;
	bool playerGotItem();

private:
	bool decLifeTime();

	int m_lifeTime;
};

//==================================================
//Bug Sprayer Class
class BugSprayer : public Item{
public:
	BugSprayer(int x, int y, StudentWorld* world);
	virtual ~BugSprayer();
	virtual void setDead();

protected:
	virtual void useItemEffect();
};

//==================================================
//Bug Spray Class
class BugSpray : public Item{
public:
	BugSpray(int x, int y, StudentWorld* world);

protected:
	virtual void useItemEffect();
};

//==================================================
//ExtraLifeGoodie Class
class ExtraLifeGoodie : public Item{
public:
	ExtraLifeGoodie(int x, int y, StudentWorld* world);

protected:
	virtual void useItemEffect();
};

//==================================================
//WalkThruGoodie Class
class WalkThruGoodie : public Item{
public:
	WalkThruGoodie(int x, int y, StudentWorld* world);

protected:
	virtual void useItemEffect();
};

//==================================================
//ExtraSprayerGoodie Class
class ExtraSprayerGoodie : public Item{
public:
	ExtraSprayerGoodie(int x, int y, StudentWorld* world);

protected:
	virtual void useItemEffect();
};

//==================================================
//Zumi Class
class Zumi : public Actor{
public:
	Zumi(int graphID, int x, int y, StudentWorld* world);
	virtual ~Zumi() = 0 {};
	virtual void doSomething();
	virtual void setDead();

protected:
	void setTickPerMove(int tick);
	virtual void act();
	virtual bool canMove(int x, int y) const;
	void setDir(int dir);

private:
	void setDir();
	bool shouldAct();

	int m_dir;
	int m_tickPerMove;
	int m_tickCount;
};

//==================================================
//SimpleZumi Class
class SimpleZumi : public Zumi{
public:
	SimpleZumi(int x, int y, StudentWorld* world);
	virtual void setDead();
};

//==================================================
//ComplexZumi Class
class ComplexZumi : public Zumi{
public:
	ComplexZumi(int x, int y, StudentWorld* world);
	virtual void setDead();

protected:
	virtual void act();

private:
	int getDistToPlayer(int sr, int sc);
	int m_smellDistance;
};

//==================================================
//Auxillary Function Declarations
int getRand(int n);

#endif // ACTOR_H_
