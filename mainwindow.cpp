
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
    connect(&scanner, &QFileSystemWatcher::fileChanged, this, &main_window::update_file);
    qRegisterMetaType<my_file>("my_file");
    ui->lineEdit->setHidden(true);
    ui->label->setHidden(true);
    ui->actionCancel->setHidden(true);
    //scan_directory(QDir::homePath());
}

main_window::~main_window()
{}

void main_window::select_directory()
{
    dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.length() == 0){
        return;
    }
    ui->treeWidget->clear();
    ui->actionIndex_Directory->setDisabled(true);
    ui->lineEdit->setHidden(true);
    ui->label->setHidden(true);
    scanner.removePaths(scanner.directories());
    QThread *Thread = new QThread();
    trigram_counter *counter = new trigram_counter(dir);
    counter->moveToThread(Thread);
    files.clear();
    connect(Thread, &QThread::finished, Thread, &QObject::deleteLater);
    connect(Thread, &QThread::started, counter, &trigram_counter::process_directory);
    //connect(this, &main_window::start_indexing, counter, &trigram_counter::process_directory);
    connect(counter,
                         SIGNAL(send_files(my_file)),
                         this,
                         SLOT(show_files(my_file)));
    connect(counter, SIGNAL(finish()),
            this,
            SLOT(onFinish()));
    //emit start_indexing(dir);
    Thread->start();
}


void main_window::show_about_dialog()
{
    QMessageBox::aboutQt(this);
}



void main_window::searching()
{
    QString pattern = ui->lineEdit->text();
    QThread *Thread = new QThread();
    trigram_counter *counter = new trigram_counter(dir);
    ui->treeWidget->clear();
    ui->actionIndex_Directory->setDisabled(true);
    //ui->lineEdit->setHidden(true);
    //ui->label->setHidden(true);

    ui->treeWidget->clear();
    counter->moveToThread(Thread);
    connect(Thread, &QThread::finished, Thread, &QObject::deleteLater);
    connect(this, &main_window::start_indexing, counter, &trigram_counter::find_substring_directory);
    connect(counter, SIGNAL(finish()),
            this,
            SLOT(onFinish()));
    connect(counter,
                         SIGNAL(send_index(QString, int)),
                         this,
                         SLOT(show_index(QString, int)));
    Thread->start();
    emit start_indexing(pattern, files);
}

void main_window::update_file(const QString &filename)
{
    QFile fileinfo(filename);
    int ind = -1;
    for (int i = 0; i < files.size(); i++){
        if (files[i].filename == filename){
            ind = i;
            break;
        }
    }
    if (!fileinfo.exists() && ind != -1){
        files.remove(ind);
        scanner.removePath(filename);
    }
    trigram_counter t;
    if (fileinfo.exists()){
        my_file cur = my_file(filename);
        t.process_file(cur);
        if (ind == -1 && cur.is_good)
            files.push_back(cur);
        else{
            if (!cur.is_good){
                if (ind != -1)
                    files.remove(ind);
            }
            else{
                if (cur.is_good && ind != -1){
                    files.remove(ind);
                    files.push_back(cur);
                }
            }

        }

    }
}

void main_window::show_files(my_file t)
{
    files.push_back(t);
    scanner.addPath(t.filename);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, t.filename);
    item->setText(1, QString::number(t.trigrams.size()));
    ui->treeWidget->addTopLevelItem(item);
}

void main_window::onFinish()
{
    ui->label->setHidden(false);
    ui->lineEdit->setHidden(false);
    ui->actionIndex_Directory->setDisabled(false);
}

void main_window::show_index(QString filename, int ind)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, filename);
    item->setText(1, QString::number(ind));
    ui->treeWidget->addTopLevelItem(item);
}
