#include "trigram_counter.h"

#include <qdiriterator.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qvector.h>
#include <QThread>

void trigram_counter::process_directory(){
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while(it.hasNext()){
        my_file cur = my_file(it.next());
        process_file(cur);
        if (cur.is_good)
            emit send_files(cur);
        if (QThread::currentThread()->isInterruptionRequested()){
            emit finish();
            return;
        }
    }
    emit finish();
}

void trigram_counter::find_substring_directory(QString pattern, QVector<my_file> files){
    QVector<quint64> my_vector;
    QSet<quint64> trigrams;
    for (int i = 0; i < pattern.size(); i++){
        my_vector.push_back(pattern[i].unicode());
    }
    for (int i = 2; i < my_vector.size(); i++)
        trigrams.insert(get_hash(my_vector[i - 2], my_vector[i - 1], my_vector[i]));
    for (auto it : files){
        if (QThread::currentThread()->isInterruptionRequested()){
            emit finish();
            return;
        }
        int v = check(trigrams, it, pattern);
        if (v != -1)
            emit send_index(it.filename, v);
    }
    emit finish();

}

int trigram_counter::check(QSet<quint64> &set, my_file &t, QString &pattern){
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
    bool is_first_line = true;
    QChar predlast = -1;
    QChar last = -1;
    bool can_we = true;


    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString str;
        if (!is_first_line && can_we){
            str += predlast;
            str += last;
        }
        else{
            if (!can_we)
                is_first_line = false;
        }
        str += in.readLine();
        get_trigram(str, t);
        if (t.trigrams.size() > MAX_TRIGRAMS)
        {
            t.is_good = false;
            t.trigrams.clear();
            return;
        }
        if (str.size() > 2){
            can_we = true;
            last = str[str.size()-1];
            predlast = str[str.size() - 2];
        }
        else{
            can_we = false;
        }
    }

}

void trigram_counter::get_trigram(QString &str, my_file &t)
{
    QVector<quint64> my_vector;
    for (int i = 0; i < str.size(); i++){
        my_vector.push_back(str[i].unicode());
    }
    for (int i = 2; i < my_vector.size(); i++)
        t.trigrams.insert(get_hash(my_vector[i - 2], my_vector[i - 1], my_vector[i]));
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
