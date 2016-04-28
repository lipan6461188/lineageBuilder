#include "bitree.h"
#include "quadprog.h"
#include "uquadprog.h"
#include<QList>
#include <QFile>
#include <QStringList>
#include <QTextStream>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

//#include "common.h"

namespace ublas = boost::numeric::ublas;
int edgeIndex = 0;      //记录边的索引，是一个全局变量

int current_index = 0;
int timer_start_index = 0;      //计时器开启时的线程堆索引
//int timer_end_index = 0;        //计时器响应时的线程索引

//得到一个新的节点
TreeNode getNewNode(QString name)
{
    TreeNode child = new BiTreeNode;
    child->lchild = NULL;
    child->rchild = NULL;
    child->lchildIndex = child->rchildIndex = 0; //把索引初始化为0
    child->nodeName = name;
    return child;
}

BiTree::BiTree(const QString filename,float Ci,float Cd,double psi,int width,int height)
{
    leafExpression = NULL;
    allExpression = NULL;
    this->Ci = Ci;
    this->Cd = Cd;
    this->psi = psi;
    this->TREE_HEIGHT = height;
    this->TREE_WIDTH = width;
    this->nMax = 0;
    for(int i=0;i<MAX;i++) nNumArray[i] = 0;
    nodeAmount = edgeAmount = 0;
    highestExpression = 0;
    this->root = NULL;
    //开始操作文件
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        /*这里应该有错误处理！！！*/
    {
        std::cerr << "无法打开文件"<<endl;
        exit(-1);
    }
    QTextStream in(&file);
    QStringList NodeNames;  //存储所有节点的名称,以便后面可以排重
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split("\t");
        if(fields.size() >= 2)
        {
          //  std::cout<<qPrintable(fields[0])<<std::endl;
            TreeNode father,lchild,rchild;
            if(NodeNames.contains(fields[0]))
                father = Nodes.at( NodeNames.indexOf(fields[0]) );  //Nodes是一个成员变量
            else
            {
                father = getNewNode(fields[0]);
                NodeNames.append(fields[0]);
                Nodes.append(father);
            }
            //如果左子节点是一个空节点
            if(fields[1] == "")
                lchild = NULL;
            else if(NodeNames.contains(fields[1]))
                lchild = Nodes.at( NodeNames.indexOf(fields[1]) );
            else
            {
                lchild = getNewNode(fields[1]);
                NodeNames.append(fields[1]);
                Nodes.append(lchild);
            }
            //如果右子节点是一个空节点
            if(fields.size() == 2 || fields[2] == "" )
                rchild = NULL;
            else if(NodeNames.contains(fields[2]))
                rchild = Nodes.at( NodeNames.indexOf(fields[2]) );
            else
            {
                rchild = getNewNode(fields[2]);
                NodeNames.append(fields[2]);
                Nodes.append(rchild);
            }

            father->lchild = lchild;
            father->rchild = rchild;
        }
    }
    file.close();
   // nodeAmount = NodeNames.size();

    //检查所有的节点是否可以组成一颗树
    bool *checkHistory = new bool[NodeNames.size()];
    for(int i=0; i<NodeNames.size();i++) checkHistory[i] = 0;
    CheckAllNode(checkHistory,Nodes.first());
    bool is_first_time = 1;
    int inValid_node_amount = 0;  //记录没有扫描到的节点数
    for(int i=0; i<NodeNames.size();i++)
    {
        if(checkHistory[i] == 0)
        {
            inValid_node_amount++;
            if(is_first_time)
            {
               /*此处应该有处理！！！！*/
                cout << "Warning： Existing Some Nodes Not Combine To Tree\n";
                is_first_time = 0;
            }
            cout << qPrintable(Nodes[i]->nodeName) << "\t";
            delete Nodes[i];
            Nodes[i] = NULL;    //空指针方便后面回收内存
        }
    }
    delete checkHistory;

  //  nodeAmount -= inValid_node_amount;
   // edgeAmount = nodeAmount - 1;

    //计算节点数与边的数量
    for(int i=0;i<Nodes.size();i++)
    {
        if(Nodes[i] == NULL) continue;
        nodeAmount++;
        if( Nodes[i]->lchild )
            edgeAmount++;
        if( Nodes[i]->rchild )
            edgeAmount++;
    }

    //返回结果
    this->root = Nodes.first();
    //给每一个节点编号
    TraversalTree(RootNode());
    //计算每一个节点的坐标
    computeCoordinate(RootNode(),TREE_HEIGHT,TREE_WIDTH);

    leafNodesNum = 0;
    for(int i=0;i<Nodes.size();i++)
    {
        if (Nodes[i] == NULL) continue;
        if( Nodes[i]->lchild == NULL && Nodes[i]->rchild == NULL )
            leafNodesNum++;
    }
}


//检查所有的节点是不是可以组成一棵树，这是一个递归
void BiTree::CheckAllNode(bool *checkHistory, TreeNode root)
{
    if(root == NULL) return;

    int index = Nodes.indexOf(root);
    checkHistory[index] = 1;

    CheckAllNode(checkHistory,root->lchild);
    CheckAllNode(checkHistory,root->rchild);
}



/*先序遍历，记录树的节点号与边号*/
void BiTree::TraversalTree(TreeNode root,int currentFloor,int index)
{
    if(index == 1) root->index = 1;
    if(root != NULL)
    {
        root->depth = currentFloor+1;
        nNumArray[currentFloor]++;
        if(nMax <= currentFloor)
        {
            nMax = currentFloor + 1; //当前树的最大层次
        }
        if(root->lchild)
        {
            root->lchildIndex = edgeIndex++;
            root->lchild->index = index * 2;
        }
        if(root->rchild)
        {
            root->rchildIndex = edgeIndex++;
             root->rchild->index = index * 2 + 1;
        }
    }else{
        return;
    }
    TraversalTree(root->lchild,currentFloor+1,index*2);
    TraversalTree(root->rchild,currentFloor+1,index*2+1);
}

