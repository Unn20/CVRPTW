#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <climits>
#include <queue>

//generator liczb pseudolosowych
thread_local std::mt19937 gen{std::random_device{}()};

template<typename T>
T random(T min, T max) {
    return std::uniform_int_distribution<T>{min, max}(gen);
}

using namespace std;
using namespace std::chrono;

struct Customer {
    int CID, Xcoord, Ycoord, Demand, ReadyTime, DueTime, Service;
    Customer();
    ~Customer() = default;
};
Customer::Customer() {
    CID = 0;
    Xcoord = 0;
    Ycoord = 0;
    Demand = 0;
    ReadyTime = 0;
    DueTime = 0;
    Service = 0;
}
struct Vehicle {
    int Capacity;
    vector<Customer> Route;
    Vehicle(int VehicleCapacity);
    ~Vehicle() = default;
};
Vehicle::Vehicle(int VehicleCapacity) {
    Capacity = VehicleCapacity;
}
struct Result {
    int Vehicles = 0;
    long double Time = 0;
    Result(int x, long double y);
};
Result::Result(int x, long double y) {
    Vehicles = x;
    Time = y;
}

bool ReadingFromFile(string &Filename, vector<Customer> &CustomersVector, int &VehicleCapacity) {
    string Tmp[15];
    string Check[15] = {"VEHICLE", "NUMBER", "CAPACITY", "CUSTOMER", "CUST", "NO.", "XCOORD.", "YCOORD.", "DEMAND",
                        "READY", "TIME", "DUE", "DATE", "SERVICE", "TIME"};
    string ProblemID;
    int NumberOfVehicles;

    ifstream File;
    File.open(Filename);
    if (!File.good()) {
        cout << "Błąd otwarcia pliku" << endl;
        return false;
    } else {
        File >> ProblemID;

        for (int i = 0; i < 3; i++) {
            File >> Tmp[i];
            if (Tmp[i] != Check[i]) {
                cout << "Niepoprawny format danych" << endl;
                return false;
            }
        }
        File >> NumberOfVehicles >> VehicleCapacity;
        for (int i = 3; i < 15; i++) {
            File >> Tmp[i];
            if (Tmp[i] != Check[i]) {
                cout << "Niepoprawny format danych" << endl;
                return false;
            }
        }

        while (true) {
            Customer ToAdd;
            File >> ToAdd.CID >> ToAdd.Xcoord >> ToAdd.Ycoord >> ToAdd.Demand >> ToAdd.ReadyTime >> ToAdd.DueTime
                 >> ToAdd.Service;
            if (File.eof()) break;
            CustomersVector.push_back(ToAdd);
        }
    }
    File.close();
    return true;
};
bool ReadingFromFileInParts(string &Filename, vector<Customer> &CustomersVector, int &VehicleCapacity, int CustomersNumber) {
    string Tmp[15];
    string Check[15] = {"VEHICLE", "NUMBER", "CAPACITY", "CUSTOMER", "CUST", "NO.", "XCOORD.", "YCOORD.", "DEMAND",
                        "READY", "TIME", "DUE", "DATE", "SERVICE", "TIME"};
    string ProblemID;
    int NumberOfVehicles;

    ifstream File;
    File.open(Filename);
    if (!File.good()) {
        cout << "Błąd otwarcia pliku" << endl;
        return false;
    } else {
        File >> ProblemID;

        for (int i = 0; i < 3; i++) {
            File >> Tmp[i];
            if (Tmp[i] != Check[i]) {
                cout << "Niepoprawny format danych" << endl;
                return false;
            }
        }
        File >> NumberOfVehicles >> VehicleCapacity;
        for (int i = 3; i < 15; i++) {
            File >> Tmp[i];
            if (Tmp[i] != Check[i]) {
                cout << "Niepoprawny format danych" << endl;
                return false;
            }
        }

        for (int i = 0; i <= CustomersNumber; i++) {
            Customer ToAdd;
            File >> ToAdd.CID >> ToAdd.Xcoord >> ToAdd.Ycoord >> ToAdd.Demand >> ToAdd.ReadyTime >> ToAdd.DueTime
                 >> ToAdd.Service;
            if (File.eof()) break;
            CustomersVector.push_back(ToAdd);
        }
    }
    File.close();
    return true;
};
bool CheckCondition(vector<Customer> &CustomersVector, int &VehicleCapacity, long double **Matrix) {
    long double value;
    for (int i = 1; i < CustomersVector.size(); i++) {
        value = 0;
        if (CustomersVector[i].Demand > VehicleCapacity) return false;                                                  //czy zapotrzebowanie klienta nie jest większe od ładowności ciężarówki?
        if (Matrix[0][i] > CustomersVector[i].DueTime) return false;                                                    //czy do każdego sklepu można dojechać przed jego zamknieciem?
        value += max(Matrix[0][i], (long double) CustomersVector[i].ReadyTime);
        value += CustomersVector[i].Service;
        value += Matrix[i][0];
        if (value > CustomersVector[0].DueTime) return false;                                                           //czy zawsze zdaży się spowrotem do magazynu?
    }
    return true;
}
void CustomersPrinting(vector<Customer> &CustomersVector) {
    for (int i = 0; i < CustomersVector.size(); i++) {
        cout << "Klient numer: " << CustomersVector[i].CID << " Zapotrzebowanie: " << CustomersVector[i].Demand
             << " Gotowość: " << CustomersVector[i].ReadyTime << " Zamknięcie: " << CustomersVector[i].DueTime
             << " Czas obsługi: " << CustomersVector[i].Service << endl;
    }
    cout << endl;
}
long double DistanceEvaluator(Customer &A, Customer &B) {
    long double value = sqrt(
            (long double) pow((A.Xcoord - B.Xcoord), 2) + (long double) pow((A.Ycoord - B.Ycoord), 2));
    return value;
}
void DistanceMatrix(long double **Matrix, vector<Customer> &CustomersVector) {
    for (int i = 0; i < CustomersVector.size(); i++)
        for (int j = 0; j < CustomersVector.size(); j++) {
            if (i == j) Matrix[i][j] = 0;
            else Matrix[i][j] = DistanceEvaluator(CustomersVector[i], CustomersVector[j]);
        }
    return;
}
long double ServiceBeginning(vector<Customer> &Route, int No) {
    if (No == 0) return 0;
    long double value =
            ServiceBeginning(Route, No - 1) + Route[No - 1].Service + DistanceEvaluator(Route[No - 1], Route[No]);
    if (value > Route[No].ReadyTime) return value;
    else return Route[No].ReadyTime;
}                                                     //moment rozpoczęcia obsługi danego punktu
long double SolutionValue(vector<Vehicle> &VehiclesVector) {
    long double value = 0;
    for (int i = 0; i < VehiclesVector.size(); i++) {
        int LastCustomer = VehiclesVector[i].Route.size() - 1;
        value += ServiceBeginning(VehiclesVector[i].Route, LastCustomer) +
                  VehiclesVector[i].Route[LastCustomer].Service +
                  DistanceEvaluator(VehiclesVector[i].Route[LastCustomer], VehiclesVector[i].Route[0]);
    }
    return value;
}
void SolutionPrinting(vector<Vehicle> &VehiclesVector) {
    cout << VehiclesVector.size() << " " << SolutionValue(VehiclesVector) << endl;
    for (int i = 0; i < VehiclesVector.size(); i++) {
        cout << "Ciężarówka numer " << i << ": ";
        for (int j = 1; j < VehiclesVector[i].Route.size(); j++) {
            cout << VehiclesVector[i].Route[j].CID << " ";
        }
        cout << endl;
    }
}
bool SaveSolutionToFile(vector<Vehicle> &VehiclesVector, string output_filename) {
    ofstream File;
    File.open(output_filename);
    if (!File.good()) {
        cout << "Błąd otwarcia pliku do zapisu" << endl;
        return false;
    } else {
        File << VehiclesVector.size() << " " << setprecision(16) << SolutionValue(VehiclesVector) << "\n";
        for (int i = 0; i < VehiclesVector.size(); i++) {
            for (int j = 1; j < VehiclesVector[i].Route.size(); j++) {
                File << VehiclesVector[i].Route[j].CID << " ";
            }
            File << "\n";
        }
    }
    File.close();
    return true;
}

