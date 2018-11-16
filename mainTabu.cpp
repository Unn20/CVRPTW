#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
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

bool CheckCondition(vector<Customer> &CustomersVector, int &VehicleCapacity, long double **Matrix) {
    long double value;
    for (int i = 1; i < CustomersVector.size(); i++) {
        value = 0;
        //czy ciezar nie jest za maly
        if (CustomersVector[i].Demand > VehicleCapacity) return false;
        //czy dojedzie przed koncem okna do klienta
        if (Matrix[0][i] > CustomersVector[i].DueTime) return false;
        value += max(Matrix[0][i], (long double) CustomersVector[i].ReadyTime);
        value += CustomersVector[i].Service;
        value += Matrix[i][0];
        //czy zdazy do depo
        if (value > CustomersVector[0].DueTime) return false;
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
    long double result = sqrt(
            (long double) pow((A.Xcoord - B.Xcoord), 2) + (long double) pow((A.Ycoord - B.Ycoord), 2));
    return result;
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
    long double Tmp =
            ServiceBeginning(Route, No - 1) + Route[No - 1].Service + DistanceEvaluator(Route[No - 1], Route[No]);
    if (Tmp > Route[No].ReadyTime) return Tmp;
    else return Route[No].ReadyTime;
} //moment rozpoczęcia obsługi danego punktu
long double SolutionValue(vector<Vehicle> &VehiclesVector) {
    long double Result = 0;
    for (int i = 0; i < VehiclesVector.size(); i++) {
        int LastCustomer = VehiclesVector[i].Route.size() - 1;
        Result += ServiceBeginning(VehiclesVector[i].Route, LastCustomer) +
                  VehiclesVector[i].Route[LastCustomer].Service +
                  DistanceEvaluator(VehiclesVector[i].Route[LastCustomer], VehiclesVector[i].Route[0]);
        //cout << ServiceBeginning(VehiclesVector[i].Route, LastCustomer) << ";" << VehiclesVector[i].Route[LastCustomer].Service << ";" << DistanceEvaluator(VehiclesVector[i].Route[LastCustomer], VehiclesVector[i].Route[0]) << endl;
    }
    return Result;
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

bool SaveToFile(vector<Vehicle> &VehiclesVector) {
    ofstream File;
    File.open("solution.txt");
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

void Test(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity) {
    for (int i = 1; i < CustomersVector.size(); i++) {
        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[i - 1].Route.push_back(CustomersVector[0]);
        VehiclesVector[i - 1].Route.push_back(CustomersVector[i]);
    }
} //przydzielanie każdemu klientowi jednej ciężarówki
void Test2(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity) {
    VehiclesVector.push_back(Vehicle(VehicleCapacity));
    for (int i = 0; i < CustomersVector.size(); i++) {
        VehiclesVector[0].Route.push_back(CustomersVector[i]);
    }
} //przydzielenie jednej ciezarowki wszystkim klientom
void
Random_Greedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix) {
    bool *VisitedCustomers = new bool[CustomersVector.size()];
    int *RandTable = new int[CustomersVector.size()-1];                           //tablica do losowego posortowania
    for (int i = 0; i < CustomersVector.size(); i++) VisitedCustomers[i] = false;
    for (int i = 0; i < CustomersVector.size() - 1; i++) RandTable[i] = i + 1;

    //losowanie indeksow
    for (int i = 0; i < CustomersVector.size() - 1; i++) {
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
        int CustomerNo = 0;
//
        //szukanie losowego wierzcholka
        for (int i = 0; i < CustomersVector.size() -1; i++) {

            RandCustomer = RandTable[i];
            if (VisitedCustomers[RandCustomer]) continue;
            else break;
        }
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[RandCustomer]);
        VisitedCustomers[RandCustomer] = true;
        CustomerNo++;

        CurrentTime = max(Matrix[CurrentPosition][RandCustomer], (long double)CustomersVector[RandCustomer].ReadyTime) + CustomersVector[RandCustomer].Service;

        AvailableTime = CustomersVector[0].DueTime;                         //dostepny czas na odwiedzanie
        AvailableCapacity -= CustomersVector[RandCustomer].Demand;                   //zapas ladunku w ciezarowce
        CurrentPosition = RandCustomer;                                     //obecna pozycja

        //szukanie najblizszego wierzcholka z obecnej pozycji "rand"
        bool possible = true;
        long double best_value;
        long double possible_value = 0;
        int node_to_use = 0;
        while (possible) {
            best_value = 9999999;
            possible = false;
            for (int i = 1; i < CustomersVector.size(); i++) {
                if (VisitedCustomers[i]) continue;       //czy odwiedzony
                if (CustomersVector[i].Demand > AvailableCapacity) continue;   //czy ciezarowka ma dosc ladunku
                if (CurrentTime + Matrix[CurrentPosition][i] > CustomersVector[i].DueTime) continue;
                if (max(CurrentTime + Matrix[CurrentPosition][i], (long double) CustomersVector[i].ReadyTime) +
                    CustomersVector[i].Service + Matrix[i][0] > AvailableTime)
                    continue; //czy starczy czasu

                possible = true;
                possible_value = max(Matrix[CurrentPosition][i],
                                     (long double) CustomersVector[i].ReadyTime - CurrentTime);
                if (possible_value < best_value) {       //warunek sprawdza czy warto jechac do i-tego wierzcholka
                    best_value = possible_value;
                    node_to_use = i;
                }
            }
            if (!possible) break;

            VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[node_to_use]);
            VisitedCustomers[node_to_use] = true;
            CurrentTime += max(Matrix[CurrentPosition][node_to_use], (long double)CustomersVector[node_to_use].ReadyTime - CurrentTime) + CustomersVector[node_to_use].Service;
            AvailableCapacity -= CustomersVector[node_to_use].Demand;          //zapas ladunku w ciezarowce
            CurrentPosition = node_to_use;
        }

        CurrentVehicle++;
        //warunek zakonczenia petli po odwiedzeniu wszystkich wierzcholkow
        Done = true;
        for (int i = 1; i < CustomersVector.size(); i++) {
            if (!VisitedCustomers[i])
                Done = false;
        }
    }

    delete[] VisitedCustomers;
    delete[] RandTable;
    return;
}

