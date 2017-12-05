#include <iostream>
#include <vector>

#include <math.h>

#include <simlib.h>


using namespace std;

const bool debug = false;

#define DEBUG(id, x) if (debug) cout << Time << " : " << id << " : "<< x << endl


long START = 0;
long END = 24 * 60;

double TRAINS_PER_HOUR = 15.32;                   // podla dat je priemer 15.32 vlaku za hodinu
//double TRAINS_PER_HOUR = 30;
long HOUR = 60;

double TIME_ON_RAILWAY = 7.61666;                     //podla dat je priemerny cas cakania na nastupisti 7:37 to je 7.61666 min
double FAST_TRAIN_PROBABILITY = 0.2486;               //podla dat je 24% pravdepodobnost ze vlak je rychlik

double DELAY_PROBABILITY = 0.086;                     //pravdepodobnost meskania vlaku je cca 8%

double delay_table [] = {  // CDF
    0.54375,    //5 min
    0.775,      //10 min
    0.85,       //15 min
    0.90625,    //20 min
    0.9375,     //25 min
    0.96875,    //30 min
    0.98125,    //35 min
    0.9875,     //40 min
    0.99375,    //45 min
};

double default_railway_probabilities [] = {
    0.1663974152,
    0.1615508885,
    0.169628433,
    0.1227786753,
    0.1195476575,
    0.1373182553,
    0.02746365105,
    0.02261712439,
    0.03554119548,
    0.03715670436
};

vector<double> railway_possibilities;
vector<double> railway_possibilities_CDF;

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

        double r = Random();

        if (r <= DELAY_PROBABILITY) {
            double rr = Random();

            if (rr <= delay_table[0]) Wait(5);
            else if (rr <= delay_table[1]) Wait(10);
            else if (rr <= delay_table[2]) Wait(15);
            else if (rr <= delay_table[3]) Wait(20);
            else if (rr <= delay_table[4]) Wait(25);
            else if (rr <= delay_table[5]) Wait(30);
            else if (rr <= delay_table[6]) Wait(35);
            else if (rr <= delay_table[7]) Wait(40);
            else if (rr <= delay_table[8]) Wait(45);
            else Wait(Uniform(50, 130));
        }

        long index = -1;

try_again_train:

        r = Random();
        for(long i = 0 ; i < NUM_OF_RAILS ; i++) {
            if (r <= railway_possibilities_CDF[i]) {
                index = i;
                goto sieze;
            }
        }

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
                train_queue.InsFirst(this);                 // fast train
            }
            Passivate();
            goto try_again_train;
        }
sieze:
        Seize(*rail[index]);
        Wait(Exponential(TIME_ON_RAILWAY));
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
        Train * t = new Train(/*is fast?*/Random() < FAST_TRAIN_PROBABILITY, ++last_train_id);
        TRAIN_COUNTER++;
        t->Activate();
        this->Activate(Time + Exponential(HOUR / TRAINS_PER_HOUR));
    }
};


int main() {

    double sum = 0;
    if (NUM_OF_RAILS == 1) {
        sum = 1;
        railway_possibilities_CDF.push_back(sum);
        goto next;
    }

    if ((sizeof(default_railway_probabilities) / sizeof(double)) == NUM_OF_RAILS) {
        railway_possibilities.assign(default_railway_probabilities, default_railway_probabilities + (sizeof(default_railway_probabilities) / sizeof(double)));
    } else {
        long num_of_prob = ((sizeof(default_railway_probabilities) / sizeof(double)));
        long diff = NUM_OF_RAILS - num_of_prob;

//        cout << diff << endl;

        for (long i = 0 ; i < num_of_prob ; i++){
            railway_possibilities.push_back(default_railway_probabilities[i]);
        }


        if (diff < 0) {

            for (long i = 0 ; i < abs(diff) ; i++) {
                double value = railway_possibilities.back();
//                cout << "value is " << value << " " << railway_possibilities.size() << endl;
                railway_possibilities.pop_back();
                cout << railway_possibilities.size() << endl;
                for (long j = 0 ; j < railway_possibilities.size() ; j++) {
//                    cout << "rv is " << railway_possibilities[j] << endl;
                    railway_possibilities[j] += value * railway_possibilities[j];
//                    cout << "rv is " << railway_possibilities[j] << endl << endl;
                }
            }

        } else {

            for (long i = 0 ; i < diff ; i++){
                double new_value = 0;
                for (long j = 0 ; j < railway_possibilities.size() ; j++) {
                    double tmp = railway_possibilities[j] * railway_possibilities[j];
                    railway_possibilities[j] -= tmp;
                    new_value += tmp;
                }
                railway_possibilities.push_back(new_value);
            }
        }
    }

    for(long i = 0 ; i < NUM_OF_RAILS ; i++) {
        sum += railway_possibilities[i];
        railway_possibilities_CDF.push_back(sum);
    }

next:

    // double sum = 0;
    // for (long i = 0 ; i < NUM_OF_RAILS ; i++){
    //     cout << railway_possibilities_CDF[i] << endl;
    //     sum += railway_possibilities_CDF[i];
    // }
    // cout << endl << sum << endl;

    //return 0;

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
