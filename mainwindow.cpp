#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <qdiriterator.h>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);


    qRegisterMetaType<QString>("QString");
    qRegisterMetaType<QVector<my_file>>("QVector<my_file>");
    QCommonStyle style;
    ui->actionIndex_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionIndex_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &main_window::searching);
    connect(ui->actionCancel, SIGNAL(released()), SLOT(stop_search()));

    qRegisterMetaType<my_file>("my_file");
    ui->lineEdit->setHidden(true);
    ui->label->setHidden(true);
    ui->label_2->setHidden(true);
    ui->actionCancel->setHidden(true);
    ui->progressBar->setHidden(true);
    //scan_directory(QDir::homePath());
}

main_window::~main_window()
{
    Thread->quit();
    Thread->wait();
}

void main_window::select_directory()
{
    dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.length() == 0){
        return;
    }
    count = 0;
    t.start();
    ui->label_2->setHidden(true);
    ui->treeWidget->clear();
    ui->actionIndex_Directory->setDisabled(true);
    ui->lineEdit->setHidden(true);
    ui->label->setHidden(true);
    ui->actionCancel->setHidden(false);
    ui->progressBar->setHidden(false);
    ui->progressBar->setValue(1);
    if (f){
        Thread = new QThread();
        counter = new trigram_counter(dir);
        counter->moveToThread(Thread);
        connect(Thread, &QThread::finished, Thread, &QObject::deleteLater);
        connect(this, &main_window::startt, counter, &trigram_counter::process_directory);
        //connect(this, &main_window::start_indexing, counter, &trigram_counter::process_directory);
        connect(counter, SIGNAL(finish1(int)),
                this,
                SLOT(onFinish1(int)));
        connect(counter, SIGNAL(send_status(qint16)), this, SLOT(show_status(qint16)));

        //emit start_indexing(dir);
        Thread->start();
        emit startt();
        f = false;
    }
    else{
        counter->prepare(dir);
        emit startt();
    }
}


void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}



void main_window::searching()
{
    t.start();
    count = 0;
    QString pattern = ui->lineEdit->text();
    ui->label_2->setHidden(true);
    //Thread = new QThread();
    ui->treeWidget->clear();
    ui->actionIndex_Directory->setDisabled(true);
    ui->actionCancel->setHidden(false);
    ui->progressBar->setValue(1);
    ui->treeWidget->clear();
    //counter->moveToThread(Thread);
    if (is_first_searching){
        connect(Thread, &QThread::finished, Thread, &QObject::deleteLater);
        connect(this, &main_window::start_indexing, counter, &trigram_counter::find_substring_directory);
        connect(counter, SIGNAL(finish()),
                this,
                SLOT(onFinish()));
        connect(counter,
                             SIGNAL(send_index(QString, int)),
                             this,
                             SLOT(show_index(QString, int)));
        connect(counter, SIGNAL(send_status(qint16)), this, SLOT(show_status(qint16)));
        is_first_searching = false;
    }
    //Thread->start();
    emit start_indexing(pattern);
}



void main_window::onFinish()
{
    ui->label_2->setText("Finished in " + QString::number(t.elapsed()) + " ms. Found " + QString::number(count) + " files");
    ui->label_2->setHidden(false);
    ui->label->setHidden(false);
    ui->lineEdit->setHidden(false);
    ui->actionIndex_Directory->setDisabled(false);
    ui->actionCancel->setHidden(true);
    //Thread->quit();
    //Thread->wait();
}

void main_window::show_index(QString filename, int ind)
{
    count++;
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, filename);
    item->setText(1, QString::number(ind));
    ui->treeWidget->addTopLevelItem(item);
}

void main_window::stop_search()
{
    Thread->requestInterruption();
}

void main_window::show_status(qint16 x)
{
    ui->progressBar->setValue(x);
}

void main_window::onFinish1(int sz)
{
    //count = q.size();
    //files = q;
    //for (int i = 0; i < files.size(); i++)
        //scanner.addPath(files[i].filename);
    ui->label_2->setText("Finished in " + QString::number(t.elapsed()) + " ms. Found " + QString::number(sz) + "  ");
    ui->label_2->setHidden(false);
    ui->label->setHidden(false);
    ui->lineEdit->setHidden(false);
    ui->actionIndex_Directory->setDisabled(false);
    ui->actionCancel->setHidden(true);
    //Thread->quit();
    //Thread->wait();
}
