#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <list>
#include <string>

class Actor;
class Player;
class Level;

class StudentWorld : public GameWorld
{
public:
	virtual ~StudentWorld();
	int init();
	int move();
	void cleanUp();

	//Mutator
	void addActor(Actor* actor);
	void completeLevel();
	void incNumSprayers();
	void decNumSprayers();

	//Accessor
	template<typename Type>
	bool contains(int x, int y) const{
		for (list<Actor*>::const_iterator it = m_actorList.begin(); it != m_actorList.end(); it++){
			if (dynamic_cast<Type*>(*it) && (*it)->getX() == x && (*it)->getY() == y)
				return true;
		}
		return false;
	}
	int getNumSprayers() const;
	std::list<Actor*>& getActors();
	Player* getPlayer() const;
	Level* getCurrentLevel() const;
	
private:
	int setMap(int levelNumber);
	std::string toStrFileName(int levelNumber);
	void removeDead();
	void exposeExit();
	void setDisplayText();

	std::list<Actor*> m_actorList;
	Player* m_player;
	Level* m_level;
	bool m_levelCompleted;
	bool m_exitRevealed;
	int m_numSprayers;
	int m_bonus;
};

#endif // STUDENTWORLD_H_
