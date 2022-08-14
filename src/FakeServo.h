
/// This modules emulates a servo mechanism. It provides an comms interface to operate it and internally a model is 
/// executed to simulate its behaviour.
class FakeServo {

    /*************************************************************************/
    /********** PUBLIC TYPES *************************************************/
    /*************************************************************************/

    struct Config {
        double dt;          ///< Execution frequency of the model and controller.
    }

    struct Options {
        double positionKp;
        double positionKi;
        double positionKd;
        double positionTd;
        double rateLimit;
        double rateKp;
        double rateKi;
        double rateKd;
        double rateTd;
        double forceLimit;

    }

    struct ModelOptions {
        double mass;
        double stiffness;
        double damping;
        double staticFriction;
        double coloumbFriction;
        double stribeckFriction;
    }

    struct ModelSignals {
        double position;
        double rate;
    }

    struct Command {
        double positionSetpoint;
        double openLoopForce;
    }

    struct ServoMode {
        
    }

    /*************************************************************************/
    /********** PUBLIC FUNCTIONS *********************************************/
    /*************************************************************************/

    void init() {

    }

    void servoCommand() {

    }

    void servoOptions() {

    }

    void servoData() {
    }

    void servoMode() {

    }

    void service() {
        
    }

    void model() {

    }

    /*************************************************************************/
    /********** PRIVATE VARIABLES ********************************************/
    /*************************************************************************/

};