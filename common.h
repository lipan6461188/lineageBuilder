#ifndef COMMON_H
#define COMMON_H

/*
#define TREE_WIDTH   1500
#define TREE_HEIGHT   500
*/
#include <iostream>
#include <QString>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QStringList>
#include <QIODevice>
#include <QHash>

#include <CGAL/basic.h>
#include <CGAL/QP_functions.h>
#include <CGAL/QP_models.h>
#include <CGAL/QP_solution.h>
#include <CGAL/Quotient.h>
#include <cassert>

using namespace std;

#ifdef CGAL_USE_GMP
#include <CGAL/Gmpzf.h>
typedef CGAL::Gmpzf ET;
#else
#include <CGAL/MP_Float.h>
typedef CGAL::MP_Float ET;
#endif

typedef CGAL::Quadratic_program<ET> Program;
typedef CGAL::Quadratic_program_solution<ET> Solution;

typedef struct LeafClass
{
    QHash<QString, int> lClass; //每一个叶子节点描述的类型
    int classAmount;            //类别数
}*leafClass;


//构造叶子节点的分类结构
leafClass constructLeafClass(QString filename);

/*
struct BiTreeNode;          //定义节点结构，表示二叉树中的每一个节点
struct AllExpression;       //链表结构，记录多线程时所有的表达值结果
struct LeafExpression;      //链表结构，记录多线程时所有的叶子节点
struct QuadProgPPStruct;    //结构体，多线程时，单个线程的参数
class BiTree;               //二叉树类，并带有各种计算的函数
class MyThread;             //多线程类

typedef struct BiTreeNode *TreeNode;
typedef struct AllExpression *SingleGeneExpression;
typedef struct LeafExpression *SingleGeneLeafExpresion;
typedef struct QuadProgPPStruct *ProgPP;
*/

#endif // COMMON_H

