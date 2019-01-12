#ifndef TRIGRAM_COUNTER_H
#define TRIGRAM_COUNTER_H

#include <my_file.h>
#include <QVector>
#include <QObject>
#include <QFileSystemWatcher>

static const int MAX_TRIGRAMS = 20000;
const int BUFFER_SZ = 10 * 1024 * 1024;

class trigram_counter : public QObject
{

Q_OBJECT

public:
    trigram_counter() = default;
    trigram_counter(QString t);
public slots:
    void find_substring_directory(QString);
    void process_directory();
    void process_file(my_file &file);
    void prepare(QString dir);
    void update_file(const QString &);
signals:
    //void send_status(QString const &);
    //void send_progress(qint16);
    void send_files(my_file);
    void send_index(QString, int);
    void send_status(qint16 x);
    void finish();
    void finish1(int);


private:
    int is_file(my_file& filename, QString& pattern);
    int check(QSet<int64_t> &set, my_file &t, QString &pattern);
    void get_trigram(QString &str, my_file &t);
    qint16 get_percent();
    //qint32 get_hash(const QChar &a, const QChar &b, const QChar &c);
    QString dir;
    int total = 0;
    int cur = 0;
    bool is_stopped;
    QVector<my_file> my_files;
    QFileSystemWatcher scanner;
};

#endif // TRIGRAM_COUNTER_H