Result Test(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity) {
    for (int i = 1; i < CustomersVector.size(); i++) {
        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[i - 1].Route.push_back(CustomersVector[0]);
        VehiclesVector[i - 1].Route.push_back(CustomersVector[i]);
    }
    Result x = Result(VehiclesVector.size(), SolutionValue(VehiclesVector));
    return x;
}                                                                                                                       //przydzielanie każdemu klientowi jednej ciężarówki
void Test2(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity) {
    VehiclesVector.push_back(Vehicle(VehicleCapacity));
    for (int i = 0; i < CustomersVector.size(); i++) {
        VehiclesVector[0].Route.push_back(CustomersVector[i]);
    }
}              //przydzielenie jednej ciezarowki wszystkim klientom

void RandomAlg(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix){
    bool *VisitedCustomers = new bool[CustomersVector.size()];
    int *RandTable = new int[CustomersVector.size() - 1];                                                               //tablica do losowego posortowania
    for (int i = 0; i < CustomersVector.size(); i++) VisitedCustomers[i] = false;
    for (int i = 0; i < CustomersVector.size() - 1; i++) RandTable[i] = i + 1;

    for (int i = 0; i < CustomersVector.size() - 1; i++) {                                                              //losowanie indeksow
        int RandomNumber = random(0, (int) CustomersVector.size() - 2);
        int temp = RandTable[RandomNumber];
        RandTable[RandomNumber] = RandTable[i];
        RandTable[i] = temp;
    }

    bool Done = false;
    int CurrentVehicle = 0, RandCustomer = 0;

    while (!Done) {
        long double AvailableTime;
        int AvailableCapacity = VehicleCapacity, CurrentPosition = 0;

        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[0]);
        long double CurrentTime = 0;

        for (int i = 0; i < CustomersVector.size() - 1; i++) {

            RandCustomer = RandTable[i];
            if (VisitedCustomers[RandCustomer]) continue;
            else break;
        }                                                                                                               //szukanie losowego wierzcholka
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[RandCustomer]);
        VisitedCustomers[RandCustomer] = true;

        CurrentTime =
                max(Matrix[CurrentPosition][RandCustomer], (long double) CustomersVector[RandCustomer].ReadyTime) +
                CustomersVector[RandCustomer].Service;

        AvailableTime = CustomersVector[0].DueTime;                                                                     //dostepny czas na odwiedzanie
        AvailableCapacity -= CustomersVector[RandCustomer].Demand;                                                      //zapas ladunku w ciezarowce
        CurrentPosition = RandCustomer;                                                                                 //obecna pozycja

        bool Possible = true;
        int CustomerToVisit = 0;
        while (Possible) {                                                                                              //szukanie najblizszego wierzcholka z obecnej pozycji "rand"
            Possible = false;
            for (int i = 1; i < CustomersVector.size(); i++) {
                if (VisitedCustomers[i]) continue;                                                                      //czy klient był już odwiedzony?
                if (CustomersVector[i].Demand > AvailableCapacity) continue;                                            //czy ciężarówka ma dość ładunku, by obsłużyć danego klienta?
                if (CurrentTime + Matrix[CurrentPosition][i] > CustomersVector[i].DueTime) continue;                    //czy ciężarówka zdąży przyjechać przed zamknięciem sklepu?
                if (max(CurrentTime + Matrix[CurrentPosition][i], (long double) CustomersVector[i].ReadyTime) +
                    CustomersVector[i].Service + Matrix[i][0] > AvailableTime)
                    continue;                                                                                           //czy ciężarówka zdąży wrócić do magazynu przed jego zamknięciem?

                Possible = true;
                CustomerToVisit = i;
            }
            if (!Possible) break;

            VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[CustomerToVisit]);
            VisitedCustomers[CustomerToVisit] = true;
            CurrentTime += max(Matrix[CurrentPosition][CustomerToVisit],
                               (long double) CustomersVector[CustomerToVisit].ReadyTime - CurrentTime) +
                           CustomersVector[CustomerToVisit].Service;
            AvailableCapacity -= CustomersVector[CustomerToVisit].Demand;                                               //zapas ładunku pozostałego w ciężarówce
            CurrentPosition = CustomerToVisit;
        }

        CurrentVehicle++;
        Done = true;                                                                                                    //warunek zakonczenia petli po odwiedzeniu wszystkich wierzcholkow
        for (int i = 1; i < CustomersVector.size(); i++) {
            if (!VisitedCustomers[i])
                Done = false;
        }
    }

    delete[] VisitedCustomers;
    delete[] RandTable;
    return;
}
Result TestRandomAlg(vector<Customer> &CustomersVector, vector<Vehicle> &BestVector, int &VehicleCapacity, long double **DistancesMatrix, double &AlgTime){
    high_resolution_clock::time_point t1, t2;
    duration<double> Timer = (duration<double>) 0;
    t1 = high_resolution_clock::now();

    vector<Vehicle> TempVector;
    long double TimeResult, BestTime;
    int VehiclesResult, BestVehicles;

    RandomAlg(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix);
    BestTime = SolutionValue(BestVector);
    BestVehicles = (int) BestVector.size();
    cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime << endl;
    if (true){
        do {
            TempVector.clear();
            RandomAlg(CustomersVector, TempVector, VehicleCapacity, DistancesMatrix);
            TimeResult = SolutionValue(TempVector);
            VehiclesResult = (int) TempVector.size();

            if (TimeResult < BestTime && VehiclesResult == BestVehicles) {
                BestTime = TimeResult;
                //BestVehicles = VehiclesResult;
                BestVector.clear();
                for (int i = 0; i < TempVector.size(); i++) {
                    BestVector.push_back(TempVector[i]);
                }
                cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime
                     << endl;
            } else if (VehiclesResult < BestVehicles) {
                BestTime = TimeResult;
                BestVehicles = VehiclesResult;
                BestVector.clear();
                for (int i = 0; i < TempVector.size(); i++) {
                    BestVector.push_back(TempVector[i]);
                }
                cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime
                     << endl;
            }
            t2 = high_resolution_clock::now();
            Timer = duration_cast<duration<double>>(t2 - t1);
        } while (Timer.count() < AlgTime);
    }
    Result value = Result(BestVehicles, BestTime);
    return value;
}

void RandomGreedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix) {
    bool *VisitedCustomers = new bool[CustomersVector.size()];
    int *RandTable = new int[CustomersVector.size() - 1];                                                               //tablica do losowego posortowania
    for (int i = 0; i < CustomersVector.size(); i++) VisitedCustomers[i] = false;
    for (int i = 0; i < CustomersVector.size() - 1; i++) RandTable[i] = i + 1;

    for (int i = 0; i < CustomersVector.size() - 1; i++) {                                                              //losowanie indeksow
        int RandomNumber = random(0, (int) CustomersVector.size() - 2);
        int temp = RandTable[RandomNumber];
        RandTable[RandomNumber] = RandTable[i];
        RandTable[i] = temp;
    }

    bool Done = false;
    int CurrentVehicle = 0, RandCustomer = 0;

    while (!Done) {
        long double AvailableTime;
        int AvailableCapacity = VehicleCapacity, CurrentPosition = 0;

        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[0]);
        long double CurrentTime = 0;

        for (int i = 0; i < CustomersVector.size() - 1; i++) {                                                          //szukanie losowego wierzcholka

            RandCustomer = RandTable[i];
            if (VisitedCustomers[RandCustomer]) continue;
            else break;
        }
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[RandCustomer]);
        VisitedCustomers[RandCustomer] = true;

        CurrentTime =
                max(Matrix[CurrentPosition][RandCustomer], (long double) CustomersVector[RandCustomer].ReadyTime) +
                CustomersVector[RandCustomer].Service;

        AvailableTime = CustomersVector[0].DueTime;                                                                     //dostepny czas na odwiedzanie
        AvailableCapacity -= CustomersVector[RandCustomer].Demand;                                                      //zapas ladunku w ciezarowce
        CurrentPosition = RandCustomer;                                                                                 //obecna pozycja


        bool Possible = true;
        long double BestValue;
        long double PossibleValue = 0;
        int CustomerToVisit = 0;
        while (Possible) {                                                                                              //szukanie najblizszego wierzcholka z obecnej pozycji "rand"
            BestValue = INT_MAX;
            Possible = false;
            for (int i = 1; i < CustomersVector.size(); i++) {
                if (VisitedCustomers[i]) continue;                                                                      //czy klient był już odwiedzony?
                if (CustomersVector[i].Demand > AvailableCapacity) continue;                                            //czy ciężarówka ma dość ładunku, by obsłużyć danego klienta?
                if (CurrentTime + Matrix[CurrentPosition][i] > CustomersVector[i].DueTime) continue;                    //czy ciężarówka zdąży przyjechać przed zamknięciem sklepu?
                if (max(CurrentTime + Matrix[CurrentPosition][i], (long double) CustomersVector[i].ReadyTime) +
                    CustomersVector[i].Service + Matrix[i][0] > AvailableTime)
                    continue;                                                                                           //czy ciężarówka zdąży wrócić do magazynu przed jego zamknęciem?

                Possible = true;
                PossibleValue = max(Matrix[CurrentPosition][i],
                                     (long double) CustomersVector[i].ReadyTime - CurrentTime);
                if (PossibleValue < BestValue) {                                                                      //warunek sprawdza czy warto jechac do i-tego wierzcholka
                    BestValue = PossibleValue;
                    CustomerToVisit = i;
                }
            }
            if (!Possible) break;

            VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[CustomerToVisit]);
            VisitedCustomers[CustomerToVisit] = true;
            CurrentTime += max(Matrix[CurrentPosition][CustomerToVisit],
                               (long double) CustomersVector[CustomerToVisit].ReadyTime - CurrentTime) +
                           CustomersVector[CustomerToVisit].Service;
            AvailableCapacity -= CustomersVector[CustomerToVisit].Demand;                                                   //zapas pozostałego ładunku w ciężarówce
            CurrentPosition = CustomerToVisit;
        }

        CurrentVehicle++;
        Done = true;                                                                                                    //warunek zakonczenia petli po odwiedzeniu wszystkich wierzcholkow
        for (int i = 1; i < CustomersVector.size(); i++) {
            if (!VisitedCustomers[i])
                Done = false;
        }
    }

    delete[] VisitedCustomers;
    delete[] RandTable;
    return;
}
void Greedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix) {
    bool *VisitedCustomers = new bool[CustomersVector.size()];
    for (int i = 0; i < CustomersVector.size(); i++) VisitedCustomers[i] = false;

    bool Done = false;
    int CurrentVehicle = 0;
    int CustomerNo = 0;

    while (!Done) {
        long double AvailableTime = CustomersVector[0].DueTime;

        int AvailableCapacity = VehicleCapacity;
        long double CurrentTime = 0;
        int CurrentPosition = 0;

        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[0]);

        bool Possible = true;
        long double BestValue;
        long double PossibleValue = 0;
        int CustomerToVisit = 0;
        while (Possible) {                                                                                              //szukanie najblizszego wierzcholka
            BestValue = INT_MAX;
            Possible = false;
            for (int i = 1; i < CustomersVector.size(); i++) {
                if (VisitedCustomers[i]) continue;                                                                      //czy klient był już odwiedzony?
                if (CustomersVector[i].Demand > AvailableCapacity) continue;                                            //czy ciężarówka ma dość ładunku, by obsłużyć danego klienta?
                if (CurrentTime + Matrix[CurrentPosition][i] > CustomersVector[i].DueTime) continue;                    //czy ciężarówka zdąży przyjechać przed zamknięciem sklepu?
                if (max(CurrentTime + Matrix[CurrentPosition][i], (long double) CustomersVector[i].ReadyTime) +
                    CustomersVector[i].Service + Matrix[i][0] > AvailableTime)
                    continue;                                                                                           //czy ciężarówka zdąży wrócić do magazynu przed jego zamknęciem?

                Possible = true;
                PossibleValue = max(Matrix[CurrentPosition][i],
                                     (long double) CustomersVector[i].ReadyTime - CurrentTime);
                if (PossibleValue < BestValue) {                                                                        //warunek sprawdza czy warto jechac do i-tego wierzcholka
                    BestValue = PossibleValue;
                    CustomerToVisit = i;
                }
            }
            if (!Possible) break;

            VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[CustomerToVisit]);
            VisitedCustomers[CustomerToVisit] = true;
            CustomerNo++;
            CurrentTime += max(Matrix[CurrentPosition][CustomerToVisit],
                               (long double) CustomersVector[CustomerToVisit].ReadyTime - CurrentTime) +
                           CustomersVector[CustomerToVisit].Service;
            AvailableCapacity -= CustomersVector[CustomerToVisit].Demand;                                                   //zapas pozostałego ładunku w ciężarówce
            CurrentPosition = CustomerToVisit;
        }

        CurrentVehicle++;
        Done = true;                                                                                                    //warunek zakonczenia petli po odwiedzeniu wszystkich wierzcholkow
        for (int i = 1; i < CustomersVector.size(); i++) {
            if (!VisitedCustomers[i])
                Done = false;
        }
    }

    delete[] VisitedCustomers;
    return;
}
Result TestRandomGreedy(vector<Customer> &CustomersVector, vector<Vehicle> &BestVector, int &VehicleCapacity, long double **DistancesMatrix, double &AlgTime) {
    high_resolution_clock::time_point t1, t2;
    duration<double> Timer = (duration<double>) 0;
    t1 = high_resolution_clock::now();

    vector<Vehicle> TempVector;
    long double TimeResult, BestTime;
    int VehiclesResult, BestVehicles;

    Greedy(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix);
    BestTime = SolutionValue(BestVector);
    BestVehicles = (int) BestVector.size();
    cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime << endl;
    do {
        TempVector.clear();
        RandomGreedy(CustomersVector, TempVector, VehicleCapacity, DistancesMatrix);
        TimeResult = SolutionValue(TempVector);
        VehiclesResult = (int) TempVector.size();

        if (TimeResult < BestTime && VehiclesResult == BestVehicles) {
            BestTime = TimeResult;
            //BestVehicles = VehiclesResult;
            BestVector.clear();
            for (int i = 0; i < TempVector.size(); i++) {
                BestVector.push_back(TempVector[i]);
            }
            cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime
            << endl;
        } else if (VehiclesResult < BestVehicles) {
            BestTime = TimeResult;
            BestVehicles = VehiclesResult;
            BestVector.clear();
            for (int i = 0; i < TempVector.size(); i++) {
                BestVector.push_back(TempVector[i]);
            }
            cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime
            << endl;
        }
        t2 = high_resolution_clock::now();
        Timer = duration_cast<duration<double>>(t2 - t1);
    } while (Timer.count() < AlgTime);
    Result value = Result(BestVehicles, BestTime);
    return value;
}
double TestTimeGreedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **DistancesMatrix){
    high_resolution_clock::time_point t1, t2;
    t1 = high_resolution_clock::now();
    Greedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix);
    t2 = high_resolution_clock::now();
    return duration_cast<nanoseconds>(t2 - t1).count();
}