//后序遍历树结构，把每一个节点的表达量都计算出来
void BiTree::computeExpression(TreeNode root)
{
    if(root == NULL) return;
    computeExpression(root->lchild);
    computeExpression(root->rchild);
    //如果是上一次运行过的结果，就先删除
    root->vector.clear();
    //当为叶子节点时
    if( root->lchild == NULL &&  root->rchild == NULL)
    {
        int index = leafNodes.indexOf(root->nodeName);
        root->expression = leafNodeExpression[ index ];
        for(int i=0;i<2 * edgeAmount;i++) root->vector.append(0);
    }
    else if(root->lchild){
        //当节点有左孩子时优先使用左孩子
        root->expression = root->lchild->expression;
        root->vector = root->lchild->vector;
        root->vector[root->lchildIndex] -= 1;
        root->vector[root->lchildIndex + edgeAmount] += 1;
    }
    else{
        //没有左孩子时使用右孩子的表达量
        root->expression = root->rchild->expression;
        root->vector = root->rchild->vector;
        root->vector[root->rchildIndex] -= 1;
        root->vector[root->rchildIndex + edgeAmount] += 1;
    }
}

//先序遍历，构造等式矩阵
void BiTree::constructEqaulMatrix(TreeNode root)
{
    if(root == NULL) return;
    //当同时有左右子节点时才构造等式
    if( root->lchild && root->rchild )
    {
        QList<int> newList;
        for(int i=0;i<2*edgeAmount;i++) newList.append(0);
        newList[root->lchildIndex] = 1;
        newList[root->lchildIndex + edgeAmount] = -1;
        newList[root->rchildIndex] = -1;
        newList[root->rchildIndex + edgeAmount] = 1;

        for(int i=0;i<2*edgeAmount;i++)
            newList[i] += root->rchild->vector[i] - root->lchild->vector[i];
        equalMatrix_Value.append(root->rchild->expression - root->lchild->expression);
        equalMatrix.append(newList);
    }
    constructEqaulMatrix(root->lchild);
    constructEqaulMatrix(root->rchild);
}

//读完所有叶子节点的表达值以后判断一下是否所有的叶子节点都有表达值
bool BiTree::checkAllLeafNodeHasExpression(TreeNode root)
{
    if(root == NULL) return 1;

    //节点为叶子节点
    if( root->lchild == NULL &&  root->rchild == NULL )
        //没有找到该叶子节点的表达值
        if( leafNodes.indexOf(root->nodeName) == -1 )
        {
            cout<< "---->" << qPrintable(root->nodeName) << "<-----" <<endl;
            return 0;
        }

    return checkAllLeafNodeHasExpression(root->lchild) && checkAllLeafNodeHasExpression(root->rchild);
}

//从文件中读取叶子节点的表达，但是仅仅只读到列表中，没有读入到树结构中，后面的步骤会整合到树中
void BiTree::getLeafNodeExpression(TreeNode root,QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList fields = line.split("\t");
        if(fields.size() == 2)
        {
            leafNodes.append(fields[0]);
            leafNodeExpression.append(fields[1].toFloat());
        }
    }
    file.close();
    //检查所有的叶子节点是否都有表达
    if( !checkAllLeafNodeHasExpression(root) )
        std::cerr << "叶子节点表达值不全，错误!" << endl;
}

//根据i_and_d计算出每一个节点的表达量，这是二次规划以后的事情
void BiTree::computeFinallyExpression(TreeNode root)
{
    if(root == NULL) return;

    float sum = 0;
    for(int i=0;i<edgeAmount;i++)
    {
        sum += i_and_d[i] * root->vector[i];
        sum += i_and_d[i+edgeAmount] * root->vector[i+edgeAmount];
    }
    root->expression += sum;
    if(root->expression < 0.01)
        root->expression = 0;
    if(highestExpression < root->expression)
        highestExpression = root->expression;

    computeFinallyExpression(root->lchild);
    computeFinallyExpression(root->rchild);
}


//先续遍历，从中取出所有的表达量构造表达式矩阵
void BiTree::extractExpression(TreeNode root)
{
    if(root == NULL) return;

    ExpressionMatrix.append(root->vector);
    ExpressionValue.append(root->expression);

    extractExpression(root->lchild);
    extractExpression(root->rchild);
}

//打印节点的坐标信息
void BiTree::printFloorNodeInfo(TreeNode root)
{
    if(root == NULL) return;
    cout<<root->index<<"\t"<<root->x<<"\t"<<root->y<<endl;
    printFloorNodeInfo(root->lchild);
    printFloorNodeInfo(root->rchild);
}

//计算每一个节点的横纵坐标
void BiTree::computeCoordinate(TreeNode root,float paintHeight, float paintWidth)
{
    if(root == NULL) return;
    //最后一层的节点之间的距离
    float width = paintWidth/( pow(2.0,nMax-1) + 1 );
    //各个层之间的高度距离
    float height = paintHeight/(nMax);

    //当前层节点之间的距离
    float widthOfCurFloor = width * pow(2.0,nMax-root->depth);
    //当前层第一个节点的横坐标
    float xOfCurFloor = width * pow(2.0,nMax-root->depth-1) + width/2;

    //横坐标赋值
    root->x = (root->index - pow(2.0,root->depth-1)) * widthOfCurFloor + xOfCurFloor;
    //纵坐标赋值
    root->y = height/2 + (root->depth-1) * height;

    computeCoordinate(root->lchild,paintHeight,paintWidth);
    computeCoordinate(root->rchild,paintHeight,paintWidth);
}

