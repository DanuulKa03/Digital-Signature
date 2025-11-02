#include <QCoreApplication>
#include "src/controller/controller.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Initialize and run the NTRUSign controller (console interface)
    NtruController controller;
    controller.run();

    return 0;
}
