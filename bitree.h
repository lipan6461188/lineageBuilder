#include "quadprog.h"
#include "common.h"

const int MAX = 100;  //树的最大层数

//定义节点结构，表示二叉树中的每一个节点
typedef struct BiTreeNode
{
    QString nodeName;
    int lchildIndex;    //左右两条边的索引
    int rchildIndex;
    BiTreeNode* lchild;
    BiTreeNode* rchild;
    char color;
    int thickness;
    int depth;                  //在树中的深度
    int index;                  //在满二叉树中的索引
    float x;                    //横坐标
    float y;                    //纵坐标
    QList<int> vector;          //记录i/d表达量的向量，长度为边数的两倍
    float expression;           //记录最左叶子节点的表达量
    QHash<QString, float> geneExpression;   //记录所有基因的表达量
} *TreeNode;

//记录计算完成以后所有细胞中单个基因的表达
typedef struct AllExpression
{
    QString geneName;
    float quadprogminValue;
    QList<float> i_and_d;   //每种基因的i_and_d
    AllExpression* next;
} *SingleGeneExpression;

//记录计算之前单个基因在所有的叶子节点中的表达
typedef struct LeafExpression
{
    QString geneName;
    QHash<QString, float> leafExpression;
    LeafExpression *next;       //指向下一个节点
} *SingleGeneLeafExpresion;

//定义二次规划执行函数的参数结构
typedef struct QuadProgPPStruct
{
    int edgeAmount;
    int nodeAmount;
    QList< QList<int> > equalMatrix;
    QList<float> equalMatrix_Value;
    QList< QList<int> > ExpressionMatrix;
    QList<float> ExpressionValue;
    QList<float> i_and_d;
    float minValue;
    double psi;
    float ci;
    float cd;
} *ProgPP;


//定义二叉树类
class BiTree:QObject
{
   // Q_OBJECT
public:
    //构造函数的参数为构造二叉树的文件名
    BiTree(const QString filename, float Ci, float Cd, double psi, int width, int height);
    //析构函数要负责回收节点空间
    ~BiTree(){};

    //先序遍历树，扫描所有的节点，并给节点和边进行编号
    void TraversalTree(TreeNode root, int currentFloor=0, int index=1);
    //打印节点的信息
    void printFloorNodeInfo(TreeNode root);
    //计算每一个节点的坐标
    void computeCoordinate(TreeNode root,float paintHeight, float paintWidth);
    //从文件中读取叶子节点的表达
    void getLeafNodeExpression(TreeNode root, QString filename);
    //先序遍历，构造等式矩阵
    void constructEqaulMatrix(TreeNode root);
    //后序遍历树结构，把每一个节点的表达量都计算出来
    void computeExpression(TreeNode root);
    //根据i_and_d计算出每一个节点的表达量
    void computeFinallyExpression(TreeNode root);
    //先续遍历，构造表达量矩阵
    void extractExpression(TreeNode root);
    //检查所有的节点是不是可以组成一棵树，这是一个递归
    void CheckAllNode(bool *checkHistory, TreeNode root);
    //读完所有叶子节点的表达值以后判断一下是否所有的叶子节点都有表达值
    bool checkAllLeafNodeHasExpression(TreeNode root);
    //从文件中同时读取多个基因的表达值
    void readMultiGeneExpressionFromFile(const QString filename);
    //多线程运行程序
    void runOnMultiThread(int threadsNum, bool inBoost=false);
    //一次执行所有流程
    void runInFlash(QString filename="../data/expression");
    //初始化环境，以便下一次执行
    void initEviroment();
    //更具链表给定的节点初始化叶子表达
    void initNewGene(SingleGeneLeafExpresion singlegene);
    //根据多线程计算的结果，把每一个基因在每一个细胞中的表达量计算出来
    void computeExpressionEveryCell();
    //存储所有每一个细胞中所有基因的表达值
    void saveExpressionIntoFile(QString fileName);
    //从所有的文件中读取基因表达数据
    void importGeneExpressionToTree(QString fileName);
    //得到每一个非叶子节点的asymmetry
    QHash<QString, float> computeAsymmetry();

    //构造二次规划运行矩阵，并运行
  //  static float runQuadProgPP(ProgPP quadprogpp);
  //  static float runQuadProgPP_inboost(ProgPP quadprogpp);

