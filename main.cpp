#include <iostream>
#include <math.h>
#include <cstring>
#include <list>

using namespace std; 

// Parameters for random number generation. 
#define MAX_INT 2147483647       // Maximum positive integer 2^31 - 1 
#define THE_END_OF_TIME 10080 // Simulation end time (minutes in a week)
#define LAMBDA_SING 3.0     // mean singing time
#define LAMBDA_SLEEP 30.0   // mean sleeping time
#define BIRDS_NUM 11    // number of budgies 

// Parameters for BLAN state 
#define QUIET_MODE 0 
#define SING_MODE 1
#define SQUAWKY_MODE 2

// information for a budgie 
typedef struct Budgies{
    int birdId;
    int state;
    float singDuration;
    float sleepDuration;
    float event_time;
} Budgie;

// network wide state
typedef struct BlanState{
    int birdId;
    int count;
    float singingBirds; 
    float blanQuiet;
    float quietStart;
    float blanSquawky;
    float squawkyStart;
    float melody;
    float melodyStart;
    int state;
    float perfectSongs; 
} BState;

list<Budgie> events; // list of events occuring in network 

// custom link comparison; for sorting events
bool compareEvent(const Budgie &first, const Budgie &second)
{
  return (first.event_time < second.event_time);
}

// Generate a random floating point value uniformly distributed in [0,1] 
float Uniform01(){
    float randnum;

    //get a random positive integer from random() 
    randnum = (float) 1.0 * random();

    // divide by max int to get something in 0..1
    randnum = (float) randnum / (1.0 * MAX_INT); 

    return( randnum );
}

// Generate a random floating point number from an exponential distribution with mean mu.
float Exponential( float mu){
    float randnum, ans;

    randnum = Uniform01();
    ans = -(mu) * log(randnum);

    return( ans );
}

