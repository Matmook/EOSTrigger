#include "viewform.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("Matmook");
    a.setApplicationName("EOSTrigger");

    ViewForm v;

    if (!v.started())
    {
        //return 0;
    }
    v.show();

    return a.exec();
}
