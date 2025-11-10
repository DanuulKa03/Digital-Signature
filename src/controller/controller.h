#pragma once
#include <QtCore/QString>

class NtruController {
public:
    NtruController();
    void run();

private:
    bool handleKeyGeneration();
    bool handleSigning();
    bool handleVerification();
};
