#include<stdio.h>
#include<stdlib.h>
#include<GL/glut.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<time.h>
#include<Windows.h>
#include<mmsystem.h>
//#include <unistd.h>

#define WIDTH 620
#define HEIGHT 620
#define CELL_SIZE 20
#define TIMER_INTERVAL 1000 // Timer interval in milliseconds
#define TIME_LIMIT 60

int maze[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
int visited[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
int path[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
int ratRow, ratCol;
int entranceRow, entranceCol;
int exitRow, exitCol;
int gameState = 0; // 0: Start menu, 1: In-game, 2: Win, 3: Lose, 4: Show path
int timer;

void init()
{
    glClearColor(0.5, 0.5, 0.5, 0.0); // Grey background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, HEIGHT, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand(time(NULL));
}

void generateMaze(int row, int col)
{
    visited[row][col] = 1;

    // Directions: right, down, left, up
    int dir[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int permutation[4] = {0, 1, 2, 3};
    // Randomizing directions
    for (int i = 0; i < 4; i++)
    {
        int j = rand() % 4;
        int temp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = temp;
    }

    for (int i = 0; i < 4; i++)
    {
        int r = row + dir[permutation[i]][0] * 2;
        int c = col + dir[permutation[i]][1] * 2;
        // Cutting through the maze
        if (r >= 0 && r < HEIGHT / CELL_SIZE && c >= 0 && c < WIDTH / CELL_SIZE && !visited[r][c])
        {
            maze[row + dir[permutation[i]][0]][col + dir[permutation[i]][1]] = 1;
            // Cutting adjacent cell in the same direction
             maze[row + dir[permutation[i]][0] * 2][col + dir[permutation[i]][1] * 2] = 1;
            generateMaze(r, c);
        }
    }
}

typedef struct {
    int row;
    int col;
} Cell;

// Queue for BFS
Cell queue[HEIGHT * WIDTH];
int front = -1, rear = -1;

void enqueue(Cell cell) {
    if (rear == HEIGHT * WIDTH - 1) {
        printf("Queue is full\n");
        return;
    }
    if (front == -1) {
        front = 0;
    }
    printf("%2d %2d\n",queue[rear].row, queue[rear].col);
    rear++;
    queue[rear] = cell;
}

bool isEmpty() {
    return front == -1 || front > rear;
}

Cell dequeue() {
    if (isEmpty()) {
        printf("Queue is empty\n");
        exit(1);
    }
    Cell cell = queue[front];
    front++;
    return cell;
}


bool isValid(int row, int col, int maze[][WIDTH / CELL_SIZE], int visited[][WIDTH / CELL_SIZE]) {
    return (row >= 0 && row < HEIGHT / CELL_SIZE && col >= 0 && col < WIDTH / CELL_SIZE && maze[row][col] == 1 && !visited[row][col]);
}

void pathFinder(int maze[][WIDTH / CELL_SIZE], int entranceRow, int entranceCol, int exitRow, int exitCol, int path[][WIDTH / CELL_SIZE]) {
    memset(visited, 0, sizeof(visited)); // Reset visited matrix
    memset(path, 0, sizeof(path)); // Initialize path matrix

    Cell entrance = {entranceRow, entranceCol};
    enqueue(entrance);
    visited[entranceRow][entranceCol] = 1;

    while (!isEmpty()) {
        Cell current = dequeue();
        int row = current.row;
        int col = current.col;

        // Check if reached exit
        if (row == exitRow && col == exitCol) {
            // Mark the path
            while (row != entranceRow || col != entranceCol) {
                path[row][col] = 1;
                Cell parent = queue[row * WIDTH + col];
                row = parent.row;
                col = parent.col;
            }
            // Mark entrance as part of the path
            path[entranceRow][entranceCol] = 1;
            break;
        }

        // Check neighbors
        int rowOffsets[] = {-1, 0, 1, 0};
        int colOffsets[] = {0, 1, 0, -1};

        for (int i = 0; i < 4; ++i) {
            int nextRow = row + rowOffsets[i];
            int nextCol = col + colOffsets[i];
            if (isValid(nextRow, nextCol, maze, visited)) {
                Cell nextCell = {nextRow, nextCol};
                enqueue(nextCell);
                visited[nextRow][nextCol] = 1;

                // Mark the parent cell for reconstructing the path later
                queue[nextRow * WIDTH + nextCol] = current;
            }
        }
    }
}

void drawHelveticaString(const char* str)
{
    while (*str)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);
        str++;
    }
}

void drawRomanString(const char* str)
{
    while (*str)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);
        str++;
    }
}

