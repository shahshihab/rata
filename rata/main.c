#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define CELL_SIZE 20

int maze[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
int visited[HEIGHT / CELL_SIZE][WIDTH / CELL_SIZE];
int ratRow, ratCol;

void init() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, WIDTH, HEIGHT, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand(time(NULL));
}

void generateMaze(int row, int col) {
    visited[row][col] = 1;

    int dir[4][2] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    int permutation[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        int j = rand() % 4;
        int temp = permutation[i];
        permutation[i] = permutation[j];
        permutation[j] = temp;
    }

    for (int i = 0; i < 4; i++) {
        int r = row + dir[permutation[i]][0] * 2;
        int c = col + dir[permutation[i]][1] * 2;

        if (r >= 0 && r < HEIGHT / CELL_SIZE && c >= 0 && c < WIDTH / CELL_SIZE && !visited[r][c]) {
            maze[row + dir[permutation[i]][0]][col + dir[permutation[i]][1]] = 1;
            generateMaze(r, c);
        }
    }
}

void drawRat() {
    glColor3f(0.0, 0.0, 1.0); // Blue color for rat
    glBegin(GL_QUADS);
    glVertex2i(ratCol * CELL_SIZE, ratRow * CELL_SIZE);
    glVertex2i(ratCol * CELL_SIZE + CELL_SIZE, ratRow * CELL_SIZE);
    glVertex2i(ratCol * CELL_SIZE + CELL_SIZE, ratRow * CELL_SIZE + CELL_SIZE);
    glVertex2i(ratCol * CELL_SIZE, ratRow * CELL_SIZE + CELL_SIZE);
    glEnd();
}

void drawMaze() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0, 0.0, 0.0);

    // Draw maze walls
    for (int i = 0; i < HEIGHT / CELL_SIZE; i++) {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++) {
            if (maze[i][j] == 1) {
                glBegin(GL_QUADS);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE);
                glVertex2i(j * CELL_SIZE + CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glVertex2i(j * CELL_SIZE, i * CELL_SIZE + CELL_SIZE);
                glEnd();
            }
        }
    }

    // Draw rat
    drawRat();

    glutSwapBuffers();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            if (ratRow > 0 && maze[ratRow - 1][ratCol] == 0)
                ratRow--;
            break;
        case GLUT_KEY_DOWN:
            if (ratRow < HEIGHT / CELL_SIZE - 1 && maze[ratRow + 1][ratCol] == 0)
                ratRow++;
            break;
        case GLUT_KEY_LEFT:
            if (ratCol > 0 && maze[ratRow][ratCol - 1] == 0)
                ratCol--;
            break;
        case GLUT_KEY_RIGHT:
            if (ratCol < WIDTH / CELL_SIZE - 1 && maze[ratRow][ratCol + 1] == 0)
                ratCol++;
            break;
    }

    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Maze Game");
    init();

    for (int i = 0; i < HEIGHT / CELL_SIZE; i++) {
        for (int j = 0; j < WIDTH / CELL_SIZE; j++) {
            maze[i][j] = 0;
            visited[i][j] = 0;
        }
    }

    generateMaze(1, 1);

    // Set rat position at entrance
    ratRow = 1;
    ratCol = 0;

    glutDisplayFunc(drawMaze);
    glutSpecialFunc(specialKeys);
    glutMainLoop();

    return 0;
}
