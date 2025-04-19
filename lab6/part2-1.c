#define AUDIO_BASE 0xff203040
	
int main(){
	volatile int* audioPtr = (int*) AUDIO_BASE;
	int left, right, fifospace;
	while(1){
		fifospace = *(audioPtr + 1);
		if((fifospace & 0x000000ff) > 0){
			left = *(audioPtr + 2);
			right = *(audioPtr + 3);
			*(audioPtr + 2) = left;
			*(audioPtr + 3) = right;
		}
	}
}