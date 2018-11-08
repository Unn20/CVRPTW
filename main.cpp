#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>

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
        File << VehiclesVector.size() << " " << SolutionValue(VehiclesVector) << "\n";
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
Greedy(vector<Customer> &CustomersVector, vector<Vehicle> &VehiclesVector, int &VehicleCapacity, long double **Matrix) {
    bool *VisitedCustomers = new bool[CustomersVector.size()];
    int *RandTable = new int[CustomersVector.size()];                           //tablica do losowego posortowania
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
        long double AvailableTime = 0;
        int AvailableCapacity = VehicleCapacity, CurrentPosition;

        VehiclesVector.push_back(Vehicle(VehicleCapacity));
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[0]);
        long double CurrentTime = 0;
        int CustomerNo = 0;

        //szukanie losowego wierzcholka
        for (int i = 1; i < CustomersVector.size(); i++) {
            RandCustomer = RandTable[i];
            if (VisitedCustomers[RandCustomer]) continue;
            else break;
        }
        VehiclesVector[CurrentVehicle].Route.push_back(CustomersVector[RandCustomer]);
        VisitedCustomers[RandCustomer] = true;
        CustomerNo++;

        CurrentTime = ServiceBeginning(VehiclesVector[CurrentVehicle].Route, CustomerNo) +
                      CustomersVector[RandCustomer].Service;;
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
            CurrentTime += Matrix[CurrentPosition][node_to_use] + CustomersVector[node_to_use].Service;
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

void TestGreedy(vector<Customer> &CustomersVector, vector<Vehicle> &BestVector, int &VehicleCapacity,
                long double **DistancesMatrix, double &AlgTime) {
    high_resolution_clock::time_point t1, t2;
    duration<double> Timer = (duration<double>) 0;
    t1 = high_resolution_clock::now();

    vector<Vehicle> TempVector;
    long double TimeResult, BestTime;
    int VehiclesResult, BestVehicles;
    int Epsilon = 100;

    Greedy(CustomersVector, BestVector, VehicleCapacity, DistancesMatrix);
    BestTime = SolutionValue(BestVector);
    BestVehicles = (int) BestVector.size();
    do {
        TempVector.clear();
        Greedy(CustomersVector, TempVector, VehicleCapacity, DistancesMatrix);
        TimeResult = SolutionValue(TempVector);
        VehiclesResult = (int) TempVector.size();

        if (TimeResult < BestTime) {
            BestTime = TimeResult;
            BestVehicles = VehiclesResult;
            BestVector.clear();
            for (int i = 0; i < TempVector.size(); i++) {
                BestVector.push_back(TempVector[i]);
            }
        } else if ((TimeResult < (BestTime + Epsilon)) && VehiclesResult < BestVehicles) {
            TempVector.clear();
            Greedy(CustomersVector, TempVector, VehicleCapacity, DistancesMatrix);
            TimeResult = SolutionValue(TempVector);
            VehiclesResult = (int) TempVector.size();

            if (TimeResult < BestTime) {
                BestTime = TimeResult;
                BestVehicles = VehiclesResult;
                BestVector.clear();
                for (int i = 0; i < TempVector.size(); i++) {
                    BestVector.push_back(TempVector[i]);
                }
            }
        }
        t2 = high_resolution_clock::now();
        Timer = duration_cast<duration<double>>(t2 - t1);
    } while (Timer.count() < AlgTime);
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
        } else {
            File << -1;
        }
    } else {
        TestGreedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime);
        SolutionPrinting(VehiclesVector);
        SaveToFile(VehiclesVector);
    }

    for (int i = 0; i < CustomersVector.size(); i++)
        delete[] DistancesMatrix[i];
    delete[] DistancesMatrix;
    getchar();
    return 0;
}
