#include <QApplication>
#include <iostream>
#include "paintbitree.h"
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QDir>
#include <QTime>
using namespace std;

/*
 *  float circleMagnitude;      //20
    float lineMagnitude;        //5
 */

void PrintHelp() {
    cout << "\nLineageBuilder -h|--help print this help and quit" << '\n';
    cout << "LineageBuilder -f <filename> [--help|-draw <ITEMS>|-Ci|-e|-Cd|-ksi|--threads|-sef|-d|-w|-h|-v]" << '\n';
    cout << "-f <filename>: cell division file. REQUIRED\n"; //结构文件
    cout << "--expression <filename>: read all gene expression and plot. If -e is specific, this item will be ignored.\n";
    cout << "-e <filename>: terminal cell expression file\n"; //叶子节点细胞表达文件
    cout << "-a <filename>: output the asymmetry of the nonleaf nodes to the file.\n";
    cout << "--draw <ITEMS>. ITEMS: S/N/E  S:Structure Only, N:Structure with Names, E:Structure with Expression (default: S)\n";
    cout << "-Ci <float>: Value of Ci (default: 1).\n";
    cout << "-Cd <float>: Value of Cd (default: 1).\n";
    cout << "-ksi <float>: Value of ksi,for example: 10e-10 (default:10^-14).\n";
    cout << "--threads <int>: threads to run (default: 1)\n";
    cout << "--sef <filename>: save expression file \n";
    cout << "--boost: Run Qudratic Programming in Boost (deafult: no)\n";
    cout << "-d <rootPath>: the directory files to save (default: ./)\n";
    cout << "--width <int>: picture width (default: 1500)\n";
    cout << "--height <int>: picture height (default: 500)\n";
    cout << "--random <int,int,int,string>: generate random expression (default no)\n";
    cout << "-v: verbose output (default no)\n\n";
    cout << "\n\n";
    cout << "Drawing Parameters(--sap will show all Options): \n";
    cout << "--lineColor <ColorType>: the color of the lines.(default: blue)\n";
    cout << "--circleColor <ColorType>: the color of the circles.(default: red)\n";
    cout << "--bgColor <ColorType>: the color of the background.(default: white)\n";
    cout << "--wordColor <ColorType>: the color of the word.(default: magenta)\n";
    cout << "--lineStyle <lineStyleType>: the tyle of lines.(default: SolidLine)\n";
    cout << "--circleMagnitude <float>: the magnitude of GeneExpression Circle.(default: 20)\n";
    cout << "--lineMagnitude <float>: the magnitude of differentiation map.(default: 5)\n";
    cout << "\n\n";

}