//TABU ALGORITHM

bool fitness(vector<Vehicle> &Base, vector<Vehicle> &Candidate){                    //funkcja zwraca True, jesli Candidate jest lepszy od Base
    if (Candidate.size() < Base.size())
        return true;
    else if (Candidate.size() == Base.size() && SolutionValue(Candidate) < SolutionValue(Base))
        return true;
    else
        return false;
}

bool member(queue< vector<Vehicle> > &Queue, vector<Vehicle> &Member){              //funkja zwraca True, jesli Member nalezy do kolejki Queue

    vector <Vehicle> temp;
    int que_size = Queue.size();
    bool result = false;
    bool conditionA, conditionB;
    for (int i = 0; i < que_size; i++)
    {
    temp.clear();
    temp = Queue.front();
    Queue.pop();
    conditionA = true, conditionB = true;
    if (result == false){
        if (temp.size() != Member.size())
            {Queue.push(temp); continue;}

        for (int j = 0; j < Member.size(); j++){
            if (Member[j].Route.size() != temp[j].Route.size() || conditionB == false)
                {conditionA = false; break;}
            for (int k = 0; k < Member[j].Route.size(); k++){
                if (Member[j].Route[k].CID != temp[j].Route[k].CID)
                    {conditionB = false; break;}
                }
            }
        if (conditionA && conditionB)
            result = true;
        }
    Queue.push(temp);
    }
    return result;
}

