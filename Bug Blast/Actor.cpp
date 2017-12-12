#include "Actor.h"
#include <queue>
#include <cstdlib>
#include "Level.h"
#include <list>
#include "StudentWorld.h"
#include <time.h>

using namespace std;

//=================================
//Implementation for Actor

//Public
Actor::Actor(int graphID, int x, int y, StudentWorld* world) : GraphObject(graphID, x, y){
	setVisible(true);
	m_alive = true;
	m_world = world;
}

void Actor::setDead(){
	m_alive = false;
}

bool Actor::isAlive() const{
	return m_alive;
}

//Protected
bool Actor::tryMoveIn(int dir){
	switch (dir){
	case (KEY_PRESS_LEFT) :
		if (canMove(getX() - 1, getY())){
			moveTo(getX() - 1, getY());
			return true;
		}
		break;
	case(KEY_PRESS_RIGHT) :
		if (canMove(getX() + 1, getY())){
			moveTo(getX() + 1, getY());
			return true;
		}
		break;
	case (KEY_PRESS_UP) :
		if (canMove(getX(), getY() + 1)){
			moveTo(getX(), getY() + 1);
			return true;
		}
		break;
	case (KEY_PRESS_DOWN) :
		if (canMove(getX(), getY() - 1)){
			moveTo(getX(), getY() - 1);
			return true;
		}
		break;
	}
	return false;
}

bool Actor::canMove(int x, int y) const{
	return x >= 0 && x < VIEW_WIDTH && y >= 0 && y < VIEW_HEIGHT;
}

bool Actor::samePos(const Actor* other) const{
	return getX() == other->getX() && getY() == other->getY();
}

StudentWorld* Actor::getWorld() const{
	return m_world;
}

Level* Actor::getLevel() const{
	return m_world->getCurrentLevel();
}

Player* Actor::getPlayer() const{
	return m_world->getPlayer();
}

//=================================
//Implementation for Bricks

//Public
Brick::Brick(int graphID, int x, int y, StudentWorld* world) : Actor(graphID, x, y, world){
}

void Brick::doSomething(){
}

//=================================
//Implementation for PermaBrick

//Public
PermaBrick::PermaBrick(int x, int y, StudentWorld* world) : Brick(IID_PERMA_BRICK, x, y, world){
}

//=================================
//Implementation for DestroyableBrick

//Public
DestroyableBrick::DestroyableBrick(int x, int y, StudentWorld* world) : Brick(IID_DESTROYABLE_BRICK, x, y, world){
}

//=================================
//Implementation for Player

//Public
Player::Player(int x, int y, StudentWorld* world) : Actor(IID_PLAYER, x, y, world){
	m_maxNumSprayers = getLevel()->getOptionValue(optionMaxBoostedSprayers);
	m_walkThruTick = 0;
	m_extraSprayerTick = 0;
};

void Player::doSomething(){
	if (!isAlive())
		return;

	if (getWorld()->contains<Zumi>(getX(), getY()) || getWorld()->contains<BugSpray>(getX(), getY()) || 
		getWorld()->contains<PermaBrick>(getX(), getY()) || getWorld()->contains<DestroyableBrick>(getX(), getY()) && m_walkThruTick <= 0){
			setDead();
			return;
	}

	if (m_walkThruTick > 0)
		m_walkThruTick--;

	if (m_extraSprayerTick > 0)
		m_extraSprayerTick--;

	int dir;
	if (getWorld()->getKey(dir)){
		if (dir == KEY_PRESS_SPACE){
			//Check the player is allowed to drop more sprayers
			int numSprayers = getWorld()->getNumSprayers();
			if (numSprayers < NUM_MAX_SPRAYERS_ALLOWED || m_extraSprayerTick>0 && numSprayers<m_maxNumSprayers)
				//Check the location is not occupied by an invalid object
				if (!getWorld()->contains<Brick>(getX(), getY()) && !getWorld()->contains<BugSprayer>(getX(), getY()))
					getWorld()->addActor(new BugSprayer(getX(), getY(), getWorld()));
		}
		else
			tryMoveIn(dir);
	}

}