void drawRat()
{
    glColor3f(0.33, 0.0,0.0); // Brown color for rat
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ratCol * CELL_SIZE + CELL_SIZE / 2, ratRow * CELL_SIZE + CELL_SIZE / 2); // Center of the circle

    float radius = CELL_SIZE / 2;
    int numTriangles = 20; // Number of triangles to approximate the circle
    for (int i = 0; i <= numTriangles; ++i)
    {
        float angle = 2.0 * M_PI * i / numTriangles;
        float x = ratCol * CELL_SIZE + CELL_SIZE / 2 + radius * cos(angle);
        float y = ratRow * CELL_SIZE + CELL_SIZE / 2 + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

void drawEntranceAndExit()
{
    glColor3f(1.0, 1.0, 0.0); // Yellow font

    // Draw start and finish
    glRasterPos2i(0, 15);
    drawHelveticaString("START");
    glRasterPos2i(WIDTH - 40, HEIGHT - 5);
    drawHelveticaString("END");

    // Entrance (green)
    glColor3f(0.0, 1, 0.0); // Green color for entrance
    glBegin(GL_QUADS);
    glVertex2i(entranceCol * CELL_SIZE, entranceRow * CELL_SIZE);
    glVertex2i(entranceCol * CELL_SIZE + CELL_SIZE, entranceRow * CELL_SIZE);
    glVertex2i(entranceCol * CELL_SIZE + CELL_SIZE, entranceRow * CELL_SIZE + CELL_SIZE);
    glVertex2i(entranceCol * CELL_SIZE, entranceRow * CELL_SIZE + CELL_SIZE);
    glEnd();

    // Exit (red)
    glColor3f(1, 0.0, 0.0); // Red color for exit
    glBegin(GL_QUADS);
    glVertex2i(exitCol * CELL_SIZE, exitRow * CELL_SIZE);
    glVertex2i(exitCol * CELL_SIZE + CELL_SIZE, exitRow * CELL_SIZE);
    glVertex2i(exitCol * CELL_SIZE + CELL_SIZE, exitRow * CELL_SIZE + CELL_SIZE);
    glVertex2i(exitCol * CELL_SIZE, exitRow * CELL_SIZE + CELL_SIZE);
    glEnd();
}

void drawStartMenu()
{
    glColor3f(0.0, 0.0, 0.0); // Black font

    // Draw menu options
    glRasterPos2i(30, 235);
    drawRomanString("Start Menu");

    glRasterPos2i(30, 240);
    drawRomanString("...................");

    glRasterPos2i(30, 260);
    drawHelveticaString("Press F1 to Start New Game");

    glRasterPos2i(30, 280);
    drawHelveticaString("Press F2 to Quit");

    glRasterPos2i(30, 315);
    drawRomanString("How to Play");

    glRasterPos2i(30, 320);
    drawRomanString("....................");

    glRasterPos2i(30, 340);
    drawHelveticaString("Use arrow keys to move and escape the maze before time runs out.");

    glRasterPos2i(30, 380);
    drawHelveticaString("You are given 60 seconds to escape the maze.");

    glRasterPos2i(30, 400);
    drawHelveticaString("The countdown starts the moment you start a new game.");

    glRasterPos2i(30, 440);
    drawHelveticaString("The maze does not change when you start a new game.");

    glRasterPos2i(30, 460);
    drawHelveticaString("If you lose, feel free to return to Start Menu and try again.");

    glRasterPos2i(30, 500);
    drawHelveticaString("Revealing the path will end the game.");

    glRasterPos2i(30, 540);
    drawHelveticaString("To try a new maze, quit the program and restart it.");
}

void drawTimer()
{
    glColor3f(1.0, 0.0, 1.0); // Pink font
    glRasterPos2i(WIDTH - 100, 15);
    char timeStr[15];
    snprintf(timeStr, sizeof(timeStr), "Time: %d s", timer);
    drawHelveticaString(timeStr);
}

void drawMaze()
{
    glColor3f(0.0, 0.0, 0.0); // Black walls
    // Draw maze walls
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            if (maze[i][j] == 0)
            {
                glBegin(GL_QUADS);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glEnd();
            }
        }
    }

    drawEntranceAndExit();

    drawRat();

    drawTimer();

    glColor3f(0.0, 1.0, 0.0);
    glRasterPos2i(WIDTH / 2 - 100, HEIGHT - 5);
    drawHelveticaString("Press F1 to Reveal Path");
}

void drawPath()
{

     glColor3f(0.0, 0.0, 0.0); // Black walls
    // Draw maze walls
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            if (maze[i][j] == 0)
            {
                glBegin(GL_QUADS);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glEnd();
            }
        }
    }

    drawEntranceAndExit();

    glColor3f(0.0, 1.0, 0.0); // Green path
    // Draw path
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            if (path[i][j] == 1)
            {
                glBegin(GL_QUADS);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glEnd();
            }
        }
    }

    // Return to start menu message
    glColor3f(0.0, 1.0, 0.0);
    glRasterPos2i(WIDTH / 2 - 100, HEIGHT - 5);
    drawHelveticaString("Press F1 to return to Start Menu");
}

void winnerScreen()
{
    // Game over message (player wins)
    glColor3f(0.0, 1.0, 0.0); // Green font
    glRasterPos2i(WIDTH / 2 - 40, HEIGHT / 2);
    drawRomanString("YOU WIN!");

    // Return to start menu message
    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2i(WIDTH / 2 - 100, HEIGHT / 2 + 20);
    drawHelveticaString("Press F1 to return to Start Menu");
}