int main(){
    int id = 0;
    float now, blanQuiet = 0.0, quietStart = 0.0, blanSquawky = 0.0, squawkyStart = 0.0, totalEvent = 0, melody =0.0, melodyStart = 0.0, totalEvents = 0.0;
    BState blanState;

    //srandom(SEED);

    // schedule sleeping; all budgies start off sleeping 
    for(int i =0; i < BIRDS_NUM; i++){

        //add new budgie
        Budgie newBudgie; 
        newBudgie.birdId = id; 
        newBudgie.state = QUIET_MODE; 
        newBudgie.sleepDuration = Exponential(LAMBDA_SLEEP); // determine sleep time
        newBudgie.event_time = newBudgie.sleepDuration;
        
        events.push_back(newBudgie);
        id++;
    }

    events.sort(compareEvent); // sort events by start time
    
    now = 0.0; 
    blanState.count = 0;
    blanState.blanQuiet = 0.0;
    blanState.blanSquawky = 0.0; 
    blanState.melody = 0.0; 
    blanState.melodyStart = 0.0; 
    blanState.quietStart = 0.0; 
    blanState.squawkyStart = 0.0;
    blanState.singingBirds = 0;  
    blanState.state = QUIET_MODE; 
    blanState.perfectSongs = 0; 
    while ((now < THE_END_OF_TIME) && (BIRDS_NUM > 0)){

        // get current event
        Budgie currentEvent = events.front();
        events.pop_front();
        now = currentEvent.event_time; // update time 

        //cout << "Event at: " << currentEvent.event_time << " Budgie ID: " << currentEvent.birdId << " State: " << currentEvent.state << endl;

        if ((currentEvent.state == SING_MODE) && (now <= THE_END_OF_TIME)){

            //schedule stop event 
            Budgie newBudgie; 
            newBudgie.birdId = currentEvent.birdId; 
            newBudgie.state = QUIET_MODE; 
            newBudgie.sleepDuration = Exponential(LAMBDA_SLEEP); // determine sleep time

            // If last event start time exceeds time limit, schedule it for the last possible event
            if (newBudgie.sleepDuration + now > THE_END_OF_TIME) {
                newBudgie.event_time = THE_END_OF_TIME;
            } else {
               newBudgie.event_time = now + currentEvent.singDuration; 
            }

            events.push_back(newBudgie);
            events.sort(compareEvent); // sort events by start time
            //cout <<" Budgie: "<< currentEvent.birdId << " sing time: " << currentEvent.singDuration << " will stop at: "<<newBudgie.event_time<< endl;

            if (blanState.count == 0){
                // update state variables
                if (blanState.state == SING_MODE){ // last state was Melodious 
                    blanState.melody += now - blanState.melodyStart;
                }else if (blanState.state == SQUAWKY_MODE) { // last state was Squawky 
                    blanState.blanSquawky += now - blanState.squawkyStart;
                }else if (blanState.state == QUIET_MODE) { // last state was Quiet  
                    blanState.blanQuiet += now - blanState.quietStart;
                }

                blanState.count = 1; // set mode to melodious state 
                blanState.singingBirds++; 
                blanState.birdId = currentEvent.birdId;
        
                //cout << "  " << blanState.singingBirds << " total birds singing"<< endl;
                blanState.state = SING_MODE; // set BLAN state to melodious
                blanState.melodyStart = now; // record melodious start time
            }else{
                blanState.singingBirds++; 
                //cout << "  " << blanState.singingBirds << " total birds singing now"<< endl;
            }
            
            // 1 or more birds are singing
            if (blanState.singingBirds > 1){
                // update state variables 
                if (blanState.state == SING_MODE){ // last state was Melodious 
                    blanState.melody += now - blanState.melodyStart;
                }else if (blanState.state == SQUAWKY_MODE) { // last state was Squawky 
                    blanState.blanSquawky += now - blanState.squawkyStart;
                }else if (blanState.state == QUIET_MODE) { // last state was Quiet  
                    blanState.blanQuiet += now - blanState.quietStart;
                }

                //cout << "  No of birds singing: " << blanState.singingBirds << " BLAN is squawky"<< endl;
                blanState.state = SQUAWKY_MODE; // set current state 
                blanState.squawkyStart = now; // set timer for squawky period 
            }
            totalEvents++; 
        }else{ // state is QUIET mode 

            // determine if there is only 1 bird singing at a given time (a perfect song)
            if (blanState.singingBirds == 1){
                blanState.perfectSongs++; 
            }
            if (blanState.singingBirds > 0){
                blanState.singingBirds--; // update singing birds 
            }

            //schedule sing event 
            Budgie newBudgie; 
            newBudgie.birdId = currentEvent.birdId; 
            newBudgie.state = SING_MODE; 
            float durationS = Exponential(LAMBDA_SING); // determine sing time
            newBudgie.singDuration = durationS;

            // If last event start time exceeds time limit, schedule it for the last possible event
            if (durationS + now > THE_END_OF_TIME) {
                newBudgie.event_time = THE_END_OF_TIME;
            } else {
                newBudgie.event_time = now + currentEvent.sleepDuration;
            }

            events.push_back(newBudgie);
            events.sort(compareEvent); // sort events by start time
            //cout <<" Budgie: "<< currentEvent.birdId << " sleep time: " << currentEvent.sleepDuration << " sing at: "<<newBudgie.event_time<< endl;

            // reset blan state
            if (blanState.birdId == currentEvent.birdId){
                blanState.birdId = 0;
                blanState.count = 0; 
            }
            
            // Blan is quiet 
            if (blanState.singingBirds == 0) {
                // update state variables 
                if (blanState.state == SING_MODE){ // last state was Melodious 
                    blanState.melody += now - blanState.melodyStart;
                }else if (blanState.state == SQUAWKY_MODE) { // last state was Squawky 
                    blanState.blanSquawky += now - blanState.squawkyStart;
                }else if (blanState.state == QUIET_MODE) { // last state was Quiet  
                    blanState.blanQuiet += now - blanState.quietStart;
                }
                
                blanState.state = QUIET_MODE; 
                blanState.quietStart = now; 
                //cout << "  Everyone is quiet. No of birds singing: " << blanState.singingBirds<< " Quiet time: "<< blanState.blanQuiet<<endl;
            }
            if (blanState.singingBirds == 1) {
                // update state variables 
                if (blanState.state == SING_MODE){ // last state was Melodious 
                    blanState.melody += now - blanState.melodyStart;
                }else if (blanState.state == SQUAWKY_MODE) { // last state was Squawky 
                    blanState.blanSquawky += now - blanState.squawkyStart;
                }else if (blanState.state == QUIET_MODE) { // last state was Quiet  
                    blanState.blanQuiet += now - blanState.quietStart;
                }
                
                blanState.state = SING_MODE; 
                blanState.melodyStart = now; 
                //cout << "  Melodious again. No of birds singing: " << blanState.singingBirds<< " Squawky time: "<< blanState.blanSquawky<<endl;
            }
        }
    }

    if (BIRDS_NUM == 0){
        blanState.blanQuiet = THE_END_OF_TIME;
    }

    // print final statistics 
    cout << "Total birds: " << BIRDS_NUM << " Mean Sing time: "<<LAMBDA_SING << " Mean Sleep time: "<< LAMBDA_SLEEP<< endl;
    cout << "Total time: "<< now << " Quiet time: "<< blanState.blanQuiet << " Melodious time: "<< blanState.melody<< " Squawky time: "<< blanState.blanSquawky<<endl;
    cout << "Proportion of time: " << " Quiet: "<< blanState.blanQuiet/now << " Melodious: "<< blanState.melody/now << " Squawky: "<< blanState.blanSquawky/now <<endl;
    cout << "Total songs: "<< totalEvents << " Perfect songs: "<< blanState.perfectSongs << " Proportion: " << blanState.perfectSongs/totalEvents<<endl;
    return 0; 
}