void Player::setDead(){
	Actor::setDead();
	getWorld()->playSound(SOUND_PLAYER_DIE);
}

void Player::setWalkThruTick(int tick){
	m_walkThruTick = tick;
}

void Player::setExtraSprayerTick(int tick){
	m_extraSprayerTick = tick;
}

//Protected
bool Player::canMove(int x, int y) const{
	if (!Actor::canMove(x, y))
		return false;

	list<Actor*> actors = getWorld()->getActors();
	for (list<Actor*>::iterator it = actors.begin(); it != actors.end(); it++){
		if (((*it)->getX() == x && (*it)->getY() == y) && (dynamic_cast<PermaBrick*>(*it) || dynamic_cast<DestroyableBrick*>(*it) && m_walkThruTick <= 0))
			return false;
	}

	return true;
}

//=================================
//Implementation for Exit

//Public
Exit::Exit(int x, int y, StudentWorld* world) : Actor(IID_EXIT, x, y, world){
	setVisible(false);
	m_activated = false;
}

void Exit::doSomething(){
	if (samePos(getWorld()->getPlayer()) && m_activated){
		getWorld()->playSound(SOUND_FINISHED_LEVEL);
		getWorld()->completeLevel();
	}
}

void Exit::activate(){
	setVisible(true);
	m_activated = true;
	getWorld()->playSound(SOUND_REVEAL_EXIT);
}

//=================================
//Implementation for Item

//Public
Item::Item(int graphID, int x, int y, StudentWorld* world) : Actor(graphID, x, y, world){
}

void Item::doSomething(){
	if (!isAlive())
		return;

	if (decLifeTime())
		setDead();
	else
		useItemEffect();
}

void Item::setLifeTime(int lifeTime){
	m_lifeTime = lifeTime;
}

//Protected
bool Item::playerGotItem(){
	if (samePos(getWorld()->getPlayer())){
		getWorld()->playSound(SOUND_GOT_GOODIE);
		getWorld()->increaseScore(DEFAULT_GOODIE_POINTS);
		setDead();
		setLifeTime(0);
		return true;
	}
	return false;
}

//Private
bool Item::decLifeTime(){
	if (m_lifeTime > 0)
		m_lifeTime--;
	return m_lifeTime <= 0;
}

//=================================
//Implementation for BugSprayer

//Public
BugSprayer::BugSprayer(int x, int y, StudentWorld* world) : Item(IID_BUGSPRAYER, x, y, world){
	world->incNumSprayers();
	setLifeTime(BUGSPRAYER_TICK);
}

BugSprayer::~BugSprayer(){
	getWorld()->decNumSprayers();
}

void BugSprayer::setDead(){
	Actor::setDead();

	getWorld()->playSound(SOUND_SPRAY);

	//Add the sprays
	//current location
	getWorld()->addActor(new BugSpray(getX(), getY(), getWorld()));

	bool goNegDir = true, goPosDir = true;
	//Horizaontal
	for (int i = 1; i <= 2; i++){
		if (goPosDir && !getWorld()->contains<PermaBrick>(getX() + i, getY()))
			getWorld()->addActor(new BugSpray(getX() + i, getY(), getWorld()));
		if (getWorld()->contains<Brick>(getX() + i, getY()))
			goPosDir = false;

		if (goNegDir && !getWorld()->contains<PermaBrick>(getX() - i, getY()))
			getWorld()->addActor(new BugSpray(getX() - i, getY(), getWorld()));
		if (getWorld()->contains<Brick>(getX() - i, getY()))
			goNegDir = false;
	}

	//Vertical
	goNegDir = true, goPosDir = true;
	for (int i = 1; i <= 2; i++){
		if (goPosDir && !getWorld()->contains<PermaBrick>(getX(), getY() + i))
			getWorld()->addActor(new BugSpray(getX(), getY() + i, getWorld()));
		if (getWorld()->contains<Brick>(getX(), getY() + i))
			goPosDir = false;

		if (goNegDir && !getWorld()->contains<PermaBrick>(getX(), getY() - i))
			getWorld()->addActor(new BugSpray(getX(), getY() - i, getWorld()));
		if (getWorld()->contains<Brick>(getX(), getY() - i))
			goNegDir = false;
	}
}

