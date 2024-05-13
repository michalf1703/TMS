////////////////////////////////////////////////////////
///////      Podstawowa konfiguracja
////////////////////////////////////////////////////////

//#define CPP_EXAMPLE

#define BUFFERSYNC

#define WIN_PLOT
#define NazwaPlikuDanych "Data/TMSLABoutputData.csv"
#define CSV_NAGLOWEK "Wsp. x,Wsp. y1,Wsp. y2\n"
#define CSV_DANE "%i,%i,%i\n",Tim, y[0], y[1]

////////////////////////////////////////////////////////


#include "main.h"

#ifdef TMSLAB_WIN
  #include "stdio.h"
#endif

unsigned long *ekran; // Adres obszaru graficznego LCD [8*128*2]
#ifdef TMSLAB_C2000
unsigned char *textEkran; // Adres obszaru tekstowego [40*16/2]
#endif

#ifdef TMSLAB_WIN
unsigned short int *textEkran; // Adres obszaru tekstowego [40*16/2]
extern int (*PartialRefresh)();
char credits[43]="-               DEMO DISC                -";  // Tekst wyswietlany w naglowku symulatora
long Timer2IsrPeriod=1; // okres pracy symulowanego licznika Timer2 podany w przybliï¿½eniu w ms
#ifdef WIN_PLOT
  FILE* outputCSV=0;
#endif
  #endif

int Tim = 0;                // Licznik uzytkownika
unsigned int preScale = 0;  // Preskaler licznika uzytkownika
volatile char EnableRefresh = 0;    //Zezwolenie na odswiezenie zawartosci pamieci graficznej

R_P_LCD_TMSLAB LCD;             // Obiekt obslugujacy LCD
R_P_KEYBOARD_TMSLAB KEYBOARD;   // Obiekt obslugujacy klawiature
R_P_LEDBAR_TMSLAB LEDBAR;       // Obiekt obslugujacy diody LED


//Tablice danych/obiektow graficznych
#define MaxObj 200
#ifdef CPP_EXAMPLE
square objects[MaxObj];
#else
int dx[MaxObj];
int dy[MaxObj];
int s[MaxObj];
int x[MaxObj];
int y[MaxObj];
#endif
//Regulator tworzenie zmiennych globalnych
int Zadana = 0;
int Obciazenie = 0;
float SprzezenieZwrotne = 0;
int WykresZadana [240] = {0};
int WykresObroty [240] = {0};
int DlugoscWykresu = 0;
unsigned int Preskaler100ms = 0;
long PreskalerObroty = 0;
long Obroty = 0;

int WykresUchyb [240] = {0};
bool widocznosZadana = false;
bool widocznoscUchyb = false;
bool widocznoscObroty = false;
//
//Regulator koniec tworzenia zmiennych globalnych

