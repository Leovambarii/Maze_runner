#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osgText/Text>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osg/PositionAttitudeTransform>

#include <random>
#include <chrono>
#include <thread>

#include "MazeGenerator.h"
#include "CameraController.h"

// g++ -o osg Main.cpp MazeGenerator.cpp CameraController.cpp -losgViewer -losgDB -losg -losgGA -losgUtil -lOpenThreads
int MAZE_SIZE = 25; // Default maze size
#define MAX_MAZE 50 // Maximum maze size
#define START_X 1 // Starting x-cordinate of player in the maze
#define START_Y 1 // Starting y-cordinate of player in the maze

#define WINDOW_X 700
#define WINDOW_Y 100
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define END_SCREEN_TIME 3 // Amount of seconds to display end screen before exiting

// Structure to hold the result of maze generation
struct MazeCreationResult {
    osg::Node* mazeNode; // Root node of the maze
    std::vector<std::vector<CellType>> mazeVec; // Maze data represented as a 2D vector
};

// Texture weights for random texture selection
std::vector<std::pair<std::string, int>> textureWeights = {
    {"assets/cobbled_deepslate.png", 200},
    {"assets/deepslate.png", 400},
    {"assets/deepslate_coal_ore.png", 60},
    {"assets/deepslate_copper_ore.png", 40},
    {"assets/deepslate_iron_ore.png", 30},
    {"assets/deepslate_gold_ore.png", 20},
    {"assets/deepslate_lapis_ore.png", 15},
    {"assets/deepslate_redstone_ore.png", 10},
    {"assets/deepslate_emerald_ore.png", 7},
};

#define MAZE_END_TEXTURE "assets/deepslate_diamond_ore.png"

/**
 * @brief Function to get a random texture based on weights.
 *
 * @param textureWeights Vector of texture-weight pairs.
 * @return std::string - Randomly selected texture path.
 */
std::string getRandomTexture(const std::vector<std::pair<std::string, int>>& textureWeights) {
    int totalWeight = 0;
    for (const auto& tex : textureWeights) {
        totalWeight += tex.second;
    }

    int randomNumber = std::rand() % totalWeight + 1;
    int cumulativeWeight = 0;
    for (const auto& tex : textureWeights) {
        cumulativeWeight += tex.second;
        if (randomNumber <= cumulativeWeight) {
            return tex.first;
        }
    }

    return "";
}

/**
 * @brief Creates a floor geometry with the specified texture.
 *
 * @param texturePath Path to the texture for the floor.
 * @return osg::Geode* - Pointer to the created floor geometry.
 */
osg::Geode* createFloor(std::string texturePath) {
    static osg::ref_ptr<osg::Geode> floorPlane;
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage(osgDB::readImageFile(texturePath));
    
    osg::ref_ptr<osg::Drawable> drawable = osg::createTexturedQuadGeometry(osg::Vec3(-0.5f,-0.5f, 0.0f), osg::X_AXIS, osg::Y_AXIS );
    drawable->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
    
    floorPlane = new osg::Geode;
    floorPlane->addDrawable(drawable.get());

    return floorPlane.get();
}

/**
 * @brief Creates a wall geometry with the specified texture.
 *
 * @param texturePath Path to the texture for the wall.
 * @return osg::Geode* - Pointer to the created wall geometry.
 */
osg::Geode* createWall(std::string texturePath) {
    static osg::ref_ptr<osg::Geode> wallBox;
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( osgDB::readImageFile(texturePath));
    
    osg::ref_ptr<osg::Drawable> drawable = new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, 0.5f), 1.0f));
    drawable->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
    
    wallBox = new osg::Geode;
    wallBox->addDrawable(drawable.get());

    return wallBox.get();
}

/**
 * @brief Creates a maze and returns the result.
 *
 * @param size Size of the maze.
 * @param x X-coordinate of the starting point.
 * @param y Y-coordinate of the starting point.
 * @return MazeCreationResult - Result of maze creation, including root node and maze data.
 */
