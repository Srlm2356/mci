#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QDirIterator>
#include <QDebug>
#include <QDataStream>

QTextStream cout(stdout);
QTextStream cin(stdin);

// Функция для поиска позиций пакетов
QList<int> indexesOf(QByteArray barr, QByteArray sep)
{
    QList <int> indexes;
    int pos = 0;
    while (pos != -1)
    {
        pos = barr.indexOf(sep, pos+sep.size()); //Возвращает индекс поз  первого вхождения массива sep в массив barr
        indexes.append(pos); // сохраняет все вхождения в QList <int> indexes
    }
    indexes.removeLast(); //Удаляет последний элемент в списке
    return indexes;
}

int main()
{
    //Задаем директорию, где находятся файлы формата *.mci
    qDebug() << "Enter the address: \t";
    QString DirectoryName;
    QStringList list, full_list, name, full_name, dir, full_dir;

    DirectoryName = cin.readLine();
    QDir itdir(DirectoryName);

    //Проверяем, существует ли такая директория
    while(!itdir.exists()){
        qDebug() << "The directory was not found";
        printf("Enter the address: \t");
        DirectoryName.clear();
        itdir = 0;
        DirectoryName = cin.readLine();
        itdir = DirectoryName;
    }

    if(itdir.exists()){
        QDirIterator it(DirectoryName, QDirIterator::Subdirectories);

        while(it.hasNext()){
            list.append(it.next());
            name.append(it.fileName());
            dir.append(it.path());
        }

        //Формат файлов
        QRegExp maska("*.mci");
        maska.setPatternSyntax(QRegExp::Wildcard);

        //Открываем поочередно найденные файлы. Если файлов нет, то программа не начнет работу
        QFile mci;
        for(int i=0; i<list.size(); i++){
            if(maska.exactMatch(list[i]) != 0){
                full_list.append(list[i]);
                full_name.append(name[i]);
                full_dir.append(dir[i]);
            }
        }
        if(full_list.size() == 0){
            qDebug() << "The files was not found!";
        }
        //Если файлы были найдены, начинается вывод и обработка файлов
        if(full_list.size() != 0){

            for(int i = 0; i < full_list.size(); i++){
                qDebug() << "***" << full_name[i] << "***" << "\n";

                mci.setFileName(full_list[i]);

                if (!mci.open(QIODevice::ReadOnly)){
                    qDebug() << mci.fileName() << "not found!";
                    return -1;
                }

                // Читаем из файла всё сразу и закидываем это в массив байт barr
                QByteArray barr;
                barr = mci.readAll();
                mci.close();

                // Находим позиции пакетов (по байтам 0x9c и 0x3e) и сохраняем их в список pack_pos
                QByteArray sep; // Это именно тот массив значений, который нужно проверить на вхождения в массив barr
                sep.append(0x9c);
                sep.append(0x3e);
                QList<int> pack_pos = indexesOf(barr, sep);  //В pack_pos помещаем indexes
                pack_pos.append(barr.size()-1); // также добавлям в список конечную позицию файла

                // По найденым позициям собираем пакеты в отдельные массивы байт
                QList <QByteArray> packets, packets_full;
                for (int i=1; i<pack_pos.size(); i++){
                    if(pack_pos.size() != 2)
                        packets.append(barr.mid(pack_pos[i-1], pack_pos[i]));
                }
                if(packets.size() !=0){
                    //В сформированные пакеты добавляем в начало размер данных,посчитанный в 2х байтовых словах, а уже потом всё остальное
                    for(int i=0; i<packets.size(); i++)
                    {
                        QByteArray arr;
                        short sh = (packets[i].size())/2;
                        QDataStream stream(&arr, QIODevice::WriteOnly);
                        stream << sh;
                        arr.append(packets[i]);
                        packets_full.append(arr);
                        arr.clear();
                    }
                    //Сформированные пакеты помещаем в отдельные файлы
                    QString pack_name = full_name[i];  // Наименование файла
                    QStringList Nlist = pack_name.split("."); //"Откидываем" часть с расширением файла

                    QString path = full_dir[i] + "/mci_full/";
                    QDir dir_path = QDir::root();
                    dir_path.mkpath(path);

                    //QString name_file = full_dir[i] + "/" + Nlist[0];
                    QString name_file = path + Nlist[0];

                    // Вывод найденных пакетов
                    for (int i=0; i<packets_full.size(); i++)
                    {
                        //Формируем каждый пакет в файл *.mci

                        QString name_file1 = name_file + "%1" + ".mci";
                        name_file1 = name_file1.arg(i);
                        QFile file;

                        file.setFileName(name_file1);

                        if (file.open(QIODevice::WriteOnly)){
                            qDebug() << "File " +  file.fileName() + " is created " << "\n";
                        }
                        else
                        {
                            qDebug() << file.fileName() << "not found!";
                            return -1;
                        }

                        file.write(packets_full[i]);
                        file.close();
                    }
                }
                else
                    qDebug() << "0";
            }
        }
    }

    return 0;
}
