struct Person {
    glm::vec3 position;
    glm::vec3 rotation;  // Euler angles (pitch, yaw, roll)
    float scale;
    int targetRow;
    int targetCol;

    // Animacione promenljive
    enum State {
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

    // Putanja kretanja
    std::vector<glm::vec3> pathPoints;
    int currentPathPoint;

    Person() : position(0.0f), rotation(0.0f), scale(1.0f),
        targetRow(-1), targetCol(-1), state(ENTERING),
        animationTime(0.0f), currentPathPoint(0) {}
};