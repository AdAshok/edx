//
// breakout.c - Standard Edition
//
// Computer Science 50
// Problem Set 4
//

// standard libraries
#define _XOPEN_SOURCE
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 400

// number of rows of bricks
#define ROWS 5

// number of columns of bricks
#define COLS 10

// lives
#define LIVES 3

// paddle rectangle attributes in pixels
// make paddle bottom spacing scale with window height
#define PADDLE_HEIGHT 10
#define PADDLE_WIDTH 60
#define PADDLE_BOTTOM_OFFSET HEIGHT * (9 / 10.)

// bricks rectangle attributes in pixels
// make brick width and brick spacing scale with the window width 
#define BRICK_HEIGHT PADDLE_HEIGHT
#define BRICK_WIDTH WIDTH * (1 / 11.)
#define BRICK_SPACING WIDTH * (1 / 100.)

// ball attributes
#define BALL_DIMENSION 20
#define BALL_RADIUS (BALL_DIMENSION / 2)
#define BALL_VELOCITY 0.15

// prototypes
void initBricks(GWindow window);
GOval initBall(GWindow window);
GRect initPaddle(GWindow window);
GLabel initScoreboard(GWindow window);
void updateScoreboard(GWindow window, GLabel label, int points);
GObject detectCollision(GWindow window, GOval ball);

int main(void)
{
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate bricks
    initBricks(window);

    // instantiate ball, centered in middle of window
    GOval ball = initBall(window);
    add(window, ball);

    // instantiate paddle, centered at bottom of window
    GRect paddle = initPaddle(window);
    add(window, paddle);

    // instantiate scoreboard, centered in middle of window, just above ball
    GLabel label = initScoreboard(window);
    add(window, label);
    setLocation(label, WIDTH / 2, HEIGHT / 2);

    // number of bricks initially
    int bricks = COLS * ROWS;

    // number of lives initially
    int lives = LIVES;

    // number of points initially
    int points = 0;
    
    // let scoreboard show up at the beginning of the game
    updateScoreboard(window, label, points);
    
    // declare once useful variables for game loop
    double mouse_x, paddle_x;
    double ball_x, ball_y;
    
    // start the ball with random x and y velocity (scaled down by const)
    double ball_x_velocity = fmod(drand48() * BALL_VELOCITY, BALL_VELOCITY);
    double ball_y_velocity = 0.1;
    
    // wait for click before entering the game loop
    waitForClick(); 
  
    // keep playing until game over
    while (lives > 0 && bricks > 0)
    {
        // move the ball  
        move(ball, ball_x_velocity, ball_y_velocity);
        
        // check user input (mouse movements)
        GEvent event = getNextEvent(MOUSE_EVENT);
        if (event != NULL && getEventType(event) == MOUSE_MOVED)
        {
            // set paddle x to be the paddle center aligned to the x mouse event 
            // keep paddle y constant (don't move through the y axis)
            mouse_x = getX(event);
            paddle_x = mouse_x - PADDLE_WIDTH / 2;
            // check mouse event x to keep paddle in the canvas
            if (mouse_x <= PADDLE_WIDTH / 2)
                paddle_x = 0;
            else if (mouse_x >= WIDTH - PADDLE_WIDTH / 2)
                paddle_x = WIDTH - PADDLE_WIDTH;
            // update paddle position
            setLocation(paddle, paddle_x, PADDLE_BOTTOM_OFFSET);    
        }
        
        // store x and y for ball and prevent further functions calls
        ball_x = getX(ball);
        ball_y = getY(ball);
        
        // bouncing checks
        // top window bouncing
        if (ball_y <= 0) 
            ball_y_velocity = -ball_y_velocity;
        // bottom window bouncing
        else if (ball_y + BALL_DIMENSION >= HEIGHT)
        {
            // spawn ball from the center
            setLocation(ball, WIDTH / 2 - BALL_RADIUS, HEIGHT / 2 - BALL_RADIUS);
            // center the paddle
            setLocation(paddle, WIDTH / 2 - PADDLE_WIDTH / 2, 
                        PADDLE_BOTTOM_OFFSET);
            // pause 1 sec, update lives and starting ball velocity to random 
            pause(1000);
            ball_x_velocity = fmod(drand48() * BALL_VELOCITY, BALL_VELOCITY);
            lives--;
        }    
        // left and right bouncing
        if (ball_x + BALL_DIMENSION >= WIDTH || ball_x <= 0)
            ball_x_velocity = -ball_x_velocity;
            
        // check for collisions
        GObject collided_obj = detectCollision(window, ball);
        if (collided_obj != NULL)
        {
            // check for rectangular object type
            if (strcmp(getType(collided_obj), "GRect") == 0)
            {   
                // ball hit the paddle
                if (collided_obj == paddle)
                {  
                    ball_y_velocity = -ball_y_velocity;
                    // prevent paddle-ball overlapping
                    setLocation(ball, ball_x, 
                                PADDLE_BOTTOM_OFFSET - BALL_RADIUS * 2);
                }
                // ball hit a brick: remove it, update game status variable
                else if (collided_obj != paddle)
                {
                    ball_y_velocity = -ball_y_velocity;
                    removeGWindow(window, collided_obj);
                    points++; 
                    bricks++;
                    updateScoreboard(window, label, points); 
                }
            }
        }   
    }
    
    // wait for click before exiting
    waitForClick();

    // game over
    closeGWindow(window);
    return 0;
}

