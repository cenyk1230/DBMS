#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->exec, SIGNAL(clicked()), this, SLOT(handleExec()));
    connect(ui->execFile, SIGNAL(clicked(bool)), this, SLOT(handleFileExec()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleExec(){
    ui->exec->setEnabled(false);
    ui->execFile->setEnabled(false);
    QString inputStr = ui->textInput->toPlainText();
    QFile fout("temp.swap");
    fout.open(QIODevice::WriteOnly);
    QTextStream out(&fout);
    out << inputStr << endl;
    fout.close();
    system("./dbms temp.swap 2>err.swap 1>out.swap");
    QFile output("out.swap");
    QFile log("err.swap");
    output.open(QIODevice::ReadOnly);
    QString res, logres;
    QTextStream readres(&output);
    res = readres.readAll();
    output.close();
    ui->textOutput->setText(res);
    log.open(QIODevice::ReadOnly);
    QTextStream readlogres(&log);
    logres = readlogres.readAll();
    log.close();
    ui->textLog->setText(logres);

    ui->exec->setEnabled(true);
    ui->execFile->setEnabled(true);
}

void MainWindow::handleFileExec() {
    ui->exec->setEnabled(false);
    ui->execFile->setEnabled(false);
    QString inputStr = ui->fileName->text();
    system(("./dbms " + inputStr.toStdString() + " 2>err.swap 1>out.swap").c_str());
    QFile output("out.swap");
    QFile log("err.swap");
    output.open(QIODevice::ReadOnly);
    QString res, logres;
    QTextStream readres(&output);
    res = readres.readAll();
    output.close();
    ui->textOutput->setText(res);
    log.open(QIODevice::ReadOnly);
    QTextStream readlogres(&log);
    logres = readlogres.readAll();
    log.close();
    ui->textLog->setText(logres);
    
    ui->exec->setEnabled(true);
    ui->execFile->setEnabled(true);
}
