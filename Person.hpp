struct Person {
    glm::vec3 position;
    glm::vec3 rotation;
    float scale;
    int targetRow;
    int targetCol;
    int modelIndex;  // DODATO - koji model koristi (0-14)

    enum State {
        WAITING_TO_ENTER,  // DODATO - ?eka da u?e
        ENTERING,
        WALKING_TO_STAIRS,
        CLIMBING_STAIRS,
        WALKING_TO_SEAT,
        TURNING_TO_SCREEN,
        SITTING,
        STANDING_UP,
        LEAVING
    };
    State state;
    float animationTime;
    std::vector<glm::vec3> pathPoints;
    int currentPathPoint;

    Person() : position(0.0f), rotation(0.0f), scale(1.0f),
        targetRow(-1), targetCol(-1), modelIndex(0),
        state(WAITING_TO_ENTER), animationTime(0.0f),
        currentPathPoint(0) {}
};