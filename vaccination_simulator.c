#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> // for errno, the number of last error
#include <time.h>

#define MAX_NAME_LENGTH 64

int menu();

void choiceHandlerer(int choice);
bool validateDatas(char* name, int birth, char* phone, char extraService);

void registerPatient();
void modifyPatient();
void deletePatient();
void listOfPatients();
int startSimulation();
void setAllisVaccinatedToFalse();

void signalHandler();
int randomNumberGenerator(const int min, const int max);

struct Patient { 
    char name[MAX_NAME_LENGTH];
    char phoneNumber[16];
    int birthYear;
    int extraService;
    bool isVaccinated;
};


int main() {
    int choice = menu();
    choiceHandlerer(choice);
}

int menu()  {
    int choice = -1;

    printf("\n\nVakcin-acio: Adminisztracios panel\n");
    printf("Kerem valasszon a lenti lehetosegek kozul!\n");
    printf("------------------------------\n");
    printf("1. Jelentkezes felvetele\n");
    printf("2. Jelentkezes modositasa\n");
    printf("3. Jelentkezes torlese\n");
    printf("4. Jelentkezesi lista keszitese\n");
    printf("5. Oltasi szimulacio inditasa\n");
    printf("6. Mindenki ujra oltast akar!\n");
    printf("0. Kilepes a programbol\n");
    printf("Az On valasztasa: "); scanf("%d",&choice); getchar();
    fflush(stdin);

    return choice;
}
void choiceHandlerer(int choice) {
    switch(choice) {
        case 1: //Paciens felvétele
            registerPatient();
            fflush(stdin);
            break;

        case 2: // Paciens modositasa
            modifyPatient();
            fflush(stdin);
            break;

        case 3: // Paciens torlese
            deletePatient();
            fflush(stdin);
            break;

        case 4: // Jelentkezesi lista
            listOfPatients();
            fflush(stdin);
            break;
        
        case 5: //II. Beadandó feladata - Oltás szimuláció
            fflush(stdin);
            int result = startSimulation();
            fflush(stdin);
            if(result == 1)     choiceHandlerer(menu()); 
            else                return;

            break;
        
        case 6: 
            setAllisVaccinatedToFalse();
            fflush(stdin);
            break;

        case 0: // Kilépés
            printf("--- Kilepes a programbol --- \n");
            return;

        default: //Téves bemenet
            printf("\n\nTeves bemenet! A program biztonsagi okok miatt leall!\n\n");
            sleep(1); // átírni linuxban!
            exit(EXIT_FAILURE);
    }
}
bool validateDatas(char* name, int birth, char* phone, char extraService) {
    int countName = 0;
    int countPhone = 0;

    for(countName = 0; name[countName] != '\0'; countName++) {
        if(isdigit(name[countName]) != 0) {
            return false;
        }
    }
    if(countName <= 1) return false;

    for(countPhone = 0; phone[countPhone] != '\0'; countPhone++) {
        if(isalpha(phone[countPhone]) != 0) {
            return false;
        }
    }
    if(countPhone < 7 || countPhone >= 12) return false;

    if(birth <= 1900 || birth > 2021) {
        return false;
    }

    if(extraService == 'Y' || extraService == 'y' || extraService == 'I' || extraService == 'i' || extraService == 'N' || extraService == 'n')
        return true;
    else return false;

    return true;
}

