#define AUDIO_BASE 0xff203040
#define SW_BASE 0xff200040
	
int main(){
	volatile int* audioPtr = (int*) AUDIO_BASE;
	volatile int* swPtr = (int*)SW_BASE;
	
	int samples, isHigh, sw, frequency;
	isHigh = 1;		//default output 1 and then 0
	
	while(1){
		
		sw = *swPtr;
		frequency = 100 + ((sw * 1900) / 1023);
		samples = 8000 / frequency;
		 
		for(int i = 0; i < samples/2; i++){
			if((*(audioPtr + 1) & 0xFF000000) != 0){
				if(isHigh == 1){
					*(audioPtr + 2) = 0x7fffff;
					*(audioPtr + 3) = 0x7fffff;
				}else{
					*(audioPtr + 2) = 0;
					*(audioPtr + 3) = 0;
				}
			}		
			
		}
		//revert the output low/high
		isHigh = !isHigh;
		
	}
}
