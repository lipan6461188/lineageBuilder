#ifndef PAINTBITREE_H
#define PAINTBITREE_H

#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include "common.h"

class PaintBitree
{
public:
    PaintBitree(BiTree *bitree);
    //只绘制树的结构
    bool paintTreeStructureAndSave(QString filePath);

private:
    BiTree *bitree;

private:
    bool recursivePaintTreeStructure(TreeNode root);
};

#endif // PAINTBITREE_H
