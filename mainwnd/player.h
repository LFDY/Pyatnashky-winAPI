#ifndef PLAYER_H_
#define PLAYER_H_
#include <tchar.h>

class Player{
private:
	TCHAR *name;
	int record;

	static void recordToDisk(Player *arr, int size);	//������� ������ �� ����
	static Player* readFromDisk();						//������� ���������� � �����

public:
	Player();
	Player(TCHAR *name); 
	Player(const Player &x);
	~Player();

	TCHAR* getName();
	void setName(TCHAR *str);
	
	int getRecord();
	void setRecord(int r);

	static void clearRecords();

	static Player* start();
	static void stop(Player *arr, int size);
};

#endif