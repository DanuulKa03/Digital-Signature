#include <QCoreApplication>
#include "src/controller/controller.h"
#include "src/model/model.h"
#include "src/view/view.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    Model model;
    View view;
    Controller controller(&model, &view);

    controller.run();

    return app.exec();
}