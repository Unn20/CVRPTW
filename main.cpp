#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>
#include <climits>

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
bool SaveSolutionToFile(vector<Vehicle> &VehiclesVector) {
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
                if (VisitedCustomers[i]) continue;                                                                      //czy klient był już odwiedzony?
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
                if (VisitedCustomers[i]) continue;                                                                      //czy klient był już odwiedzony?
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

void TestingAtLesson() {
    double AlgTime;
    string Filename;
    vector<Customer> CustomersVector;
    vector<Vehicle> VehiclesVector;
    int VehicleCapacity;

    cout << "Nazwa pliku z instancją: ";
    cin >> Filename;
    if (ReadingFromFile(Filename, CustomersVector, VehicleCapacity)) cout << endl << "Dane wczytane" << endl << endl;
    else exit(-1);

    cout << "Czas działania algorytmu (w sekundach): ";
    cin >> AlgTime;

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
            exit(-1);
        } else {
            File << -1;
        }
    } else {
        TestRandomGreedy(CustomersVector, VehiclesVector, VehicleCapacity, DistancesMatrix, AlgTime);
        SolutionPrinting(VehiclesVector);
        SaveSolutionToFile(VehiclesVector);
    }

    for (int i = 0; i < CustomersVector.size(); i++)
        delete[] DistancesMatrix[i];
    delete[] DistancesMatrix;
}

int main() {
    //TestingInHome();
    TestingAtLesson();
    getchar();
    return 0;
}