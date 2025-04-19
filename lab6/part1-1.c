#define KEY_BASE 0xff200050
#define LED_BASE 0xff200000

int main(){
	volatile int* keyPtr = (int*) KEY_BASE;
	volatile int* ledPtr = (int*) LED_BASE;
	
	int edgeCapture = 0;
	
	while(1){
		edgeCapture = *(keyPtr + 3);		//get content of edge capture
		if((edgeCapture & 1) > 0){
			*(keyPtr + 3) = edgeCapture;	//clear edge capture register
			*(ledPtr) = 1023;
		}else if((edgeCapture & 2) > 0){
			*(ledPtr) = 0;
		}
	}

	return 0;
}