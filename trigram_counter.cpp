#include "trigram_counter.h"

#include <qdiriterator.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvector.h>
#include <QThread>

void trigram_counter::process_directory(){
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    my_files.clear();
    QVector<my_file> files;
    total = 0;
    cur = 0;
    while(it.hasNext()){
        files.push_back(my_file(it.next()));
    }
    total = files.size();
    int ind = 0;
    for (int i = 0; i < files.size(); i++){
        ind++;
        process_file(files[i]);
        cur++;
        if (ind % 10 == 0)
            send_status(get_percent());
        if (files[i].is_good)
            my_files.push_back(files[i]);
        if (QThread::currentThread()->isInterruptionRequested()){
            emit send_status(get_percent());
            emit finish1(my_files);
            return;
        }
    }
    emit send_status(100);
    emit finish1(my_files);
}
void trigram_counter::find_substring_directory(QString pattern, QVector<my_file> files){
    QSet<int64_t> trigrams;

    my_file v;
    get_trigram(pattern, v);
    QSet<int64_t> set1 = v.trigrams;
    total = files.size();
    cur = 0;
    int ind = 0;
    for (auto it : files){
        ind++;
        if (QThread::currentThread()->isInterruptionRequested()){
            emit send_status(get_percent());
            emit finish();
            return;
        }
        int v = check(set1, it, pattern);
        cur++;
        if (ind % 10 == 0)
            send_status(get_percent());
        if (v != -1)
            emit send_index(it.filename, v);
    }
    emit send_status(100);
    emit finish();

}

int trigram_counter::check(QSet<int64_t> &set, my_file &t, QString &pattern){
    for (auto it : set){
        if (t.trigrams.find(it) == t.trigrams.end()){
            return -1;
        }
    }
    int q = is_file(t, pattern);
    return q;
}


trigram_counter::trigram_counter(QString t)
{
    dir = t;
}

void trigram_counter::process_file(my_file &t)
{
    QFile file(t.get_filename());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        t.is_good = false;
        return;
    }


    QTextStream in(&file);
    QString buffer;
    while (buffer.append(in.read(BUFFER_SZ)).size() >= 3) {
       QString cop = "";
       get_trigram(buffer, t);
       if (t.trigrams.size() > MAX_TRIGRAMS)
       {
           t.is_good = false;
           t.trigrams.clear();
           return;
       }
       for (int i = buffer.size() - 3 + 1; i < buffer.size(); i++){
           cop += buffer[i];
       }
       buffer.clear();
       buffer += cop;
    }
}

void trigram_counter::get_trigram(QString &str, my_file &t)
{
    if (str.size() < 3)
        return;
    auto data = str.data();
    int64_t tri = (int64_t(data[1].unicode()) << 32) + (int64_t(data[0].unicode()) << 16);
    for (int i = 2; i < str.size(); i++){
        tri = (tri >> 16) + (int64_t(data[i].unicode()) << 32);
        if (t.trigrams.size() > MAX_TRIGRAMS)
            break;
        t.trigrams.insert(tri);
    }
}

qint16 trigram_counter::get_percent()
{
    if (total == 0)
        return 100;
    double x = static_cast<double>(((1.0) * cur) / total);
    return static_cast<qint16>(100 * x);
}

int trigram_counter::is_file(my_file& filename, QString& pattern)
{
    QFile fileinfo(filename.get_filename());
    if(!fileinfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        fileinfo.close();
        return -1;
    }
    QTextStream stream(&fileinfo);
    QString buffer;
    while (buffer.append(stream.read(BUFFER_SZ)).size() >= pattern.size()) {
       for (int i = 0; i <= BUFFER_SZ - pattern.size(); i++) {
           int j = 0;
           for (; pattern[j] == buffer[i + j] && j < pattern.size(); j++);
           if (j == pattern.size()) {
               return i;
           }
       }
       QString cop = "";
       for (int i = buffer.size() - pattern.size() + 1; i < buffer.size(); i++){
           cop += buffer[i];
       }
       buffer.clear();
       buffer += cop;
    }
    fileinfo.close();
    return -1;

}


quint64 trigram_counter::get_hash(const quint64 &a, const quint64 &b, const quint64 &c)
{
    return ((a << 32) | (b << 16) | (c));
}
