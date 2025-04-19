#define AUDIO_BASE 0xff203040
	
int main(){
	struct audio_t {
		volatile unsigned int control; // The control/status register
		volatile unsigned char rarc; // the 8 bit RARC register
		volatile unsigned char ralc; // the 8 bit RALC register
		volatile unsigned char wsrc; // the 8 bit WSRC register
		volatile unsigned char wslc; // the 8 bit WSLC register
		volatile unsigned int ldata;
		volatile unsigned int rdata;
	};
	
	struct audio_t *const audiop = ((struct audio_t *) AUDIO_BASE);
	double damped = 0.4;
	int leftAccu[3200] = {0};
	int rightAccu[3200] = {0};
	int delayIdx = 0;
	int leftCurrent, rightCurrent;
	
	while(1){
		
		if(audiop->rarc > 0){
			leftCurrent = audiop->ldata;
			rightCurrent = audiop->rdata;
		}else{
			leftCurrent = 0;
			rightCurrent = 0;
		}
        if(delayIdx < 3200){
            leftAccu[delayIdx] = leftCurrent + leftAccu[delayIdx] * damped;
            rightAccu[delayIdx] = rightCurrent + rightAccu[delayIdx] * damped;
        }else{
            delayIdx = 0;
            leftAccu[delayIdx] = leftCurrent + leftAccu[delayIdx] * damped;
            rightAccu[delayIdx] = rightCurrent + rightAccu[delayIdx] * damped;
        }
        
        audiop->ldata = leftAccu[delayIdx];
        audiop->rdata = rightAccu[delayIdx];
        delayIdx += 1;
    }
}
