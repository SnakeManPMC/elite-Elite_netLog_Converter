#include "netlogconvert.h"
#include "ui_netlogconvert.h"

#include <QFileDialog>
#include <QDir>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QTimeZone>
#include <QTextStream>

netLogConvert::netLogConvert(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::netLogConvert)
{
	ui->setupUi(this);
	numberOfSystems = 0;
}

netLogConvert::~netLogConvert()
{
	delete ui;
}


void netLogConvert::on_actionBrowse_Elite_Logs_Directory_triggered()
{
	LogsDirName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
						      "",
						      QFileDialog::ShowDirsOnly
						      | QFileDialog::DontResolveSymlinks);

	ui->textEdit->append("Elite Logs directory set to:\n" + LogsDirName);
}


void netLogConvert::on_processNetLogs_clicked()
{
	// if user has not set directory to be scanned, we exit
	if (LogsDirName.isNull())
	{
		ui->textEdit->append("Sorry you did not set Elite Logs directly, please use File -> Browse Elite Logs Directory first.");
		return;
	}

	ScanNetLogs(LogsDirName);
}


void netLogConvert::ScanNetLogs(const QString& DirName)
{
	QStringList nameFilter("netLog*.log");
	QDir jDir(DirName);
	// create string list of the netLog file names, sorted by name
	QStringList netLogFiles = jDir.entryList(nameFilter, QDir::NoFilter, QDir::Name);

	// wrong dir, ERR!
	if (netLogFiles.count() == 0)
	{
		ui->textEdit->append("Directory " + DirName + " does not contain any netLog*.log files, please check what you're doing.");
		return;
	}

	// open output text file
	QFile CmdrLog("elitelog-from-netlogs.txt");

	if (!CmdrLog.open(QIODevice::WriteOnly))
	{
		qDebug() << "Unable to open elitelog-from-netlogs.txt for writing " << CmdrLog.errorString();
		return;
	}

	QTextStream out(&CmdrLog);

	// process netLogs one by one
	for (int i = 0; i < netLogFiles.count(); i++)
	{
		ui->textEdit->append("Opened: " + netLogFiles[i]);
		QString fileName = DirName + "\\" + netLogFiles[i];

		QFile file(fileName);

		if (!file.open(QIODevice::ReadOnly))
		{
			ui->textEdit->append("Unable to open netLog file: " + file.errorString());
			return;
		}

		qDebug() << "Opened fileName: " << fileName;

		// beginning of the file grab the date
		getNetLogDate(file.readLine());

		// read line by line
		while (!file.atEnd())
		{
			ParseNetLog(file.readLine(), out);
		}

		file.close();
	}
	CmdrLog.close();

	// ui shizzle
	ui->textEdit->append("All done, number of systems created: " + QString::number(numberOfSystems));
}


// get datatime from a specific string
void netLogConvert::getNetLogDate(QString line)
{
	// this is pure horror kode ;)
	line.resize(8);
	//14-10-01-03:44 FLE Daylight Time  (00:44 GMT)
	netLogDate = QDate::fromString(line,"yy-MM-dd").addYears(100);
}


// parse netlog, get system name and UTC time, send to the output file
void netLogConvert::ParseNetLog(QString line, QTextStream& cmdrlog)
{
	// check if we have new system
	QString OldSystem = MySystem;

	// if we have system, any system...
	if (line.contains("System:"))
	{
		QStringList finale;
		QString myString;

		// get the time from {hh:mm:ss} string beginning of line
		QStringList pmctime = line.split("{");
		QStringList pmcfinaltime = pmctime[1].split("}");
		QTime rocktime = QTime::fromString(pmcfinaltime[0], "hh:mm:ss");

		// if we have the v2.1 the engineers version netlog
		if (line.contains("System:\""))
		{
			// {07:31:19} System:"Shinrarta Dezhra" StarPos:(55.719,17.594,27.156)ly  NormalFlight
			QStringList parsed = line.split(QRegExp("(.*)System:\""));
			finale = parsed[1].split("\" StarPos:");
		}
		// if we have the v2.0 and older netlog
		else
		{
			// regexp reads "<ANYTHING>System:<ANY_NUMBER_OF_DIGITS>(" for match
			QStringList parsed = line.split(QRegExp("(.*)System:?\\d+\\("));
			finale = parsed[1].split(") Body");
		}

		// check if we ignore Training system (playing in training missions)
		if ( !(finale[0] == "Training" || finale[0] == "Destination") )
		{
			// get new system name
			MySystem = finale[0];
			QDateTime stardate = QDateTime(netLogDate, rocktime, Qt::UTC);
			myString = MySystem + "," + stardate.toString(Qt::ISODate);
		}

		// we have a new system!
		if (OldSystem.compare(MySystem))
		{
			cmdrlog << myString << "\n";
			// increase number of systems for UI stats
			numberOfSystems++;
		}
	}
}
