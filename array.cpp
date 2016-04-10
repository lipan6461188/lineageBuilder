#include "Array.h"

/**
  Index utilities
 */
namespace QuadProgPP{

std::set<unsigned int> seq(unsigned int s, unsigned int e)
{
    std::set<unsigned int> tmp;
    for (unsigned int i = s; i <= e; i++)
        tmp.insert(i);

    return tmp;
}

std::set<unsigned int> singleton(unsigned int i)
{
    std::set<unsigned int> tmp;
    tmp.insert(i);

    return tmp;
}

}
