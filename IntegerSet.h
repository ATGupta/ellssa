#include "IntegerObject.h"
#include <string>

class IntegerSet{
	IntegerObject *head = NULL;
	IntegerObject *tail = NULL;
	
	bool present(int val){
		IntegerObject* ob = head;
		while(ob != NULL) {
			if(val == ob->val) {
				return true;
			}
			ob = ob->next;
		}
		return false;
	}
	
	public:
	int length = 0;
	void add(int val){
		if(present(val)){
			return;
		}
		IntegerObject* ob = new IntegerObject();
		ob->val = val;
		
		if(head == NULL){
			head = ob;
			tail = ob;
		}
		else{
			tail->next = ob;
			tail = ob;
		}
		length++;
	}
	
	void selectionSort(int *a){
		for(int i = 0; i < length-1; i++){
			for(int j = i + 1; j < length; j++){
				if(a[i] > a[j]){
					int t = a[i];
					a[i] = a[j];
					a[j] = t;
				}
			}
		}
	}
	
	int* toArray(){
		int* l = (int*)malloc(length * sizeof(int));
		
		IntegerObject* ob = head;
		for(int i = 0; i < length; i++){
			l[i] = ob->val;
			ob = ob->next;
		}
		selectionSort(l);
		return l;
	}
};
