
#ifndef main_CL
#define main_CL

//Rgulator dodanie plików nag³ówkowych
#include "UAR.h"
#include "PID.h"
#include "InercModel.h"
//------------------------------------
#include "setup.h"
#include "stdlib.h"
#include "R_P_LCD_TMSLAB.h"
#include "R_P_KEYBOARD_TMSLAB.h"
#include "R_P_LEDBAR_TMSLAB.h"
#include "square.h"

#ifdef TMSLAB_C2000
#include "F2837xD_device.h"
#include "systemInit.h"
#endif


void SetUpPeripherials();
void EnableInterrupts();
void InitData();
void ClearScreen();
void DrawPixels(int Key);
//
void RysujOkrag();
void RysujKwadrat(int x, int y);
void RysujObrot(int pozycja);
void CzyscText();
void CzyscEkran();
void CzyscPodSilnikiem();
void CzyscPodWykresem();
void WyswietlDaneTextowe(int zadana, int uchyb, int sterujacy, int obciazenie, int obroty);
void RysujWykresy(int Z, int O);
void RysujWykresyVER2(int Z, int U, int O, bool widocznoscZ, bool widocznoscU, bool widocznoscO);
//Koniec regulator main.h


#endif