int vehicle_total_capacity(vector<Customer> &CustomersVector, Vehicle &truck)           //funkcja zwraca wartosc, ile ciezarowka ladunku zuzywa
{
    int capacity = 0;
    for (int i = 1; i < truck.Route.size(); i++)
        capacity += truck.Route[i].Demand;
    return capacity;
}

bool validate_route(vector<Customer> &CustomersVector, Vehicle &truck, long double **Matrix)   //nie sprawdza czy ciezarowka jest przeladowana; funkcja zwraca True jak droga jest poprawna
{
    int CurrentPosition, PreviousPosition = 0;
    long double CurrentTime = 0;
    for(int i = 1; i < truck.Route.size(); i++)
    {
        CurrentPosition = truck.Route[i].CID;
        CurrentTime += max(Matrix[PreviousPosition][CurrentPosition],
                               (long double) CustomersVector[CurrentPosition].ReadyTime - CurrentTime) +
                           CustomersVector[CurrentPosition].Service;
        if (CurrentTime - CustomersVector[CurrentPosition].Service > CustomersVector[CurrentPosition].DueTime)
            return false;
        PreviousPosition = CurrentPosition;
    }
    if (CurrentTime + Matrix[CurrentPosition][0] > CustomersVector[0].DueTime)
        return false;
    else
        return true;
}

