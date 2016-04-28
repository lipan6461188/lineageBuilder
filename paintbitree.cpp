#include "paintbitree.h"
//#include <QTest>
#include <unistd.h>

PaintBitree::PaintBitree(BiTree *bitree, int width, int height, QString lineColor,
                        QString circleColor, QString bgColor,
                         QString wordColor,QString lineStyle,
                         float circleMagnitude,float lineMagnitude)
{
    this->bitree = bitree;
    this->draw_expression = false;
    this->draw_name = false;
    this->draw_class = false;
    this->maxExpression = 0;
    this->draw_ellipse = false;
    this->circleMagnitude = circleMagnitude;
    this->lineMagnitude = lineMagnitude;

    this->TREE_HEIGHT = height;
    this->TREE_WIDTH = width;

    this->lc = NULL;

    this->drawNameDepth = 0;
    this->bias = -999;
    //wordPen.setBrush(QBrush(Qt::red,Qt::SolidPattern));//设置画刷形式

    //对所有的颜色进行初始化
    QStringList colorList = qtColorStr.split(",");
    for(int i=0;i<colorList.size();i++)
        allColors.insert(colorList[i],qtColors[i]);

    //对所有的线条风格进行初始化
    QStringList lineStyleList = qtLineStyleStr.split(",");
    for(int i=0;i<lineStyleList.size();i++)
        allLineStyle.insert(lineStyleList[i],qtLineStyle[i]);

    //初始化所有的颜色
    if(colorList.indexOf(lineColor) != -1)
    {
     //   cout << lineColor.toStdString() << "\n";
      //  cout << allColors[lineColor] << endl;
        this->lineColor = QColor( allColors[lineColor] );
    }else{
        cout << "ERROR: --lineColor "<< lineColor.toStdString() <<endl;
        this->lineColor = QColor( Qt::blue );
    }

    if(colorList.indexOf(circleColor) != -1)
    {
        this->circleColor = QColor( allColors[circleColor] );
    }else{
        this->circleColor = QColor( Qt::red );
        cout << "ERROR: --circleColor "<< circleColor.toStdString() <<endl;
    }

    if(colorList.indexOf(bgColor) != -1)
    {
        this->bgColor = QColor( allColors[bgColor] );
    }else{
        this->bgColor = QColor( Qt::white );
        cout << "ERROR: --bgColor "<< bgColor.toStdString() <<endl;
    }

    if(colorList.indexOf(wordColor) != -1)
    {
        this->wordColor = QColor( allColors[wordColor] );
    }else{
        this->wordColor = QColor( Qt::magenta );
        cout << "ERROR: --wordColor "<< wordColor.toStdString() <<endl;
    }


    if(lineStyleList.indexOf(lineStyle) != -1)
    {
        this->lineStyle = allLineStyle[lineStyle];
        cout << "Line Style: " << this->lineStyle  << endl;
    }else{
        this->lineStyle = Qt::SolidLine;
        cout << "ERROR: --lineStyle "<< lineStyle.toStdString() <<endl;
    }
    
    linePen.setColor(this->lineColor);
    linePen.setStyle(this->lineStyle);
    linePen.setWidth(1);

    wordPen.setColor(this->wordColor);
    wordPen.setWidth(1);

    //输出所有的绘图信息
    cout << "Magnitude: " <<circleMagnitude << "\t" << lineMagnitude<<endl;

}

/*
 *     //绘图属性
    QColor lineColor;
    QColor circleColor;
    QColor bgColor;
    QColor wordColor;
    Qt::PenStyle lineStyle;
    float circleMagnitude;      //20
    float lineMagnitude;        //5
 *
 * */


bool PaintBitree::draw(QString filePath)
{
    std::cerr << "开始绘图" <<endl;
    QPixmap pixmap(TREE_WIDTH,TREE_HEIGHT);
    pixmap.fill(this->bgColor);
    QPainter *painter = new QPainter;
    painter->begin(&pixmap);
    painter->setPen(linePen);
    drawTree(painter);

    //如果死是画叶子节点的类的话就添加上图例
    if(draw_class)
    {
        float x_start = 10;
        float y_start = 10;
        float height = 2;
        float width = 30;
        float y_interval = 10;
        float x_interval = 30;
        for(int i=0;i<lc->classAmount;i++)
        {
            QColor color = colorList[ i ];
            QRectF rectangle(x_start, y_start + i *( y_interval + height),width, height);
            painter->fillRect(rectangle, QBrush(color, Qt::SolidPattern));
            painter->setPen(QPen( color ));
            painter->drawText(QPointF(x_start + width + x_interval,y_start + i *( y_interval + height) + 5),QString::number( i+1 ));
        }
    }

    painter->end();
    delete painter;
    cout<<"绘图结束: "<< qPrintable(filePath) <<endl;
    return pixmap.save(filePath);
}