void registerPatient() { 
    printf("\n--- Jelentkezes felvetele ---\n");
    char extraService;
    struct Patient patient;

    printf("Paciens neve: ");                           scanf("%[^\n]s",&patient.name);     getchar();   
    printf("Paciens szuletesi evszama: ");              scanf("%i",&patient.birthYear);     getchar();
    printf("Paciens telefonszama: ");                   scanf("%s",&patient.phoneNumber);   getchar();
    printf("Paciens keri a fizetos felarat?(I/N): ");   scanf("%c",&extraService);          getchar();

    if(!validateDatas(patient.name, patient.birthYear, patient.phoneNumber, extraService)) {
        printf("\n--- Sikertelen paciens regisztacio! ---");
        printf("\nAdatbevitel soran teves adatot adtak meg! Probalja ujra!\n");
        sleep(3); // átírni linuxban!
        choiceHandlerer(menu());
    }
    if(extraService == 'I' || extraService == 'i' || extraService == 'Y' || extraService == 'y')
        patient.extraService = 1;
    else if(extraService == 'N' || extraService == 'n')
        patient.extraService = 0;

    patient.isVaccinated = false;

    printf("\n--- Sikeres paciens regisztacio! ---");
    printf("\nRegisztralt paciens neve: %s", patient.name);
    printf("\nRegisztralt paciens szuletesi evszama: %i", patient.birthYear);
    printf("\nRegisztralt paciens telefonszama: %s", patient.phoneNumber);
    printf("\nRegisztralt paciens keri-e az extra szolgaltatast?: %s", patient.extraService ? "IGEN" : "NEM");

    FILE *outfile;
    outfile = fopen("datas.dat", "a");
    if (outfile == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit(1);
    }
    fwrite(&patient, sizeof(struct Patient), 1, outfile);

    if(fwrite != 0)     printf("\n\nAz adatokat sikeresen mentettuk a fajlba!\n");
    else                printf("Hiba keletkezett a fajlba iraskor!\n");

    fclose(outfile);

    sleep(3); // átírni linuxban!
    choiceHandlerer(menu());
}

