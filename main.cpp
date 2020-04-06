#include <iostream>
#include <math.h>
#include <cstring>
#include <list>

using namespace std; 

// Parameters for random number generation. 
#define MAX_INT 2147483647       // Maximum positive integer 2^31 - 1 
#define THE_END_OF_TIME 999999.9 // Simulation end time
#define LAMBDA_SING 3.0
#define LAMBDA_SLEEP 30.0
#define BIRDS_NUM 1 // number of budgies 
#define SING_MODE 0
#define QUIET_MODE 1 
#define SEED 1234

// information for a budgie 
typedef struct Budgies{
    int birdId;
    int state;
    float singDuration;
    float sleepDuration;
    float event_time;
} Budgie;

typedef struct BlanState{
    int birdId;
    int count;
    int duration; 
    bool isSquawky = false; 
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
    int id = 0, blanQuiet = 0, blanSquawky = 0, totalEvent = 0, blM =0;
    float now;
    BState blanMelodious;

    //srandom(SEED);

    // schedule singing
    for(int i =0; i < BIRDS_NUM; i++){

        //add new budgie
        Budgie newBudgie; 
        newBudgie.birdId = id; 
        newBudgie.state = SING_MODE; 
        newBudgie.singDuration = Exponential(LAMBDA_SING); // determine sing time
        newBudgie.event_time = newBudgie.singDuration;
        
        events.push_back(newBudgie);
        id++;
    }

    events.sort(compareEvent); // sort events by start time
    
    now = 0.0; 
    blanMelodious.count =0;
    while (now < THE_END_OF_TIME){

        // get current event
        Budgie currentEvent = events.front();
        events.pop_front();
        now += currentEvent.event_time; // update time 

        cout << "Current time: " << now << " Budgie ID: " << currentEvent.birdId << " State: " << currentEvent.state << endl;

        if (currentEvent.state == SING_MODE){
            //schedule stop event 
            Budgie newBudgie; 
            newBudgie.birdId = currentEvent.birdId; 
            newBudgie.state = QUIET_MODE; 
            newBudgie.singDuration = Exponential(LAMBDA_SING); // determine sing time
            newBudgie.event_time = currentEvent.singDuration + now;
            events.push_back(newBudgie);
            events.sort(compareEvent); // sort events by start time

            if (!blanMelodious.isSquawky){
                blanMelodious.birdId = currentEvent.birdId;
                blanMelodious.count = 1; 
                blanMelodious.duration = currentEvent.singDuration;
                blanMelodious.isSquawky = true; 
                blM++;
            }
             
            if ((blanMelodious.birdId != currentEvent.birdId)){
                blanSquawky++;
            }
        }else{ // state is QUIET mode 
            //schedule sing event 
            Budgie newBudgie; 
            newBudgie.birdId = currentEvent.birdId; 
            newBudgie.state = SING_MODE; 
            newBudgie.sleepDuration = Exponential(LAMBDA_SLEEP); // determine sleep time
            newBudgie.event_time = now + newBudgie.sleepDuration;            
            events.push_back(newBudgie);
            events.sort(compareEvent); // sort events by start time

            // reset blan state
            if (blanMelodious.birdId == currentEvent.birdId){
                blanMelodious.birdId = 0;
                blanMelodious.count = 0; 
                blanMelodious.duration = 0;
                blanMelodious.isSquawky = false; 
            }
            
            if ((blanMelodious.count == 0 ) && (!blanMelodious.isSquawky)){
                blanQuiet++; // one more quiet budgie 
            }
        }

        totalEvent++;
    }

    cout << "Total events: "<< totalEvent << " Quiet time: "<< blanQuiet << " Squawky time: "<< blanSquawky<< " Melodius time: "<< blM<<endl;
    //cout <<"Budgie id: "<< newBudgie.birdId << " sing time: " << newBudgie.singDuration << " sleep time: "<<newBudgie.sleepTime<< endl;

    return 0; 
}