#ifndef PAINTBITREE_H
#define PAINTBITREE_H

#include <QPixmap>
#include <QPainter>
#include "common.h"
#include "bitree.h"


const enum Qt::GlobalColor qtColors[] = {Qt::white,Qt::black,Qt::red,Qt::darkRed,Qt::green,
                Qt::darkGreen,Qt::blue,Qt::darkBlue,Qt::cyan,Qt::darkCyan,
                Qt::magenta,Qt::darkMagenta,Qt::yellow,Qt::darkYellow,
                Qt::gray,Qt::darkGray,Qt::lightGray};
//表示所有的颜色
const QString qtColorStr = "white,black,red,darkRed,green,darkGreen,blue,darkBlue,"
                           "cyan,darkCyan,magenta,darkMagenta,yellow,darkYellow,gray,"
                           "darkGray,lightGray";

//表示线条的类型
const enum Qt::PenStyle qtLineStyle[] = {Qt::SolidLine,Qt::DashLine,Qt::DotLine,
                                         Qt::DashDotLine,Qt::DashDotDotLine,Qt::CustomDashLine};

const QString qtLineStyleStr = "SolidLine,DashLine,DotLine,DashDotLine,DashDotDotLine,CustomDashLine";

class PaintBitree
{
public:
    PaintBitree(BiTree *bitree, int width, int height, QString lineColor,
                            QString circleColor, QString bgColor,
                             QString wordColor, QString lineStyle,
                            float circleMagnitude, float lineMagnitude);
    //只绘制树的结构
    bool paintTreeStructureAndSave(QString filePath, int drawNameDepth=0);
    //绘制数的结构以及节点的名字
    bool paintTreeStructureWithNameAndSave(QString filePath, int drawNameDepth=0);
    //绘制树的结构与多线程表达量
    bool paintTreeStructureWithExpression(QString filePath, QString geneName, int drawNameDepth=0);
    float findMaxExpression(QString geneName); //获取某一个基因的最大表达量
    void paintAllGene(QString directory, int drawNameDepth=0);   //在目录下画所有基因表达图片
    void drawAsymetryPlot(QString fileName, int drawNameDepth=0);    //画differentiation map

    bool drawLeafClass(leafClass lc, QString filePath,int depth=0, int bias = 25);   //画出叶子节点的分类

private:
    BiTree *bitree;
    QPen wordPen;
    QPen linePen;
    float maxExpression; //最大表达量
    bool draw_name; //是否绘制名字
    bool draw_expression; //是否绘制表达量
    bool draw_ellipse;      //是否绘制基因表达量
    bool draw_class;    //是否绘制叶子节点的类别
    QString drawGeneExpression; //绘制哪一个基因的表达量

    //所有颜色的哈希
    QHash<QString, enum Qt::GlobalColor> allColors;
    //所有线条的类型
    QHash<QString, enum Qt::PenStyle> allLineStyle;

    int TREE_WIDTH;
    int TREE_HEIGHT;

    //绘图属性
    QColor lineColor;
    QColor circleColor;
    QColor bgColor;
    QColor wordColor;
    QList<QColor> colorList;   //绘制类别
    Qt::PenStyle lineStyle;
    float circleMagnitude;      //20
    float lineMagnitude;        //5
    int drawNameDepth;          //当深度小于这个数时就绘制名字

    leafClass lc;

private:
    void drawTree(QPainter *paint);
    bool draw(QString filePath);
    void recursiveFindMaxExpression(TreeNode root, QString geneName, float& maxExpression);

    //临时变量
private:
    int bias;   //绘制类别时字符的偏移数
};

#endif // PAINTBITREE_H