//REgulator tworzenie obiektow
PID* Regulator = new PID(0.1,0.3,0.8,0.1,-500,500);
InercModel* Silnik = new InercModel(0.3);
UAR* SilnikObciazony = new UAR();
//Koniec tworzenia obiektow
unsigned char Bufor[] = "KCode:  ";

    int main()

    {
        SetUpPeripherials();
#ifdef TMSLAB_C2000
        LCD.LCD_Init(ekran, textEkran);
#endif

#ifdef TMSLAB_WIN
        LCD.LCD_Init(&ekran,&textEkran);
#ifdef WIN_PLOT
  outputCSV=fopen(NazwaPlikuDanych,"w+" );
  fprintf(outputCSV,CSV_NAGLOWEK);
#endif
#endif

        KEYBOARD.InitKB(100);

        LEDBAR.InitLedBar();

        InitData();

        EnableInterrupts();

        while (1)
        {
           EnableRefresh = 1;
            LCD.Synchronize();
            EnableRefresh = 0;

          //  unsigned char Key = KEYBOARD.GetKey();
            unsigned char Key = KEYBOARD.GetKey();
            if(Key==10){ if(Zadana<500) Zadana++; }
            if(Key==11){ if(Zadana>0) Zadana--; }
            if(Key==6){ if(Obciazenie<100) Obciazenie++; }
            if(Key==7){ if(Obciazenie>-100) Obciazenie--; }

            if(Key==16){widocznosZadana = true;}
            if(Key==15){widocznoscUchyb = true;}
            if(Key==14){widocznoscObroty = true;}
            if(Key==8){widocznosZadana=false; widocznoscUchyb=false; widocznoscObroty=false; }













            //PRZERWANIE TO TRZEBA PRZZENIES NA ZAJECIAHC DO INNEGO PRZERWANIA

           CzyscPodWykresem();
                                      //WYSWIETLENIE WYKRESU
           //1.04
           RysujWykresyVER2(Zadana, Zadana - SprzezenieZwrotne, SilnikObciazony->getOutput(),widocznosZadana, widocznoscUchyb, widocznoscObroty);
           //
           //                          RysujWykresy(Zadana, SilnikObciazony->getOutput());
                                      //KONIEC WYSWIETLANIA WYKRESU

                                      //WYSWIETLANIE DANYCH TEXTOWYCH
                                      WyswietlDaneTextowe(Zadana, Zadana - SprzezenieZwrotne, Regulator->getOutput(), Obciazenie, SilnikObciazony->getOutput());
                                      //KONIEC WYSWIETLANIA TEXTOWEGO
                          //}
                          //



                                  //OBROTY

                                  CzyscPodSilnikiem();
                                  RysujOkrag();
                                  RysujObrot(Obroty%8);




                         //---------------------------------------------------------------













           // LEDBAR.SetValue(Tim);

           // Bufor[6] = Key / 10 + '0';
            //Bufor[7] = Key % 10 + '0';
           // PrintText(textEkran, Bufor, 8, 16, 7);
            //ClearScreen();
           // DrawPixels(Key);
#ifdef TMSLAB_WIN
            if(PartialRefresh()) return 0;
#ifdef WIN_PLOT
	// Zapis danych do pliku
	fprintf(outputCSV,CSV_DANE);
	printf("time %i \n",Tim);
	fflush(outputCSV);
	fflush(stdout);
#endif
#endif

        }
    }

#ifdef TMSLAB_C2000

    interrupt
    void Timer2Isr()
    {

#ifdef BUFFERSYNC
        if (EnableRefresh)
        LCD.PartialRefresh();
#else
        LCD.PartialRefresh();
#endif

        KEYBOARD.PartialRefresh();

        if (++preScale == 50000)
        {
            preScale = 0;
            Tim++;
        }


        if(++Preskaler100ms==10000)
        {
       Preskaler100ms=0;
        //UKLAD ZAMKNIETY
       //CzyscEkran();


                                              Regulator->setInput(Zadana - SprzezenieZwrotne);
                                              Regulator->Calculate();

                                              Silnik->setInput(Regulator->getOutput());
                                              Silnik->Calculate();

                                              SilnikObciazony->setInput(Silnik->getOutput() - Obciazenie);
                                              SilnikObciazony->Calculate();

                                              SprzezenieZwrotne = SilnikObciazony->getOutput();




                                              //KONIEC UKLAD ZAMKNIETY
    }







                                              if(SprzezenieZwrotne>0)
                                                               {
                                                  if(++PreskalerObroty>=750000/SprzezenieZwrotne)
                                                    { PreskalerObroty=0;





                                                    Obroty++;}
                                                               }







    }

    unsigned long ADRFTECHED = 0;
    interrupt
    void NoIsr()
    {
        ADRFTECHED = PieCtrlRegs.PIECTRL.bit.PIEVECT;
        asm(" ESTOP0");
    }

    void EnableInterrupts()
    {

        EALLOW;
        //Ustawienie wektorow przerwan
        unsigned long VECTBEG = (unsigned long) &PieVectTable;
        unsigned long VECTLAST = (unsigned long) &PieVectTable
        + sizeof(PieVectTable);
        while (VECTBEG >= VECTLAST)
        *(unsigned long*) VECTBEG++ = (unsigned long) NoIsr;
        PieVectTable.TIMER2_INT = Timer2Isr;

        CpuTimer2Regs.TCR.bit.TIE = 1;
        CpuTimer2Regs.TCR.bit.TRB = 1;

        IER = IER_MASK;//Odblokuj przerwania
        asm(" push ier");
        asm(" pop dbgier");

        PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
        PieCtrlRegs.PIEACK.all = 0xffff;
        EDIS;
        EINT;
    }

    void SetUpPeripherials()
    {
        SetupCoreSystem();
        ekran = (unsigned long *) 0x8000;              //[8*128*2]
        textEkran = (unsigned char*) 0x8a00;//[40*16/2]
        EALLOW;
        //Okres licznika T2
        CpuTimer2Regs.PRD.all = System_Clk * Timer2ISR_Period;
        EDIS;
    }
    extern "C"
    {
    int _system_pre_init()
    {
        EALLOW;
        WdRegs.WDWCR.all = 0x68;
        EDIS;
        return (1);
    }
    }
