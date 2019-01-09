#ifndef MY_FILE_H
#define MY_FILE_H
#include "QString"
#include "QSet"


class my_file
{
public:
    my_file();
    my_file(QString name);
    QString get_filename();
    QSet<quint64> trigrams;
    bool is_good;
    QString filename;
};

#endif // MY_FILE_H