//Protected
void BugSprayer::useItemEffect(){
}

//=================================
//Implementation for BugSpray

//Public
BugSpray::BugSpray(int x, int y, StudentWorld* world) : Item(IID_BUGSPRAY, x, y, world){
	setLifeTime(BUGSPRAY_TICK);
}

//Protected
void BugSpray::useItemEffect(){
	//Check if NPCs should be killed
	list<Actor*>* actors = &(getWorld()->getActors());
	for (list<Actor*>::iterator it = actors->begin(); it != actors->end(); it++){
		if (samePos(*it)){
			if (dynamic_cast<DestroyableBrick*>(*it) || dynamic_cast<Zumi*>(*it))
				(*it)->setDead();
			else if (dynamic_cast<BugSprayer*>(*it))
				dynamic_cast<BugSprayer*>(*it)->setLifeTime(0);
		}
	}
	//Check if the PC should be killed
	Player* player = getWorld()->getPlayer();
	if (samePos(player))
		player->setDead();
}

//=================================
//Implementation for ExtraLifeGoodie

//Public
ExtraLifeGoodie::ExtraLifeGoodie(int x, int y, StudentWorld* world) : Item(IID_EXTRA_LIFE_GOODIE, x, y, world){
	setLifeTime(getLevel()->getOptionValue(optionGoodieLifetimeInTicks));
}

//Protected
void ExtraLifeGoodie::useItemEffect(){
	if (playerGotItem()){
		getWorld()->incLives();
		setDead();
	}
}

//=================================
//Implementation for WalkThruGoodie
WalkThruGoodie::WalkThruGoodie(int x, int y, StudentWorld* world) : Item(IID_WALK_THRU_GOODIE, x, y, world){
	setLifeTime(getLevel()->getOptionValue(optionGoodieLifetimeInTicks));
}

void WalkThruGoodie::useItemEffect(){
	if (playerGotItem()){
		getWorld()->getPlayer()->setWalkThruTick(getLevel()->getOptionValue(optionWalkThruLifetimeTicks));
		setDead();
	}
}

//=================================
//Implementation for ExtraSprayerGoodie
ExtraSprayerGoodie::ExtraSprayerGoodie(int x, int y, StudentWorld* world) : Item(IID_INCREASE_SIMULTANEOUS_SPRAYER_GOODIE, x, y, world){
	setLifeTime(getLevel()->getOptionValue(optionGoodieLifetimeInTicks));
}

void ExtraSprayerGoodie::useItemEffect(){
	if (playerGotItem()){
		getWorld()->getPlayer()->setExtraSprayerTick(getLevel()->getOptionValue(optionBoostedSprayerLifetimeTicks));
		setDead();
	}
}

//=================================
//Implementation for Zumi

//Public
Zumi::Zumi(int graphID, int x, int y, StudentWorld* world) : Actor(graphID, x, y, world){
	m_tickCount = 0;
	setTickPerMove(getLevel()->getOptionValue(optionTicksPerSimpleZumiMove));
	setDir();
}

void Zumi::doSomething(){
	if (!isAlive())
		return;

	Player* player = getWorld()->getPlayer();
	if (samePos(player)){
		getWorld()->playSound(SOUND_PLAYER_DIE);
		player->setDead();
	}

	if (shouldAct()){
		//If we should act, we use the current Zumi Type's act
		act();
	}
}

