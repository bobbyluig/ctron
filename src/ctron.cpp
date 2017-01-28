#include <curses.h>
#include <thread>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <set>
#include <iostream>
#include <random>
#include <iterator>
#include <map>

#define VEL 10.0000

typedef std::pair<int, int> Coordinate;
class Field;


enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

enum Block {
	EMPTY,
	WALL,
	TRON
};


class Timer {
	public:
		Timer();
		void start();
		double split();
		bool isStarted();
	private:
		std::chrono::high_resolution_clock::time_point t;
		bool started;
};

Timer::Timer() {
	this->started = false;
}

void Timer::start() {
	this->t = std::chrono::high_resolution_clock::now();
	this->started = true;
}

double Timer::split() {
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(now - this->t);
	
	return duration.count();
}

bool Timer::isStarted() {
	return this->started;
}


class Tron {
	public:
		Tron(int color, bool human, int maxWalls);
		
		void setPosition(int x, int y);
		Coordinate getPosition();
		
		void setDirection(Direction direction);
		Direction getDirection();
		
		void move();
		void think(Field* field);
		
		int getColor();
		bool isHuman();
		
		bool isAlive();
		void kill();
		
		std::vector<Coordinate> walls;
	private:
		int color;
		bool human;
		int maxWalls;
		
		int posX;
		int posY;
		Direction direction;
		bool alive;
};

Tron::Tron(int color, bool human, int maxWalls) {
	this->color = color;
	this->human = human;
	this->maxWalls = maxWalls;
	this->alive = true;
}

Direction Tron::getDirection() {
	return this->direction;
}

void Tron::think(Field* field) {
	
}

bool Tron::isHuman() {
	return this->human;
}

bool Tron::isAlive() {
	return this->alive;
}

void Tron::kill() {
	this->alive = false;
}

void Tron::setPosition(int x, int y) {
	this->posX = x;
	this->posY = y;
}

void Tron::setDirection(Direction direction) {
	this->direction = direction;
}

void Tron::move() {
	Coordinate pos = this->getPosition();
	
	if (this->walls.size() > this->maxWalls) {
		this->walls.erase(this->walls.begin());
	}
	
	this->walls.push_back(pos);
	
	switch(this->direction) {
	case Direction::LEFT:
		this->posX -= 1;
		break;
	case Direction::RIGHT:
		this->posX += 1;
		break;
	case Direction::UP:
		this->posY -= 1;
		break;
	case Direction::DOWN:
		this->posY += 1;
		break;
	}
}

Coordinate Tron::getPosition() {
	return std::make_pair(this->posX, this->posY);
}

int Tron::getColor() {
	return this->color;
}


class Field {
	public:
		Field(int minX, int minY, int maxX, int maxY);
		~Field();
		
		void addTron(Tron* tron);
		Tron* getTron(int index);
		
		void setupField();
		void move();
		void render();
		
		int getNumAlive();
		int getOptimalWalls();
	private:
		int minX;
		int minY;
		int maxX;
		int maxY;
		std::vector<Tron*> trons;
		
		std::map<Coordinate, Block> map;
		void updateMap();
		bool isColliding(Tron* tron);
};

Field::Field(int minX, int minY, int maxX, int maxY) {
	this->minX = minX;
	this->minY = minY;
	this->maxX = maxX;
	this->maxY = maxY;
	
	// Build initial map.
	for (int x = this->minX; x < this->maxX; x++) {
		for (int y = this->minY; y < this->maxY; y++) {
			this->map[std::make_pair(x, y)] = Block::EMPTY;
		}
	}
}

Field::~Field() {
	for (auto tron : this->trons) {
		delete tron;
	}
}

bool Field::isColliding(Tron* tron) {
	Coordinate pos = tron->getPosition();
	return 	(this->map[pos] != Block::EMPTY) || 
			(pos.first < this->minX) || (pos.first > this->maxX) ||
			(pos.second < this->minY) || (pos.second > this->maxY);
}

int Field::getOptimalWalls() {
	// Best is around 5% of field area.
	return std::round((this->maxX - this->minX) * (this->maxY - this->minY) * 0.05);
}

void Field::updateMap() {
	for (auto const& coordinate : this->map) {
		this->map[coordinate.first] = Block::EMPTY;
	}
	
	for (auto tron : this->trons) {
		Coordinate pos = tron->getPosition();
		this->map[pos] = Block::TRON; 
		
		for (auto wallPos : tron->walls) {
			this->map[wallPos] = Block::WALL;
		}
	}
}

