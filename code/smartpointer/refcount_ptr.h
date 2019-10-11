#ifndef REFCOUNT_PTR_H
#define REFCOUNT_PTR_H

#include <iostream>
#define DEBUG
#define REFCOUNT_PTR_DEBUG(x) x.debug(#x)

/*! Smart pointer with reference counting, proof of concept */
template <typename T>
class refcount_ptr {
public:
	refcount_ptr(T * element)
		: m_element(element),
		  m_refcount(new unsigned(1)) {
		;
	}
	refcount_ptr(const refcount_ptr<T> & copied)
		: m_element(copied.m_element),
		  m_refcount(copied.m_refcount) {
		acquire();
	}
	refcount_ptr& operator= (const refcount_ptr<T> & copied) {
		if (this != &copied) {
			release();
			m_element = copied.m_element;
			m_refcount = copied.m_refcount;
			acquire();
		}
		return *this;
	}
	~refcount_ptr() {
		release();
	}
	T& operator*() {return *m_element;}
	T* operator->() const {return m_element;}
	
	void debug(const std::string & name) {
		std::cerr << name << " = "
			  << "refcount_ptr {\n"
			  << "        m_element = 0x" << std::hex << m_element << "\n"
			  << "        *m_refcount = " << std::dec << *m_refcount << "\n"
			  << "};" << std::endl;
	}
private:
	void acquire() {
		(*m_refcount)++;
	}
	void release() {
		(*m_refcount)--;
		if ((*m_refcount) == 0) {
#ifdef DEBUG
			debug("deleting element");
#endif
			delete m_element;
			delete m_refcount;
		}
		
	}
	T * m_element;
	unsigned * m_refcount;
};
#endif // REFCOUNT_PTR_H
