#ifndef COMMON_H
#define COMMON_H

#define TREE_WIDTH   1500
#define TREE_HEIGHT   500

#include <iostream>
#include <QString>
#include <QList>
#include <QHash>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QStringList>
using namespace std;

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

#endif // COMMON_H