bool VertexIns(vector<Customer> &CustomersVector, vector<Vehicle> &Primary, long double **Matrix)            //zwraca True jesli rozwiazanie sie zmienilo
{
    Vehicle TempTruck(Primary[0].Capacity);

    int vehicle_n0 = random(0, (int)(Primary.size() - 1));
    int vehicle_route_size = Primary[vehicle_n0].Route.size();
    int insert_node_n0 = random(1, vehicle_route_size - 1);

    int insert_node_CID = Primary[vehicle_n0].Route[insert_node_n0].CID;

    //szukaj ciezarowki do wsadzenia wierzcholka
    for (int i = 0; i < Primary.size(); i++)
    {
        if (i == vehicle_n0)                    //aby nie wlozyc wierzcholka w to samo miejsce
            continue;
        if (vehicle_total_capacity(CustomersVector, Primary[i]) + CustomersVector[insert_node_CID].Demand > Primary[i].Capacity) //czy starczy miejsca w ciezarowce
            continue;

        TempTruck.Route.clear();
        TempTruck.Route.push_back(CustomersVector[0]);
        bool pushed = false;
        long double PossibleTime, CurrentTime = 0;
        int CurrentPosition, PreviousPosition = 0;
        for(int j = 1; j < Primary[i].Route.size(); j++)
        {
            CurrentPosition = Primary[i].Route[j].CID;

            PossibleTime = CurrentTime + max(Matrix[PreviousPosition][insert_node_CID],
                               (long double) CustomersVector[insert_node_CID].ReadyTime - CurrentTime) +
                           CustomersVector[insert_node_CID].Service;
            CurrentTime += max(Matrix[PreviousPosition][CurrentPosition],
                               (long double) CustomersVector[CurrentPosition].ReadyTime - CurrentTime) +
                           CustomersVector[CurrentPosition].Service;
            if (PossibleTime < CurrentTime && !pushed)
            {
                TempTruck.Route.push_back(CustomersVector[insert_node_CID]);
                pushed = true;
                CurrentTime = PossibleTime;
                CurrentTime += max(Matrix[insert_node_CID][CurrentPosition],
                               (long double) CustomersVector[CurrentPosition].ReadyTime - CurrentTime) +
                           CustomersVector[CurrentPosition].Service;
            }

            TempTruck.Route.push_back(Primary[i].Route[j]);

            PreviousPosition = CurrentPosition;
        }
        if (pushed && validate_route(CustomersVector, TempTruck, Matrix))
        {

            Primary[i].Route.clear();
            for (int a=0; a < TempTruck.Route.size(); a++)
                Primary[i].Route.push_back(TempTruck.Route[a]);

            if (vehicle_route_size == 2)                    //jesli ciezarowka jechala tylko do jednego klienta to trzeba ja usunac
                Primary.erase(Primary.begin()+vehicle_n0);
            else                                            // a jak nie to trzeba wyrzucic wierzcholek z jej trasy
                Primary[vehicle_n0].Route.erase(Primary[vehicle_n0].Route.begin()+insert_node_n0);

            return true;
        }

    }
    return false;
}


