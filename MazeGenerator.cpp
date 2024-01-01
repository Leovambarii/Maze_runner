#include "MazeGenerator.h"

MazeGenerator::MazeGenerator(int size, int x, int y): size(size), startX(x), startY(y), maze(size, std::vector<CellType>(size, WALL)) {
    generateMaze(startX, startY);
    maze[startX][startY] = START;
    maze[size-1][size-(startY+1)] = END;
    printMaze();
}

bool MazeGenerator::isValidPosition(int x, int y) {
    return x >= 0 && x < size && y >= 0 && y < size && maze[x][y] == WALL;
}

bool MazeGenerator::hasThreeWalls(int x, int y) {
    int walls = 0;
    if (x > 0 && maze[x - 1][y] == PATH)
        walls++;
    if (x < size - 1 && maze[x + 1][y] == PATH)
        walls++;
    if (y > 0 && maze[x][y - 1] == PATH)
        walls++;
    if (y < size - 1 && maze[x][y + 1] == PATH)
        walls++;

    return walls >= 3;
}

void MazeGenerator::generateMaze(int x, int y) {
    // Define possible movement directions
    int dirX[] = {0, 0, 1, -1};
    int dirY[] = {1, -1, 0, 0};

    maze[x][y] = PATH; // Mark the current cell as a path

    // Randomize the order of directions for exploration
    for (int i = 0; i < 4; ++i) {
        int randomIndex = rand() % 4;
        int tmp = dirX[i];
        dirX[i] = dirX[randomIndex];
        dirX[randomIndex] = tmp;

        tmp = dirY[i];
        dirY[i] = dirY[randomIndex];
        dirY[randomIndex] = tmp;
    }

    // Explore in randomized directions
    for (int i = 0; i < 4; ++i) {
        int newX = x + 2 * dirX[i];
        int newY = y + 2 * dirY[i];

        if (isValidPosition(newX, newY) && !hasThreeWalls(newX, newY)) {
            // Carve a path to the new position and continue the generation recursively
            maze[x + dirX[i]][y + dirY[i]] = PATH;
            generateMaze(newX, newY);
        }
    }
}

void MazeGenerator::printMaze() {    
    std::cout << "MAP OF THE MAZE: " << std::endl;
    for (int i = 0; i < size; i++) {
        std::cout << "\t";
        for (int j = 0; j < size; j++) {
            switch (maze[i][j]) {
                case WALL:
                    std::cout << "# ";
                    break;
                case PATH:
                    std::cout << "  ";
                    break;
                case START:
                    std::cout << "S ";
                    break;
                case END:
                    std::cout << "D ";
                    break;
            }
        }
        std::cout << std::endl;
    }
}

std::vector<std::vector<CellType>> MazeGenerator::getMaze() {
    return maze;
}