#include "mainwindow.h"
#include "serviceclass.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, _host(nullptr)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_OpenButton_clicked()
{
	if (this->_host == nullptr) {
		this->_host = new QTobubusHost(ui->Pipe->text());
		this->_host->publish("/host", QSharedPointer<QObject>(new ServiceClass));
		this->_host->listen();
		ui->OpenButton->setText(tr("Close"));
		ui->Log->appendPlainText(tr("Publish service at /host"));
		ui->Log->appendPlainText(tr("Open at ") + ui->Pipe->text());
		connect(this->_host, &QTobubusHost::clientConnected,
			[this](QString id) { this->ui->Log->appendPlainText(tr("Connected from ") + id); });
		connect(this->_host, &QTobubusHost::objectPublished, [this](QString id, QString path) {
			this->ui->Log->appendPlainText(tr("Object published at ") + path + tr(" by ") + id);
		});
		connect(this->_host, &QTobubusHost::methodCalled,
			[this](QString id, QString path, QString method, QVariantList params) {
				this->ui->Log->appendPlainText(
					tr("Method called from ") + id + " " + path + "#" + method);
			});
	} else {
		this->_host->close();
		delete this->_host;
		this->_host = nullptr;
		ui->OpenButton->setText(tr("Open"));
		ui->Log->appendPlainText(tr("Close host"));
	}
	bool enable = (this->_host != nullptr);
	ui->Method->setEnabled(enable);
	ui->Path->setEnabled(enable);
	ui->Parameter->setEnabled(enable);
	ui->SendButton->setEnabled(enable);
}

void MainWindow::on_SendButton_clicked()
{
	QJsonDocument document = QJsonDocument::fromJson(ui->Parameter->toPlainText().toUtf8());
	if (document.isArray()) {
		auto array = document.array();
		Call::Status status;
		ui->Log->appendPlainText(tr("Send Method: ") + ui->Path->text() + "#" + ui->Method->text());
		QVariant result = this->_host->apply(
			ui->Path->text(), ui->Method->text(), &status, array.toVariantList());

		QString output;
		QDebug(&output) << status;

		qDebug() << status;
		qDebug() << result;
		ui->Log->appendPlainText(tr("Status: ") + output);
		ui->Log->appendPlainText(tr("Result: ") + QString(result.toJsonDocument().toJson()));
	} else {
		ui->Log->appendPlainText(tr("Parameter is not valid array."));
	}
}
