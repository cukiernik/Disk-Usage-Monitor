#include <QApplication>
#include "DiskUsageMonitor.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Utwórz i wyświetl monitor dysku
    DiskUsageMonitor monitor;
    monitor.show();

    return app.exec();
}