bool PaintBitree::paintTreeStructureWithExpression(QString filePath,QString geneName,int drawNameDepth)
{
    maxExpression = findMaxExpression(geneName);
    draw_ellipse = true;
    drawGeneExpression = geneName;
    this->drawNameDepth = drawNameDepth;
    bool result = draw(filePath);
    this->drawNameDepth = 0;
    draw_ellipse = false;
    maxExpression = 0;
    return result;
}

bool PaintBitree::paintTreeStructureAndSave(QString filePath,int drawNameDepth)
{
    this->drawNameDepth = drawNameDepth;
    bool result = draw(filePath);
    this->drawNameDepth = 0;
    return result;
}



bool PaintBitree::paintTreeStructureWithNameAndSave(QString filePath,int drawNameDepth)
{
    draw_name = true;
    this->drawNameDepth = drawNameDepth;
    bool result = draw(filePath);
    this->drawNameDepth = 0;
    draw_name = false;
    return result;
}

void PaintBitree::drawTree(QPainter *paint)
{
//paint->drawText(QPointF(10,10),"Hello");
    QList<TreeNode> AllNodes = bitree->getNodes();

    cout << "节点的数目: " << AllNodes.size() << endl;
    for(int i=0;i<AllNodes.size();i++)
    {
        TreeNode root = AllNodes[i];
        if(root == NULL) continue;

        if(draw_name)
        {
            paint->setPen(wordPen);
            paint->drawText(QPointF(root->x-10,root->y),root->nodeName);
            paint->setPen(linePen);
        }

        if(draw_ellipse)
        {
            //计算粗细
            float myGeneExpression = root->geneExpression[drawGeneExpression];
            float myMaxExpression = maxExpression;
            float thickness = (myGeneExpression)/(myMaxExpression) * circleMagnitude;
            paint->setBrush(QBrush(this->circleColor,Qt::SolidPattern));
            paint->drawEllipse(root->x-thickness/2,root->y-thickness/2,thickness,thickness);
            paint->setPen(linePen);
        }

        if(draw_expression)
        {
            paint->setPen(wordPen);
            paint->drawText(QPointF(root->x-10,root->y+20),QString::number( root->geneExpression[drawGeneExpression] ));
            paint->setPen(linePen);
        }

        //计算粗细
       // float myGeneExpression = root->geneExpression[drawGeneExpression];
        //float myMaxExpression = maxExpression;
       // float thickness = (myGeneExpression+myMaxExpression)/(myMaxExpression*2) * 3;

        if(root->lchild)
        {
            //float HalfHeight = (root->lchild->y - root->y) / 2;
            QLineF line1(root->x, root->y, root->lchild->x, root->y);
            QLineF line2(root->lchild->x, root->y, root->lchild->x, root->lchild->y);
        /*    if(maxExpression != 0)
            {
                linePen.setWidthF(thickness);
                paint->setPen(linePen);
            }*/
            paint->drawLine(line1);
            paint->drawLine(line2);
            linePen.setWidthF(1);
            paint->setPen(linePen);
        }
        if(root->rchild)
        {
            //float HalfHeight = (root->rchild->y - root->y) / 2;
            QLineF line1(root->x, root->y, root->rchild->x, root->y);
            QLineF line2(root->rchild->x, root->y, root->rchild->x, root->rchild->y);

            paint->drawLine(line1);
            paint->drawLine(line2);
            linePen.setWidthF(1);
            paint->setPen(linePen);
         }

        if(root->lchild == NULL && root->rchild == NULL)
        {   //如果是叶子节点
            if(draw_class)
            {
                float lastLeafWidth = TREE_WIDTH/( pow(2.0,bitree->getHeightOfBiTree()-1) + 1 );

                QRectF rectangle(root->x-lastLeafWidth/2, root->y-10, lastLeafWidth, 20);
                QColor color = colorList[ lc->lClass[ root->nodeName ] - 1 ];
                paint->fillRect(rectangle, QBrush(color, Qt::SolidPattern));
                paint->setPen(linePen);
                if(this->bias >= -100)
                    paint->drawText( QPointF(root->x-lastLeafWidth/2,root->y+bias), QString::number(lc->lClass[ root->nodeName ]));
            }
        }

        if( root->depth <= this->drawNameDepth )
        {
           // paint->setPen(wordPen);
            paint->drawText( QPointF(root->x,root->y),root->nodeName );
         //   paint->setPen(linePen);
        }
    }
}