/**
 * Initializes window with a grid of bricks.
 */
void initBricks(GWindow window)
{
    string color[5] = {"BLACK", "RED", "ORANGE", "GREEN", "CYAN"};
    // stay consistant with paddle bottom offset, paddle and bricks offsets are symmetrical
    int brick_top_offset = HEIGHT * 1/10 - BRICK_HEIGHT;
    // set brick width to scale with window width
    int brick_width = WIDTH * 1/11;
    
    for (int i = 0; i < 5; i++)
    {
        // update brick column offset on outer loop
        int y = brick_top_offset + (BRICK_SPACING * i) + (BRICK_HEIGHT * i);
        for (int j = 0; j < 10; j++)
        {
            // update brick row
            int x = (BRICK_SPACING * j + 1) + (j * brick_width);
            // instantiate a new brick
            GRect brick = newGRect(x, y, brick_width, BRICK_HEIGHT);
            // set different brick colors for each row and add brick to canvas
            setFilled(brick, true);
            setColor(brick, color[i]);
            add(window, brick);
        }
    }
}

/**
 * Instantiates ball in center of window.  Returns ball.
 */
GOval initBall(GWindow window)
{
    // initialize a ball on the center of the screen
    GOval ball = newGOval((getWidth(window) / 2) - BALL_RADIUS,
                          (getHeight(window) / 2) - BALL_RADIUS, BALL_DIMENSION, 
                           BALL_DIMENSION);
    // set up ball color
    setFilled(ball, true);
    setColor(ball, "BLACK");
    return ball;
}

/**
 * Instantiates paddle in bottom-middle of window.
 */
GRect initPaddle(GWindow window)
{
    // set up the paddle horizontal position to the middle of the window
    // set up the paddle vertical position to be the window 1/10th from bottom border
    GRect paddle = newGRect((WIDTH / 2 - PADDLE_WIDTH / 2), 
                             PADDLE_BOTTOM_OFFSET, PADDLE_WIDTH, PADDLE_HEIGHT);
    // set up paddle color
    setFilled(paddle, true);
    setColor(paddle, "BLUE");
    return paddle;
}

/**
 * Instantiates, configures, and returns label for scoreboard.
 */
GLabel initScoreboard(GWindow window)
{
    GLabel scoreboard = newGLabel("");
    setFont(scoreboard, "SansSerif-30");
    setColor(scoreboard, "RED");
    return scoreboard;
}

/**
 * Updates scoreboard's label, keeping it centered in window.
 */
void updateScoreboard(GWindow window, GLabel label, int points)
{
    // update label
    char s[12];
    sprintf(s, "%i", points);
    setLabel(label, s);

    // center label in window
    double x = (getWidth(window) - getWidth(label)) / 2;
    double y = (getHeight(window) - getHeight(label)) / 2;
    setLocation(label, x, y);
}

/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
GObject detectCollision(GWindow window, GOval ball)
{
    // ball's location
    double x = getX(ball);
    double y = getY(ball);

    // for checking for collisions
    GObject object;

    // check for collision at ball's top-left corner
    object = getGObjectAt(window, x, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x + 2 * BALL_RADIUS, y);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x, y + 2 * BALL_RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * BALL_RADIUS, y + 2 * BALL_RADIUS);
    if (object != NULL)
    {
        return object;
    }

    // no collision
    return NULL;
}