void getNeighbors(vector<Customer> &CustomersVector, vector< vector<Vehicle> > &Neighborhood, vector<Vehicle> &Candidate, long double **Matrix, int Neighbors, int TabuMoves)   //szuka sasiedztwa rozwiazania
{
    int choice;
    vector<Vehicle> tempvector;

    for (int a = 0; a < Neighbors; a++)
    {
        tempvector.clear();
        for (int i = 0; i < Candidate.size(); i++)    //przepisanie Candidata do tempa
                tempvector.push_back(Candidate[i]);

        for (int b = 0; b < TabuMoves; b++)
            {
                choice = 0;
                switch(choice)                      //mozna doimplementowac wiecej tabu_moves jak ktos chce
                {
                case 0:
                    VertexIns(CustomersVector, tempvector, Matrix);
                    break;
                case 1:
                    //VertexEx()
                    break;
                }
            }
    Neighborhood.push_back(tempvector);
    }
    return;
}

void SortNeighbors (vector< vector<Vehicle> > &SNeighborhood)
{
    int index, min_vehicles;
    long double min_result;
    vector< vector<Vehicle> > SNeighborhood_temp;
    for (int i = 0; i<SNeighborhood.size(); i++)
        SNeighborhood_temp.push_back(SNeighborhood[i]);

    int iterations = SNeighborhood_temp.size();
    SNeighborhood.clear();
    for (int j = 0; j<iterations; j++)
    {
        min_result = 9999999;
        min_vehicles = 999999;
    for (int i = 0; i<SNeighborhood_temp.size(); i++)
    {
        if(SNeighborhood_temp[i].size() < min_vehicles)
        {
            min_vehicles = SNeighborhood_temp[i].size();
            min_result = SolutionValue(SNeighborhood_temp[i]);
            index = i;
        }
        else if (SNeighborhood_temp[i].size() == min_vehicles && SolutionValue(SNeighborhood_temp[i]) < min_result)
        {
            min_result = SolutionValue(SNeighborhood_temp[i]);
            index = i;
        }
    }
        SNeighborhood.push_back(SNeighborhood_temp[index]);
        SNeighborhood_temp.erase(SNeighborhood_temp.begin()+index);
    }

    SNeighborhood_temp.clear();
    return;
}

void TabuAlgorithm(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix, double &AlgTime, int TabuSize, int Neighbors, int TabuMoves){
    high_resolution_clock::time_point t1, t2;
    duration<double> Timer = (duration<double>) 0;
    t1 = high_resolution_clock::now();

    queue< vector<Vehicle> > TabuList;
    vector< vector<Vehicle> > SNeighborhood;

    vector<Vehicle> SBest;
    for (int i = 0; i < VehiclesVector.size(); i++) {
                SBest.push_back(VehiclesVector[i]);
            }
    vector<Vehicle> BestCandidate;
    for (int i = 0; i < VehiclesVector.size(); i++) {
                BestCandidate.push_back(VehiclesVector[i]);
            }

    VehiclesVector.clear();
    TabuList.push(VehiclesVector);

    bool Stop = false;
    while(!Stop)
    {
        t2 = high_resolution_clock::now();
        Timer = duration_cast<duration<double>>(t2 - t1);
        if (Timer.count() > AlgTime) Stop = true;

        SNeighborhood.clear();
        getNeighbors(CustomersVector, SNeighborhood, BestCandidate, Matrix, Neighbors, TabuMoves);
        SortNeighbors(SNeighborhood);

        for (int i = 0; i < SNeighborhood.size(); i++){
            if (!member(TabuList, SNeighborhood[i])){
            BestCandidate.clear();
                for (int j = 0; j < SNeighborhood[i].size(); j++) {
                    BestCandidate.push_back(SNeighborhood[i][j]);
                }
            }
        }

        if (fitness(SBest, BestCandidate)){
            SBest.clear();
            for (int j = 0; j < BestCandidate.size(); j++) {
                    SBest.push_back(BestCandidate[j]);
            }
        }

        TabuList.push((BestCandidate));

        if (TabuList.size() > TabuSize)
            TabuList.pop();
    }

    for (int i = 0; i < SBest.size(); i++) {
                VehiclesVector.push_back(SBest[i]);
            }

    return;
}




Result TestTabuAlgorithm(vector<Customer> &CustomersVector, vector<Vehicle> &BestVector, int &VehicleCapacity, long double **DistancesMatrix, double &AlgTime, int TabuSize, int Neighbors, int TabuMoves) {

    long double TimeResult, BestTime;
    int VehiclesResult, BestVehicles;

    //inicjowanie poczatkowego rozwiazania
    Greedy(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix);
    BestTime = SolutionValue(BestVector);
    BestVehicles = (int) BestVector.size();

    cout << "Liczba ciężarówek: " << BestVehicles << " Łączny czas: " << setprecision(10) << BestTime << endl;

    TabuAlgorithm(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix, AlgTime, TabuSize, Neighbors, TabuMoves);
    TimeResult = SolutionValue(BestVector);
    VehiclesResult = (int) BestVector.size();

    Result value = Result(BestVehicles, BestTime);
    return value;
}

//TABU ALGORITHM END

