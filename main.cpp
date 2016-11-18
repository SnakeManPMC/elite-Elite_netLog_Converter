#include "netlogconvert.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	netLogConvert w;
	w.show();

	return a.exec();
}