/*
 *
 *记录基因表达值的文件格式为
 *
 *基因1   基因2 基因3 基因4
 *细胞名字1 基因1表达值  基因2表达值  基因3表达值
 *细胞名字2 基因1表达值  基因2表达值  基因3表达值
 *
 */

//读取叶子节点的表达量
void BiTree::readMultiGeneExpressionFromFile(const QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        /*这里应该有错误处理！！！*/
    {
        cout<<"ERROR: terminal cell expression file"<<endl;
        exit(-1);
    }


    QTextStream in(&file);
    bool is_first_line = 1;
   // QStringList geneNameList;   //记录所有基因的名字
    QList< QList<QString> > leafExpressinMatrix;    //所有叶子节点的表达量矩阵
   // QStringList readInLeafNodeNames;
    int columnsAmount=0;          //列数
    int leafNodeAmount=0;         //叶子节点的数目
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();

        if( line.isEmpty() || line.startsWith("#") ){ continue; }
        QStringList fields = line.split("\t");
        if ( is_first_line )
        {
            //readInLeafNodeNames = fields;
            columnsAmount = fields.size() + 1;
            geneList.append(fields);
            fields.insert(0,"###");
            leafExpressinMatrix.append(fields);
            is_first_line = 0;
            continue;
        }
        //跳过空白行与nonValid行
        if(fields.size() < 2)
            continue;
        if( fields.size() != columnsAmount )
        {
            cerr << "Error: 文件格式不正确：" << qPrintable(line) << endl;
            exit(-1);
        }
        leafExpressinMatrix.append(fields);
        leafNodeAmount++;
     }
    file.close();

    //std::cout << leafExpressinMatrix.size() << "\t" << this->leafNodesNum << endl;

  //  cout<<leafExpressinMatrix.size()<<endl;

    //检查是否所有的叶子节点都有表达量
    QStringList leafNodesNames;
    for(int i=0;i<Nodes.size();i++)
    {
        if (Nodes[i] == NULL) continue;
        if( Nodes[i]->lchild == NULL && Nodes[i]->rchild == NULL )
            leafNodesNames.append( Nodes[i]->nodeName );
    }
    bool can_go_on = true;
    for(int i=0; i<leafNodesNames.size(); i++)
    {
        bool find = false;
        for(int j=1;j<leafExpressinMatrix.size();j++)
        {
            if(leafNodesNames[i].toUpper() == leafExpressinMatrix[j][0].toUpper() )
            {
               // cout << qPrintable(readInLeafNodeNames[j].toUpper());
                find = true;
                continue;
            }
        }
        if( !find )
        {
            can_go_on = false;
            cout << "leaf node cannot be found: "<<qPrintable( leafNodesNames[i] ) << endl;
        }
    }
    if( !can_go_on ) exit(-1);
    cout << endl << endl;
    //检查是否所有提供的叶子节点表达量都在树中
    for(int j=1;j<leafExpressinMatrix.size();j++)
    {
        bool find = true;
        for(int i=0; i<leafNodesNames.size(); i++)
        {
            bool find = false;
            if(leafNodesNames[i].toUpper() == leafExpressinMatrix[j][0].toUpper() )
            {
                find = true;
                continue;
            }
        }
        if( !find )
        {
            can_go_on = false;
            cout << "leaf with expression cannot be found: "<<qPrintable( leafExpressinMatrix[j][0] ) << endl;
        }
    }
    if( !can_go_on ) exit(-1);


    for(int i=0; i<columnsAmount-1; i++)
    {
        SingleGeneLeafExpresion myLeafExpression= new LeafExpression;
        myLeafExpression->geneName = leafExpressinMatrix[0][i+1];
        for(int j=1;j<leafNodeAmount+1;j++)
        {
            myLeafExpression->leafExpression.insert(leafExpressinMatrix[j][0],leafExpressinMatrix[j][i+1].toFloat());
        }
        myLeafExpression->next = leafExpression;
        leafExpression = myLeafExpression;
    }
}

//一个函数执行所有命令，表示叶子节点表达量文件
void BiTree::runInFlash(QString filename)
{
    initEviroment();
    cout<<"清空环境完成"<<endl;
   // TraversalTree(RootNode());
    //cout<<"计算每一个节点的坐标"<<endl;
    //computeCoordinate(RootNode(),TREE_HEIGHT,TREE_WIDTH);
    cout<<"从文件中读取叶子节点的表达量"<<endl;
    getLeafNodeExpression(RootNode(),filename);
    cout<<"开始计算每一个节点的表达量"<<endl;
    computeExpression(RootNode());
    cout<<"表达量计算完毕"<<endl;
    constructEqaulMatrix(RootNode());
    cout<<"等式矩阵构造完毕"<<endl;
    extractExpression(RootNode());
    ProgPP progPP = new QuadProgPPStruct;
    progPP->edgeAmount = edgeAmount;
    progPP->nodeAmount = nodeAmount;
    progPP->equalMatrix = equalMatrix;
    progPP->equalMatrix_Value = equalMatrix_Value;
    progPP->ExpressionMatrix = ExpressionMatrix;
    progPP->ExpressionValue = ExpressionValue;
    cout<<"开始进行计算："<<endl;
    //runQuadProgPP(progPP);

    MyThread thread(progPP,"NULL");
    thread.start();
    cout<<"开始运行新线程1"<<endl;

    thread.wait();
    cout<<"线程运行结束"<<endl;

    /*得到参数以后销毁结构*/
    this->i_and_d = progPP->i_and_d;
    this->quadprogminValue = progPP->minValue;
    delete progPP;
    computeFinallyExpression(RootNode());
}