void TestingInHome() {
    double AlgTime;
    string Filename;
    vector<Customer> CustomersVector;
    vector<Vehicle> VehiclesVector;
    int VehicleCapacity;

    fstream ResultsFile;
    ResultsFile.open("testing.txt", ios::app);
    ResultsFile << "Liczba klientów \t" << "Liczba ciężarówek \t" << "Łączny czas \t" << endl;

    cout << "Nazwa pliku z instancją: ";
    cin >> Filename;

    cout << "Czas działania algorytmu (w sekundach): ";
    cin >> AlgTime;

    for (int CustomersNumber = 20; CustomersNumber <= 1000; CustomersNumber += 20) {
        if (ReadingFromFileInParts(Filename, CustomersVector, VehicleCapacity, CustomersNumber))
            cout << endl << "Dane dla " << CustomersNumber << " klientów wczytane" << endl << endl;
        else exit(-1);

        //alokowanie tablicy dwuwymiarowej dla odleglosci
        long double **DistancesMatrix = new long double *[CustomersVector.size()];
        for (int i = 0; i < CustomersVector.size(); i++) DistancesMatrix[i] = new long double[CustomersVector.size()];
        DistanceMatrix(DistancesMatrix, CustomersVector);
        //CustomersPrinting(CustomersVector);

        if (!CheckCondition(CustomersVector, VehicleCapacity, DistancesMatrix)) {
            cout << "Brak rozwiązania" << endl;
            ofstream File;
            File.open("solution.txt");
            if (!File.good()) {
                cout << "Błąd odczytu pliku do zapisu" << endl;
                File.close();
                exit(-1);
            } else {
                File << -1;
            }
        } else {
            Result x = TestRandomGreedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime);
            //Result x = TestRandomAlg(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime);
            ResultsFile << CustomersVector.size() - 1 << "\t" << x.Vehicles << "\t" << setprecision(10) << x.Time << endl;
            //ResultsFile << setprecision(10) << TestTimeGreedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix) << endl;
        }

        for (int i = 0; i < CustomersVector.size(); i++)
            delete[] DistancesMatrix[i];
        delete[] DistancesMatrix;
        CustomersVector.clear();
        VehiclesVector.clear();
    }
}

void TestingTabu(string input, string output, double time, int TabuSize, int Neighbors, int TabuMoves) {
    double AlgTime = time;
    string Filename;
    vector<Customer> CustomersVector;
    vector<Vehicle> VehiclesVector;
    int VehicleCapacity;

    Filename = input;
    if (ReadingFromFile(Filename, CustomersVector, VehicleCapacity)) cout << endl << "Dane wczytane" << endl << endl;
    else exit(-1);

    //alokowanie tablicy dwuwymiarowej dla odleglosci
    long double **DistancesMatrix = new long double *[CustomersVector.size()];
    for (int i = 0; i < CustomersVector.size(); i++) DistancesMatrix[i] = new long double[CustomersVector.size()];
    DistanceMatrix(DistancesMatrix, CustomersVector);
    //CustomersPrinting(CustomersVector);

    if (!CheckCondition(CustomersVector, VehicleCapacity, DistancesMatrix)) {
        cout << "Brak rozwiązania" << endl;
        ofstream File;
        File.open(output);
        if (!File.good()) {
            cout << "Błąd odczytu pliku do zapisu" << endl;
            exit(-1);
        } else {
            File << -1;
        }
    } else {
        TestTabuAlgorithm(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime, TabuSize, Neighbors, TabuMoves);
        SolutionPrinting(VehiclesVector);
        SaveSolutionToFile(VehiclesVector, output);
    }

    for (int i = 0; i < CustomersVector.size(); i++)
        delete[] DistancesMatrix[i];
    delete[] DistancesMatrix;
}

int main(int argc, char *argv[]) {
    //magiczne parametry
    double AlgTime = 300;
    int TabuSize = 7;
    int Neighbors = 11;
    int TabuMoves = 3;

    if (argc < 3){
    cout << "Usage: " << argv[0] << " input_filename output_filename algorithm_time<seconds> tabu_size<int_value> how_many_neighbors<int_value> tabu_moves_count<int_value>" << endl;
    cout << "Default time: " << (int)AlgTime << endl;
    cout << "Default tabu_size: " << TabuSize << endl;
    cout << "Default neighbors_count: " << Neighbors << endl;
    cout << "Default tabu_moves: " << TabuMoves << endl;
    exit(-1);
    }
    string input_filename = argv[1];
    string output_filename = argv[2];
    if (argc >= 4)
        AlgTime = atof(argv[3]);
    if (argc >= 5)
        TabuSize = atoi(argv[4]);
    if (argc >= 6)
        Neighbors = atoi(argv[5]);
    if (argc >= 7)
        TabuMoves = atoi(argv[6]);

    //string input_filename = "m2kvrptw-0.txt";
    //string output_filename = "solution_po.txt";
    //TestingInHome();
    TestingTabu(input_filename, output_filename, AlgTime, TabuSize, Neighbors, TabuMoves);
    //getchar();
    return 0;
}
