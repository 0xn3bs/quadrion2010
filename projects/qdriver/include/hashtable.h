#ifndef _HASH_
#define _HASH_

#include 	<iostream>
#include 	<vector>


template <class T>
class HashTable {
	struct ItemStruct {
		std::string indexName;
		T data;
	};
private:
	std::vector <ItemStruct> stack;

public:
	void clear()
	{
		stack.clear();
	}

	long size() {
		return stack.size();
	}
	void add(std::string indexName, T data) {
		for(long i=0; i<stack.size(); i++) 
			if(stack[i].indexName == indexName) {
				stack[i].data=data;
				return;
			}                  
			ItemStruct obj;
			obj.indexName=indexName;
			obj.data = data;
			stack.push_back(obj);
	}

	std::string getIndexName(long index)
	{
		if(index < 0) index=0;
		return stack[index].indexName;
	}

	T  &operator [] (long index) {
		if(index < 0) index=0;
		return stack[index].data;
	}
	T &operator [](std::string indexName) {

		for(long i=0; i<stack.size(); i++) 
			if(stack[i].indexName == std::string(indexName)) {
				return   stack[i].data;
			}
			long idx = stack.size();
			ItemStruct obj;
			obj.indexName=indexName;
			stack.push_back(obj);  
			return stack[idx].data;             
	}        
};
#endif