//初始化环境，以便下一次执行
void BiTree::initEviroment()
{
    leafNodes.clear();
    leafNodeExpression.clear();
    equalMatrix.clear();
    equalMatrix_Value.clear();
    highestExpression = 0;
    ExpressionMatrix.clear();
    ExpressionValue.clear();
    quadprogminValue = 0;
}


//更具链表给定的节点初始化叶子表达
void BiTree::initNewGene(SingleGeneLeafExpresion singlegene)
{
    leafNodes.clear();
    leafNodeExpression.clear();
    //int size = singlegene->leafExpression.size();
    QHashIterator<QString, float> i(singlegene->leafExpression);
    while(i.hasNext())
    {
        i.next();
        //cout << qPrintable(i.key()) << i.value() << endl;
        leafNodes.append( i.key() );
        leafNodeExpression.append( i.value() );
    }
}

//多线程运行程序
void BiTree::runOnMultiThread(int threadsNum,bool inBoost,int _timer)
{
    int rubushThreads = 0;
   // QThread::setTerminationEnabled(true);
    int leafExpressionAmount = 0;   //需要运行的基因个数
    SingleGeneLeafExpresion item = leafExpression;
    while(item) { leafExpressionAmount++; item = item->next; }

    //首先申请threadsNum个参数块
    ProgPP progPP = new QuadProgPPStruct[threadsNum];
    int geneLeft = leafExpressionAmount;
    //得到第一个结点
    SingleGeneLeafExpresion start_gene = leafExpression;
    int threadCount = 0;
    while(geneLeft >= threadsNum)
    {
        QStringList geneNames;
        for(int i=0;i<threadsNum;i++)
        {
            initEviroment();
            initNewGene(start_gene);
            computeExpression(RootNode());
            constructEqaulMatrix(RootNode());
            extractExpression(RootNode());
            progPP[i].edgeAmount = edgeAmount;
            progPP[i].nodeAmount = nodeAmount;
            progPP[i].equalMatrix = equalMatrix;
            progPP[i].equalMatrix_Value = equalMatrix_Value;
            progPP[i].ExpressionMatrix = ExpressionMatrix;
            progPP[i].ExpressionValue = ExpressionValue;
            progPP[i].i_and_d.clear();
            progPP[i].psi = this->psi;
            progPP[i].ci = this->Ci;
            progPP[i].cd = this->Cd;
           // progPP[i].thread_stop = false;
            geneNames.append(start_gene->geneName);
            start_gene = start_gene->next;
        }
        //QList<MyThread *> Threads;
        for(int i=0; i< threadsNum;i++)
        {
            Threads.append(new MyThread( &progPP[i],geneNames[i],inBoost));
            cout<<"开启新线程: "<< threadCount << "  " << qPrintable(Threads.last()->getGeneName()) <<endl;
            Threads.last()->start();
            threadCount++;
        }
        geneNames.clear();
        current_index++;
        //等待所有的线程执行完毕
        for(int i=0;i < threadsNum;i++)
        {
            bool finished = Threads.at(i)->wait(_timer);
            if( !finished )
                //如果线程还没有结束
            {
                *(Threads.at(i)->thread_stop) = true;
                Threads.at(i)->wait();
                rubushThreads++;
                cout << "线程被强制关闭："<< Threads.at(i)->currentThreadId() << "  " << qPrintable( Threads.at(i)->getGeneName() ) <<endl;
                cout << "垃圾线程数：" << rubushThreads << endl;
            }else{
                SingleGeneExpression geneExpression = new AllExpression;
                geneExpression->geneName = Threads.at(i)->getGeneName();
                geneExpression->i_and_d =Threads.at(i)->getquadprogpp()->i_and_d;
                geneExpression -> next = allExpression;
                geneExpression->quadprogminValue = Threads.at(i)->getquadprogpp()->minValue;
                allExpression = geneExpression;
                cout<<"线程执行完毕: "<< Threads.at(i)->currentThreadId() << "  " << qPrintable(geneExpression->geneName) <<endl;
            }
        }
        //回收所有的线程
        qDeleteAll(Threads);
        Threads.clear();
        geneLeft -= threadsNum;
    }

    //剩余的没有运行完的继续
    QStringList geneNames; //存放所有基因的名字
    for(int i=0; i< geneLeft;i++)
    {
        initEviroment();
        initNewGene(start_gene);
        computeExpression(RootNode());
        constructEqaulMatrix(RootNode());
        extractExpression(RootNode());
        progPP[i].edgeAmount = edgeAmount;
        progPP[i].nodeAmount = nodeAmount;
        progPP[i].equalMatrix = equalMatrix;
        progPP[i].equalMatrix_Value = equalMatrix_Value;
        progPP[i].ExpressionMatrix = ExpressionMatrix;
        progPP[i].ExpressionValue = ExpressionValue;
        progPP[i].i_and_d.clear();
        progPP[i].psi = this->psi;
        progPP[i].ci = this->Ci;
        progPP[i].cd = this->Cd;
     //   progPP[i].thread_stop = false;
        geneNames.append(start_gene->geneName);
        start_gene = start_gene->next; //向下走一步
    }
    //QList<MyThread *> Threads;
    for(int i=0; i< geneLeft;i++)
    {
        Threads.append(new MyThread( &progPP[i],geneNames[i],inBoost));
        cout<<"开启新线程: "<< threadCount << "  " << qPrintable(Threads.last()->getGeneName()) <<endl;
        Threads.last()->start();
        threadCount++;
    }
    current_index++;
    for(int i=0;i < geneLeft;i++)
    {
       // Threads.at(i)->setTerminationEnabled(true);
        bool finished = Threads.at(i)->wait(_timer);
        if( !finished )
            //如果是强制关闭的
        {
            *(Threads.at(i)->thread_stop) = true;
            Threads.at(i)->wait();
            rubushThreads++;
            //Threads.at(i)->exit(-1);
            cout << "线程被强制关闭："<< Threads.at(i)->currentThreadId() << "  " << qPrintable(Threads.at(i)->getGeneName()) <<endl;
            cout << "垃圾线程数：" << rubushThreads << endl;

        }else{
            SingleGeneExpression geneExpression = new AllExpression;
            geneExpression->geneName = Threads.at(i)->getGeneName();
            geneExpression->i_and_d =Threads.at(i)->getquadprogpp()->i_and_d;
            geneExpression -> next = allExpression;
            geneExpression->quadprogminValue = Threads.at(i)->getquadprogpp()->minValue;
            allExpression = geneExpression;
            cout<<"线程执行完毕: "<< Threads.at(i)->currentThreadId() << "  " << qPrintable(geneExpression->geneName) <<endl;
        }
    }
    qDeleteAll(Threads);
    Threads.clear();

    //这里还不能删除叶子节点的表达量，后面还要使用
    computeExpressionEveryCell();
}

