#ifndef NTRU_CONTROLLER_H
#define NTRU_CONTROLLER_H

#include "src/model/model.h"
#include "src/view/view.h"

/**
 * Controller class that orchestrates user input, model operations, and view output.
 */
class NtruController {
public:
    NtruController();
    // Run the main control loop
    void run();

private:
    ConsoleView view;
    NtruParams params;
    bool paramsLoaded;  // flag to indicate if parameters have been loaded

    // Helper to handle key generation workflow
    void handleKeyGeneration();
    // Helper to handle signing workflow
    static void handleSigning();
    // Helper to handle verification workflow
    static void handleVerification() ;
};

#endif // NTRU_CONTROLLER_H
