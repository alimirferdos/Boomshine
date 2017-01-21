// Boomshine.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time*/
#include <vector>
#include <iostream>

#define window_x 600 // graphical window's width
#define window_y 500 // graphical window's height
#define ball_radius 10 // default radius of balls
#define dt 0.3 // move accuracy used in moving, extending, and shrinking
#define click event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left // if's true statement when the user clicks
#define PI 3.14159265 // the pi number

using namespace std;
using namespace sf;


RenderWindow window(VideoMode(window_x, window_y), "Boomshine!");


/* ball struct is sfml's CircleShape class with some add features*/
typedef struct ball{
	CircleShape shape;
	double dx = 1, dy = 1 , dr = 0;
	int duration = -1;

	/* Default ball Constructor
		position, color, and move directions are generated randomly*/
	ball() {

		// position
		shape.setPosition((float)(rand() % window_x), (float)(rand() % window_y));

		//color
		shape.setFillColor(Color((float)(rand() % 255), (float)(rand() % 255), (float)(rand() % 255)));

		//circle shape
		shape.setRadius(ball_radius);

		// move direction
		if (rand() % 2 == 0)
			dx = -1;
		else
			dx = 1;

		if (rand() % 2 == 0)
			dy = -1;
		else
			dy = 1;
	}

	/* ball Constructor for the clicked ball
		color is generated randomly
		none of move_x or move_y are changed*/
	ball(float x , float y) {

		// position
		shape.setPosition(x, y);

		//color
		shape.setFillColor(Color((float)(rand() % 256), (float)(rand() % 256), (float)(rand() % 256)));

		//circle shape
		shape.setRadius(ball_radius);

		//Extending State
		dr = 1;
		dx = 0;
		dy = 0;
	}

	/* Keeping the ball is inside the window
		if the ball hits the window edge it will be returned 
		with the same angle and velocity

		This function is called by the customized move function*/
	void keepInside() {
		int right = shape.getPosition().x + dt < window_x - ball_radius * 2;
		int left = shape.getPosition().x + dt > 0;
		int down = shape.getPosition().y + dt < window_y - ball_radius * 2;
		int up = shape.getPosition().y + dt > 0;

		if (!down || !up) {
			dy *= -1;
		}
		if (!right || !left){
			dx *= -1;
		}
	}

	/* Customized shape.setPosition() which checks keepInside() as well
		Int the case the ball is in the extension state it will be resized*/
	void move() {

		shape.setPosition(dt * dx + shape.getPosition().x, dt * dy + shape.getPosition().y);
		
		time_t now = time(0);
		
		// Extension Completed!
		if (shape.getRadius() >= ball_radius * 4 && dr ==1) {
			dr = 0;
			duration = now;
		}
		
		// Shrink start!
		if (now - duration >= 1.5 && duration != -1) {
			dr = -1;
		}
		
		shape.setRadius(shape.getRadius() + dt * dr);

		keepInside();
	}

} Ball;

/* checking if two balls have intersection
	Balls are passed to this function by reference.
	intersection is checked through the following formula:
	|r2-r1| <= d <= r1 + r2
	where d is the distance between the two centers*/
int haveIntersect(Ball* const b1 , Ball* const b2) {
	double d = sqrt(pow( (*b1).shape.getPosition().x - (*b2).shape.getPosition().x , 2) + pow((*b1).shape.getPosition().y - (*b2).shape.getPosition().y, 2));
	
	if (d < (*b1).shape.getRadius() + (*b2).shape.getRadius() && abs((*b1).shape.getRadius() - (*b2).shape.getRadius()) < d)
		return 1;
	else
		return 0;
}

/* Checking if ball b has intersection with any of the extended_balls
if they have intersection, b will be removed from the ball list and 
will be added to the extended ball list.
If the above happened it will return 1 so we can visualize it 
otherwise 1 will be returned.*/
int checkIntersection(ball* b , vector<Ball> & extended_balls , vector<Ball> & balls , int b_position) {
	for (int i = 0; i < extended_balls.size(); i++){
		if (haveIntersect(b, &extended_balls[i]) == 1) {//they have intersection
			return 1; // b should be extended if 1 was returned
		}
	}
	return 0;
}

void drawBalls(vector<Ball> & balls, vector<Ball> & extended_balls) {
	window.clear();
	for (int i = 0; i < balls.size(); i++) // drawing the moving balls
		window.draw(balls[i].shape);
	for (int i = 0; i < extended_balls.size(); i++) // drawing the extended balls
		window.draw(extended_balls[i].shape);
	window.display();
	sleep(milliseconds(1.f));
}

void moveBalls(vector<Ball> & balls, vector<Ball> & extended_balls , int * wonballs) {
	for (int i = 0; i < balls.size() + extended_balls.size(); i++) {

		// moving ball i
		if (i < balls.size()) {
			balls[i].move();
			// checking the intersection for this movement
			if (checkIntersection(&balls[i], extended_balls, balls, i) == 1 && extended_balls.size() != 0) { // It has intersection
				extended_balls.push_back(balls[i]); // adding b to extended_balls
				balls.erase(balls.begin() + i); // removing b from balls

				extended_balls.back().dr = 1;
				extended_balls.back().dx = 0;
				extended_balls.back().dy = 0;

				(*wonballs)++;
			}
		}

		if (i < extended_balls.size()) {
			extended_balls[i].move();
			if (extended_balls[i].shape.getRadius() <= 0)
				extended_balls.erase(extended_balls.begin() + i);
		}
	}
	drawBalls(balls, extended_balls);
}

int main()
{
	// Initializing the window and some other things...
	
	srand(time(NULL));
	int level = 1 , score = 0;
	vector<Ball> balls;
	vector<Ball> extended_balls;

	while (window.isOpen()){
		
		// Initializing the lists
		int wonballs = 0;

		// Initializing the balls and adding them to the list
		for (int i = 0; i < level * 5; i++)
			balls.push_back(Ball());

		// Click initializing
		Event event;
		int clicked = 0;

		while(1 == 1) { // It will be looped until the level finishes
			
			// Moving the balls
			moveBalls(balls, extended_balls , &wonballs);

			// mouse clicked!
			while (window.pollEvent(event) && clicked != 1) {
				if (click) {
					clicked = 1; // not letting more clicks 
					extended_balls.push_back(ball(event.mouseButton.x, event.mouseButton.y)); // Initializing a ball in the clicked position
				}
			}

			// Finishing the game
			if (clicked == 1 && extended_balls.size() == 0) 
				break;
		}

		// Displaying the result
		if (wonballs >= level) {
			system("cls");
			score += wonballs;
			cout << "Well done! You finished this level! Keep going!" << endl << "Your current score is: " << score;
			level++;
		}
		else {
			system("cls");
			cout << "You need "<< level - wonballs <<" more balls! Try again!";
		}
		
		// Memory management
		balls.clear();
		extended_balls.clear();

		// the game has finished
		if (level > 12)
			break;
	}
	
	system("cls");
	cout << "Well done! You finished the game!";

	return 0;
}
