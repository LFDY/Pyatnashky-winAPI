#include <fstream>
#include <tchar.h>
#include "atlstr.h"
#include "player.h"
#pragma warning(disable:4996)

Player::Player(){
	name = new TCHAR[100];
	wcscpy(this->name, L"Player");
	record = 0;
}
Player::Player(TCHAR *name){
	this->name = new TCHAR[100];
	wcscpy(this->name, name);
	record = 0;
}
Player::Player(const Player &x) {
	this->name = new TCHAR[100];
	wcscpy(this->name, x.name);
	this->record = x.record;
}
Player::~Player(){
	delete[] name;
}
TCHAR* Player::getName(){
	return name;
}
int Player::getRecord(){
	return record;
}
void Player::setName(TCHAR *name){
	wcscpy(this->name, name);
}
void Player::setRecord(int r){
	this->record = r;
}
void Player::recordToDisk(Player *arr, int size){
	std::ofstream fout("records.bin");
	for (int i = 0; i < size; ++i){
		if (i >= 10) break;
		else{
			fout << CW2A(arr[i].name) << std::endl<< arr[i].record;
		}
	}
	fout.close();
}
Player* Player::readFromDisk(){
	Player *arr = new Player[10];
	std::ifstream fin("records.bin");
	int i = 0;
	char *tmp = new char[100];
	while (fin.getline(tmp, 100)){
		wcscpy(arr[i].name, CA2T(tmp));
		fin >> arr[i].record;
		++i;
	}
	if (i < 10) {
		wcscpy(arr[i].name, L"__EOFflag");
		arr[i].record = 0;
	}
	fin.close();
	return arr;
}

Player* Player::start(){
	return readFromDisk();
}

void Player::stop(Player *arr, int size){
	recordToDisk(arr, size);
}

void Player::clearRecords(){
	recordToDisk(NULL, 0);
}