#endif

#ifdef TMSLAB_WIN
    void EnableInterrupts()
    {}
    void SetUpPeripherials()
    {}
    void Timer2Isr()
    {
        if(++preScale==500)
        {   preScale=0;Tim++;}
    }
#endif

    void InitData()
    {
        for (int a = 0; a < (128 * 8); a++)
            ekran[a] = 0;
        for (int a = 0; a < (40 * 16); a++)
            textEkran[a] = ' ';
    #ifndef CPP_EXAMPLE
        for (int a = 0; a < MaxObj; a++)
        {
            s[a] = (rand() & 0x1f) + 1;
            x[a] = rand() & 0x03ff;
            if (x[a] > 239 * 4 - s[a])
                x[a] -= 239 * 4 - s[a];
            y[a] = rand() & 0x1ff;
            if (y[a] > 127 * 4 - s[a])
                y[a] -= 127 * 4 - s[a];
            dx[a] = ((rand() & 0x1) ? 1 : -1) * (((rand() & 0x7) + 2) >> 1);
            dy[a] = ((rand() & 0x1) ? 1 : -1) * (((rand() & 0x7) + 2) >> 1);
        }
    #endif
    }
    void ClearScreen()
    {

        for (int a = 0; a < (128 * 8); a++)
        ekran[a] = 0;
    }
    void DrawPixels(int Key)
    {
    #ifdef CPP_EXAMPLE
        for (int c=0;c<MaxObj;c++)
        {
            objects[c].move(Key,Key);
            objects[c].draw();
        }
    #else
        for (int c = 0; c < MaxObj; c++)
        {
            y[c] += dy[c] + 6 - Key;
            x[c] += dx[c] + 6 - Key;
            if (x[c] < 0)
                x[c] += 239 * 4 - s[c];
            if (y[c] < 0)
                y[c] += 127 * 4 - s[c];
            if (x[c] > 239 * 4 - s[c])
                x[c] -= 239 * 4 - s[c];
            if (y[c] > 127 * 4 - s[c])
                y[c] -= 127 * 4 - s[c];
            long sdec = (((x[c] - 120 * 4L) * (x[c] - 120 * 4L)) >> 13)
                    + (((y[c] - 58 * 4L) * (y[c] - 58 * 4L)) >> 9);
            long size = s[c] - sdec;
            if (size < 0)
                size = 0;
            size = s[c] - size;

            for (int b = y[c] >> 2; b < (y[c] + size) >> 2; b++)
                for (int a = x[c] >> 2; a < (x[c] + size) >> 2; a++)
                    SetPixel(ekran, a, b);

        }
    #endif
    }
    //Regulator funkcje
    void RysujOkrag(){
        //RYSOWANIE OKRÊGU
        int srodek_okregu_x = 200;
        int srodek_okregu_y = 22;
        for(int x=0; x<250 ; x++){
            for(int y=0; y<120; y++){
                float rownanie_okregu_lewa = ((x-srodek_okregu_x)*(x-srodek_okregu_x))+((y-srodek_okregu_y)*(y-srodek_okregu_y));
                int rownanie_okregu_prawa=22*22;
                        if(rownanie_okregu_lewa >= rownanie_okregu_prawa-20 && rownanie_okregu_lewa <= rownanie_okregu_prawa+20){
                            SetPixel(ekran,x,y);
                        }
                }
        }
    }

    void RysujKwadrat(int x, int y){
        //RYSOWANIE KWADRATU Z 9 PIKSEL
        SetPixel(ekran,x,y);
        SetPixel(ekran,(x+1),y);
        SetPixel(ekran,(x-1),y);
        SetPixel(ekran,x,(y+1));
        SetPixel(ekran,x,(y-1));
        SetPixel(ekran,(x+1),(y+1));
        SetPixel(ekran,(x+1),(y-1));
        SetPixel(ekran,(x-1),(y+1));
        SetPixel(ekran,(x-1),(y-1));
    }

    void RysujObrot(int pozycja){
        int x=200;
        int y=22;
        //NARYSOWANIE ŒRODKOWEGO KWADRATU
        RysujKwadrat(200,22);
        //NARYSOWANIE PIERWSZEJ POZYCJI
        if(pozycja == 0){
            for(int i=0; i<21 ; i=i+3){
                RysujKwadrat(x,y+3+i);
                //RysujKwadrat(x,y-3-i);
            }
        }
        //NARYSOWANIE DRUGIEJ POZYCJI
        else if(pozycja == 1){
            for(int i=0; i<15 ; i=i+3){
               // RysujKwadrat(x+3+i,y-3-i);
                RysujKwadrat(x-3-i,y+3+i);
            }
        }
        //NARYSOWANIE TRZECIEJ POZYCJI
        else if(pozycja == 2){
            for(int i=0; i<21 ; i=i+3){
           // RysujKwadrat(x+3+i,y);
            RysujKwadrat(x-3-i,y);
            }
        }
        //NARYSOWANIE CZWARTEJ POZYCJI
        else if(pozycja == 3){
            for(int i=0; i<15 ; i=i+3){
            RysujKwadrat(x-3-i,y-3-i);
           // RysujKwadrat(x+3+i,y+3+i);
            }
        }
        /////////////////////////////////////////////
        else if(pozycja == 4){
            for(int i=0; i<21 ; i=i+3){
                //RysujKwadrat(x,y+3+i);
                RysujKwadrat(x,y-3-i);
            }
        }
        //NARYSOWANIE DRUGIEJ POZYCJI
        else if(pozycja == 5){
            for(int i=0; i<15 ; i=i+3){
                RysujKwadrat(x+3+i,y-3-i);
               // RysujKwadrat(x-3-i,y+3+i);
            }
        }
        //NARYSOWANIE TRZECIEJ POZYCJI
        else if(pozycja == 6){
            for(int i=0; i<21 ; i=i+3){
            RysujKwadrat(x+3+i,y);
           // RysujKwadrat(x-3-i,y);
            }
        }
        //NARYSOWANIE CZWARTEJ POZYCJI
        else if(pozycja == 7){
            for(int i=0; i<15 ; i=i+3){
           // RysujKwadrat(x-3-i,y-3-i);
           RysujKwadrat(x+3+i,y+3+i);
            }
        }
    }
    //Czyszczenie textu
    void CzyscText(){
        for(int i = 0; i < 16*40 ; i++){
        textEkran[i]=' ';
        }
    }
    //Czyszczenie ekranu
    void CzyscEkran(){
        for (int i = 0; i < (128 * 8); i++)
        ekran[i] = 0;
    }

    void CzyscPodSilnikiem(){
        for (int i = 0; i < (128 * 3)-8; i++)
        ekran[i] = 0;
    }

    void CzyscPodWykresem(){
        for (int i = (128 * 3)-8; i < (128 * 8); i++)
        ekran[i] = 0;
    }
    void WyswietlDaneTextowe(int zadana, int uchyb, int sterujacy, int obciazenie, int obroty){
    //WYŒWIETLENIE ZADANA
    textEkran[0]='Z';
    textEkran[1]='A';
    textEkran[2]='D';
    textEkran[3]='A';
    textEkran[4]='N';
    textEkran[5]='A';
    textEkran[11] = zadana/100 + '0';
    textEkran[12] = (zadana%100)/10 + '0';
    textEkran[13] = zadana%10 + '0';
    //WYŒWIETLENIE UCHYB
    textEkran[40]='U';
    textEkran[41]='C';
    textEkran[42]='H';
    textEkran[43]='Y';
    textEkran[44]='B';
    if(uchyb>=0){
    textEkran[50] = ' ';
    }
    else{
    textEkran[50] = '-';
    uchyb=-uchyb;
    }
    textEkran[51] = uchyb/100 + '0';
    textEkran[52] = (uchyb%100)/10 + '0';
    textEkran[53] = uchyb%10 + '0';
    //STERUJACY
    textEkran[80]='S';
    textEkran[81]='T';
    textEkran[82]='E';
    textEkran[83]='R';
    textEkran[84]='U';
    textEkran[85]='J';
    textEkran[86]='A';
    textEkran[87]='C';
    textEkran[88]='Y';
    if(sterujacy>=0){
    textEkran[90] = ' ';
    }
    else{
    textEkran[90] = '-';
    sterujacy=-sterujacy;
    }
    textEkran[91] = sterujacy/100 + '0';
    textEkran[92] = (sterujacy%100)/10 + '0';
    textEkran[93] = sterujacy%10 + '0';
    //OBCI¥¯ENIE
    textEkran[120]='O';
    textEkran[121]='B';
    textEkran[122]='C';
    textEkran[123]='I';
    textEkran[124]='A';
    textEkran[125]='Z';
    textEkran[126]='E';
    textEkran[127]='N';
    textEkran[128]='I';
    textEkran[129]='E';
    if(obciazenie>=0){
    textEkran[130] = ' ';
    }
    else{
    textEkran[130] = '-';
    obciazenie=-obciazenie;
    }
    textEkran[131] = obciazenie/100 + '0';
    textEkran[132] = (obciazenie%100)/10 + '0';
    textEkran[133] = obciazenie%10 + '0';
    //OBROTY
    textEkran[160]='O';
    textEkran[161]='B';
    textEkran[162]='R';
    textEkran[163]='O';
    textEkran[164]='T';
    textEkran[165]='Y';
    textEkran[171] = obroty/100 + '0';
    textEkran[172] = (obroty%100)/10 + '0';
    textEkran[173] = obroty%10 + '0';
    }

    void RysujWykresy(int Z, int O){
    int WykresZadanaPrzeskalowany [240] = {0};
    int WykresObrotyPrzeskalowany [240] = {0};
    //Wczytanie aktualnych wartosci
    WykresZadana [0] = Z;
    WykresObroty [0] = O;
    //Dlugosc wykresu
    if(DlugoscWykresu<239){
    DlugoscWykresu++;
    }
    //Przesuwanie wykresu


    //DAJE JE DO PRZERWANIA

    for(int i=DlugoscWykresu; i>0; i--)
    {
    WykresZadana[i]=WykresZadana[i-1];
    WykresObroty[i]=WykresObroty[i-1];
    }

    //Wyliczenie automatycznej skali
    int MAX = 0;
    int SkalaAuto = 1;
    for(int i=DlugoscWykresu; i>0; i--)
    {
        if(WykresZadana[i]>MAX){
            SkalaAuto=(WykresZadana[i]/80)+1;
            MAX=WykresZadana[i];
        }
        if(WykresObroty[i]>MAX){
            SkalaAuto=(WykresObroty[i]/80)+1;
            MAX=WykresObroty[i];
        }
    }
    //Przeskalowanie wykresu
    for(int i=DlugoscWykresu; i>0; i--)
    {
    WykresZadanaPrzeskalowany[i]=WykresZadana[i]/SkalaAuto;
    WykresObrotyPrzeskalowany[i]=WykresObroty[i]/SkalaAuto;
    }
    //Wyswietlenie na ekranie
    for(int i=0; i<DlugoscWykresu; i++)
    {
        //Sprawdzanie czy s¹ w zakresie wyswietlacza
        if(WykresZadanaPrzeskalowany[i]>=0 && WykresZadanaPrzeskalowany[i]<=80){
        SetPixel(ekran,(240-i),(127-WykresZadanaPrzeskalowany[i]));
        }
        if( WykresObrotyPrzeskalowany[i]>=0 &&  WykresObrotyPrzeskalowany[i]<=80){
        SetPixel(ekran,(240-i),(127- WykresObrotyPrzeskalowany[i]));
        }
    }
    //Wyswietlenie linjii pomocniczych
    for(int i=20; i<240; i=i+20)
        {
        SetPixel(ekran,i,127);
        SetPixel(ekran,i,87);
        SetPixel(ekran,i,47);
        }
    //Wyswietlenie oznaczeñ osi Y
    int Zakres = SkalaAuto * 80;
    textEkran[200] = Zakres/100 + '0';
    textEkran[201] = (Zakres%100)/10 + '0';
    textEkran[202] = (Zakres/2)%10 + '0';
    textEkran[400] = (Zakres/2)/100 + '0';
    textEkran[401] = ((Zakres/2)%100)/10 + '0';
    textEkran[402] = (Zakres/2)%10 + '0';
    textEkran[600]='0';

    }

    //1.04
    void RysujWykresyVER2(int Z, int U, int O, bool widocznoscZ, bool widocznoscU, bool widocznoscO){
            //Wyswietlanie danych tekstowych dotyczacych opcji wykresu
            textEkran[16] = 'W';
            textEkran[17] = 'Y';
            textEkran[18] = 'S';
            textEkran[19] = 'W';
            textEkran[20] = 'I';
            textEkran[21] = 'E';
            textEkran[22] = 'T';
            textEkran[23] = 'L';
            //
            textEkran[56] = '1';
            textEkran[57] = '-';
            textEkran[58] = 'Z';
            textEkran[59] = 'A';
            textEkran[60] = 'D';
            textEkran[61] = 'A';
            textEkran[62] = 'N';
            textEkran[63] = 'A';
            //
            textEkran[96] = '2';
            textEkran[97] = '-';
            textEkran[98] = 'U';
            textEkran[99] = 'C';
            textEkran[100] = 'H';
            textEkran[101] = 'Y';
            textEkran[102] = 'B';
            //
            textEkran[136] = '3';
            textEkran[137] = '-';
            textEkran[138] = 'O';
            textEkran[139] = 'B';
            textEkran[140] = 'R';
            textEkran[141] = 'O';
            textEkran[142] = 'T';
            textEkran[143] = 'Y';
            //
            textEkran[176] = '4';
            textEkran[177] = '-';
            textEkran[178] = 'C';
            textEkran[179] = 'Z';
            textEkran[180] = 'Y';
            textEkran[181] = 'S';
            textEkran[182] = 'C';

            int WykresZadanaPrzeskalowany [240] = {0};
            int WykresUchybPrzeskalowany [240] = {0};
            int WykresObrotyPrzeskalowany [240] = {0};
            //Wczytanie aktualnych wartosci
            WykresZadana [0] = Z;
            WykresUchyb [0] = U;
            WykresObroty [0] = O;
            //Dlugosc wykresu
            if(DlugoscWykresu<239){
            DlugoscWykresu++;
            }
            //Przesuwanie wykresu
            for(int i=DlugoscWykresu; i>0; i--)
            {
            WykresZadana[i]=WykresZadana[i-1];
            WykresUchyb[i]=WykresUchyb[i-1];
            WykresObroty[i]=WykresObroty[i-1];
            }
            //Wyliczenie automatycznej skali
                int MAX = 0;
                int MIN = 0;
                int SkalaAuto = 1;
                for(int i=DlugoscWykresu; i>0; i--)
                {
                    //Obliczenie MAX
                    if(WykresZadana[i]>MAX && widocznoscZ == true){
                        MAX=WykresZadana[i];
                    }
                    if(WykresUchyb[i]>MAX && widocznoscU == true){
                        MAX=WykresUchyb[i];
                    }
                    if(WykresObroty[i]>MAX && widocznoscO == true){
                        MAX=WykresObroty[i];
                    }
                    //Obliczenie MIN
                    if(WykresZadana[i]<MIN && widocznoscZ == true){
                        MIN=WykresZadana[i];
                    }
                    if(WykresUchyb[i]<MIN && widocznoscU == true){
                        MIN=WykresUchyb[i];
                    }
                    if(WykresObroty[i]<MIN && widocznoscO == true){
                        MIN=WykresObroty[i];
                    }
                }
                if(MAX < (-MIN)){
                    SkalaAuto = ((-MIN)/40)+1;
                }
                else{
                    SkalaAuto = (MAX/40)+1;
                }
            //Przeskalowanie wykresu
            for(int i=DlugoscWykresu; i>0; i--)
            {
            WykresZadanaPrzeskalowany[i]=WykresZadana[i]/SkalaAuto;
            WykresUchybPrzeskalowany[i]=WykresUchyb[i]/SkalaAuto;
            WykresObrotyPrzeskalowany[i]=WykresObroty[i]/SkalaAuto;
            }
            //Wyswietlenie na ekranie
            for(int i=0; i<DlugoscWykresu; i++)
            {
                //Sprawdzanie czy s¹ w zakresie wyswietlacza
                if(WykresZadanaPrzeskalowany[i]>=-40 && WykresZadanaPrzeskalowany[i]<=40){
                    if(widocznoscZ==true){
                        SetPixel(ekran,(240-i),(87-WykresZadanaPrzeskalowany[i]));
                        textEkran[65] = '+';
                    }
                    else
                    {
                        textEkran[65] = ' ';
                    }
                }
                if(WykresUchybPrzeskalowany[i]>=-40 && WykresUchybPrzeskalowany[i]<=40){
                    if(widocznoscU==true){
                        SetPixel(ekran,(240-i),(87- WykresUchybPrzeskalowany[i]));
                        textEkran[105] = '+';
                    }
                    else
                    {
                        textEkran[105] = ' ';
                    }
                }
                if(WykresObrotyPrzeskalowany[i]>=-40 && WykresObrotyPrzeskalowany[i]<=40){
                    if(widocznoscO==true){
                        SetPixel(ekran,(240-i),(87- WykresObrotyPrzeskalowany[i]));
                        textEkran[145] = '+';
                    }
                    else
                    {
                        textEkran[145] = ' ';
                    }
                }
            }
            //Wyswietlenie linji pomocniczych
            for(int i=25; i<240; i=i+20)
                {
                SetPixel(ekran,i,127);
                SetPixel(ekran,i,87);
                SetPixel(ekran,i,47);
                }
            //Wyswietlenie oznaczeñ osi Y
            textEkran[200] = '+';
            textEkran[201] = (SkalaAuto * 40)/100 + '0';
            textEkran[202] = ((SkalaAuto * 40)%100)/10 + '0';
            textEkran[203] = ((SkalaAuto * 40)/2)%10 + '0';

            textEkran[401] = '0';
            textEkran[402] = '0';
            textEkran[403] = '0';

            textEkran[600] = '-';
            textEkran[601] = (SkalaAuto * 40)/100 + '0';
            textEkran[602] = ((SkalaAuto * 40)%100)/10 + '0';
            textEkran[603] = ((SkalaAuto * 40)/2)%10 + '0';
            }
    //Koniec regulator funkcje
