#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QThread>
#include <memory>
#include "trigram_counter.h"
#include <QTime>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT
    QThread *Thread;
public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void select_directory();
    //void index_directory(QString const& dir);
    void show_about_dialog();
    void searching();
    //void update_directory(const QString &);
    void update_file(const QString &);
    void show_files(my_file);
    void onFinish();
    void show_index(QString, int);
    void stop_search();
    void show_status(qint16 x);
    void onFinish1(QVector<my_file>);
signals:
    void start_indexing(QString, QVector<my_file>);
private:
    QString dir;
    std::unique_ptr<Ui::MainWindow> ui;
    QVector<my_file> files;
    QTime t;
    int count;
    QFileSystemWatcher scanner;
    //trigram_counter *counter = nullptr;
};

#endif // MAINWINDOW_H