void loserScreen()
{
    // Game over message (player loses)
    glColor3f(1.0, 0.0, 0.0); // Red font
    glRasterPos2i(WIDTH / 2 - 40, HEIGHT / 2);
    drawRomanString("YOU LOSE!");

    // Return to start menu message
    glColor3f(0.0, 0.0, 0.0);
    glRasterPos2i(WIDTH / 2 - 100, HEIGHT / 2 + 20);
    drawHelveticaString("Press F1 to return to Start Menu");
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameState == 0)
    {
        drawStartMenu();
    }
    else if (gameState == 1)
    {
        drawMaze();
    }
    else if (gameState == 2)
    {
        winnerScreen();
    }
    else if (gameState == 3)
    {
        loserScreen();
    }
    else if (gameState == 4)
    {
        drawPath();
    }

    glutSwapBuffers();
}

void handleStartMenuKeys(int key)
{
    if (key == GLUT_KEY_F1)   // Start new game
    {
        gameState = 1;
        ratRow = entranceRow; // Set rat's position to entrance
        ratCol = entranceCol;
        timer = TIME_LIMIT; // Reset timer
        glutPostRedisplay();
    }
    else if (key == GLUT_KEY_F2)     // Quit
    {
        exit(0);
    }
}

void handleInGameStateKeys(int key)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        if (ratRow > 0 && maze[ratRow - 1][ratCol] == 1)
            ratRow--;
        break;
    case GLUT_KEY_DOWN:
        if (ratRow < HEIGHT / CELL_SIZE - 1 && maze[ratRow + 1][ratCol] == 1)
            ratRow++;
        break;
    case GLUT_KEY_LEFT:
        if (ratCol > 0 && maze[ratRow][ratCol - 1] == 1)
            ratCol--;
        break;
    case GLUT_KEY_RIGHT:
        if (ratCol < WIDTH / CELL_SIZE - 1 && maze[ratRow][ratCol + 1] == 1)
            ratCol++;
        break;
    case GLUT_KEY_F1:
        gameState = 4;
        break;
    }

    // Check if the rat reached the exit
    if (ratRow == exitRow && ratCol == exitCol)
    {
        gameState = 2; // Player wins
    }

    glutPostRedisplay();
}

void handleEndGameStateKeys(int key)
{
    if (key == GLUT_KEY_F1)   // Return to start menu
    {
        gameState = 0;
        glutPostRedisplay();
    }
}

void specialKeys(int key, int x, int y)
{
    if (gameState == 0)
    {
        handleStartMenuKeys(key);
    }
    else if (gameState == 1)
    {
        handleInGameStateKeys(key);
    }
    else if (gameState == 2 || gameState == 3 || gameState == 4)
    {
        handleEndGameStateKeys(key);
    }
}


void updateTimer(int value)
{
    if (gameState == 1)
    {
        timer--;
        if (timer <= 0)
        {
            gameState = 3; // Player loses
        }
    }
    glutPostRedisplay();
    glutTimerFunc(TIMER_INTERVAL, updateTimer, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("RatA");
    init();

    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            // Initializing with 0 for walls
            maze[i][j] = 0;
            visited[i][j] = 0;
        }
    }

    generateMaze(1, 1);
     //Draw on terminal
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            // Initializing with 0 for walls
            if(maze[i][j]) printf("1 ");
            else printf("0 ");
        }
        printf("\n");
    }
    printf("\n");

    for (int i = 0; i < HEIGHT / CELL_SIZE; i++)
    {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++)
        {
            // Initializing with 0 for walls
            if(visited[i][j]) printf("1 ");
            else printf("0 ");
        }
        printf("\n");
    }

    // Set entrance and exit positions
    entranceRow = 1;
    entranceCol = 1;
    exitRow = HEIGHT / CELL_SIZE - 2;
    exitCol = WIDTH / CELL_SIZE - 1;

    // Set entrance and exit open
    maze[entranceRow][entranceCol] = 1;
    maze[exitRow][exitCol] = 1;

    pathFinder(maze, entranceRow, entranceCol, exitRow, exitCol, path);



    // Save mazes in a .txt file
    FILE *file = fopen("maze.txt", "a"); // Open the file in append mode
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }
    // Write maze data to the file
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++) {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++) {
            if(maze[i][j]) fprintf(file, "1 ");
            else fprintf(file, "0 ");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\n");
    fclose(file);


    // Set rat position at entrance
    ratRow = entranceRow;
    ratCol = entranceCol;

    // Start timer
    glutTimerFunc(TIMER_INTERVAL, updateTimer, 0);

    glutDisplayFunc(display);
    PlaySound(TEXT("C:\\Users\\ASUS\\Documents\\ggg\\rata\\candyland.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
    glutSpecialFunc(specialKeys);
    glutMainLoop();

    return 0;
}
