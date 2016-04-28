#include "common.h"

/*
typedef struct LeafClass
{
    QHash<QString, int> lClass; //每一个叶子节点描述的类型
    int classAmount;            //类别数
}*leafClass;
*/

/*
 *这个文件应该是一个两列的文件:
 * 细胞1  类别（大于等于1的整数）
 * 细胞2  类别（大于等于1的整数）
 */

leafClass constructLeafClass(QString filename)
{

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        /*这里应该有错误处理！！！*/
    {
        std::cerr << "无法打开文件："<< qPrintable(filename) <<endl;
        exit(-1);
    }
    QTextStream in(&file);

    leafClass lc = new LeafClass;
    lc->classAmount = 0;

    QList<int> all_class;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if( line.isEmpty() || line.startsWith("#") ){ continue; }
        QStringList fields = line.split("\t");
        if( fields.size() != 2 )
        {
            std::cerr << "ERROR: " << qPrintable(line) << "   " << qPrintable(filename) << std::endl;
            exit(-1);
        }
        bool *success = new bool(false);
        int myclass = fields[1].toInt(success);
        if( ! *success )
        {
            std::cerr << "ERROR: " << qPrintable(line) << "   " << qPrintable(filename) << std::endl;
            exit(-1);
        }
        delete success;
        all_class.append( myclass );
        lc->lClass.insert(fields[0],myclass);
    }
    file.close();

    //下面检测all_class中的类别是否正确
    for(int i=0;i<all_class.size();i++)
    {
        if( lc->classAmount < all_class.at(i) )
        {
            lc->classAmount = all_class.at(i);
        }
        //如果出现
        if( all_class.at(i) <= 0 )
        {
            std::cerr << "File Format Error: " << all_class.at(i) << "\tat line: " << i <<"\t" << qPrintable(filename) << std::endl;
            exit(-1);
        }
    }
    return lc;
}