MazeCreationResult createMaze(int size, int x, int y) {
    MazeGenerator maze(size, x, y);
    std::vector<std::vector<CellType>> mazeVec = maze.getMaze();

    osg::ref_ptr<osg::Group> scn = new osg::Group;
    for (int i=0; i<size; i++) {
        for (int j=0; j<size; j++) {
            osg::ref_ptr<osg::MatrixTransform> t = new osg::MatrixTransform;
            t->setMatrix(osg::Matrix::translate(i, j, 0.0f));
            scn->addChild(t.get());
            if (mazeVec[i][j] == WALL) {
                t->addChild(createWall(getRandomTexture(textureWeights)));
            } else if (mazeVec[i][j] == END) {
                t->addChild(createWall(MAZE_END_TEXTURE));
            } else {
                t->addChild(createFloor(getRandomTexture(textureWeights)));
            }
        }
    }
    MazeCreationResult result;
    result.mazeNode = scn.release();
    result.mazeVec = mazeVec;

    return result;
}

/**
 * @brief Sets up the viewer with the specified root, controller, and maze result.
 *
 * @param viewer osgViewer::Viewer reference.
 * @param root Root node of the scene.
 * @param controller Reference to the camera controller.
 * @param maze MazeCreationResult& - Result of maze creation.
 */
void setupViewer(osgViewer::Viewer& viewer, osg::Group* root, CameraController*& controller, MazeCreationResult& maze) {
    viewer.setSceneData(root);
    viewer.setUpViewInWindow(WINDOW_X, WINDOW_Y, WINDOW_WIDTH, WINDOW_HEIGHT);

    controller = new CameraController(&viewer, maze.mazeVec);
    controller->setInitCamPos(START_X, START_Y);
    viewer.setCameraManipulator(controller);
    viewer.realize();
}

/**
 * @brief Switches the viewer to the TrackballManipulator and sets specified home position.
 *
 * @param viewer osgViewer::Viewer reference.
 */
void switchToTrackballManipulator(osgViewer::Viewer& viewer) {
    osg::ref_ptr<osgGA::TrackballManipulator> trackballManipulator = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(trackballManipulator);
    trackballManipulator->setHomePosition(  osg::Vec3d(MAZE_SIZE, -10.0, 30.0),
                                            osg::Vec3d(MAZE_SIZE / 2.0, MAZE_SIZE / 2.0, 0.0),
                                            osg::Vec3d(0.0, 0.0, 1.0));
    viewer.home();
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        MAZE_SIZE = std::max(5, std::atoi(argv[1]));
        MAZE_SIZE = std::min(MAZE_SIZE, MAX_MAZE);
        if (MAZE_SIZE % 2 == 0) {
            MAZE_SIZE += 1;
        }
    }

    srand(static_cast<unsigned>(time(nullptr)));
    std::cout << "--+> YOU NEED TO FIND THE DIAMONDS <+--\n" << std::endl;

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
    root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    MazeCreationResult maze = createMaze(MAZE_SIZE, START_X, START_Y);
    osg::ref_ptr<osg::Group> scn_1 = new osg::Group, scn_2 = new osg::Group;
    scn_1->addChild(maze.mazeNode);
    scn_2->addChild(maze.mazeNode);
    root->addChild(scn_1);

    osgViewer::Viewer viewer;
    CameraController* controller = nullptr;
    setupViewer(viewer, root, controller, maze);

    bool found = false;
    auto startTime = std::chrono::high_resolution_clock::now();
    while (!viewer.done()) {
        viewer.frame();

        // Check if the player has found the diamonds
        if (!found && controller->checkExit()) {
            std::cout << "\n--+> YOU FOUND THE DIAMONDS! <+--" << std::endl;
            found = true;
            root->replaceChild(scn_1, scn_2);
            switchToTrackballManipulator(viewer);
            startTime = std::chrono::high_resolution_clock::now();
        }

        // Check if the end screen time has elapsed
        if (found) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime-startTime).count();
            if (duration >= END_SCREEN_TIME) {
                viewer.setDone(true);
            }
        }
    }

    return 0;
}
