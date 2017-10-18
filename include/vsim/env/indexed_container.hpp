#ifndef __VSIM_INDEXED_CONTAINER_HPP__
#define __VSIM_INDEXED_CONTAINER_HPP__

#include <vector>
#include <map>

template<class T>
class IndexedContainer: public std::vector<T> {

public:

    void add(T item, const std::string &id) {

        if ( !id.empty() )
            lookup_.insert({id, this->size()}) ;

        this->push_back(item) ;
    }

    T find(const std::string &id) const {
        auto it = lookup_.find(id) ;
        if ( it != lookup_.end() )
            return this->at(it->second) ;
        else
            return T() ;
    }

    std::map<std::string, int> lookup_ ;
};






#endif