void Zumi::setDead(){
	Actor::setDead();
	
	//Play the sound
	getWorld()->playSound(SOUND_ENEMY_DIE);

	//Decide whether we should drop a goodie
	if (getRand(100) < (int)(getLevel()->getOptionValue(optionProbOfGoodieOverall))){
		//Decide which type to drop
		int probExtraLife = getLevel()->getOptionValue(optionProbOfExtraLifeGoodie);
		int probExtraSprayer = getLevel()->getOptionValue(optionProbOfMoreSprayersGoodie);
		int probWalkthru = getLevel()->getOptionValue(optionProbOfWalkThruGoodie);
		int rand = getRand(probExtraLife + probExtraSprayer + probWalkthru);

		//Life Goodie
		if (rand < probExtraLife)
			getWorld()->addActor(new ExtraLifeGoodie(getX(), getY(), getWorld()));
		//Extra Sprayer Goodie
		else if (probExtraLife <= rand && rand < probExtraLife + probExtraSprayer)
			getWorld()->addActor(new ExtraSprayerGoodie(getX(), getY(), getWorld()));
		//Walk Thru Goodie
		else if (probExtraLife + probExtraSprayer <= rand && rand < probExtraLife + probExtraSprayer + probWalkthru)
			getWorld()->addActor(new WalkThruGoodie(getX(), getY(), getWorld()));

	}
}

//Protected
void Zumi::setTickPerMove(int tick){
	m_tickPerMove = tick;
}

void Zumi::act(){
	//If we fail to move, that means we need to change direction
	if (!tryMoveIn(m_dir)){
		setDir();
	}
}

bool Zumi::canMove(int x, int y) const{
	return Actor::canMove(x, y) && !getWorld()->contains<Brick>(x, y) && !getWorld()->contains<BugSprayer>(x, y);
}

void Zumi::setDir(int dir){
	m_dir = dir;
}

//Private
void Zumi::setDir(){
	int dirs[] = { KEY_PRESS_LEFT, KEY_PRESS_RIGHT, KEY_PRESS_DOWN, KEY_PRESS_UP };
	m_dir = dirs[getRand(4)];
}

bool Zumi::shouldAct(){
	m_tickCount = (m_tickCount + 1) % m_tickPerMove;
	return m_tickCount == 0;
}

//=================================
//Implementation for SimpleZumi

//Public
SimpleZumi::SimpleZumi(int x, int y, StudentWorld* world) : Zumi(IID_SIMPLE_ZUMI, x, y, world){
}

void SimpleZumi::setDead(){
	Zumi::setDead();
	getWorld()->increaseScore(SIMPLE_ZUMI_SCORE);
}

//=================================
//Implementation for ComplexZumi

//Public
ComplexZumi::ComplexZumi(int x, int y, StudentWorld* world) : Zumi(IID_COMPLEX_ZUMI, x, y, world){
	setTickPerMove(getLevel()->getOptionValue(optionTicksPerComplexZumiMove));
	m_smellDistance = getLevel()->getOptionValue(optionComplexZumiSearchDistance);
}

//Protected
void ComplexZumi::setDead(){
	Zumi::setDead();
	getWorld()->increaseScore(COMPLEX_ZUMI_SCORE);
}

void ComplexZumi::act(){
	int horizDist = abs(getWorld()->getPlayer()->getX() - getX());
	int vertiDist = abs(getWorld()->getPlayer()->getY() - getY());

	if (horizDist <= m_smellDistance && vertiDist <= m_smellDistance){
		//Search for the fastest route to the player
		int shortestDist = INT_MAX, dist = 0, dir;
		//Up
		dist = getDistToPlayer(getX(), getY() + 1);
		if (shortestDist > dist){
			shortestDist = dist;
			dir = KEY_PRESS_UP;
		}
		//Down
		dist = getDistToPlayer(getX(), getY() - 1);
		if (shortestDist > dist){
			shortestDist = dist;
			dir = KEY_PRESS_DOWN;
		}
		//Left
		dist = getDistToPlayer(getX() - 1, getY());
		if (shortestDist > dist){
			shortestDist = dist;
			dir = KEY_PRESS_LEFT;
		}
		//RIGHT
		dist = getDistToPlayer(getX() + 1, getY());
		if (shortestDist > dist){
			shortestDist = dist;
			dir = KEY_PRESS_RIGHT;
		}
		//Check if the path exist. If so, move in the shortest direction
		if (shortestDist != INT_MAX){
			tryMoveIn(dir);
			setDir(dir);
			return;
		}
	}
	//If the path does't exsit or the player is not within the smell distance, then act like a regular Zumi
	Zumi::act();
}