void modifyPatient() {
    printf("--- Jelentkezes modositasa ---\n");

    //Összesítés és struct tömb létrehozása ->
    FILE *infileForModifySumming;
    struct Patient patientsDataForModifySumming;

    infileForModifySumming = fopen ("datas.dat", "r");
    if (infileForModifySumming == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit(1);
    }

    struct Patient modifiablePatients[128];
    int modifiablePatientsNum = 0;
    printf("\n");
    while(fread(&patientsDataForModifySumming, sizeof(struct Patient), 1, infileForModifySumming)) {
        printf ("Paciens neve: %s | Paciens szuletesi eve: %i | Paciens telefonszama %s | Extra szolgaltas: %s\n", 
            patientsDataForModifySumming.name, patientsDataForModifySumming.birthYear, patientsDataForModifySumming.phoneNumber, patientsDataForModifySumming.extraService ? "IGEN" : "NEM");
        
        modifiablePatients[modifiablePatientsNum++] = patientsDataForModifySumming;
    }
    fclose (infileForModifySumming);
    ////////////////////////////////////////////////////////////////////////////////////////////

    //Módosítandó elem kiválasztása -> 
    if(modifiablePatientsNum == 0) { 
        printf("\nNincs senki az adatbazisban! Modositas nem lehetseges!\n");
        sleep(3);
        choiceHandlerer(menu());
    }

    char modifiableName[MAX_NAME_LENGTH];
    int modifiablePosition;
    bool modifiableFound = false;

    printf("\nModositando paciens neve: "); scanf("%[^\n]s", &modifiableName); getchar();
    for(int i=0; i<modifiablePatientsNum; i++) {
        if(strcmp(modifiablePatients[i].name, modifiableName) == 0) {
            modifiablePosition = i;
            modifiableFound = true;
            break;
        }
    } 
    if(!modifiableFound) {
        printf("\nHiba! Nincs ilyen nev az adatbazisban!\n");
        sleep(3);
        choiceHandlerer(menu());
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    //Módosítás ->
    printf("\n\n-- Modositas megkezdve --\n\n");

    char newName[MAX_NAME_LENGTH];
    char newPhone[16];
    char newExtraServiceChar;
    int newBirth;
    int newExtraService;

    printf("Paciens uj neve: ");                            scanf("%[^\n]s", &newName);         getchar();
    printf("Paciens uj szuletesi dataum: ");                scanf("%i", &newBirth);             getchar();
    printf("Paciens uj telefonszama: ");                    scanf("%s", &newPhone);             getchar();
    printf("Paciens ker extra szolgaltatast? (I/N): ");     scanf("%c", &newExtraServiceChar);  getchar();

    if(!validateDatas(newName, newBirth, newPhone, newExtraServiceChar)) {
        printf("\n--- Sikertelen paciens modositas! ---");
        printf("\nAdatbevitel soran teves adatot adtak meg! Probalja ujra!\n");
        sleep(3); // átírni linuxban!
        choiceHandlerer(menu());
    }
    if(newExtraServiceChar == 'I' || newExtraServiceChar == 'i' || newExtraServiceChar == 'Y' || newExtraServiceChar == 'y')
        newExtraService = 1;
    else if(newExtraServiceChar == 'N' || newExtraServiceChar == 'n')
        newExtraService = 0;

    strcpy(modifiablePatients[modifiablePosition].name, newName);
    strcpy(modifiablePatients[modifiablePosition].phoneNumber, newPhone);
    modifiablePatients[modifiablePosition].birthYear = newBirth;
    modifiablePatients[modifiablePosition].extraService = newExtraService;

    FILE *outfileForModifyOverwriteing;
    outfileForModifyOverwriteing = fopen("datas.dat", "w+");
    if (outfileForModifyOverwriteing == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit (1);
    }

    for(int i=0; i<modifiablePatientsNum; i++)
        fwrite(&modifiablePatients[i], sizeof(struct Patient), 1, outfileForModifyOverwriteing);

    if(fwrite != 0) printf("\nSikeres modositas!\n");
    else            printf("Hiba keletkezett a fajlba iraskor!\n");

    fclose(outfileForModifyOverwriteing);
    ////////////////////////////////////////////////////////////////////////////////////////////

    sleep(3);
    choiceHandlerer(menu());
}

void deletePatient() {
    printf("--- Jelentkezes torlese ---\n");

    //Összesítés és struct tömb létrehozása ->
    FILE *infileForSumming;
    struct Patient patientsDataForSumming;

    infileForSumming = fopen ("datas.dat", "r");
    if (infileForSumming == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit (1);
    }

    struct Patient patients[128];
    int patientsNum = 0;
    printf("\n");
    while(fread(&patientsDataForSumming, sizeof(struct Patient), 1, infileForSumming)) {
        printf ("Paciens neve: %s | Paciens szuletesi eve: %i | Paciens telefonszama %s | Extra szolgaltas: %s\n", 
            patientsDataForSumming.name, patientsDataForSumming.birthYear, patientsDataForSumming.phoneNumber, patientsDataForSumming.extraService ? "IGEN" : "NEM");
        patients[patientsNum++] = patientsDataForSumming;
    }
    fclose (infileForSumming);
    ////////////////////////////////////////////////////////////////////////////////////////////

    //Törlendő elem kiválasztása -> 
    if(patientsNum == 0) { 
        printf("\nNincs senki az adatbazisban! Torles nem lehetseges!\n");
        sleep(3);
        choiceHandlerer(menu());
    }

    char deleteName[MAX_NAME_LENGTH];
    int deletePosition;
    bool found = false;

    printf("\nTorlendo paciens neve: "); scanf("%[^\n]s", &deleteName); getchar();
    for(int i=0; i<patientsNum; i++) {
        if(strcmp(patients[i].name, deleteName) == 0) {
            deletePosition = i;
            found = true;
            break;
        }
    } 
    if(!found) {
        printf("\nHiba! Nincs ilyen nev az adatbazisban!\n");
        sleep(3);
        choiceHandlerer(menu());
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    //Törlés ->
    for(int i=deletePosition; i<patientsNum-1; i++)
        patients[i] = patients[i+1];
    patientsNum--;

    FILE *outfileForOverwriteing;
    outfileForOverwriteing = fopen("datas.dat", "w+");
    if (outfileForOverwriteing == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit (1);
    }

    for(int i=0; i<patientsNum; i++)
        fwrite(&patients[i], sizeof(struct Patient), 1, outfileForOverwriteing);

    if(fwrite != 0) printf("\nSikeres torles!\n");
    else            printf("Hiba keletkezett a fajlba iraskor!\n");

    fclose(outfileForOverwriteing);
    ////////////////////////////////////////////////////////////////////////////////////////////

    sleep(3);
    choiceHandlerer(menu());
}

void listOfPatients() {
    printf("--- Jelentkezesi lista ---\n");
    FILE *infile;
    struct Patient patientInput;

    infile = fopen ("datas.dat", "r");
    if (infile == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit (1);
    }

    printf("\n");
    while(fread(&patientInput, sizeof(struct Patient), 1, infile))
        printf ("Paciens neve: %s | Paciens szuletesi eve: %i | Paciens telefonszama %s | Extra szolgaltas: %s | Oltva: %s\n", 
                patientInput.name, patientInput.birthYear, patientInput.phoneNumber, 
                patientInput.extraService ? "IGEN" : "NEM", patientInput.isVaccinated ? "IGEN" : "NEM");

    fclose (infile);

    sleep(3);
    choiceHandlerer(menu());
}

int startSimulation() {

    printf("\n\nOltasi szimulacio inditasa... Adatok osszegyujtese...\n\n");
    sleep(1);

    int day = 0;
    int needsToBeVaccinated = 0;
    do {
        //Páciensek struct tömbbe gyűjtése fájl alapján és inicializálás----->
        FILE *infileForSimulation;
        struct Patient patientsDataForSimulation;

        infileForSimulation = fopen ("datas.dat", "r");
        if (infileForSimulation == NULL) {
            fprintf(stderr, "\nHiba a datas.dat fajl megnyitasakor! Program leall!\n");
            exit(1);
        }

        struct Patient patients[128];
        int patientsNum = 0;
        needsToBeVaccinated = 0;

        printf("\n");
        while(fread(&patientsDataForSimulation, sizeof(struct Patient), 1, infileForSimulation)) {
            patients[patientsNum] = patientsDataForSimulation;
            if(!patients[patientsNum].isVaccinated)
                needsToBeVaccinated++;
            patientsNum++;
        }
        fclose (infileForSimulation);
        ////////////////////////////////////////////////////////

        day++;
        if(patientsNum <= 4 || needsToBeVaccinated <= 4) { 
            printf("Nincs eleg oltasra jelentkezo %i. napon, szimulacio leall!\n", day);
            sleep(3); 
            return 1;
        }

        
        signal(SIGUSR1, signalHandler);

        printf("=========================== UJ NAP INDUL ===========================\n");
        printf("%i paciens szeretne oltast %i. napon!\n", needsToBeVaccinated, day);
        sleep(3);

        if(needsToBeVaccinated > 4 && needsToBeVaccinated < 10) { // 1 oltóbusz indul
            printf("1 oltobusz fog indulni!\n\n");
            
            pid_t bus_1;

            int pipeToBus[2];
            int pipeFromBus[2];

            struct Patient readedWaitedPatients[5];
            struct Patient readedVaccinatedPatients[5];

            pipe(pipeToBus);
            pipe(pipeFromBus);

            if(( bus_1 = fork() ) == -1)         { perror("Hiba forkolaskor! Program leall...");     exit(EXIT_FAILURE); }

            if(bus_1 == 0) { // OLTÓBUSZ
                printf("OLTOBUSZ_1: kuldi HARCRA_FEL jelzest a torzsnek!\n");
                sleep(2);
                kill(getppid(), SIGUSR1);

                close(pipeToBus[1]);
                read(pipeToBus[0], readedWaitedPatients, sizeof(readedWaitedPatients));
                close(pipeToBus[0]);
                printf("OLTOBUSZ_1: megkapta a torzstol a mai napra az oltasra varok adatait!\n\n");

                for(int i=0; i<5; i++) {
                    printf("OLTOBUSZ_1: %i. oltasra varo szemely adatai: Nev: %s | Telefonszam: %s \n", i+1, readedWaitedPatients[i].name, readedWaitedPatients[i].phoneNumber);
                    sleep(1);
                }
                printf("\n\n");
                
                int vaccinatedDb = 0;
                for(int i=0; i<5; i++) {
                    srand((time(NULL)));
                    int rnd = randomNumberGenerator(1,10+1);

                    if(rnd != 10) { // Eljön
                        printf("%s el jott az oltasra\n", readedWaitedPatients[i].name);
                        printf("OLTOBUSZ_1: Elkezdi %s beoltasat...\n", readedWaitedPatients[i].name);
                        sleep(3);
                        readedWaitedPatients[i].isVaccinated = true;
                        vaccinatedDb++;
                    } else { // Nem jön el 
                        printf("%s nem jott el az oltasra\n", readedWaitedPatients[i].name);
                        printf("OLTOBUSZ_1: Elore veszi a kovetkezo pacienst...\n");
                        sleep(3);
                    }
                }
                
                needsToBeVaccinated -= vaccinatedDb;
                
                printf("\nOLTOBUSZ_1: A nap vegen osszesen %i pacienst sikerult beoltani! %i kell meg beoltani overall\n", vaccinatedDb, needsToBeVaccinated);
                sleep(5);
                printf("OLTOBUSZ_1: Elkezdi elkuldeni az adatokat a csovezeteken a torzsnek...\n");
                sleep(1);

                close(pipeFromBus[0]);
                write(pipeFromBus[1], readedWaitedPatients, sizeof(readedWaitedPatients));


                printf("OLTUBUSZ_1: Processz befejezodott!\n"); break;

            } else { // TÖRZS
                pause(); 
                printf("TORZS: HARCRA_FEL jelzes megerkezett! Torzs elkezdi kikuldeni az SMS-eket...\n\n",SIGUSR1);
                sleep(1);
                
                struct Patient waitedPatients[5];
                int waitedPatientsNum = 0;
                for(int i=0; i < patientsNum && waitedPatientsNum < 5; i++) {
                    if(!patients[i].isVaccinated) { 
                        waitedPatients[waitedPatientsNum] = patients[i];
                        printf("TORZS: SMS-t kuldott '%s' paciensnek, pacienst varjak az oltocbuszhoz!\n", waitedPatients[waitedPatientsNum].name);

                        waitedPatientsNum++;
                        sleep(1);
                    }
                }

                printf("\nTORZS: Elkezdi csovezeteken elkuldeni az adatokat az oltobusznak!\n\n"); 
                sleep(1);

                close(pipeToBus[0]);
                write(pipeToBus[1], waitedPatients, sizeof(waitedPatients));

                printf("TORZS: Elkuldte csovezeteken az osszes oltasra ertesitett szemely adatait az oltobusznak!\n");
                sleep(1);

                close(pipeFromBus[1]);
                read(pipeFromBus[0], readedVaccinatedPatients, sizeof(readedVaccinatedPatients));
                close(pipeFromBus[0]);
                printf("\nTORZS: Elkezdi megkapni az OLTBUSZ_1-tol a nap vegi adatokat!\n\n");
                sleep(1);


                //printf("TORZS: Osszesen %i paciens lett beoltva!\n", vaccinatedDb);
                for(int i=0; i<5; i++) {
                    if(readedVaccinatedPatients[i].isVaccinated) {
                        printf("TORZS: %s megkapta az oltast, elkezdi bejegyezni az adatallomanyba...\n", readedVaccinatedPatients[i].name);
                        sleep(2);
                    } else {
                        printf("TORZS: %s NEM kapta meg az oltast, mert nem jott el az olasra, igy nem tortenik bejegyzes az adatallomanyban!\n", readedVaccinatedPatients[i].name);
                        sleep(2);
                    }
                }
                printf("\n");
                
                for(int i=0; i<5; i++) {
                    for(int j=0; j<patientsNum; j++) {
                        if(strcmp(readedVaccinatedPatients[i].name, patients[j].name) == 0) {
                            if(readedVaccinatedPatients[i].isVaccinated) {
                                patients[j].isVaccinated = true; 
                                continue;
                            }
                        }
                    }
                }

                //Adatállomány felülírása---------------------------->
                FILE *outfileForOverwriteing;
                outfileForOverwriteing = fopen("datas.dat", "w+");
                if (outfileForOverwriteing == NULL) {
                    fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
                    exit (1);
                }

                for(int i=0; i<patientsNum; i++)
                    fwrite(&patients[i], sizeof(struct Patient), 1, outfileForOverwriteing);

                if(fwrite != 0) printf("\nAdatallomany sikeresen felul lett irva!\n");
                else            printf("Hiba keletkezett a fajlba iraskor!\n");

                fclose(outfileForOverwriteing);

                printf("TORZS: Processz befejezodott\n");


    
                //sleep(3);
                //choiceHandlerer(menu());
            }

        } else if(needsToBeVaccinated >= 10) { // 2 oltóbusz indul
            printf("2 oltobusz fog indulni!\n");

            pid_t bus_1, bus_2;

            int pipeToBus_1[2],     pipeToBus_2[2];
            int pipeFromBus_1[2],   pipeFromBus_2 [2];

            struct Patient readedWaitedPatients_1[5],       readedWaitedPatients_2[5];
            struct Patient readedVaccinatedPatients_1[5],   readedVaccinatedPatients_2[5];

            pipe(pipeToBus_1);      pipe(pipeToBus_2);
            pipe(pipeFromBus_1);    pipe(pipeFromBus_2);

            if(( bus_1 = fork() ) == -1)         { perror("Hiba forkolaskor! Program leall...");     exit(EXIT_FAILURE); }

            if(bus_1 == 0) { //1-es OLTÓBUSZ
            
                printf("OLTOBUSZ_1: kuldi HARCRA_FEL jelzest a torzsnek!\n");
                sleep(2);
                kill(getppid(), SIGUSR1);

                close(pipeToBus_1[1]);
                read(pipeToBus_1[0], readedWaitedPatients_1, sizeof(readedWaitedPatients_1));
                close(pipeToBus_1[0]);
                printf("OLTOBUSZ_1: megkapta a torzstol a mai napra az oltasra varok adatait!\n\n");
                sleep(3);

                for(int i=0; i<5; i++) {
                    printf("OLTOBUSZ_1: %i. oltasra varo szemely adatai: Nev: %s | Telefonszam: %s \n", 
                            i+1, readedWaitedPatients_1[i].name, readedWaitedPatients_1[i].phoneNumber);
                    sleep(1);
                }
                printf("\n\n");
                
                int vaccinatedDb = 0;
                for(int i=0; i<5; i++) {
                    srand((time(NULL)));
                    int rnd = randomNumberGenerator(1,10+1);

                    if(rnd != 10) { // Eljön
                        printf("%s el jott az oltasra\n", readedWaitedPatients_1[i].name);
                        printf("OLTOBUSZ_1: Elkezdi %s beoltasat...\n", readedWaitedPatients_1[i].name);
                        sleep(3);
                        readedWaitedPatients_1[i].isVaccinated = true;
                        vaccinatedDb++;
                    } else { // Nem jön el 
                        printf("%s nem jott el az oltasra\n", readedWaitedPatients_1[i].name);
                        printf("OLTOBUSZ_1: Elore veszi a kovetkezo pacienst...\n");
                        sleep(3);
                    }
                }
                needsToBeVaccinated -= vaccinatedDb;    
                printf("\nOLTOBUSZ_1: A nap vegen osszesen %i pacienst sikerult beoltani!\n", vaccinatedDb);
                sleep(5);

                printf("OLTOBUSZ_1: Elkezdi elkuldeni az adatokat a csovezeteken a torzsnek...\n");
                sleep(1);
                close(pipeFromBus_1[0]);
                write(pipeFromBus_1[1], readedWaitedPatients_1, sizeof(readedWaitedPatients_1));


                printf("OLTUBUSZ_1: Processz befejezodott!\n"); break;


            } else { 
                if(( bus_2 = fork() ) == -1)     { perror("Hiba forkolaskor! Program leall...");     exit(EXIT_FAILURE); }

                if(bus_2 == 0) { //2-es OLTÓBUSZ 
                    printf("OLTOBUSZ_2: kuldi HARCRA_FEL jelzest a torzsnek!\n");
                    sleep(2);
                    kill(getppid(), SIGUSR1);

                    close(pipeToBus_2[1]);
                    read(pipeToBus_2[0], readedWaitedPatients_2, sizeof(readedWaitedPatients_2));
                    close(pipeToBus_2[0]);
                    printf("OLTOBUSZ_2: megkapta a torzstol a mai napra az oltasra varok adatait!\n\n");
                    sleep(3);

                    for(int i=0; i<5; i++) {
                        printf("OLTOBUSZ_2: %i. oltasra varo szemely adatai: Nev: %s | Telefonszam: %s \n", 
                            i+1, readedWaitedPatients_2[i].name, readedWaitedPatients_2[i].phoneNumber);
                        sleep(1);
                    }
                    printf("\n\n");
                    
                    int vaccinatedDb = 0;
                    for(int i=0; i<5; i++) {
                        srand((time(NULL)));
                        int rnd = randomNumberGenerator(1,10+1);
                        rnd = randomNumberGenerator(1,10+1); // Még egyszer generálunk, hogy eltérő eredmény jöjjön ki!

                        if(rnd != 10) { // Eljön
                            printf("%s el jott az oltasra\n", readedWaitedPatients_2[i].name);
                            printf("OLTOBUSZ_2: Elkezdi %s beoltasat...\n", readedWaitedPatients_2[i].name);
                            sleep(3);
                            readedWaitedPatients_2[i].isVaccinated = true;
                            vaccinatedDb++;
                        } else { // Nem jön el 
                            printf("%s nem jott el az oltasra\n", readedWaitedPatients_2[i].name);
                            printf("OLTOBUSZ_2: Elore veszi a kovetkezo pacienst...\n");
                            sleep(3);
                        }
                    }
                    needsToBeVaccinated -= vaccinatedDb;    
                    printf("\nOLTOBUSZ_2: A nap vegen osszesen %i pacienst sikerult beoltani!\n", vaccinatedDb);
                    sleep(5);

                    printf("OLTOBUSZ_2: Elkezdi elkuldeni az adatokat a csovezeteken a torzsnek...\n");
                    sleep(1);
                    close(pipeFromBus_2[0]);
                    write(pipeFromBus_2[1], readedWaitedPatients_2, sizeof(readedWaitedPatients_2));

                    printf("OLTOBUSZ_2: Processz befejezodott!"); break;

                } else { // TÖRZS
                    pause(); 
                    printf("TORZS: HARCRA_FEL jelzes megerkezett az oltobuszoktol! Torzs elkezdi kikuldeni az SMS-eket...\n\n", SIGUSR1);
                    sleep(1);

                    struct Patient waitedPatients[10];
                    int waitedPatientsNum = 0;
                    for(int i=0; i < patientsNum && waitedPatientsNum < 10; i++) {
                        if(!patients[i].isVaccinated) { 
                            waitedPatients[waitedPatientsNum] = patients[i];
                            printf("TORZS: SMS-t kuldott '%s' paciensnek, pacienst varjak az oltocbuszhoz!\n", waitedPatients[waitedPatientsNum].name);

                            waitedPatientsNum++;
                            sleep(1);
                        }
                    }

                    printf("\nTORZS: Kette osztja az oltando pacienseket...\n"); sleep(3);

                    struct Patient waitedPatientsToBus_1[5];    int index_1 = 0;
                    struct Patient waitedPatientsToBus_2[5];    int index_2 = 0;
                    for(int i=0; i<10; i++) {
                        if(i < 5)   waitedPatientsToBus_1[index_1++] = waitedPatients[i];
                        else        waitedPatientsToBus_2[index_2++] = waitedPatients[i]; 
                    }

                    printf("\nTORZS: Elkezdi csovezeteken elkuldeni az adatokat az oltobuszoknak!\n\n"); 
                    sleep(1);

                    close(pipeToBus_1[0]);
                    write(pipeToBus_1[1], waitedPatientsToBus_1, sizeof(waitedPatientsToBus_1));

                    close(pipeToBus_2[0]);
                    write(pipeToBus_2[1], waitedPatientsToBus_2, sizeof(waitedPatientsToBus_2));

                    printf("TORZS: Elkuldte csovezeteken az osszes oltasra ertesitett szemely adatait az oltobuszoknak!\n");
                    sleep(1);

                    close(pipeFromBus_1[1]);
                    read(pipeFromBus_1[0], readedVaccinatedPatients_1, sizeof(readedVaccinatedPatients_1));
                    close(pipeFromBus_1[0]);
                    printf("\nTORZS: Elkezdi megkapni az OLTBUSZ_1-tol a nap vegi adatokat!\n\n");
                    sleep(1);

                    close(pipeFromBus_2[1]);
                    read(pipeFromBus_2[0], readedVaccinatedPatients_2, sizeof(readedVaccinatedPatients_2));
                    close(pipeFromBus_2[0]);
                    printf("\nTORZS: Elkezdi megkapni az OLTBUSZ_2-tol a nap vegi adatokat!\n\n");
                    sleep(1);

                    //Összeítés
                    printf("TORZS: Elkezdi osszesiteni az oltobuszoktol kapott adatokat...\n\n"); sleep(1);
                    struct Patient readedVaccinatedPatients[10];    int idx_1 = 0; int idx_2 = 0;
                    for(int i=0; i<10; i++) {
                        if(i<5)         readedVaccinatedPatients[i] = readedVaccinatedPatients_1[idx_1++];
                        else            readedVaccinatedPatients[i] = readedVaccinatedPatients_2[idx_2++];
                    }


                    //printf("TORZS: Osszesen %i paciens lett beoltva!\n", vaccinatedDb);
                    for(int i=0; i<10; i++) {
                        if(readedVaccinatedPatients[i].isVaccinated) {
                            printf("TORZS: %s megkapta az oltast, elkezdi bejegyezni az adatallomanyba...\n", readedVaccinatedPatients[i].name);
                            sleep(2);
                        } else {
                            printf("TORZS: %s NEM kapta meg az oltast, mert nem jott el az olasra, igy nem tortenik bejegyzes az adatallomanyban!\n", readedVaccinatedPatients[i].name);
                            sleep(2);
                        }
                    }
                    printf("\n");
                    
                    for(int i=0; i<10; i++) {
                        for(int j=0; j<patientsNum; j++) {
                            if(strcmp(readedVaccinatedPatients[i].name, patients[j].name) == 0) {
                                if(readedVaccinatedPatients[i].isVaccinated) {
                                    patients[j].isVaccinated = true; 
                                    continue;
                                }
                            }
                        }
                    }

                    //Adatállomány felülírása---------------------------->
                    FILE *outfileForOverwriteing;
                    outfileForOverwriteing = fopen("datas.dat", "w+");
                    if (outfileForOverwriteing == NULL) {
                        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
                        exit (1);
                    }

                    for(int i=0; i<patientsNum; i++)
                        fwrite(&patients[i], sizeof(struct Patient), 1, outfileForOverwriteing);

                    if(fwrite != 0) printf("\nAdatallomany sikeresen felul lett irva!\n");
                    else            printf("Hiba keletkezett a fajlba iraskor!\n");

                    fclose(outfileForOverwriteing);

                    printf("TORZS: Processz befejezodott\n");


                }
            }

        }
    }
    while(needsToBeVaccinated > 4);
}

void signalHandler() { printf("\nHARCRA_FEL szignal elkuldve!\n\n"); }

int randomNumberGenerator(const int min, const int max) {
    return ( rand() % (max-min) + min );
}


void setAllisVaccinatedToFalse() {
    FILE *infileForChange;
    struct Patient patientsData;

    infileForChange = fopen ("datas.dat", "r");
    if (infileForChange == NULL) {
        fprintf(stderr, "\nHiba a datas.dat fajl megnyitasakor! Program leall!\n");
        exit(1);
    }

    struct Patient patients[128];
    int patientsNum = 0;

    printf("\n");
    while(fread(&patientsData, sizeof(struct Patient), 1, infileForChange)) {
        patients[patientsNum] = patientsData;
        patientsNum++;
    }
    fclose (infileForChange);

    for(int i=0; i<patientsNum; i++) 
        patients[i].isVaccinated = false;

    //Adatállomány felülírása---------------------------->
    FILE *outfileForOverwriteing;
    outfileForOverwriteing = fopen("datas.dat", "w+");
    if (outfileForOverwriteing == NULL) {
        fprintf(stderr, "\nHiba a datas.txt fajl megnyitasakor! Program leall!\n");
        exit (1);
    }

    for(int i=0; i<patientsNum; i++)
        fwrite(&patients[i], sizeof(struct Patient), 1, outfileForOverwriteing);

    if(fwrite != 0) printf("\nAdatallomany sikeresen felul lett irva!\n");
    else            printf("Hiba keletkezett a fajlba iraskor!\n");

    fclose(outfileForOverwriteing);
    choiceHandlerer(menu()); 
}