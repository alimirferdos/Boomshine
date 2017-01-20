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
#define ball_size 10 // default radius of balls
#define move_acc 0.1 // move accuracy used in moving, extending, and shrinking
#define click event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left // if's true statement when the user clicks
#define PI 3.14159265 // the pi number

using namespace std;
using namespace sf;


RenderWindow window(VideoMode(window_x, window_y), "Boomshine!");


/* ball struct is sfml's CircleShape class with some add features*/
typedef struct ball{
	CircleShape shape;
	double move_x = 1, move_y = 1;
	int duration = -1;

	/* Default ball Constructor
		position, color, and move directions are generated randomly*/
	ball() {

		// position
		shape.setPosition((float)(rand() % window_x), (float)(rand() % window_y));

		//color
		shape.setFillColor(Color((float)(rand() % 255), (float)(rand() % 255), (float)(rand() % 255)));

		//circle shape
		shape.setRadius(ball_size);

		// move direction
		if (rand() % 2 == 0)
			move_x = -1;
		else
			move_x = 1;

		if (rand() % 2 == 0)
			move_y = -1;
		else
			move_y = 1;
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
		shape.setRadius(ball_size);
	}

	/* Keeping the ball is inside the window
		if the ball hits the window edge it will be returned 
		with the same angle and velocity

		This function is called by the customized move function*/
	void keepInside() {
		int right = shape.getPosition().x + move_acc < window_x - ball_size * 2;
		int left = shape.getPosition().x + move_acc > 0;
		int down = shape.getPosition().y + move_acc < window_y - ball_size * 2;
		int up = shape.getPosition().y + move_acc > 0;

		if (!down || !up) {
			move_y *= -1;
		//	cout << "x: " << shape.getPosition().x << "y: " << shape.getPosition().y << endl;
		}
		if (!right || !left){
			move_x *= -1;
		//	cout << "x: " << shape.getPosition().x << "y: " << shape.getPosition().y << endl;
		}
	}

	// Customized shape.setPosition() which checks keepInside() as well
	void move() {
		//shape.move(move_acc * move_x, move_acc * move_y);
		shape.setPosition(move_acc * move_x + shape.getPosition().x, move_acc * move_y + shape.getPosition().y);
		keepInside();
	}

	// extending the ball
	void extend() {
		shape.setRadius(shape.getRadius() + move_acc/10);
	}

	// shrinking the ball
	void shrink() {
		shape.setRadius(shape.getRadius() - move_acc );
	}
} Ball;

/* checking if two balls have intersection
	Balls are passed to this function by reference.
	intersection is checked through the following formula:
	|r2-r1| <= d <= r1 + r2
	where d is the distance between the two centers*/
int intersect(Ball* const b1 , Ball* const b2) {
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
		if (intersect(b, &extended_balls[i]) == 1) {//they have intersection
		//	cout << "Some intersection" << endl;
			extended_balls.push_back(*b); // adding b to extended_balls
			balls.erase(balls.begin() + b_position); // removing b from balls
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
	sleep(milliseconds(.8f));
}

void moveBalls(vector<Ball> & balls, vector<Ball> & extended_balls , int * wonballs) {
	for (int i = 0; i < balls.size(); i++) {

		// moving ball i
		balls[i].move();

		// checking the intersection for this movement
		if (checkIntersection(&balls[i], extended_balls, balls, i) == 1 && extended_balls.size() != 0) { // It has intersection
			(*wonballs)++;
			// Extending the ball while keeping the other balls moving
			while (extended_balls.back().shape.getRadius() < ball_size * 4) {
				extended_balls.back().extend();

				// Keeping other balls moving
				moveBalls(balls, extended_balls, wonballs);
			}
			time_t now = time(0);
			extended_balls.back().duration = now;
		}
	}
	drawBalls(balls, extended_balls);
}

int main()
{
	// Initializing the window and some other things...
	
	srand(time(NULL));
	int level = 1;
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

			// Checking for shrinking
			time_t now = time(0);
			for (int i = 0; i < extended_balls.size(); i++) {
				if (now - extended_balls[i].duration >= 1) {
					while (extended_balls[i].shape.getRadius() >= 0) {
						extended_balls[i].shrink();

						// Moving the balls
						moveBalls(balls, extended_balls, &wonballs);
					}
					extended_balls.erase(extended_balls.begin() + i); // shrinkg has finished and the ball is removed
				}
			}
				
			// mouse clicked!
			while (window.pollEvent(event) && clicked != 1) {
				if (click) {
					clicked = 1; // not letting more clicks 

					extended_balls.push_back(ball(event.mouseButton.x, event.mouseButton.y)); // Initializing a ball in the clicked position
					
					// Extending the clicked ball while keeping the other balls moving 
					while (extended_balls.back().shape.getRadius() < ball_size * 4) {
						extended_balls.back().extend();

						// Moving the balls
						moveBalls(balls, extended_balls, &wonballs);
					}
					time_t now = time(0);
					extended_balls.back().duration = now;
				}
			}

			// Finishing the game
			if (clicked == 1 && extended_balls.size() == 0) 
				break;
		}

		// Displaying the result
		if (wonballs >= level) {
			system("cls");
			cout << "Well done! You finished this level! Keep going!";
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
