#include <iostream>
#include <vector>

#include <simlib.h>


using namespace std;

#define DEBUG(id, x) cout << Time << " : " << id << " : "<< x << endl


long START = 0;
long END = 24 * 60;

long TRAINS_PER_HOUR = 20;
long HOUR = 60;

long PEOPLE_COUNTER = 0;
long TRAIN_COUNTER = 0;
long GOT_IN_COUNTER = 0;


const long NUM_OF_WINDOWS = 16;  // prepazok
Facility * window[NUM_OF_WINDOWS];


const long NUM_OF_RAILS = 10;
Facility * rail[NUM_OF_RAILS];
Queue train_queue("Train Queue");

class Human : public Process {
public:
    long train_id;
    bool ready;

    Human(long dest) {
        DEBUG(dest, "Human constructor");
        this->train_id = dest;
        ready = false;
        PEOPLE_COUNTER++;
    }

    void Behavior() {
        DEBUG(train_id, "Human trying to seize");

        long index = -1;
        for(long i = 0 ; i < NUM_OF_WINDOWS ; i++) {
            if(!window[i]->Busy()) {
                index = i;
                break;
            }
        }

        if(index == -1) {
            index = 0;
            long size = window[index]->QueueLen();
            for(long i = 0 ; i < NUM_OF_WINDOWS ; i++) {
                if(size > window[i]->QueueLen()) {
                    size = window[i]->QueueLen();
                    index = i;
                }
            }
        }

        Seize(*window[index]);
        DEBUG(train_id, "Human seized");
        Wait(Exponential(0.7));
        Release(*window[index]);
        DEBUG(train_id, "Human released");
        ready = true;

        Passivate();

        DEBUG(train_id, "Passenger got in");
    }
};

class PeopleGenerator : public Event {
public:
    long train_id;
    vector<Human*> people;

    PeopleGenerator(long dest) {
        DEBUG(dest, "PeopleGenerator constructor");
        this->train_id = dest;
    }

    ~PeopleGenerator() {
        DEBUG(train_id, "PeopleGenerator destructor");
    }

    void Behavior(){
        Human * h = new Human(train_id);
        h->Activate();
        people.push_back(h);
        Activate(Time + Exponential(0.5));
    }
};

class Train : public Process {
public:
    bool fast;
    long id;

    long generated_people;
    long got_in;

    Train(long train_id, bool fast_train) {
        DEBUG(train_id, "train constructor");
        this->id = train_id;
        this->fast = fast_train;
        this->generated_people = 0;
        this->got_in = 0;
    }

    ~Train() {
        DEBUG(generated_people, "<---  Number of generated people");
        DEBUG(got_in, "<---  Number of people (got in)");
        DEBUG(id, "Train destructor");
    }

    void Behavior() {

        PeopleGenerator *gen = new PeopleGenerator(id);
        gen->Activate();
        Wait(30);                           // vlak je vygenerovany 30min skor ako dojde na nadrazi
        gen->Passivate();

        vector<Human*> passengers = gen->people;
        generated_people = passengers.size();

        delete gen;

        long index = -1;

try_again_train:

        for(long i = 0 ; i < NUM_OF_RAILS ; i++) {
            if(!rail[i]->Busy()) {
                index = i;
                break;
            }
        }

        if(index == -1) {
            if(!fast) {
                train_queue.InsLast(this);
            } else {
                train_queue.InsFirst(this);
            }
            Passivate();
            goto try_again_train;
        }

        Seize(*rail[index]);
        Wait(Exponential(10));

        Release(*rail[index]);

        for(auto passenger : passengers) {
            if(passenger->ready) {
                got_in++;
                GOT_IN_COUNTER++;
                passenger->Activate();
            }
        }

        if(train_queue.size() > 0) train_queue.GetFirst()->Activate();

    }
};

class TrainGenerator : public Event {
public:
    long last_train_id;

     TrainGenerator() {
         DEBUG('X', "TrainGenerator constructor");
         last_train_id = 0;
    }

    ~TrainGenerator() {
        DEBUG('X', "TrainGenerator destructor");
    }

    void Behavior() {
        DEBUG('X', "TrainGenerator behavior");
        Train * t = new Train(/*is fast?*/false, ++last_train_id);
        TRAIN_COUNTER++;
        t->Activate();
        this->Activate(Time + Exponential(HOUR / TRAINS_PER_HOUR));

    }
};


int main() {

    for(long i = 0 ; i < NUM_OF_RAILS ; i++) {

        //string name = "Railway" + to_string(i);
        //todo pridat Railway + cislo
        rail[i] = new Facility("Railway");
    }

    for(long i = 0 ; i < NUM_OF_WINDOWS ; i++) {

        //string name = "window" + to_string(i);
        //todo pridat window + cislo
        window[i] = new Facility("Window");
    }

    RandomSeed(time(0));

    Init(START, END);

    (new TrainGenerator)->Activate();
    Run();

    for(long i = 0 ; i < NUM_OF_WINDOWS ; i++) {
        window[i]->Output();
        delete window[i];
    }

    for(long i = 0 ; i < NUM_OF_RAILS ; i++) {
        rail[i]->Output();
        delete rail[i];
    }

    train_queue.Output();

    DEBUG(PEOPLE_COUNTER, "<--- PEOPLE");
    DEBUG(TRAIN_COUNTER, "<--- TRAINS");
    DEBUG(GOT_IN_COUNTER, "<--- GOT IN");
return 0;
}