void printColorsAndStyles()
{
    cout << "\n===================ALL Options======================\n";
    cout << "All Optional Colors: \n"
            "\twhite,black,red,darkRed,green,darkGreen,\n"
            "\tblue,darkBlue,cyan,darkCyan,magenta,darkMagenta,\n"
            "\tyellow,darkYellow,gray,darkGray,lightGray"<<endl<<endl;
    cout << "All Optional LineStyles: \n"
            "\tSolidLine,DashLine,DotLine,\n"
            "\tDashDotLine,DashDotDotLine,CustomDashLine\n" << endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString structureFileName = "";
    QString leafNodeExpressionFileName = "";
    QString expressionFileName = "";
    QString rootPath = QDir::currentPath();
    QString allGeneExpressionFileName = "";
    QString randomExpressionFileName = "";
    QString asymetryFileName = "";
    bool will_draw_structre = true;
    bool will_draw_names = false;
    bool will_draw_expression = false;
    bool will_run_in_boost = false;
    float Ci = 1;
    float Cd = 1;
    int width = 1500;
    int height = 500;
    int threads_to_run = 1;
    double ksi = pow(10.0,-14);
   // bool will_compute_expression = false;
    bool will_output_to_screen = false;
    bool will_generate_random_expression = false;
    int random_amount = 0;
    int random_min = 0;
    int random_max = 0;

    //绘图属性
    QString lineColor = "blue";
    QString circleColor = "red";
    QString wordColor = "magenta";
    QString bgColor = "white";
    QString lineStyle = "SolidLine";
    float circleMagnitude = 20;
    float lineMagnitude = 5;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-h") or !strcmp(argv[i], "h") or !strcmp(argv[i], "--help") or !strcmp(argv[i], "help") or !strcmp(argv[i], "?")) {
            PrintHelp();
            exit(0);
        }
        if (!strcmp(argv[i], "--random"))
        {
            QStringList fields = QString(argv[i+1]).split(",");
            if(fields.size() != 4)
            {
                cout << "ERROR: --random params error\n";
                PrintHelp();
                exit(-1);
            }
            will_generate_random_expression = true;
            random_amount= QString(fields[0]).toInt();
            random_min = QString(fields[1]).toInt();
            random_max = QString(fields[2]).toInt();
            randomExpressionFileName = fields[3];
            i++;
            cout<<random_amount<<"\t"<<random_min<<"\t"<<random_max<<endl;
        }
        if (!strcmp(argv[i], "-f")) {
            structureFileName = argv[i + 1];
            i++;
        }
        if (!strcmp(argv[i], "--sap")) {
            printColorsAndStyles();
            exit(0);
        }
        if (!strcmp(argv[i], "-a")) {
            asymetryFileName = argv[i + 1];
            i++;
        }
        if (!strcmp(argv[i], "--expression")) {
            allGeneExpressionFileName = argv[i + 1];
            i++;
            //cout << "得到：" <<allGeneExpressionFileName.toStdString()<<endl;
        }
        if (!strcmp(argv[i], "-e")) {
            leafNodeExpressionFileName = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--draw")){
            QString flag = argv[i + 1];
            if(flag.indexOf('S') == -1) will_draw_structre = false;
            if(flag.indexOf('N') != -1) will_draw_names = true;
            if(flag.indexOf('E') != -1) { will_draw_expression = true;}
            i++;
        }
        if(!strcmp(argv[i], "-Ci")){
            Ci = QString(argv[i + 1]).toFloat();
            i++;
        }
        if(!strcmp(argv[i], "-Cd")){
            Cd = QString(argv[i + 1]).toFloat();
            i++;
        }
        if(!strcmp(argv[i], "-ksi")){
            ksi = QString(argv[i + 1]).toDouble();
            double frioir = QString(argv[i + 1]).split("e").at(0).toDouble();
            double later = QString(argv[i + 1]).split("e").at(1).toDouble();
            ksi = pow(frioir,later);
            //cout<<frioir<<"\t"<<later<<endl;
            i++;
        }
        if(!strcmp(argv[i], "-d")){
            rootPath = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--threads")){
            threads_to_run = QString(argv[i + 1]).toInt();
            i++;
        }
        if(!strcmp(argv[i], "--width")){
            width = QString(argv[i + 1]).toInt();
            i++;
        }
        if(!strcmp(argv[i], "--height")){
            height = QString(argv[i + 1]).toInt();
            i++;
        }
        if(!strcmp(argv[i], "-v")){
            will_output_to_screen = true;
        }
        if(!strcmp(argv[i], "--boost")){
            will_run_in_boost = true;
        }
        if(!strcmp(argv[i], "--sef")){
            expressionFileName = argv[i + 1];
            i++;
        }

        //判断绘图属性
        if(!strcmp(argv[i], "--lineColor")){
            lineColor = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--circleColor")){
            circleColor = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--lineStyle")){
            lineStyle = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--bgColor")){
            bgColor = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--wordColor")){
            wordColor = argv[i + 1];
            i++;
        }
        if(!strcmp(argv[i], "--circleMagnitude")){
            circleMagnitude = QString(argv[i + 1]).toFloat();
            i++;
        }
        if(!strcmp(argv[i], "--lineMagnitude")){
            lineMagnitude = QString(argv[i + 1]).toFloat();
            i++;
        }
}
    if(structureFileName == "")
    {
        cout<<"ERROR: No cell division file"<<endl;
        PrintHelp();
        exit(-1);
    }


 /*
            PaintBitree(BiTree *bitree, int width, int height, QString lineColor,
                                            QString circleColor, QString bgColor,
                                            QString wordColor,QString lineStyle,
                                            float circleMagnitude,float lineMagnitude)
*/



    BiTree *bitree = new BiTree(structureFileName,Ci,Cd,ksi,width,height);
    PaintBitree tree(bitree,width,height,lineColor,circleColor,bgColor,wordColor,
                     lineStyle,circleMagnitude,lineMagnitude);

    //产生随机表达
    if(will_generate_random_expression)
    {
        cout<<"开始产生随机数"<<endl;
        srand(QTime::currentTime().msec());
        QStringList leafNodes = bitree->getAllLeafNodeNames();
        int randAmount = leafNodes.size() * random_amount;
        QList<int> rands;
        while(rands.size() < randAmount)
        {
            int rand_num = qAbs(rand());
            int rand = rand_num%(random_max+1);
            if(rand >= random_min) rands.append(rand);
        }
        QFile file(rootPath + "/" +randomExpressionFileName);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            std::cerr << "无法打开写入文件\n"<<std::endl;
            exit(-1);
        }
        cout<<"开始写入文件"<<endl;
        QTextStream out(&file);
        for(int i=0;i<random_amount;i++)
        {
            out << QString("Gene")+QString::number(i+1);
            if(i != randAmount -1)
                out << "\t";
        }
        out << "\n";
        for(int i=0;i<leafNodes.size();i++)
        {
            out << leafNodes[i] << "\t";
            for(int j=0;j<random_amount;j++)
            {
                out << rands[ i*random_amount + j ];
                if(j != random_amount-1)
                    out << "\t";
            }
            out<<"\n";
        }
        file.close();
        cout<<"产生随机数完毕"<<endl;
        exit(0);
    }

    if(allGeneExpressionFileName != "" && leafNodeExpressionFileName == NULL)
    {
        bitree->importGeneExpressionToTree(allGeneExpressionFileName);
        if(will_draw_expression)
            tree.paintAllGene(rootPath);
    }

    //进行大量的表达计算
    if(leafNodeExpressionFileName != "")
    {
        if(leafNodeExpressionFileName=="")
        {
            cout<<"ERROR: No terminal cell expression file"<<endl;
            PrintHelp();
            exit(-1);
        }
        bitree->readMultiGeneExpressionFromFile(leafNodeExpressionFileName);
            bitree->runOnMultiThread(threads_to_run,will_run_in_boost);
        if(expressionFileName != "")
        {
            bitree->saveExpressionIntoFile(rootPath+"/"+expressionFileName);
            cout << "Save File: " << QString(rootPath+"/"+expressionFileName).toStdString() << endl;
        }


        if(will_draw_expression)
            tree.paintAllGene(rootPath);
    }
    //cout << "开始====\n"<<allGeneExpressionFileName.toStdString() <<"\n"<< leafNodeExpressionFileName.toStdString()<<endl;
    if(allGeneExpressionFileName != "" || leafNodeExpressionFileName != NULL)
    {
        //cout << "开始====\n";
        if(asymetryFileName != "")
        {
            cout<<"Begining Writing Asymetry"<<endl;
            //cout << "开始====\n";
            QFile file(rootPath + "/" + asymetryFileName);
            if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
            {
                std::cerr << "无法打开写入文件\n"<<std::endl;
                exit(-1);
            }
            QTextStream out(&file);
            QHash< QString,float > hash = bitree->computeAsymmetry();
            QStringList geneList = hash.keys();
            for(int i=0;i<geneList.size();i++)
            {
                out << geneList[i]<<"\t"<<hash[geneList[i]]<<"\n";
            }
            file.close();
             tree.drawAsymetryPlot(rootPath+"/"+asymetryFileName+".png");
        }
    }


    bool result;

    if(will_draw_structre)
        result = tree.paintTreeStructureAndSave(rootPath+"/structure.png");
    if(will_draw_names)
        result = tree.paintTreeStructureWithNameAndSave(rootPath+"/structureWithNames.png");
    if(!result)
        std::cerr << "保存失败"<<endl;


    return a.exec();
}