//把基因的表达信息整合到hash中
void itergrateIntoHash(TreeNode root,QString geneName)
{
    if(root == NULL) return;

    root->geneExpression.insert(geneName,root->expression);

    itergrateIntoHash(root->lchild,geneName);
    itergrateIntoHash(root->rchild,geneName);
}

//根据多线程计算的结果，把每一个基因在每一个细胞中的表达量计算出来
void BiTree::computeExpressionEveryCell()
{
    SingleGeneExpression gene = allExpression;     //记录所有基因的表达
    while(gene)
    {
        QString geneName = gene->geneName;
        //到叶子节点的链表中查找
        for(SingleGeneLeafExpresion leafList=leafExpression;leafList;leafList = leafList->next)
        {
            if( leafList->geneName == geneName )
            {
                //如果找到了对应的记录，就调用initNewGene函数
                initNewGene(leafList);
                break;
            }
        }
        computeExpression(RootNode());
        i_and_d  = gene->i_and_d;
        computeFinallyExpression(RootNode());
        itergrateIntoHash(RootNode(),geneName);
        minValueList.insert(geneName,gene->quadprogminValue);
        cout<< qPrintable(geneName) << "\t" << gene->quadprogminValue <<endl;
        gene = gene->next;
    }

    //删除所有的表达链表
    QList<SingleGeneExpression> list1;
    for(SingleGeneExpression i=allExpression;i;i = i->next)
        list1.append(i);
    qDeleteAll(list1);
    allExpression = NULL;

    //删除所有叶子的表达链表
    QList<SingleGeneLeafExpresion> list;
    for(SingleGeneLeafExpresion i=leafExpression;i;i = i->next)
        list.append(i);
    qDeleteAll(list);
    leafExpression = NULL;
}

//从所有的文件中读取基因表达数据
void BiTree::importGeneExpressionToTree(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        /*这里应该有错误处理！！！*/
    {
        cout<<"Cannot Open File: "<< qPrintable(fileName) <<endl;
        exit(-1);
    }
    QStringList cells; //所有细胞的名字
    QStringList genes;
    QTextStream in(&file);
    bool is_first_line = true;
    int columnsAmount=0;          //列数
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if( line.isEmpty() || line.startsWith("#") ){ continue; }
        QStringList fields = line.split("\t");
        if ( is_first_line )
        {
            columnsAmount = fields.size()+1;
            geneList = genes = fields;
            is_first_line = false;
            continue;
        }
        if(fields.size() != columnsAmount)
        {
            std::cerr<<"The Format of "<< qPrintable(fileName) << " is Error: " << qPrintable(line) <<endl;
            exit(-1);
        }
        cells.append( fields[0] );
        bool find = false;
        for(int i=0;i<Nodes.size();i++)
        {
            if(Nodes[i] == NULL) continue;
            if(Nodes[i]->nodeName == QString(fields[0]))
            {
                find = true;
                for(int j=1;j<columnsAmount;j++)
                {
                    bool *success = new bool;
                    float myExpression = QString(fields[j]).toFloat(success);
                    if( ! *success )
                    {
                        std::cerr << "The number cannot convert to Float: " << qPrintable(fields[j]) << "\t" << qPrintable(line) << "\t" << qPrintable(fileName) << endl;
                        exit(-1);
                    }
                    delete success;
                    Nodes[i]->geneExpression.insert(genes[j-1],myExpression);
                }
                break;
            }
        }
        if(!find)
        {
            std::cerr << "Warning: A Nodes named: "<< qPrintable(fields[0]) << " occured but not found in tree" <<endl;
        }
    }
    file.close();

    //检查是否所有的细胞在这个文件中都有
    for(int i=0;i<Nodes.size();i++)
    {
        if(cells.indexOf( Nodes[i]->nodeName ) == -1)
        {
            std::cerr << "TreeNode "<< qPrintable(Nodes[i]->nodeName) << " Not Found In The File: " << qPrintable(fileName) << endl;
            exit(-1);
        }
    }
}

