#ifndef NETLOGCONVERT_H
#define NETLOGCONVERT_H

#include <QMainWindow>
#include <QDateTime>
#include <QTextStream>

namespace Ui {
class netLogConvert;
}

class netLogConvert : public QMainWindow
{
	Q_OBJECT

public:
	explicit netLogConvert(QWidget *parent = 0);
	~netLogConvert();

private slots:
	void on_actionBrowse_Elite_Logs_Directory_triggered();

	void on_processNetLogs_clicked();

private:
	Ui::netLogConvert *ui;
	QString LogsDirName,MySystem;
	QDate netLogDate;
	void ScanNetLogs(const QString& DirName);
	void ParseNetLog(QString line, QTextStream& cmdrlog);
	void getNetLogDate(QString line);
	int numberOfSystems;
};

#endif // NETLOGCONVERT_H