void
Greedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix) {
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

        //szukanie najblizszego wierzcholka z obecnej pozycji "rand"
        bool possible = true;
        long double best_value;
        long double possible_value = 0;
        int node_to_use = 0;
        while (possible) {
            best_value = 9999999;
            possible = false;
            for (int i = 1; i < CustomersVector.size(); i++) {
                if (VisitedCustomers[i]) continue;       //czy odwiedzony
                if (CustomersVector[i].Demand > AvailableCapacity) continue;   //czy ciezarowka ma dosc ladunku
                if (CurrentTime + Matrix[CurrentPosition][i] > CustomersVector[i].DueTime) continue;
                if (max(CurrentTime + Matrix[CurrentPosition][i], (long double) CustomersVector[i].ReadyTime) +
                    CustomersVector[i].Service + Matrix[i][0] > AvailableTime)
                    continue; //czy starczy czasu

                possible = true;
                possible_value = max(Matrix[CurrentPosition][i],
                                     (long double) CustomersVector[i].ReadyTime - CurrentTime);
                if (possible_value < best_value) {       //warunek sprawdza czy warto jechac do i-tego wierzcholka
                    best_value = possible_value;
                    node_to_use = i;
                }
            }
            if (!possible) break;

            VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[node_to_use]);
            VisitedCustomers[node_to_use] = true;
            CustomerNo++;
            CurrentTime += max(Matrix[CurrentPosition][node_to_use], (long double)CustomersVector[node_to_use].ReadyTime - CurrentTime) + CustomersVector[node_to_use].Service;
            AvailableCapacity -= CustomersVector[node_to_use].Demand;          //zapas ladunku w ciezarowce
            CurrentPosition = node_to_use;
        }

        CurrentVehicle++;
        //warunek zakonczenia petli po odwiedzeniu wszystkich wierzcholkow
        Done = true;
        for (int i = 1; i < CustomersVector.size(); i++) {
            if (!VisitedCustomers[i])
                Done = false;
        }
    }

    delete[] VisitedCustomers;
    return;
}

bool member(queue< vector<Vehicle> > &Queue, vector<Vehicle> Member);

