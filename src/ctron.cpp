#include <curses.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <set>
#include <iostream>
#include <random>
#include <iterator>

#define DELAY 35000
#define FPS 60

typedef std::pair<double, double> Coordinate;


enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};


class Timer {
	public:
		Timer();
		void start();
		double split();
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
	if (!this->started) {
		return -1;
	}
	
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - this->t);
	
	return duration.count();
}


class Tron {
	public:
		Tron(int color);
		
		void setPosition(double x, double y);
		void setDirection(Direction direction);
		
		void move(double units);
		
		Coordinate getPosition();
		int getColor();
	private:
		int color;
		
		double posX;
		double posY;
		Direction direction;
		bool alive;
		
		std::set<Coordinate> walls;
};

Tron::Tron(int color) {
	this->color = color;
	this->alive = true;
}

void Tron::setPosition(double x, double y) {
	this->posX = x;
	this->posY = y;
}

void Tron::setDirection(Direction direction) {
	this->direction = direction;
}

void Tron::move(double amount) {
	Coordinate pos = this->getPosition();
	this->walls.insert(pos);
	
	switch(this->direction) {
	case Direction::LEFT:
		this->posX -= amount;
		break;
	case Direction::RIGHT:
		this->posX += amount;
		break;
	case Direction::UP:
		this->posY += amount;
		break;
	case Direction::DOWN:
		this->posY -= amount;
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
		void setupField();
		void getTron(int index);
	private:
		int minX;
		int minY;
		int maxX;
		int maxY;
		std::vector<Tron*> trons;
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

void Field::addTron(Tron* tron) {
	this->trons.push_back(tron);
}

void Field::getTron(int index) {
	return this->trons[index];
}

void Field::setupField() {
	int width = this->maxX - this->minX;
	int height = this->maxY - this->minY;
	
	for (int i = 0; i < this->trons.size(); i++) {
		init_pair(i + 1, this->trons[i]->getColor(), COLOR_BLACK);
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
	noecho();
	curs_set(FALSE);
	
	// Get screen size.
	int rows, cols;
	getmaxyx(stdscr, rows, cols);
	
	// Create field.
	Field field(0, 0, cols, rows);
	
	// Create all Trons and add them to the field.
	Tron* player = new Tron(COLOR_CYAN);
	field.addTron(player);
	
	for (int i = 0; i < numAI; i++) {
		Tron* ai = new Tron(COLOR_YELLOW);
		field.addTron(ai);
	}
	
	// Setup the field.
	field.setupField();
	
	while (1) {
		clear();
		// attron(COLOR_PAIR(1));
		mvprintw(0, 0, "(%d, %d)", cols, rows);
		refresh();
		usleep(DELAY);
	}
	
	return 0;
}



/*
int main(int argc, char *argv[]) {
  int x = 0,
      y = 0;

  int max_x = 0,
      max_y = 0;

  int next_x = 0;

  int direction = 1;

  initscr();
  noecho();
  curs_set(FALSE);

  getmaxyx(stdscr, max_y, max_x);

  x = max_x / 2;
  y = max_y / 2;

  while (1) {
    getmaxyx(stdscr, max_y, max_x);

    y = max_y / 2;

    clear();
    mvprintw(y, x, "o");
    refresh();

    usleep(DELAY);

    next_x = x + direction;

    if (next_x >= max_x || next_x < 0) {
      direction*= -1;
    } else {
      x+= direction;
    }

  }

  endwin();

  return 0;
}
*/