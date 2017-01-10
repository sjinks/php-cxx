#ifndef PHPCXX_MAP_H
#define PHPCXX_MAP_H

#include "phpcxx.h"

#include <map>
#include "emallocallocator.h"

namespace phpcxx {

template<typename K, typename V, typename C = std::less<K> >
using map = std::map<K, V, C, EMallocAllocator<std::pair<const K, V> > >;

}

#endif /* PHPCXX_MAP_H */