//Private
int ComplexZumi::getDistToPlayer(int sx, int sy){
	//We use a Coord class to simplify our calculation
	class Coord
	{
	public:
		Coord(int x, int y) : m_x(x), m_y(y) {}
		bool operator==(const Coord& other){
			return other.m_x == m_x && other.m_y == m_y;
		}
		int getX() { return m_x; }
		int getY() { return m_y; }
	private:
		int m_x;
		int m_y;
	};

	//We also need a variable to keep track of these distance
	int dist = 1;
	//This keep track of the coords
	queue<Coord> coords;
	//This will help us avoid over counting
	queue<int> branchCounts;

	//Get the maze
	string maze[VIEW_WIDTH], temp;
	for (int j = 0; j < VIEW_HEIGHT; j++)
		temp += ".";
	for (int i = 0; i < VIEW_WIDTH; i++)	
		maze[i] = temp;
	
	list<Actor*> * actors = &(getWorld()->getActors());
	for (list<Actor*>::iterator it = actors->begin(); it != actors->end(); it++){
		if (dynamic_cast<Brick*>(*it) || dynamic_cast<BugSprayer*>(*it))
			maze[(*it)->getX()][(*it)->getY()] = '#';
	}

	//Get where the player is
	Coord playerLoc = Coord(getWorld()->getPlayer()->getX(), getWorld()->getPlayer()->getY());

	//Get the initial coord in and check the initial coordinate is valid
	if (maze[sx][sy] != '.')
		return INT_MAX;
	coords.push(Coord(sx, sy));
	branchCounts.push(1);
	maze[sx][sy] = '#';

	while (!coords.empty()){
		//Get the number of branches
		int numBranch = branchCounts.front();
		branchCounts.pop();

		int branchCount = 0;

		//We need to traverse through all the branches 
		for (int i = 0; i < numBranch; i++){
			//Get the stuff out of the queue
			Coord currCoord = coords.front();
			coords.pop();

			//If we have reached the player, return the distance
			if (currCoord == playerLoc)
				return dist;

			//Add more coords into the queue
			//North
			if (currCoord.getY() + 1 < VIEW_HEIGHT && maze[currCoord.getX()][currCoord.getY() + 1] == '.'){
				coords.push(Coord(currCoord.getX(), currCoord.getY() + 1));
				maze[currCoord.getX()][currCoord.getY() + 1] = '&';
				branchCount++;
			}
			//South
			if (currCoord.getY() - 1 >= 0 && maze[currCoord.getX()][currCoord.getY() - 1] == '.'){
				coords.push(Coord(currCoord.getX(), currCoord.getY() - 1));
				maze[currCoord.getX()][currCoord.getY() - 1] = '&';
				branchCount++;
			}
			//West
			if (currCoord.getX() - 1 >= 0 && maze[currCoord.getX()-1][currCoord.getY()] == '.'){
				coords.push(Coord(currCoord.getX() - 1, currCoord.getY()));
				maze[currCoord.getX() - 1][currCoord.getY()] = '&';
				branchCount++;
			}
			//East
			if (currCoord.getX() + 1 < VIEW_WIDTH && maze[currCoord.getX() + 1][currCoord.getY()] == '.'){
				coords.push(Coord(currCoord.getX() + 1, currCoord.getY()));
				maze[currCoord.getX() + 1][currCoord.getY()] = '&';
				branchCount++;
			}
		}
		//Incrase the depth by 1
		dist++;
		branchCounts.push(branchCount);
	}
	return INT_MAX;
}

//=================================
//Implementation for Auxillary functions
int getRand(int n){
	return rand() % n;
}
