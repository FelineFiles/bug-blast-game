#include "StudentWorld.h"
#include "Actor.h"
#include "GameConstants.h"
#include <iomanip>
#include "level.h"
#include <list>
#include <sstream>
#include <string>

using namespace std;

GameWorld* createStudentWorld()
{
	return new StudentWorld();
}

//==========================================
//Public Functions

//Inherited
StudentWorld::~StudentWorld(){
	//The same as cleanUp()
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end(); it++){
		delete *it;
	}
	m_actorList.clear();

	delete m_player;
	m_player = nullptr;

	delete m_level;
	m_level = nullptr;
}

int StudentWorld::init()
{
	srand((unsigned int)time(0));
	m_level = new Level;
	m_numSprayers = 0;
	m_levelCompleted = false;
	m_exitRevealed = false;
	return setMap(getLevel());
}

int StudentWorld::move()
{
	// Update the Game Status Line 
	setDisplayText(); 

	//Let the player move first
	if (m_player->isAlive())
		m_player->doSomething();

	//Then the other actors
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end(); it++){
		//Let each actor act
		if ((*it)->isAlive())
			(*it)->doSomething();
		//If the player is dead, return the message
		if (!m_player->isAlive()){
			decLives();
			return GWSTATUS_PLAYER_DIED;
		}
		//If the player has completed the level, act accordingly
		if (m_levelCompleted){
			increaseScore(m_bonus);
			return GWSTATUS_FINISHED_LEVEL;
		}
	}
	
	removeDead();

	if (m_bonus>0)
		m_bonus--;
	
	//Check if all bugs are dead
	bool allZumiDead = true;
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end(); it++){
		//Looking for any living bug
		if (dynamic_cast<Zumi*>(*it) && (*it)->isAlive()){
			allZumiDead = false;
			break;
		}
	}
	if (allZumiDead && !m_exitRevealed)
		exposeExit();

	if (!m_player->isAlive()){
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}

	//If the player has won, act accordingly
	if (m_levelCompleted){
		increaseScore(m_bonus);
		return GWSTATUS_FINISHED_LEVEL;
	}

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end(); it++){
		delete *it;
	}
	m_actorList.clear();

	delete m_player;
	m_player = nullptr;

	delete m_level;
	m_level = nullptr;
}

//Mutator
void StudentWorld::addActor(Actor* actor){
	m_actorList.push_front(actor);
}

void StudentWorld::completeLevel(){
	m_levelCompleted = true;
}

void StudentWorld::incNumSprayers(){
	m_numSprayers++;
}

void StudentWorld::decNumSprayers(){
	m_numSprayers--;
}

//Accessor
int StudentWorld::getNumSprayers() const{
	return m_numSprayers;
}

list<Actor*>& StudentWorld::getActors(){
	return m_actorList;
}

Player* StudentWorld::getPlayer() const{
	return m_player;
}

Level* StudentWorld::getCurrentLevel() const{
	return m_level;
}

//==========================================
//Private Functions

int StudentWorld::setMap(int levelNumber){
	string currentLevelName = toStrFileName(levelNumber);
	Level::LoadResult result = m_level->loadLevel(currentLevelName);

	if (result == Level::load_fail_bad_format)
		return GWSTATUS_LEVEL_ERROR;
	else if (levelNumber == 0 && result == Level::load_fail_file_not_found)
		return GWSTATUS_NO_FIRST_LEVEL;
	else if (result == Level::load_fail_file_not_found)
		return GWSTATUS_PLAYER_WON;

	//Add actors into the world
	for (int i = 0; i < VIEW_WIDTH; i++){
		for (int j = 0; j < VIEW_WIDTH; j++){
			Level::MazeEntry entry = m_level->getContentsOf(i, j);
			switch(entry){
			case (Level::player):
				m_player = new Player(i, j, this);
				break;
			case (Level::perma_brick):
				addActor(new PermaBrick(i, j, this));
				break;
			case (Level::destroyable_brick):
				addActor(new DestroyableBrick(i, j, this));
				break;
			case (Level::exit) :
				addActor(new Exit(i, j, this));
				break;
			case (Level::simple_zumi):
				addActor(new SimpleZumi(i, j, this));
				break;
			case (Level::complex_zumi) :
				addActor(new ComplexZumi(i, j, this));
				break;
			}
		}
	}

	//Get values
	m_bonus = m_level->getOptionValue(optionLevelBonus);

	return GWSTATUS_CONTINUE_GAME;
}

string StudentWorld::toStrFileName(int levelNumber){
	ostringstream oss;
	string result;
	oss.fill('0');
	oss << setw(2) << levelNumber;
	result = "level" + oss.str() + ".dat";
	return result;
}

void StudentWorld::removeDead(){
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end();){
		if (!(*it)->isAlive()){
			list<Actor*>::iterator toDelete = it;
			it++;
			delete *(toDelete);
			m_actorList.erase(toDelete);
		}
		else
			it++;
	}
}

void StudentWorld::exposeExit(){
	for (list<Actor*>::iterator it = m_actorList.begin(); it != m_actorList.end(); it++)
		if (dynamic_cast<Exit*>(*it))
			(static_cast<Exit*>(*it))->activate();
	m_exitRevealed = true;
}

void StudentWorld::setDisplayText(){
	ostringstream oss;
	oss.fill('0');
	oss << "Score: " << setw(8) << getScore() << setw(0) << "  Level: " << setw(2) << getLevel() << setw(0);
	oss << "  Lives: " << setw(3) << getLives() << setw(0) << "  Bonus: ";
	oss.fill(' ');
	oss << setw(6) << m_bonus;

	string result = oss.str();
	setGameStatText(result);

}
