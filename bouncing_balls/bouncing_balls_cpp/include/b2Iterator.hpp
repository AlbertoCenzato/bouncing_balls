#ifndef B2ITERATOR_HPP
#define B2ITERATOR_HPP

template<class b2Object>
class b2Iterator {

public:
    b2Iterator(b2Object *first_list_item) 
        : current_elem(first_list_item) { }

    b2Iterator& operator++() {      // pre-increment operator
        if (current_elem != nullptr)
            current_elem = current_elem->GetNext();
        return *this;
    }    

    const b2Object* operator*() const {              // dereference operator
        return current_elem;
    }

	b2Object* operator*() {              // dereference operator
		return current_elem;
	}

private:
    b2Object* current_elem;
};


template<class b2Object>
b2Iterator<b2Object> begin(b2Object *obj) {
    return b2Iterator<b2Object>(obj);
}

template<class b2Object>
b2Iterator<b2Object> end(b2Object *obj) {
    return b2Iterator<b2Object>(nullptr);
}

template<class b2Object>
bool operator!=(const b2Iterator<b2Object> &i1, const b2Iterator<b2Object> &i2) {
    return *i1 != *i2;
}

#endif