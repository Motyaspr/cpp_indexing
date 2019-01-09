#include "my_file.h"
#include "QString"

my_file::my_file()
{
    is_good = true;
}

my_file::my_file(QString name)
{
    filename = name;
    is_good = true;
}

QString my_file::get_filename()
{
    return filename;
}
