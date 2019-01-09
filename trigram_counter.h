#ifndef TRIGRAM_COUNTER_H
#define TRIGRAM_COUNTER_H

#include <my_file.h>
#include <QObject>

static const int MAX_TRIGRAMS = 20000;
const int BUFFER_SZ = 10 * 1024 * 1024;

class trigram_counter : public QObject
{

Q_OBJECT

public:
    trigram_counter() = default;
    trigram_counter(QString t);
    void process_file(my_file &file);
    int is_file(my_file& filename, QString& pattern);
    int check(QSet<quint64> &set, my_file &t, QString &pattern);
public slots:
    void find_substring_directory(QString  dpattern, QVector<my_file> files);
    void process_directory();
signals:
    //void send_status(QString const &);
    //void send_progress(qint16);
    void send_files(my_file);
    void send_index(QString, int);
    void finish();


private:
    void get_trigram(QString &str, my_file &t);
    quint64 get_hash(const quint64& a, const quint64& b, const quint64& c);
    QString dir;
};

#endif // TRIGRAM_COUNTER_H