    //得到二叉树的深度
    int getHeightOfBiTree(){ return nMax; }
    /*得到树中的最大表达量*/
    float getHighestExpression(){ return highestExpression; }
    /*得到本树的根节点*/
    TreeNode RootNode(){ return root; }
    /*获取I_and_D*/
    QList<float> getIAndD(){ return i_and_d; }
    /*得到基因的名字*/
    QStringList getGeneList(){ return geneList; }
    /*获取所有的叶子节点的名字*/
    QStringList getAllLeafNodeNames();
    QList<TreeNode> getNodes(){ return Nodes; }

    /*调试函数*/
    /*
    void tiaoshi1(TreeNode root);
    void tiaoshi2();
    void tiaoshi3(TreeNode root);
    void tiaoshi4();
    void tiaoshi5(TreeNode root);
    void tiaoshi6(TreeNode root);
    */
    void tiaoshi7()
    {
        cout<<"进入调试"<<endl;
        SingleGeneLeafExpresion gene = leafExpression;
        while(gene)
        {
            cout<< qPrintable(gene->geneName) << endl;
            gene = gene->next;
        }
        for(gene = leafExpression; gene; gene = gene->next)
        {
            cout << gene->leafExpression["P4p"]<<endl;
        }
    }

    //输出allExpression的结果
    void tiaoshi8()
    {
        SingleGeneExpression expression =  allExpression;
        while(expression)
        {
            cout<<qPrintable( expression->geneName ) <<endl;
            cout <<expression->quadprogminValue <<endl;
            for(int i=0;i<expression->i_and_d.size();i++)
                cout << expression->i_and_d[i] <<"\t";
            cout << endl;
            expression = expression->next;
        }
    }

    //输出所有的i_and_d
    void tiaoshi9()
    {
        for(int i=0;i<i_and_d.size()/2;i++)
        {
            cout << i_and_d[i] << "\t" << i_and_d[i+i_and_d.size()/2]<<endl;
        }
    }

private:
    //根节点
    TreeNode root;

    int nMax;                               //树的深度
    int nNumArray[MAX];                     //记录所有层的节点数

    QStringList leafNodes;                  //叶子节点列表
    QList<float> leafNodeExpression;        //叶子节点的表达量
    int nodeAmount;                         //节点的数量
    int edgeAmount;                         //边的数量：为节点数减1

    QList< QList<int> > equalMatrix;        //等式矩阵
    QList<float> equalMatrix_Value;         //等式矩阵对应的值

    QList<float> i_and_d;                   //所有的I和D，二次规划得到

    float highestExpression;                //记录树中的最高表达量

    QList< QList<int> > ExpressionMatrix;   //节点的表达量矩阵
    QList<float> ExpressionValue;           //节点的表达量值

    QList<TreeNode> Nodes;                  //存储所有接节点的地址

    float quadprogminValue;                 //二次规划计算得到的最小值
    QHash<QString,float> minValueList;              //记录所有的计算的最小值

    SingleGeneLeafExpresion leafExpression; //记录所有叶子的表达
    SingleGeneExpression allExpression;     //记录所有基因的表达

    QStringList geneList;                   //基因名字的列表

    int TREE_WIDTH;
    int TREE_HEIGHT;

    //算法的三个参数
    float Ci;
    float Cd;
    double psi;


};


class MyThread : public QThread
{
 //   Q_OBJECT
protected:
    void run()
    {
        if(inBoost)
            runQuadProgPP_inboost(quadprogpp);
        else
            runQuadProgPP(quadprogpp);
    }
public:
    //构造函数
   MyThread(ProgPP quadprogpp,QString geneName,bool inBoost = false,QObject *parent = 0):QThread(parent)
    {
        this->quadprogpp = quadprogpp;
        this->inBoost = inBoost;
        this->geneName = geneName;
    }

    //构造二次规划运行矩阵，并运行
    void runQuadProgPP(ProgPP quadprogpp);
    void runQuadProgPP_inboost(ProgPP quadprogpp);
    ProgPP getquadprogpp(){return quadprogpp;}
    QString getGeneName(){ return geneName; }

private:
    ProgPP quadprogpp;
    bool inBoost;
    QString geneName;
};







