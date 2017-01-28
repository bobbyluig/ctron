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
#include <algorithm>

#define VEL 10.0000

typedef std::pair<int, int> Coordinate;


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
		void think(std::map<Coordinate, Block>& map);
		
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
		
		Block getBlockAhead(std::map<Coordinate, Block>& map, Direction direction, int steps);
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

Block Tron::getBlockAhead(std::map<Coordinate, Block>& map, Direction direction, int steps) {
	Coordinate pos = this->getPosition();
	
	switch (direction) {
	case Direction::UP:
		pos.second -= steps;
		break;
	case Direction::DOWN:
		pos.second += steps;
		break;
	case Direction::LEFT:
		pos.first -= steps;
		break;
	case Direction::RIGHT:
		pos.first += steps;
		break;
	}
	
	return map[pos];
}

void Tron::think(std::map<Coordinate, Block>& map) {
	std::vector<double> grades = {0, 0, 0, 0};
	
	// Remove reverse direction.
	switch (this->direction) {
	case Direction::LEFT:
		grades[1] -= 50.0;
		break;
	case Direction::RIGHT:
		grades[0] -= 50.0;
		break;
	case Direction::UP:
		grades[3] -= 50.0;
		break;
	case Direction::DOWN:
		grades[2] -= 50.0;
		break;
	}
	
	// Look ahead.
	for (int i = 0; i < 4; i++) {
		Direction direction = static_cast<Direction>(i);
		Block block1 = this->getBlockAhead(map, direction, 1);
		Block block2 = this->getBlockAhead(map, direction, 2);
		
		if (block1 == Block::WALL || block1 == Block::TRON || block2 == Block::TRON) {
			grades[i] -= 50.0;
		}
	}
	
	// Get max.
	int maxIndex = 0;
	double maxValue = grades[0];
	
	for (int i = 1; i < 4; i ++) {
		if (grades[i] > maxValue) {
			maxIndex = i;
			maxValue = grades[i];
		}
	}
	
	Direction optimal = static_cast<Direction>(maxIndex);
	this->setDirection(optimal);
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
}

Field::~Field() {
	for (auto tron : this->trons) {
		delete tron;
	}
}

bool Field::isColliding(Tron* tron) {
	Coordinate pos = tron->getPosition();
	
	int tronsInPos = 0;
	
	for (auto t : this->trons) {
		if (t->isAlive() && t->getPosition() == pos) {
			tronsInPos += 1;
		}
	}
	
	return 	(this->map[pos] == Block::WALL) || (tronsInPos > 1);
}

int Field::getOptimalWalls() {
	// Best is around 5% of field area.
	return std::round((this->maxX - this->minX) * (this->maxY - this->minY) * 0.05);
}

void Field::updateMap() {
	for (int x = this->minX; x <= this->maxX; x++) {
		for (int y = this->minY; y <= this->maxY; y++) {
			this->map[std::make_pair(x, y)] = Block::EMPTY;
		}
	}
	
	for (int y = this->minY - 1; y <= this->maxY + 1; y++) {
		this->map[std::make_pair(this->minX - 1, y)] = Block::WALL;
		this->map[std::make_pair(this->maxX + 1, y)] = Block::WALL;
	}
	
	for (int x = this->minX - 1; x <= this->maxX + 1; x++) {
		this->map[std::make_pair(x, this->minY - 1)] = Block::WALL;
		this->map[std::make_pair(x, this->maxY + 1)] = Block::WALL;
	}
	
	for (auto tron : this->trons) {
		if (!tron->isAlive()) {
			continue;
		}
		
		Coordinate pos = tron->getPosition(); 
		if (this->map[pos] == Block::EMPTY) {
			this->map[pos] = Block::TRON; 
		}
		
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
			// Think!
			tron->think(this->map);
		}
		else {
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
		}

		tron->move();
	}
	
	// Update field map.
	this->updateMap();
	
	// Did anyone get wrecked?
	std::vector<Tron*> deadTrons;
	
	for (auto tron : this->trons) {
		if (!tron->isAlive()) {
			continue;
		}
		
		if (this->isColliding(tron)) {
			deadTrons.push_back(tron);
		}
	}
	
	for (auto tron : deadTrons) {
		tron->kill();
	}
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
	
	// Build initial map.
	this->updateMap();
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
		
		// All dead? Exit.
		if (field.getNumAlive() == 0) {
			break;
		}
		
		// Sleep to maintain velocity.
		int remaining = std::round((1 / VEL) * 1000 - timer.split());
		
		if (remaining > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(remaining));
		}
	}
	
	endwin();
	
	return 0;
}