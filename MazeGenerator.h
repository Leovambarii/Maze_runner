#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

enum CellType {
    PATH,
    WALL,
    START, // Represents the starting point in the maze
    END // Represents the finishing point in the maze
};

/**
 * @class MazeGenerator
 * @brief Generates a maze using the depth-first search algorithm.
 */
class MazeGenerator {
public:
    /**
     * @brief Constructor for MazeGenerator.
     *
     * @param size The size (width and height) of the maze.
     * @param x The starting x-coordinate in the maze.
     * @param y The starting y-coordinate in the maze.
     */
    MazeGenerator(int size, int x, int y);

    /**
     * @brief Gets the generated maze.
     *
     * @return A 2D vector representing the maze with CellType values.
     */
    std::vector<std::vector<CellType>> getMaze();

protected:
    int size; // The size (width and height) of the maze
    int startX; // The starting x-coordinate in the maze
    int startY; // The starting y-coordinate in the maze
    std::vector<std::vector<CellType>> maze; // The maze represented with CellType values

    /**
     * @brief Checks if the given position is a valid wall position.
     *
     * @param x The x-coordinate to check.
     * @param y The y-coordinate to check.
     * @return True if the position is a valid wall position, false otherwise.
     */
    bool isValidPosition(int x, int y);

    /**
     * @brief Checks if a cell has three walls around it.
     *
     * @param x The x-coordinate of the cell.
     * @param y The y-coordinate of the cell.
     * @return True if the cell has three walls around it, false otherwise.
     */
    bool hasThreeWalls(int x, int y);

    /**
     * @brief Recursively generates the maze using the depth-first search algorithm.
     *
     * @param x The current x-coordinate in the generation process.
     * @param y The current y-coordinate in the generation process.
     */
    void generateMaze(int x, int y);

    /**
     * @brief Prints the maze to the console.
     */
    void printMaze();
};