//得到每一个非叶子节点的asymmetry
QHash< QString,float > BiTree::computeAsymmetry()
{
    /*
     *思路：
     * 1. 得到所有的同时有左右子节点的节点;
     * 2. 计算每一个节点得到的Ap;
     */
    QList<TreeNode> unLeafNodes;
    for(int i=0;i<Nodes.size();i++)
    {
        if(Nodes[i] == NULL) continue;
        if(Nodes[i]->lchild != NULL && Nodes[i]->rchild != NULL)
            unLeafNodes.append(Nodes[i]);
    }
    /*2. 计算每一个节点得到的Ap*/
   // QList< QList<float> > ApgMatrix;
    QStringList geneList = unLeafNodes[0]->geneExpression.keys();//基因的列表
    QHash< QString,float > assymmetry;  //记录每一个非叶子节点的Assymmetry
    for(int i=0;i<unLeafNodes.size();i++)
    {
        float Ap_of_this_node = 0;
       // geneExpression = unLeafNodes[i]->geneExpression;
        //QList< float > Apg_of_this_node;
        for(int j=0;j<geneList.size();j++)
        {
            float lexpression = unLeafNodes[i]->lchild->geneExpression[geneList[j]];
            float rexpression = unLeafNodes[i]->rchild->geneExpression[geneList[j]];
            float meanExpression = (lexpression + rexpression)/2;
            Ap_of_this_node += qAbs(lexpression-rexpression)/(meanExpression + 500);
           // Apg_of_this_node.append( qAbs(lexpression-rexpression) );
        }
       // ApgMatrix.append(Apg_of_this_node);
        assymmetry.insert(unLeafNodes[i]->nodeName,Ap_of_this_node);
    }
    return assymmetry;
}

//存储所有每一个细胞中所有基因的表达值
void BiTree::saveExpressionIntoFile(QString fileName)
{
    QList<QString> geneNames; //所有的基因名字
    QList<QString> cellNames; //细胞的名字
    QList< QList<float> > expressionMatrix; //表达矩阵,横纵坐标分别为细胞名与基因名

    bool is_first_time = true;
    QListIterator<TreeNode> i(Nodes);
    while(i.hasNext())
    {
        TreeNode node = i.next();
        if(node == NULL) continue;


        cellNames.append(node->nodeName);
        if(is_first_time)
        {
            cout<<"细胞数目： "<<node->geneExpression.keys().size()<<endl;
            geneNames = node->geneExpression.keys();
            is_first_time = false;
        }
        QList<float> expression_of_this_cell;
        for(int i=0;i<geneNames.size();i++)
        {
            expression_of_this_cell.append( node->geneExpression[ geneNames[i] ] );
        }
        expressionMatrix.append(expression_of_this_cell);
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        std::cerr << "无法打开写入文件\n"<<std::endl;
        exit(-1);
    }
    cout<<"开始写入文件"<<endl;
    QTextStream out(&file);

    for(int i=0;i<geneNames.size();i++)
        out<<geneNames[i]<<"\t";
    out<<"\n";
    for(int i=0;i<expressionMatrix.size();i++)
    {
        out << cellNames[i]<<"\t";
        for(int j=0;j<expressionMatrix[i].size();j++)
        {
            out<<expressionMatrix[i][j]<<"\t";
        }
        out<<"\n";
    }
    cout<<"写入文件完毕"<<endl;
    out.flush();
    file.close();
}

/*获取所有的叶子节点的名字*/
QStringList BiTree::getAllLeafNodeNames()
{
    QStringList leafNodes;
    for(int i=0;i<Nodes.size();i++)
    {
        if(Nodes[i] == NULL) continue;
        if(Nodes[i]->lchild==NULL && Nodes[i]->rchild==NULL) leafNodes.append(Nodes[i]->nodeName);
    }
    return leafNodes;
}

/*把所有的矩阵输出到文件中*/
void BiTree::outPutAllMatrix(QString path)
{
    SingleGeneLeafExpresion start_gene = leafExpression;

    std::cout << "打印矩阵到文件中" << std::endl;

    float ci = this->Ci;
    float cd = this->Cd;
    float psi = this->psi;
    QTextStream out;

    while(start_gene)
    {
        cout << qPrintable(start_gene->geneName) << endl;
       // QStringList geneNames;
        QString geneName = start_gene->geneName;
        initEviroment();
        initNewGene(start_gene);
        computeExpression(RootNode());
        constructEqaulMatrix(RootNode());
        extractExpression(RootNode());

       // geneNames.append(start_gene->geneName);
        start_gene = start_gene->next;

        /*每一个基因计算完成以后都写入文件*/
        QFile G(path+"/"+geneName+".G");
        QFile g0(path+"/"+geneName+".g0");
        QFile CE(path+"/"+geneName+".CE");
        QFile ce0(path+"/"+geneName+".ce0");
        QFile CI(path+"/"+geneName+".CI");
        QFile ci0(path+"/"+geneName+".ci0");
        if ( !G.open(QIODevice::WriteOnly|QIODevice::Text) ||
             !g0.open(QIODevice::WriteOnly|QIODevice::Text) ||
             !CE.open(QIODevice::WriteOnly|QIODevice::Text) ||
             !ce0.open(QIODevice::WriteOnly|QIODevice::Text) ||
             !CI.open(QIODevice::WriteOnly|QIODevice::Text) ||
             !ci0.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            std::cerr << "Cannot Open Matrix File: " << qPrintable(path+"/"+geneName) <<std::endl;
            exit(-1);
        }

        int n, m, p;

        //矩阵G
        out.setDevice(&G);
        //定义矩阵
        n = edgeAmount * 2;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n/2; j++)
            {
                if(i == j) out << 2*ci*psi << "\t";
                else out << 0 << "\t";
            }
            for(int j = n/2; j < n; j++)
            {
                if(i == j) out << 2*cd*psi << "\t";
                else out << 0 << "\t";
            }
            out << "\n";
        }
        G.close();

        //向量g0
        out.setDevice(&g0);
        n = edgeAmount * 2;
        for (int i = 0; i < n/2; i++)
            out << ci << "\n";
        for (int i = n/2; i < n; i++)
            out << cd << "\n";
        g0.close();

        //矩阵CE
        out.setDevice(&CE);
        //m为等式的数量
        m = equalMatrix.size();
        n = edgeAmount * 2;
        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < n; j++)
            {
                out << equalMatrix[i][j] << "\t";
            }
            out << "\n";
        }
        CE.close();

        //向量ce0
        out.setDevice(&ce0);
        m = equalMatrix_Value.size();
        for (int j = 0; j < m; j++)
            out << equalMatrix_Value[j] << "\n";
        ce0.close();

        //矩阵CI
        out.setDevice(&CI);
        //p为不等式的数量
        p = 2*edgeAmount + nodeAmount;
        n = 2*edgeAmount;
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < 2*edgeAmount; j++)
                if(i == j)
                    out << 1 << "\t";
                else
                    out << 0 << "\t";
            out << "\n";
         }
        for (int i = 0; i < nodeAmount; i++)
        {
            for (int j = 0; j < 2*edgeAmount; j++)
                out << ExpressionMatrix[i][j] << "\t";
            out << "\n";
        }
        CI.close();

        //向量ci0
        out.setDevice(&ci0);
        //p为不等式的数量
        p = 2*edgeAmount + nodeAmount;
        for (int j = 0; j < 2*edgeAmount; j++)
            out << 0 << "\n";
        for(int j = 2*edgeAmount; j<p; j++)
            out << ExpressionValue[j-2*edgeAmount] << "\n";
        ci0.close();
    }
}

