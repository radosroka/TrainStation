#include <iostream>
#include <vector>

#include <simlib.h>


using namespace std;

#define DEBUG(id, x) cout << Time << " : " << id << " : "<< x << endl


const long NUM_OF_WINDOWS = 16;  // prepazok
Facility * window[NUM_OF_WINDOWS];


const long NUM_OF_RAILS = 10;
Facility * rail[NUM_OF_RAILS];

class Human : public Process {
public:
    long train_id;
    bool ready;

    Human(long dest) {
        DEBUG(dest, "Human constructor");
        this->train_id = dest;
        ready = false;
    }
    void Behavior() {
        DEBUG(train_id, "Human trying to seize");

        //todo when non is available
        long index = -1;
        for(long i = 0 ; i < NUM_OF_WINDOWS ; i++) {
            if(!window[i]->Busy()) {
                index = i;
                break;
            }
        }

        Seize(*window[index]);
        DEBUG(train_id, "Human seized");
        Wait(5);
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
        Activate(Time + 1);
    }
};

class Train : public Process {
public:
    long id;

    Train(long train_id) {
        DEBUG(train_id, "train constructor");
        this->id = train_id;
    }

    ~Train() {
        DEBUG(id, "Train destructor");

    }

    void Behavior() {
        PeopleGenerator *gen = new PeopleGenerator(id);
        gen->Activate();
        Wait(30);
        gen->Passivate();

        vector<Human*> passengers = gen->people;
        delete gen;
        //todo when non is available
        long index = -1;
        for(long i = 0 ; i < NUM_OF_RAILS ; i++) {
            if(!rail[i]->Busy()) {
                index = i;
                break;
            }
        }
        Seize(*rail[index]);
        Wait(10);

        std::vector<Human*> rest;
        for(auto passenger : passengers) {
            if(passenger->ready)passenger->Activate();
            else rest.push_back(passenger);
        }
        Release(*rail[index]);


        for(auto passenger : rest) {

        }

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
        Train * t = new Train(++last_train_id);
        t->Activate();
        DEBUG('X', "TrainGenerator behavior");
        this->Activate(Time + 1000);

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

    Init(0,100);
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
return 0;
}