float PaintBitree::findMaxExpression(QString geneName) //获取某一个基因的最大表达量
{
    float maxExpression;
    recursiveFindMaxExpression(bitree->RootNode(),geneName,maxExpression);
    return maxExpression;
}

void PaintBitree::recursiveFindMaxExpression(TreeNode root,QString geneName,float& maxExpression)
{
    if(root == NULL) return;

    if( root->geneExpression[geneName] > maxExpression )
        maxExpression = root->geneExpression[geneName];

    recursiveFindMaxExpression(root->lchild,geneName,maxExpression);
    recursiveFindMaxExpression(root->rchild,geneName,maxExpression);
}

void PaintBitree::paintAllGene(QString directory, int drawNameDepth)
{
    //首先应该获得一个基因名字的列表
    QStringList geneList = bitree->getGeneList();

    for(int i=0;i<geneList.size();i++)
    {
        //sleep(0.5);
        paintTreeStructureWithExpression(directory+"/"+geneList[i]+".png",geneList[i],drawNameDepth);
        sleep(2);
    }
}

void PaintBitree::drawAsymetryPlot(QString fileName, int drawNameDepth)    //画differentiation map
{
    QHash<QString, float> asymetryHash = bitree->computeAsymmetry();
    QList<float> Asys = asymetryHash.values();
    float maxAsy = 0;
    for(int i=0;i<Asys.size();i++)
        if(maxAsy < Asys[i])
            maxAsy = Asys[i];
   // QStringList geneNames = asymetryHash.keys();
    QList<TreeNode> AllNodes = bitree->getNodes();

    QPixmap pixmap(TREE_WIDTH,TREE_HEIGHT);
    pixmap.fill(this->bgColor);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setPen(linePen);

    for(int i=0;i<AllNodes.size();i++)
    {
        if(AllNodes[i] == NULL) continue;
        bool has_asy = (AllNodes[i]->lchild != NULL) && (AllNodes[i]->rchild != NULL);
        TreeNode root = AllNodes[i];
        if(has_asy)
        {
            float thickness = (asymetryHash[AllNodes[i]->nodeName])/maxAsy * lineMagnitude;
            QLineF line1(root->lchild->x, root->y, root->rchild->x, root->y);
            QLineF line2(root->lchild->x, root->y, root->lchild->x, root->lchild->y);
            QLineF line3(root->rchild->x, root->y, root->rchild->x, root->rchild->y);
            linePen.setWidthF(thickness);
            painter.setPen(linePen);
            painter.drawLine(line1);
            linePen.setWidth(1);
            painter.setPen(linePen);
            painter.drawLine(line2);
            painter.drawLine(line3);
            if( root->depth <= drawNameDepth )
                painter.drawText( QPointF(root->x,root->y),root->nodeName );
            continue;
        }

        if(AllNodes[i]->lchild != NULL)
        {
            QLineF line1(root->lchild->x, root->y, root->x, root->y);
            QLineF line2(root->lchild->x, root->y, root->lchild->x, root->lchild->y);
            linePen.setColor(QColor(Qt::gray));
            painter.setPen(linePen);
            painter.drawLine(line1);
            painter.drawLine(line2);
            linePen.setColor(this->lineColor);
            painter.setPen(linePen);
        }
        if(AllNodes[i]->rchild != NULL)
        {
            QLineF line1(root->x, root->y, root->rchild->x, root->y);
            QLineF line2(root->rchild->x, root->y, root->rchild->x, root->rchild->y);
            linePen.setColor(QColor(Qt::gray));
            painter.setPen(linePen);
            painter.drawLine(line1);
            painter.drawLine(line2);
            linePen.setColor(this->lineColor);
            painter.setPen(linePen);
        }
    }
    painter.end();
    cout<<"绘图结束: "<< qPrintable(fileName) <<endl;
    pixmap.save(fileName);
}

//画出叶子节点的分类
bool PaintBitree::drawLeafClass(leafClass lc, QString filePath, int depth, int bias)
{
    this->lc = lc;
    draw_class = true;
    for(int i = 0; i<lc->classAmount;i++)
    {
        colorList.append( QColor( qrand()%254,qrand()%254,qrand()%254  ) );
    }
    this->drawNameDepth = depth;
    this->bias = bias;
    bool result = draw(filePath);
    this->drawNameDepth = 0;
    draw_class = false;
    return result;
}

