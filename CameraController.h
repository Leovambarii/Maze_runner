#pragma once

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "MazeGenerator.h"

#include <osg/MatrixTransform>
#include <osgGA/FirstPersonManipulator>
#include <osgViewer/Viewer>
#include <osg/Timer>
#include <osgText/Text>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

const float moveStep = 0.02; // Step size for camera movement
const float rotateAngle = 2.0; // Angle by which the camera rotates on rotate key press
const float zPos = 0.25; // Z-coordinate for the camera position
const float distanceFromWall = 0.15; // Minimum distance from walls

/**
 * @class CameraController
 * @brief Custom camera controller for navigating within a maze.
 *
 * Inherits from osgGA::FirstPersonManipulator to provide first-person camera control.
 */
class CameraController : public osgGA::FirstPersonManipulator {
public:
    /**
     * @brief Constructor for CameraController.
     *
     * @param inputViewer A pointer to the osgViewer::Viewer.
     * @param inputMazeVec A 2D vector representing the maze.
     */
    CameraController(osgViewer::Viewer *inputViewer, std::vector<std::vector<CellType>> inputMazeVec);

    /**
     * @brief Sets the initial camera position in the maze.
     *  
     * It also ensures that initial camera doesn't look at the wall by using getValidInitCenterPos(int x, int y) function.
     *
     * @param x The x-coordinate of the initial position.
     * @param y The y-coordinate of the initial position.
     */
    void setInitCamPos(int x, int y);

    /**
     * @brief Return bool foundExitFlag flag that idicates whether exit was found.
     * 
     * @return bool - true if exit was found, false otherwise.
     */
    bool checkExit();

protected:
    osgViewer::Viewer *viewer; // Pointer to the osgViewer::Viewer
    osg::Vec3d movement; // Vector representing camera movement
    const std::vector<std::vector<CellType>> mazeVec; // 2D vector representing the maze
    bool foundExitFlag;

    /**
     * @brief Event handler for GUI events.
     *
     * Handles keyboard events for camera movement and rotation.
     *
     * @param ea The GUIEventAdapter containing event information.
     * @param aa The GUIActionAdapter containing action information.
     * @return bool - True if the event is handled, false otherwise.
     */
    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &);

    /**
     * @brief Gets valid initial center position based on specified coordinate. 
     *
     * Checks if four adjecent maze tiles aren't walls in order to not look at the wall. 
     * Returns first xy pair that mets this condition.
     *
     * @param x The x-coordinate.
     * @param y The y-coordinate.
     * @return std::pair<int, int> - Valid initial eye xy position.
     */
    std::pair<int, int> getValidInitCenterPos(int x, int y);

    /**
     * @brief Calculate maze index corresponding to the given value.
     * 
     * Takes a float value representing a position and calculates
     * the corresponding maze index.
     * 
     * @param val Float value representing a position in the maze.
     * @return int - The calculated maze index.
     */
    int getMazeIndex(float val);

    /**
     * @brief Checks if specified position is valid within the ma.
     *
     * Checks if the rounded coordinates of the current camera position fall within 
     * the maze bounds and if the corresponding maze cell is not a wall. 
     * Returns true for a valid move, false otherwise.
     * 
     * @param pos The current camera position to be checked.
     * @return bool - True if the move is valid, false otherwise.
     */
    bool checkMove(osg::Vec3d pos);

    /**
     * @brief Adjusts the given coordinate to step back from the wall.
     *
     * Takes a coordinate and adjusts it to step back from wall
     * based on a predefined distance. Ensures that the adjusted position
     * aligns with the wall grid to prevent the camera from being too close
     * to walls.
     * 
     * @param pos The position to be adjusted.
     * @return float - The adjusted position after stepping back from walls.
     */
    float stepBackFromWall(float pos);

    /**
     * @brief Adjusts the camera's distance from walls.
     *
     * Takes current camera position and adjusts it to ensure
     * that it is at a safe distance from walls. It uses a step-back strategy
     * to prevent the camera from peeking into the walls.
     * 
     * @param pos The current camera position.
     * @return osg::Vec3 - The adjusted camera position.
     */
    osg::Vec3 correctDistanceFromWalls(osg::Vec3 pos);


    /**
     * @brief Checks if the given position corresponds to the finishing point in the maze.
     *
     * @param pos Position to be checked.
     * @return bool - true if the position corresponds to the finishing point, false otherwise.
     */
    bool checkIfFinsihed(osg::Vec3d pos);
};

#endif