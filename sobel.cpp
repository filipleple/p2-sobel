#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <string>

using namespace std;

//liczba wierszy
const int SAMPLESIZE = 3;

const int L_MASEK = 8;
const int MASKA_SZER = 3;
const int MASKA_WYS = 3;

struct BMPinfo {
    char sygnatura[2];
    short int reserved1;
    short int reserved2;
    int rozmiar;
    short int offset;

    short int infHeadSize;
    int szer;
    int wys;
    short int planes;
    short int bitcount;
    short int comp;
    int rozmiarRysunku;
    short int XPelsPM;
    short int YPelsPM;
    short int clrUses;
    short int clrImportant;
    int l_bajt_0;

};

struct pixel {
    int red, green, blue;
};


void piksele_init(pixel** piksele, BMPinfo info) {
    for (int i = 0; i < info.wys; i++) {
        piksele[i] = new pixel[info.szer];
    }

}

void piksele_init_b(pixel** piksele, BMPinfo info) {
    for (int i = 0; i < SAMPLESIZE; i++) {
        piksele[i] = new pixel[info.szer];
    }

}

void otworz_plik(string& nazwa, ifstream& plik) {
    while (1) {
        cout << "Podaj nazwe pliku: ";
        cin >> nazwa;
        

        plik.open(nazwa, ios::in | ios::binary);

        if (!plik) {
            cout << "Nie znaleziono pliku, wprowadz nazwe ponownie." << endl;
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        char sygnatura[3];
        plik.read(sygnatura, 2);
        sygnatura[2] = NULL;
        

        char sygn[3] = "BM";

        if (strcmp(sygnatura, sygn))
        {
            cout << "To nie jest plik BMP. ";
            plik.close();
            continue;
        }
        else {
            plik.seekg(28, ios::beg);
            short int bitcount;
            plik.read(reinterpret_cast<char*>(&(bitcount)), 2);
            
            if (bitcount==24)
                break;
            else {
                cout << "Oczekiwano pliku 24bit" << endl;
                continue;
            }
        }
    }

}

void wczytaj_info(BMPinfo& info, ifstream& plik) {
    plik.seekg(ios::beg);
    plik.read(reinterpret_cast<char*>(&(info.sygnatura)), 2);
    plik.read(reinterpret_cast<char*>(&(info.rozmiar)), 4);
    plik.read(reinterpret_cast<char*>(&(info.reserved1)), 2);
    plik.read(reinterpret_cast<char*>(&(info.reserved2)), 2);
    plik.read(reinterpret_cast<char*>(&(info.offset)), 4);
    plik.read(reinterpret_cast<char*>(&(info.infHeadSize)), 4);
    plik.read(reinterpret_cast<char*>(&(info.szer)), 4);
    plik.read(reinterpret_cast<char*>(&(info.wys)), 4);
    plik.read(reinterpret_cast<char*>(&(info.planes)), 2);
    plik.read(reinterpret_cast<char*>(&(info.bitcount)), 2);
    plik.read(reinterpret_cast<char*>(&(info.comp)), 4);
    plik.read(reinterpret_cast<char*>(&(info.rozmiarRysunku)), 4);
    plik.read(reinterpret_cast<char*>(&(info.XPelsPM)), 4);
    plik.read(reinterpret_cast<char*>(&(info.YPelsPM)), 4);

    info.l_bajt_0 = info.szer % 4;

}

void wypisz_info(BMPinfo& info, string nazwa) {
    cout << endl << endl << "---" << nazwa << "---" << endl << endl;
    cout << "Typ: " << info.sygnatura << endl;
    cout << "Rozmiar pliku: " << info.rozmiar << endl;
    cout << "Pozycja danych obrazowych w pliku: " << info.offset << endl;
    cout << "Rozmiar naglowka informacyjnego: " << info.infHeadSize << endl;
    cout << "Szerokosc w pikselach: " << info.szer << endl;
    cout << "Wysokosc w pikselach: " << info.wys << endl;
    cout << "Liczba platow: " << info.planes << endl;
    cout << "Liczba bitow na piksel: " << info.bitcount << endl;
    cout << "Algorytm kompresji: " << info.comp << endl;
    cout << "Rozmiar rysunku: " << info.rozmiarRysunku << endl;
    cout << "Rozdzielczosc pozioma: " << info.XPelsPM << endl;
    cout << "Rozdzielczosc pionowa: " << info.YPelsPM << endl;
    cout << endl << "-----------------------" << endl;

}

void wczytaj_piksele(ifstream& plik, BMPinfo info, pixel** piksele, fstream& output) {
    plik.seekg(info.offset, ios::beg);

    unsigned char R, G, B;

    for (int i = 0; i < (info.wys); i++)
    {
        for (int j = 0; j < (info.szer); j++) {
            plik.read((char*)&B, sizeof(unsigned char));
            plik.read((char*)&G, sizeof(unsigned char));
            plik.read((char*)&R, sizeof(unsigned char));

            piksele[i][j].red = int(R);
            piksele[i][j].green = int(G);
            piksele[i][j].blue = int(B);

        }
        if (info.l_bajt_0) {
            for (int i = 0; i < info.l_bajt_0; i++) {
                int czarnek = 0;
                plik.read((char*)&czarnek, sizeof(unsigned char));
            }
        }


    }


}

void wykonaj_splot(pixel** piksele, pixel** piksele_wy, long wys, long szer, short sobel_mask[L_MASEK][MASKA_WYS][MASKA_SZER]) {
    
    int a, b, i, j, sum;

    short max, min;
    
    min = 0;
    max = 255;

    /* czarne tlo */
    for (i = 0; i < wys; i++) {
        for (j = 0; j < szer; j++) {
            piksele_wy[i][j].red = 0;
            piksele_wy[i][j].green = 0;
            piksele_wy[i][j].blue = 0;

        }
    }

    int a_min = -((MASKA_WYS - 1) / 2);
    
    int a_max = ((MASKA_WYS - 1) / 2)+1;
    
    int b_min = -((MASKA_SZER - 1) / 2);
    
    int b_max = ((MASKA_SZER - 1) / 2)+1;
    

    for (int k=0; k<L_MASEK;k++)
    for (i = 1; i < wys - 1; i++) {
        
        for (j = 1; j < szer - 1; j++) {
            
            //r
            sum = 0;
            for (a = a_min; a < a_max; a++) {
                for (b = b_min; b < b_max; b++) {
                    sum = sum + piksele[i + a][j + b].red * sobel_mask[k][a + 1][b + 1];
                }
            }
            if (sum > max) sum = max;
            if (sum < 0) sum = 0;

            if (sum > piksele_wy[i][j].red)
                piksele_wy[i][j].red = sum;

            //g
            sum = 0;
            for (a = a_min; a < a_max; a++) {
                for (b = b_min; b < b_max; b++) {
                    sum = sum + piksele[i + a][j + b].green * sobel_mask[k][a + 1][b + 1];
                }
            }
            if (sum > max) sum = max;
            if (sum < 0) sum = 0;

            if (sum > piksele_wy[i][j].green)
                piksele_wy[i][j].green = sum;
            //b
            sum = 0;
            for (a = a_min; a < a_max; a++) {
                for (b = b_min; b < b_max; b++) {
                    sum = sum + piksele[i + a][j + b].blue * sobel_mask[k][a + 1][b + 1];
                }
            }
            if (sum > max) sum = max;
            if (sum < 0) sum = 0;

            if (sum > piksele_wy[i][j].blue)
                piksele_wy[i][j].blue = sum;

            
            


        }
    } 
} 

void wczytuj_i_zapisuj(ifstream& plik, BMPinfo info, pixel** piksele, pixel** piksele_wy, fstream& output, short sobel_mask[8][3][3]) {
    
    
    int wiersz_odczytu = 0;
    int wiersz_zapisu = 0;

    int pozycja_zapisu = info.offset, pozycja_odczytu = info.offset;

    unsigned char R, G, B;
    

    while (wiersz_odczytu < info.wys && wiersz_zapisu < info.wys){
        for (int i = 0; i < (SAMPLESIZE); i++)
        {
            for (int j = 0; j < (info.szer); j++) {
                plik.seekg(pozycja_odczytu, ios::beg);
                plik.read((char*)&B, sizeof(unsigned char));
                plik.read((char*)&G, sizeof(unsigned char));
                plik.read((char*)&R, sizeof(unsigned char));

                piksele[i][j].red = int(R);
                piksele[i][j].green = int(G);
                piksele[i][j].blue = int(B);

                
                pozycja_odczytu = plik.tellg();
            }
            
            

            
            if (info.l_bajt_0) {
                for (int i = 0; i < info.l_bajt_0; i++) {
                    int czarnek = 0;
                    plik.read((char*)&czarnek, sizeof(unsigned char));
                }
            }
            pozycja_odczytu = plik.tellg();
            


            wiersz_odczytu++;
        }

        wiersz_odczytu = wiersz_odczytu - 2;

        pozycja_odczytu = plik.tellg();
        pozycja_odczytu = pozycja_odczytu - 2 *   (info.szer + info.l_bajt_0/3) * 3;

        
        wykonaj_splot(piksele, piksele_wy, SAMPLESIZE, info.szer, sobel_mask);

        
        for (int j = 0; j < (info.szer); j++) {
            output.seekg(pozycja_zapisu, ios::beg);
            output.write((char*)&(piksele_wy[1][j].blue), sizeof(unsigned char));
            output.write((char*)&(piksele_wy[1][j].green), sizeof(unsigned char));
            output.write((char*)&(piksele_wy[1][j].red), sizeof(unsigned char));

            pozycja_zapisu = output.tellg();
        }


        
        if (info.l_bajt_0) {
            for (int i = 0; i < info.l_bajt_0; i++) {
                int czarnek = 0;
                output.write((char*)&czarnek, sizeof(unsigned char));
            }
        }
        pozycja_zapisu = output.tellg();
            
        wiersz_zapisu++;

        
    }

        
    cout << wiersz_zapisu << " z " << info.wys << endl;
}

void kopiuj_naglowki(ifstream& plik, fstream& output, char* nazwa_out) {
    plik.seekg(ios::beg);
    char head[54];
    plik.read(head, 54);

    output.open(nazwa_out, ios::out | ios::binary);
    output.write(head, 54);

}

void zapis_do_pliku(pixel** piksele_wy, fstream& output, BMPinfo info) {
    output.seekg(info.offset, ios::beg);

    for (int i = 0; i < (info.wys); i++)
    {
        for (int j = 0; j < (info.szer); j++) {
            output.write((char*)&(piksele_wy[i][j].blue), sizeof(unsigned char));
            output.write((char*)&(piksele_wy[i][j].green), sizeof(unsigned char));
            output.write((char*)&(piksele_wy[i][j].red), sizeof(unsigned char));
            

        }
        
        if (info.l_bajt_0) {
            for (int i = 0; i < info.l_bajt_0; i++) {
                int czarnek = 0;
                
                output.write((char*)&czarnek, sizeof(unsigned char));
            }
        }
        

    }

}

void wczytaj_maski(ifstream& maski, short sobel_mask[L_MASEK][MASKA_WYS][MASKA_SZER]) {
    char bufor[3];
    maski.open("maski", ios::in);
    for (int i = 0; i < L_MASEK; i++) {
        for (int j = 0; j < MASKA_WYS; j++) {
            for (int k = 0; k < MASKA_SZER; k++) {
                maski.read(bufor, 2);
                bufor[2] = NULL;
                sobel_mask[i][j][k] = atoi(bufor);
            }
        }
    }
}

int main()
{
    string nazwa;
    char nazwa_out[50];
    ifstream plik;
    fstream output;
    BMPinfo info;
    char tryb = '0';
    
    ifstream maski;
    short int sobel_mask[L_MASEK][MASKA_WYS][MASKA_SZER];
    wczytaj_maski(maski, sobel_mask);
    
    cout << "Wybierz tryb:" << endl;
    cout << "a - wczytaj caly plik naraz" << endl << "b - wczytuj po " << SAMPLESIZE << " wierszy" << endl;
    while (tryb!='a'&&tryb!='b'){
        while (!(cin >> tryb)) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            
        }
    }
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    otworz_plik(nazwa, plik);
    wczytaj_info(info, plik);
    wypisz_info(info, nazwa);

    pixel** piksele = NULL;
    pixel** piksele_wy = NULL;
    
    cout << "Podaj nazwe pliku wyjsciowego: ";
    cin >> nazwa_out;
    if (strstr(nazwa_out, ".bmp") == NULL) {
        strcat_s(nazwa_out, ".bmp");
    }


    if(tryb=='a'){
        pixel** piksele = new pixel * [info.wys];
        pixel** piksele_wy = new pixel * [info.wys];

        piksele_init(piksele, info);
        piksele_init(piksele_wy, info);

        kopiuj_naglowki(plik, output, nazwa_out);
        wczytaj_piksele(plik, info, piksele, output);
        plik.close();

        wykonaj_splot(piksele, piksele_wy, info.wys, info.szer, sobel_mask);
    
        zapis_do_pliku(piksele_wy, output, info);
    }

    if (tryb == 'b') {
        pixel** piksele = new pixel * [SAMPLESIZE];
        pixel** piksele_wy = new pixel * [SAMPLESIZE];
        piksele_init_b(piksele, info);
        piksele_init_b(piksele_wy, info);

        kopiuj_naglowki(plik, output, nazwa_out);

        wczytuj_i_zapisuj(plik, info, piksele, piksele_wy, output, sobel_mask);
    }

    output.close();
    system(nazwa_out);
    

}