void Field::move() {
	for (auto tron : this->trons) {
		if (!tron->isAlive()) {
			continue;
		}
		
		if (!tron->isHuman()) {
			tron->think(this);
		}
		
		// Get user input.
		switch (getch()) {
			case KEY_UP:
				tron->setDirection(Direction::UP);
				break;
			case KEY_DOWN:
				tron->setDirection(Direction::DOWN);
				break;
			case KEY_LEFT:
				tron->setDirection(Direction::LEFT);
				break;
			case KEY_RIGHT:
				tron->setDirection(Direction::RIGHT);
				break;
		}

		tron->move();
	}
	
	// Did anyone get wrecked?
	for (auto tron : this->trons) {
		if (!tron->isAlive()) {
			continue;
		}
		
		if (this->isColliding(tron)) {
			tron->kill();
		}
	}
	
	// Update field map.
	this->updateMap();
}

void Field::render() {
	for (int i = 0; i < this->trons.size(); i++) {
		// Get the tron.
		Tron* tron = this->trons[i];
		
		// Is alive?
		if (!tron->isAlive()) {
			continue;
		} 
		
		// Get tron position.
		Coordinate pos = tron->getPosition();
		
		// Set the color.
		attron(COLOR_PAIR(i + 1));
		
		// Render the tron.
		switch (tron->getDirection()) {
		case Direction::RIGHT:
			mvaddch(pos.second, pos.first, '>');
			break;
		case Direction::LEFT:
			mvaddch(pos.second, pos.first, '<');
			break;
		case Direction::UP:
			mvaddch(pos.second, pos.first, '^');
			break;
		case Direction::DOWN:
			mvaddch(pos.second, pos.first, 'v');
			break;
		}
		
		// Render all of the walls.
		for (auto wallPos : tron->walls) {
			if (wallPos != pos) {
				mvaddch(wallPos.second, wallPos.first, 'o');
			}	
		}
	}
}

int Field::getNumAlive() {
	int count = 0;
	
	for (auto tron : this->trons) {
		if (tron->isAlive()) {
			count += 1;
		}
	}
	
	return count;
}

void Field::addTron(Tron* tron) {
	this->trons.push_back(tron);
}

Tron* Field::getTron(int index) {
	return this->trons[index];
}

void Field::setupField() {
	int width = this->maxX - this->minX;
	int height = this->maxY - this->minY;
	
	for (int i = 0; i < this->trons.size(); i++) {
		Tron* tron = this->trons[i];
		init_pair(i + 1, tron->getColor(), COLOR_BLACK);
		
		switch (i) {
		case 0:
			tron->setDirection(Direction::RIGHT);
			tron->setPosition(this->minX, this->maxY);
			break;
		case 1:
			tron->setDirection(Direction::LEFT);
			tron->setPosition(this->maxX, this->minX);
			break;
		case 2:
			tron->setDirection(Direction::RIGHT);
			tron->setPosition(this->minX, this->minY);
			break;
		case 3:
			tron->setDirection(Direction::LEFT);
			tron->setPosition(this->maxX, this->maxY);
			break;
		}
		
	}
}


int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: ./ctron num_ai" << std::endl;
		return 1;
	}
	
	int numAI = atoi(argv[1]);
	
	if (numAI < 1) {
		std::cerr << "The minimum number of AI players is 1." << std::endl;
		return 1;
	}
	
	if (numAI > 3) {
		std::cerr << "The maximum number of AI players is 3." << std::endl;
		return 1;
	}

	// Initialize ncurses.
	initscr();
	start_color();
	curs_set(FALSE);
	
	// Setup proper keyboard input.
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	
	// Get screen size.
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	
	// Create field.
	Field field(0, 0, cols - 1, rows - 1);
	int numWalls= field.getOptimalWalls();
	
	// Create all trons and add them to the field.
	Tron* player = new Tron(COLOR_CYAN, true, numWalls);
	field.addTron(player);
	
	for (int i = 0; i < numAI; i++) {
		Tron* ai = new Tron(COLOR_YELLOW, false, numWalls);
		field.addTron(ai);
	}
	
	// Setup the field.
	field.setupField();
		
	// Create timer object.
	Timer timer;
	
	// Do an initial field render and wait 3 seconds.
	erase();
	field.render();
	refresh();
	std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	
	while (1) {
		// (Re)start timer.
		timer.start();
		
		// Move.
 		field.move();
		
		// Render.
		erase();
		field.render();
		refresh();
		
		// Sleep to maintain velocity.
		int remaining = std::round((1 / VEL) * 1000 - timer.split());
		
		if (remaining > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(remaining));
		}	
	}
	
	return 0;
}