void Test_Random_Heurestics(vector<Customer> &CustomersVector, vector<Vehicle> &BestVector, int &VehicleCapacity,
                long double **DistancesMatrix, double &AlgTime) {
    queue< vector<Vehicle> > TabuList;//

    high_resolution_clock::time_point t1, t2;
    duration<double> Timer = (duration<double>) 0;
    t1 = high_resolution_clock::now();

    vector<Vehicle> TempVector;
    long double TimeResult, BestTime;
    int VehiclesResult, BestVehicles;

    Random_Greedy(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix);
    BestTime = SolutionValue(BestVector);
    BestVehicles = (int) BestVector.size();
    do {
        TempVector.clear();
        Random_Greedy(CustomersVector, TempVector, VehicleCapacity, DistancesMatrix);

        if (TabuList.size() < 10){
            TabuList.push(TempVector);
            cout << "Pushed; " << endl;}
        cout << "Bool: " << member(TabuList, TempVector) << endl;

        TimeResult = SolutionValue(TempVector);
        VehiclesResult = (int) TempVector.size();

        if (TimeResult < BestTime && VehiclesResult == BestVehicles) {
            BestTime = TimeResult;
            //BestVehicles = VehiclesResult;
            BestVector.clear();
            for (int i = 0; i < TempVector.size(); i++) {
                BestVector.push_back(TempVector[i]);
            }
        }

        else if (VehiclesResult < BestVehicles) {
        BestTime = TimeResult;
        BestVehicles = VehiclesResult;
        BestVector.clear();
        for (int i = 0; i < TempVector.size(); i++) {
            BestVector.push_back(TempVector[i]);
            }
        }
        t2 = high_resolution_clock::now();
        Timer = duration_cast<duration<double>>(t2 - t1);
    } while (Timer.count() < AlgTime);
}

bool fitness(vector<Vehicle> &Base, vector<Vehicle> &Candidate){
    if (Candidate.size() > Base.size())
        return true;
    else if (SolutionValue(Candidate) > SolutionValue(Base) && Candidate.size() == Base.size())
        return true;
    else
        return false;
}

bool member(queue< vector<Vehicle> > &Queue, vector<Vehicle> Member){
    /*vector< vector<Vehicle> > List;
    int que_size = Queue.size();
    vector <Vehicle> temp;
    for (int i = 0; i < que_size; i++){
        temp = Queue.front();
        Queue.pop();
        List.push_back(temp);
        Queue.push(temp);
        temp.clear();
    }

    bool result = false;
    for (int i = 0; i < List.size(); i++){
    if (List[i].size() != Member.size()) continue;
        for (int j = 0; j < Member.size(); j++){
        if (List[i][j].Route.size() != Member[j].Route.size()) break;
            result = true;
            for (int k = 0; k < Member[j].Route.size(); k++){
            if (Member[j].Route[k].CID != List[i][j].Route[k].CID){ result = false; break;}
            }
        if (result == false) break;
        }
        if (result == true) return true;
    }
    return false;*/
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
            if (Member[j].Route.size() != temp[j].Route.size())
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
//void getNeighbors(vector< vector<Vehicle> > Neighborhood, vector<Vehicle> Candidate){}

void TabuAlgorithm(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix, int TabuSize, double &AlgTime){
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

        //getNeighbors(SNeighborhood, BestCandidate);

        for (int i = 0; i < SNeighborhood.size(); i++){
            if (!member(TabuList, SNeighborhood[i]) && fitness(BestCandidate, SNeighborhood[i])){
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



int main() {
    double AlgTime;
    string Filename;
    vector<Customer> CustomersVector;
    vector<Vehicle> VehiclesVector;
    int VehicleCapacity;

    cout << "Nazwa pliku z instancją: ";
    cin >> Filename;
    if (ReadingFromFile(Filename, CustomersVector, VehicleCapacity)) {
        cout << endl << "Dane wczytane" << endl << endl;
    }
    else exit(-1);
    cout << "Czas działania algorytmu (w sekundach): ";
    cin >> AlgTime;
    //alokowanie tablicy dwuwymiarowej dla odleglosci
    long double **DistancesMatrix = new long double *[CustomersVector.size()];
    for (int i = 0; i < CustomersVector.size(); i++)
        DistancesMatrix[i] = new long double[CustomersVector.size()];
    DistanceMatrix(DistancesMatrix, CustomersVector);
    CustomersPrinting(CustomersVector);

    //Test(CustomersVector, VehiclesVector, VehicleCapacity);
    //Test2(CustomersVector, VehiclesVector, VehicleCapacity);

    if (!CheckCondition(CustomersVector, VehicleCapacity, DistancesMatrix)) {
        cout << "Brak rozwiązania" << endl;
        ofstream File;
        File.open("solution.txt");
        if (!File.good()) {
            cout << "Błąd odczytu pliku do zapisu" << endl;
            exit(-1);
        } else {
            File << -1;
        }
    } else {
        Test_Random_Heurestics(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime);
        //Greedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix);                              //dla greedy nie trzeba podawac czasu dzialania algorytmu
        //SolutionPrinting(VehiclesVector);
        SaveToFile(VehiclesVector);
    }

    for (int i = 0; i < CustomersVector.size(); i++)
        delete[] DistancesMatrix[i];
    delete[] DistancesMatrix;
    getchar();
    return 0;
}