//使用CGAL的二次规划
void MyThread::runQuadProgCGAL(ProgPP quadprogpp)
{
    float ci = quadprogpp->ci;
    float cd = quadprogpp->cd;
    double ksi = quadprogpp->psi;

    int n,m,e;

    int edgeAmount = quadprogpp->edgeAmount;
    int nodeAmount = quadprogpp->nodeAmount;
    QList< QList<int> > &equalMatrix = quadprogpp->equalMatrix;
    QList<float> &equalMatrix_Value = quadprogpp->equalMatrix_Value;
    QList< QList<int> > &ExpressionMatrix = quadprogpp->ExpressionMatrix;
    QList<float> &ExpressionValue = quadprogpp->ExpressionValue;
    QList<float> &i_and_d = quadprogpp->i_and_d;
    float &minValue = quadprogpp->minValue;

    //定义二次规划对象为非负二次规划
    Program qp(CGAL::LARGER,true,0,false,0);

    //cout << "n: " << edgeAmount * 2 <<endl;
    //首先设置G
    n = edgeAmount * 2;
    for(int i=0;i<n;i++)
        for(int j=i;j<n;j++)
            if(i==j)
                if(i < edgeAmount)
                    qp.set_d(j,i,2*ksi*ci);
                else
                    qp.set_d(j,i,2*ksi*cd);
            else
                qp.set_d(j,i,0);

    //设置常数项
    qp.set_c0(0);

    //设置目标表达式中的向量
    for(int i=0;i<edgeAmount;i++)
        qp.set_c(i,ci);
    for(int i=edgeAmount;i<n;i++)
        qp.set_c(i,cd);

    //cout << "m: " << ExpressionMatrix.size() << endl;
    //设置不等式项
    m = ExpressionMatrix.size(); //不等式的项数
    for(int i=0;i<m;i++)
        for(int j=0;j<n;j++)
            qp.set_a(j,i, ExpressionMatrix[i][j] );

    //设置等式项
    e = equalMatrix.size();  //等式的项数
    for(int i=m;i<e+m;i++)
        for(int j=0;j<n;j++)
            qp.set_a(j,i, equalMatrix[i-m][j] );

    //设置向量
    for(int i=0;i<m;i++)
        qp.set_b(i, ExpressionValue[i]);
    for(int i=m;i<m+e;i++)
        qp.set_b(i,equalMatrix_Value[i-m]);

    //设置符号
    for(int i=m;i<m+e;i++)
        qp.set_r(i,CGAL::EQUAL);

    CGAL::Quadratic_program_options option;
    option.set_verbosity(1);
    option.set_pricing_strategy(CGAL::QP_FILTERED_DANTZIG);
    Solution s = CGAL::solve_nonnegative_quadratic_program(qp,ET(),option);

    assert( s.solves_nonnegative_quadratic_program(qp) );

    std::cout << s;
}

//构造二次规划运行矩阵，并运行
void MyThread::runQuadProgPP(ProgPP quadprogpp)
/*
 *需要用到的变量有：
 *  edgeAmount、nodeAmount、equalMatrix、equalMatrix_Value、ExpressionMatrix、ExpressionValue、i_and_d
 */
{
   // cout<<"运行QUADPROGPP"<<endl;

    float ci = quadprogpp->ci;
    float cd = quadprogpp->cd;
    double ksi = quadprogpp->psi;

    int edgeAmount = quadprogpp->edgeAmount;
    int nodeAmount = quadprogpp->nodeAmount;
    QList< QList<int> > &equalMatrix = quadprogpp->equalMatrix;
    QList<float> &equalMatrix_Value = quadprogpp->equalMatrix_Value;
    QList< QList<int> > &ExpressionMatrix = quadprogpp->ExpressionMatrix;
    QList<float> &ExpressionValue = quadprogpp->ExpressionValue;
    QList<float> &i_and_d = quadprogpp->i_and_d;
    float &minValue = quadprogpp->minValue;

    //定义好数据结构
    QuadProgPP::Matrix<double> G, CE, CI;
    QuadProgPP::Vector<double> g0, ce0, ci0, x;
    int n, m, p;

    //定义矩阵
    n = edgeAmount * 2;
    G.resize(n, n);
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n/2; j++)
            {
                if(i == j) G[i][j] = 2*ci*ksi;//2*ci*psi;
                else    G[i][j] = 0;
            }
            for(int j = n/2; j < n; j++)
            {
                if(i == j) G[i][j] = 2*cd*ksi; //2*cd*psi;
                else    G[i][j] = 0;
            }
        }
    }

    g0.resize(n);
    {
        for (int i = 0; i < n/2; i++)
            g0[i] = ci;
        for (int i = n/2; i < n; i++)
            g0[i] = cd;
    }

