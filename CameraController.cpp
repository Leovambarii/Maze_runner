#include "CameraController.h"

CameraController::CameraController(osgViewer::Viewer *inputViewer, std::vector<std::vector<CellType>> inputMazeVec) : 
    viewer(inputViewer), 
    movement(osg::Vec3d()), 
    mazeVec(inputMazeVec),
    foundExitFlag(false)
{ }

bool leftMouseButtonPressed = false;

bool CameraController::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) {
    FirstPersonManipulator::handle(ea, aa);
    if (!viewer) {
        return false;
    }

    // Skip movement calculation when mouse moves or drags to avoid speeding up effect
    if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE || ea.getEventType() == osgGA::GUIEventAdapter::DRAG) {
        return true;
    }

    osg::Matrix matrix = viewer->getCameraManipulator()->getMatrix();
    osg::Vec3d camPos = matrix.getTrans();

    if (checkIfFinsihed(camPos)) {
        // viewer->setDone(true);
        return true;
    }

    osg::Quat camRotation = matrix.getRotate();
    bool isMoved = true;

    switch(ea.getEventType()) {
        case(osgGA::GUIEventAdapter::KEYDOWN): {
            switch(ea.getKey()) {
                case 'w': // Move foreward
                    movement.z() = -moveStep;
                    isMoved = true;
                    break;
                case 's': // Move to the back
                    movement.z() = moveStep;
                    isMoved = true;
                    break;
                case 'a': // Move to the left
                    movement.x() = -moveStep;
                    isMoved = true;
                    break;
                case 'd': // Move to the right
                    movement.x() = moveStep;
                    isMoved = true;
                    break;
                case 'e': // Rotate camera to the right
                    camRotation *= osg::Quat(osg::DegreesToRadians(-rotateAngle), osg::Z_AXIS);
                    matrix.setRotate(camRotation);
                    break;
                case 'q': // Rotate camera to the left
                    camRotation *= osg::Quat(osg::DegreesToRadians(rotateAngle), osg::Z_AXIS);
                    matrix.setRotate(camRotation);
                    break;
                default:
                    break;
            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYUP): {
            switch(ea.getKey()) {
                case 'w':
                    movement.z() = 0.0;
                    break;
                case 's':
                    movement.z() = 0.0;
                    break;
                case 'a':
                    movement.x() = 0.0;
                    break;
                case 'd':
                    movement.x() = 0.0;
                    break;
                default:
                    break;
            }
        }
    }

    camPos += camRotation * movement;
    camPos.z() = zPos;

    // Check if move is valid and adjust camera position accordingly
    if (checkMove(camPos) && isMoved) {
        matrix.setTrans(correctDistanceFromWalls(camPos));
    }

    // Apply the modified matrix to the viewer's camera manipulator
    viewer->getCameraManipulator()->setByMatrix(matrix);

    return true;
}

void CameraController::setInitCamPos(int x, int y) {
    // Set initial camera position with adjusted center to avoid looking at walls
    std::pair<int, int> initCenter_xy = getValidInitCenterPos(x, y);

    setHomePosition(
        osg::Vec3d(static_cast<float>(x), static_cast<float>(y), zPos), 
        osg::Vec3d(static_cast<float>(initCenter_xy.first), static_cast<float>(initCenter_xy.second), zPos), 
        osg::Vec3d(0.0, 0.0, 1.0)
    );
}

std::pair<int, int> CameraController::getValidInitCenterPos(int x, int y) {
    std::pair<int, int> validIndices(x, y);

    // Get valid center position to avoid looking at the wall
    if (x-1 >= 0 && mazeVec[x-1][y] != WALL) {
        validIndices.first = x-1;
    } else if (y-1 >= 0 && mazeVec[x][y-1] != WALL) {
        validIndices.second = y-1;
    } else if (x+1 < mazeVec.size() && mazeVec[x+1][y] != WALL) {
        validIndices.first = x+1;
    } else if (y+1 < mazeVec[0].size() && mazeVec[x][y+1] != WALL) {
        validIndices.second = y+1;
    }

    return validIndices;
}

int CameraController::getMazeIndex(float val) {
    return static_cast<int>(std::round(val));
}

bool CameraController::checkMove(osg::Vec3d pos) {
    int x = getMazeIndex(pos.x());
    int y = getMazeIndex(pos.y());

    // Check if the move is within maze bounds and doesn't hit a wall
    if (x >= 0 && x < mazeVec.size() && y >= 0 && y < mazeVec[0].size()) {
        if (mazeVec[x][y] == WALL) {
            return false; // Invalid move -> wall
        } else {
            return true; // Valid move -> no obstruction
        }
    } else {
        return false; // Invalid move -> outside maze bounds
    }
}

float CameraController::stepBackFromWall(float pos) {
    float decimalPart = fmod(pos, 1.0);
    float floorPos = floor(pos);

    // Adjust position to step back from the wall based on the predefined distance
    if (decimalPart >= 0.5 && decimalPart <= 0.5+distanceFromWall) {
        pos = floorPos + 0.5 + distanceFromWall;
    } else if (decimalPart >= 0.0 && decimalPart <= distanceFromWall) {
        pos = floorPos + distanceFromWall;
    } else if (decimalPart >= 1.0 - distanceFromWall) {
        pos = ceil(pos) - distanceFromWall;
    } else if (decimalPart <= 0.5 && decimalPart >= 0.5 - distanceFromWall) {
        pos = floorPos + 0.5 - distanceFromWall;
    }

    return pos;
}

osg::Vec3 CameraController::correctDistanceFromWalls(osg::Vec3 pos) {
    osg::Vec3 camPosCopy(pos);
    float checkDistance = distanceFromWall + 0.01;

    // Lambda function to check and adjust the camera position along a coordinate axis
    auto checkAndAdjust = [&](float& camPosCoord, float& camPosCopyCoord, const std::function<void(float&)>& adjustFunc) {
        camPosCopyCoord += checkDistance;
        if (!checkMove(camPosCopy)) {
            adjustFunc(camPosCoord);
        }

        camPosCopyCoord -= 2 * checkDistance;
        if (!checkMove(camPosCopy)) {
            adjustFunc(camPosCoord);
            camPosCopyCoord += checkDistance;
        }
    };

    // Check and adjust along the X-axis and Y-axis
    checkAndAdjust(pos.x(), camPosCopy.x(), [&](float& pos) { pos = stepBackFromWall(pos); });
    checkAndAdjust(pos.y(), camPosCopy.y(), [&](float& pos) { pos = stepBackFromWall(pos); });

    return pos;
}

bool CameraController::checkIfFinsihed(osg::Vec3d pos) {
    int x = getMazeIndex(pos.x());
    int y = getMazeIndex(pos.y());

    if (mazeVec[x][y] == END) {
        foundExitFlag = true;
        return true;
    } else {
        return false;
    }
}

bool CameraController::checkExit() {
    return foundExitFlag;
}