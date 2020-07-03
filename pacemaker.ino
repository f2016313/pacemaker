  #define pacing_wire 2
  unsigned long duration_moving_avg = 0;
  int ADC_in = 0;

  int thresh = 700; //must be reprogrammed to respond to the analog circuit driving the ADC input.
  bool in_pulse = false;
  unsigned long current_pulse_time = 0;
  unsigned long latched_pulse_time = 0;
  int target_bpm = 60;
  int sync_pulse = 60*1000000/target_bpm; 
  int hard_sync = 0;
  //hard sync is to be used for patients that have major arrhythmias or node blocks, normal mode is primarily for bradycardia
  unsigned long pulse_gap = 0;
  const int ideal_pulse_length = 560000; //microsec
  const int standard_pulse_duration = sync_pulse*0.90; //microseconds, 90% length of standard PQRST wave

void setup() {
  // ENVIRONMENT INITIALISATION ---------------------
  Serial.begin(115200);

  pinMode(2,OUTPUT);}

void loop() {
  unsigned long pulse_start, gap_start;
  // main logic algorithm
  hard_sync = digitalRead(4);
  if(!hard_sync) //NORMAL MODE --------------------------------------------------
  {

    //check for too long a gap against the average
    if(micros()>(duration_moving_avg+gap_start+latched_pulse_time*0.20))
    {
      send_pulse();
      duration_moving_avg = (micros()-gap_start+duration_moving_avg)/2;
      gap_start = micros();     
    }
    //do not need overflow protection here as all are unsigned and of the same data-type
    
    

    // BPM measure and average section -- START --------------
    
    ADC_in = analogRead(A0); //read ECG input
    if(ADC_in>thresh)
      {
        in_pulse = true;
        
        pulse_start = micros();
        
        if(pulse_start>gap_start)//timer overflow protection
         pulse_gap = pulse_start - gap_start;
        else
         pulse_gap = 4294967295 - gap_start + pulse_start;
         
        duration_moving_avg = (pulse_gap+duration_moving_avg)/2;
        
        while((analogRead(A0)>(thresh-20))&&(micros()<(pulse_start+standard_pulse_duration))); 
        //waiting for pulse to end
        //adding some hysterisis (thresh-20) to accomadate any noise/fluctuation in ECG signal --- USER PARAMETER
        gap_start = micros();
        if(micros()>pulse_start)//timer overflow protection
         current_pulse_time = micros() - pulse_start;
        else
         current_pulse_time = 4294967295 - pulse_start + micros();
      
        
        in_pulse = false;
        latched_pulse_time = current_pulse_time;
        current_pulse_time = 0;   
              
      }
      
    // BPM measure and average section -- END ----------------


    
  }

  else
  {
    delayMicroseconds(sync_pulse);
    send_pulse();  
  }
}

void send_pulse()
{
   digitalWrite(pacing_wire,1);
   delayMicroseconds(10000); //10ms pulse
   digitalWrite(pacing_wire,1);
}