/*
    g0.resize(n);
    {
        for (int i = 0; i < n; i++)
            g0(i) = 1;
    }
*/
    //m为等式的数量
    m = equalMatrix.size();
    CE.resize(n, m);
    {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                CE[i][j] = equalMatrix[j][i];
    }
    ce0.resize(m);
    {
        for (int j = 0; j < m; j++)
            ce0[j] = equalMatrix_Value[j];
    }

    //p为不等式的数量
    p = 2*edgeAmount + nodeAmount;
    CI.resize(n, p);
    {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < 2*edgeAmount; j++)
                if(i == j)
                    CI[i][j] = 1;
                else
                    CI[i][j] = 0;
        for (int i = 0; i < n; i++)
            for (int j = 2*edgeAmount; j < p; j++)
                CI[i][j] = ExpressionMatrix[j-2*edgeAmount][i];
    }
    ci0.resize(p);
    {
        for (int j = 0; j < 2*edgeAmount; j++)
            ci0[j] = 0;
        for(int j = 2*edgeAmount; j<p; j++)
            ci0[j] = ExpressionValue[j-2*edgeAmount];
    }
    //x是要求的向量
    x.resize(n);

   minValue = QuadProgPP::solve_quadprog(G, g0, CE, ce0, CI, ci0, x,this->thread_stop);
   // cout<<"最小值"<<minValue<<endl;
    for(int i=0;i<2*edgeAmount;i++)
    {
        if(x[i] > 0)
            i_and_d.append(x[i]);
        else
            i_and_d.append(0);
        cout << x[i] << "\t";
    }
   // return quadprogminValue;
}

//构造二次规划运行矩阵，并使用Boost库运行
void MyThread::runQuadProgPP_inboost(ProgPP quadprogpp)

/*
 *需要用到的变量有：
 *  edgeAmount,nodeAmount,equalMatrix,equalMatrix_Value,ExpressionMatrix,ExpressionValue
 */
{
    float ci = quadprogpp->ci;
    float cd = quadprogpp->cd;
    double psi = quadprogpp->psi;

    int edgeAmount = quadprogpp->edgeAmount;
    int nodeAmount = quadprogpp->nodeAmount;
    QList< QList<int> > &equalMatrix = quadprogpp->equalMatrix;
    QList<float> &equalMatrix_Value = quadprogpp->equalMatrix_Value;
    QList< QList<int> > &ExpressionMatrix = quadprogpp->ExpressionMatrix;
    QList<float> &ExpressionValue = quadprogpp->ExpressionValue;
    QList<float> &i_and_d = quadprogpp->i_and_d;
    float &minValue = quadprogpp->minValue;

    //定义好数据结构
    ublas::matrix<double> G, CE, CI;
    ublas::vector<double> g0, ce0, ci0, x;
    int n, m, p;

    //定义矩阵
    n = edgeAmount * 2;
    G.resize(n, n);
    {
        for (int i = 0; i < n; i++)
        {
            for (int j = 0; j < n/2; j++)
            {
                if(i == j) G(i, j) = 2*ci*psi;
                else    G(i, j) = 0;
            }
            for( int j=n/2;j < n; j++)
            {
                if(i == j) G(i, j) = 2*cd*psi;
                else    G(i, j) = 0;
            }
        }
    }

    g0.resize(n);
    {
        for (int i = 0; i < n/2; i++)
            g0(i) = ci;
        for (int i = n/2; i < n; i++)
            g0(i) = cd;
    }

    //m为等式的数量
    m = equalMatrix.size();
    CE.resize(n, m);
    {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                CE(i,j) = equalMatrix[j][i];
    }
    ce0.resize(m);
    {
        for (int j = 0; j < m; j++)
            ce0(j) = equalMatrix_Value[j];
    }

    //p为不等式的数量
    p = 2*edgeAmount + nodeAmount;
    CI.resize(n, p);
    {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < 2*edgeAmount; j++)
                if(i == j)
                    CI(i,j) = 1;
                else
                    CI(i,j) = 0;
        for (int i = 0; i < n; i++)
            for (int j = 2*edgeAmount; j < p; j++)
                CI(i,j) = ExpressionMatrix[j-2*edgeAmount][i];
    }
    ci0.resize(p);
    {
        for (int j = 0; j < 2*edgeAmount; j++)
            ci0(j) = 0;
        for(int j = 2*edgeAmount; j<p; j++)
            ci0(j) = ExpressionValue[j-2*edgeAmount];

    }
    //x是要求的向量
    x.resize(n);

    minValue = uQuadProgPP::solve_quadprog(G, g0, CE, ce0, CI, ci0, x);
   // cout<<"最小值"<<minValue<<endl;
    for(int i=0;i<2*edgeAmount;i++)
        if(x[i] > 0)
            i_and_d.append(x[i]);
        else
            i_and_d.append(0);
    //return minValue;
}





