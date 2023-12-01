#include <iostream>
#include <bitset>
#include <climits>
#include <iomanip>
#include <intrin.h>
#include <string>
#include <conio.h>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;

// Объеденение памяти для конвертации разных типов данных
union ulltod
{
    unsigned long long input;
    double output;
};
union dtoull
{
    double input;
    unsigned long long output;
};

int manLength = 52;
int expLength = 11;

bool info = 1;

// Класс числа
struct double_num
{
    double_num(unsigned long long numVal)
    {        
        num = 0;
        numBin = numVal;        
        numSign = numVal >> (expLength + manLength);
        numExp = ((numVal >> manLength) & (0x3FFFFF >> (22 - expLength)));
        numMan = numVal & (0x1FFFFFFFFFFFFFFF >> (61 - manLength));
        if ((numExp == 0) && (numMan == 0)) DINN[3] = 1;
        else if ((numExp == (pow(2, expLength) - 1)) && (numMan == 0)) DINN[1] = 1;
        else if ((numExp == (pow(2, expLength) - 1)) && (numMan > 0)) DINN[2] = 1;
        else if ((numExp == 0) && (numMan > 0)) DINN[0] = 1;

        unsigned long long expBuf = numExp - (pow(2, (expLength - 1)) - 1);
        ulltod a;
        expBuf += 1023;
        unsigned long long numBuf = (expBuf << 52);
        if (manLength <= 52) numBuf += (numMan << (52 - manLength));
        else numBuf += (numMan >> (manLength - 52));
        numBuf += (numSign << 63);
        a.input = numBuf;
        num = a.output;
    }
    double_num(float numVal)
    {
        num = numVal;
        numBin = *reinterpret_cast<unsigned int*>(&numVal);
        numSign = numBin >> (expLength + manLength);
        numExp = ((numBin >> manLength) & (0xffff >> (16 - expLength)));
        numMan = numBin & (0x7FFFFFF >> (27 - manLength));
        if ((numExp == 0) && (numMan == 0)) DINN[3] = 1;
        else if ((numExp - (pow(2, (expLength - 1))) > (pow(2, (expLength - 1)))) && (numMan == 0)) DINN[1] = 1;
        else if ((numExp - (pow(2, (expLength - 1))) > (pow(2, (expLength - 1)))) && (numMan > 0)) DINN[2] = 1;
        else if ((numExp == 0) && (numMan > 0)) DINN[0] = 1;
    }

    double num;
    unsigned long long numBin;
    unsigned long long numSign;
    unsigned long long numExp;
    unsigned long long numMan;
    bool DINN[4] = { 0,0,0,0 };   // DINN (D - Denormalized, I - inf, N - NaN, N - NUL)

    void print(char mode)
    {
        bitset<64> numBinBit(numBin);
        if (mode == 'r') cout << "Res: ";
        else if (mode == 'a') cout << "A:   ";
        else if (mode == 'b') cout << "B:   ";
        cout << hex << "HEX: " << numBin << dec << "\t";
        cout << "BIN: " << numBinBit[manLength + expLength] << " ";
        for (int i = (manLength + expLength - 1); i > (manLength - 1); i--)
        {
            cout << numBinBit[i];
        }
        cout << " ";
        for (int i = (manLength - 1); i >= 0; i--)
        {
            cout << numBinBit[i];
        }
        if (DINN[2]) cout << " (NaN)" << "\t";
        else if (DINN[1]) cout << " (inf)" << "\t";
        else if (DINN[3]) cout << " (NUL)" << "\t";
        else if (DINN[0]) cout << " (Denormalized)" << "\t";
        else cout << "\t";

        if (DINN[1]) num = INFINITY;
        else if ((numExp - ((pow(2, (expLength - 1))) - 1)) > 127) num = INFINITY;
        cout << setprecision(30) << "DEC(double): " << num << endl << endl;
    }
    
};

// Функции для вывода на консоль всех операций с мантиссами
void helpManPrint(bitset<64>& man)
{
    cout << '1' << '.';
    for (int i = manLength - 1; i >= 0; i--)
    {
        cout << man[i];
    }
}
void manMultPrint(unsigned long long manA, unsigned long long manB, boost::multiprecision::uint128_t man128_r, bool& addExp, bool& rounding)
{
    bitset<64> manA_bit(manA);
    bitset<64> manB_bit(manB);
    unsigned long long h1, h2;
    h1 = static_cast<unsigned long long> (man128_r >> 64);
    h2 = static_cast<unsigned long long> (man128_r);
    bitset<128> manR_bit_high(h1);
    bitset<128> manR_bit_low(h2);
    bitset<128> manR_bit("0");
    manR_bit |= manR_bit_high << 64;
    manR_bit |= manR_bit_low;
    
    cout << "Man.A x Man.B: \t\t";
    helpManPrint(manA_bit);
    cout << " x ";
    helpManPrint(manB_bit);
    cout << " = ";

    if (addExp) cout << "10" << '.';
    else cout << '1' << '.';
    for (int i = (manLength * 2) - 1; i >= 0; i--)
    {
        cout << manR_bit[i];
    }
    cout << endl;
}

// Функция для заполнения 128-битной переменной данными из вектора
boost::multiprecision::uint128_t getUintFromBuffer(const vector<unsigned char>& buf)
{
    boost::multiprecision::uint128_t retU = -1;
    memset(&retU, 0, 16);
    memcpy(&retU, buf.data(), min(buf.size(), (size_t)16));
    return retU;
}

// Функция для умножения мантисс
unsigned long long manMult(unsigned long long manA, unsigned long long manB, bool &addExp, bool& rounding)
{
    // Запись в расширенные переменные
    unsigned long long result = 0;
    vector<unsigned char> buf; buf.resize(16);
    boost::multiprecision::uint128_t man128_a = manA;
    boost::multiprecision::uint128_t man128_b = manB;

    // Добавление мнимой единицы
    buf[7] = 0x10;
    boost::multiprecision::uint128_t add1 = getUintFromBuffer(buf);
    man128_a += (add1 >> (60 - manLength));
    man128_b += (add1 >> (60 - manLength));
    buf[7] = 0x00;

    // Умножение мантис
    boost::multiprecision::uint128_t man128_r = man128_a * man128_b;

    // Нормализация
    buf[15] = 0x02;
    boost::multiprecision::uint128_t n1 = getUintFromBuffer(buf);
    if (man128_r & (n1 >> (120 - (manLength * 2))))
    {
        man128_r >>= 1;
        addExp = true;
    } 

    // Отпиливание мнимой единицы
    buf[15] = 0x00; buf[14] = 0xff; buf[13] = 0xff; buf[12] = 0xff; buf[11] = 0xff; buf[10] = 0xff; buf[9] = 0xff; buf[8] = 0xff; buf[7] = 0xff; buf[6] = 0xff; buf[5] = 0xff; buf[4] = 0xff;
    buf[3] = 0xff; buf[2] = 0xff; buf[1] = 0xff; buf[0] = 0xff;
    boost::multiprecision::uint128_t n2 = getUintFromBuffer(buf);
    man128_r &= (n2 >> (120 - (manLength * 2)));

    // Вывод на консоль
    if (info) manMultPrint(manA, manB, man128_r, addExp, rounding); 
    if (info)
    {
        unsigned long long h1, h2;
        h1 = static_cast<unsigned long long> (man128_r >> 64);
        h2 = static_cast<unsigned long long> (man128_r);
        bitset<128> manR_bit_high(h1);
        bitset<128> manR_bit_low(h2);
        bitset<128> manR_bit("0");
        manR_bit |= manR_bit_high << 64;
        manR_bit |= manR_bit_low;

        cout << "Normalised: \t\t";
        cout << '1' << '.';
        for (int i = (manLength * 2) - 1; i > manLength - 1; i--)
        {
            cout << manR_bit[i];
        }
        cout << " | ";
        for (int i = manLength - 1; i >= 0; i--)
        {
            cout << manR_bit[i];
        }
        cout << endl;
    }

    // Округление
    vector<unsigned char> buf2; buf2.resize(16); buf2[7] = 0x10;
    boost::multiprecision::uint128_t r1_1 = getUintFromBuffer(buf2);
    buf2[7] = 0x08;
    boost::multiprecision::uint128_t r1_2 = getUintFromBuffer(buf2);
    buf2[8] = 0x00; buf2[7] = 0x08;
    boost::multiprecision::uint128_t r2_1 = getUintFromBuffer(buf2);
    buf2[8] = 0x00; buf2[7] = 0x07; buf2[6] = 0xff; buf2[5] = 0xff; buf2[4] = 0xff; buf2[3] = 0xff; buf2[2] = 0xff; buf2[1] = 0xff; buf2[0] = 0xff;
    boost::multiprecision::uint128_t r2_2 = getUintFromBuffer(buf2);
    if (((man128_r & (r1_1 >> (60 - manLength))) && (man128_r & (r1_2 >> (60 - manLength)))) || ((man128_r & (r2_1 >> (60 - manLength))) && (man128_r & (r2_2 >> (60 - manLength)))))
    {
        vector<unsigned char> buf3; buf3.resize(16); buf3[7] = 0x10;
        boost::multiprecision::uint128_t r3 = getUintFromBuffer(buf3);
        man128_r += (r3 >> (60 - manLength));
        rounding = 1;
    }

    // Запись обратно в ull
    man128_r >>= manLength;
    result = static_cast<unsigned long long> (man128_r);
    if (info) 
    {
        
        cout << "Rounded: \t\t";
        bitset<64> result_bit(result);
        cout << '1' << '.';
        for (int i = manLength - 1; i >= 0; i--)
        {
            cout << result_bit[i];
        }
        cout << "   One bit added: " << boolalpha << rounding << noboolalpha << endl;
    }
    return result;
}

// Функция для вывода на консоль всех операций с экспонентой
void expAddPrint(unsigned long long& expA, unsigned long long& expB, unsigned long long& expRes, bool& addExp)
{
    if (addExp) cout << "Exp.A + Exp.B + over: \t" << expA << " + " << expB << " = " << expRes << " + 1 = " << expRes + 1;
    else cout << "Exp.A + Exp.B: \t\t" << expA << " + " << expB << " = " << expRes;
    cout << endl << endl;
}

// Функция для сложения экспонент
int expAdd(unsigned long long &expA, unsigned long long&expB, bool &addExp)
{
    expA -= (pow(2,(expLength - 1)) - 1); expB -= (pow(2, (expLength - 1)) - 1);
    unsigned long long expRes = (expA + expB);
    if (info) expAddPrint(expA, expB, expRes, addExp);
    if (addExp) expRes++;
    return expRes + (pow(2, (expLength - 1)) - 1);
}

// Функция для склеивания знака, экспоненты и мантиссы в одну переменную
unsigned long long addRes(unsigned long long mantissa, unsigned long long exponent, unsigned long long sign)
{
    unsigned long long result = 0;
    result += mantissa;
    result += exponent << manLength;
    result += sign << (manLength + expLength);
    return result;
}

// Функция для умножения параметризованных чисел
unsigned long long mul(unsigned long long num1, unsigned long long num2)
{

    unsigned long long numSign1 = num1 >> (expLength + manLength);
    unsigned long long numExp1 = ((num1 >> manLength) & (0x3FFFFF >> (22 - expLength)));
    unsigned long long numMan1 = num1 & (0x1FFFFFFFFFFFFFFF >> (61 - manLength));

    bool DINN1[4] = { 0,0,0,0 };
    if ((numExp1 == 0) && (numMan1 == 0)) DINN1[3] = 1;
    else if ((numExp1 == (pow(2, expLength) - 1)) && (numMan1 == 0)) DINN1[1] = 1;
    else if ((numExp1 == (pow(2, expLength) - 1)) && (numMan1 > 0)) DINN1[2] = 1;
    else if ((numExp1 == 0) && (numMan1 > 0)) DINN1[0] = 1;


    unsigned long long numSign2 = num2 >> (expLength + manLength);
    unsigned long long numExp2 = ((num2 >> manLength) & (0x3FFFFF >> (22 - expLength)));
    unsigned long long numMan2 = num2 & (0x1FFFFFFFFFFFFFFF >> (61 - manLength));

    bool DINN2[4] = { 0,0,0,0 };
    if ((numExp2 == 0) && (numMan2 == 0)) DINN2[3] = 1;
    else if ((numExp2 == (pow(2, expLength) - 1)) && (numMan2 == 0)) DINN2[1] = 1;
    else if ((numExp2 == (pow(2, expLength) - 1)) && (numMan2 > 0)) DINN2[2] = 1;
    else if ((numExp2 == 0) && (numMan2 > 0)) DINN2[0] = 1;


    bool addExp = 0;
    bool rounding = 0;
    if (DINN1[0] or DINN2[0])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B = \t" << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        int expRes = 0;
        unsigned int manRes = manMult(numMan1, numMan2, addExp, rounding);
        
        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[1] or DINN2[1])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B = \t" << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        int expRes = (pow(2, (expLength)) - 1);
        unsigned int manRes = 0;

        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[2] or DINN2[2])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B = \t" << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        int expRes = (pow(2, (expLength)) - 1);
        unsigned int manRes = manMult(numMan1, numMan2, addExp, rounding);

        return addRes(manRes, expRes, signRes);
    }
    else if (DINN1[3] or DINN2[3])
    {
        unsigned int signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B = \t" << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        int expRes = 0;
        unsigned int manRes = 0;

        return addRes(manRes, expRes, signRes);
    }
    else
    {
        unsigned long long signRes = numSign1 ^ numSign2;
        if (info) cout << "sign.A xor sign.B = \t" << numSign1 << " xor " << numSign2 << " = " << signRes << endl;
        unsigned long long manRes = manMult(numMan1, numMan2, addExp, rounding);
        //manMultPrint(numMan1, numMan2, manRes, addExp, rounding);
        unsigned long long expRes = expAdd(numExp1, numExp2, addExp);
        if (expRes > (pow(2, expLength) - 1))
        {
            expRes = (pow(2, (expLength)) - 1);
            manRes = 0;
        }

        return addRes(manRes, expRes, signRes);
    }
    
}

// Функция для ввода чисел из консоли
void enterNumbers(unsigned long long& number1, unsigned long long& number2)
{
    if ((manLength == 52) && (expLength == 11))
    {
        char type1;
        char type2;
        cout << "Ввод операндов A и B в формате: тип(h/b/d) число тип(h/b/d) число ( h - HEX, b - BIN, d - DEC (double) )" << endl;

        cin >> type1;
        switch (type1)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number1 = stoull(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number1 = stoull(buf, 0, 2);
            break;
        }
        case 'd':
        {
            double buf;
            dtoull a;
            cin >> buf;
            a.input = buf;
            number1 = a.output;
            break;
        }
        }

        cin >> type2;
        switch (type2)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number2 = stoull(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number2 = stoull(buf, 0, 2);
            break;
        }
        case 'd':
        {
            double buf;
            dtoull a;
            cin >> buf;
            a.input = buf;
            number2 = a.output;
            break;
        }
        }

        cout << endl;
    }
    else
    {
        char type1;
        char type2;
        cout << "Ввод операндов A и B в формате: тип(h/b) число тип(h/b) число ( h - HEX, b - BIN )" << endl;

        cin >> type1;
        switch (type1)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number1 = stoull(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number1 = stoull(buf, 0, 2);
            break;
        }
        }

        cin >> type2;
        switch (type2)
        {
        case 'h':
        {
            string buf;
            cin >> buf;
            number2 = stoull(buf, 0, 16);
            break;
        }
        case 'b':
        {
            string buf;
            cin >> buf;
            number2 = stoull(buf, 0, 2);
            break;
        }
        }


        cout << endl;
    }
}

// Функция для ввода параметров числа
void enterSettings(int& manLength, int& expLength)
{
    cout << "*Округление выполняется к ближайшему большому целому числу\nмаксимальная и минимальная экспонента = 16 и 2, максимальная и минимальная мантисса = 60 и 3\nчисло не должно быть больше 64 бит*" << endl << endl;
    cout << "Введите длинну экспоненты и мантиссы (без мнимой единицей) через пробел" << endl;
    cin >> expLength;
    cin >> manLength;
}

// Функция проверяющая результат если числа соответствуют стандарту double
void checker_64(double num1, double num2, double numRes)
{
    double numResC = num1 * num2;
    if (numResC == numRes) cout << "CHECKER: CORRECT" << endl << endl;
    else
    {
        cout << "CHECKER: ERROR" << endl;
        cout << "MY RESULT: " << numRes << endl;
        cout << "C  RESULT: " << numResC << endl << endl;
    }
}

// Функция проверяющая результат если числа соответствуют стандарту float
void checker_32(float num1, float num2, float numRes)
{
    float numResC = num1 * num2;
    if (numResC == numRes) cout << "CHECKER: CORRECT" << endl << endl;
    else
    {
        cout << "CHECKER: ERROR" << endl;
        cout << "MY RESULT: " << numRes << endl;
        cout << "C  RESULT: " << numResC << endl << endl;
    }
}

int main()
{
    // Включение русского языка
    setlocale(LC_ALL, "rus");
    
    // Цикл программы
    for(int i = 1; i != 27;)
    {
        enterSettings(manLength, expLength);
        unsigned long long number1 = 0;
        unsigned long long number2 = 0;
        enterNumbers(number1, number2);

        // Создание экземпляров класса, ввод в них данных и вывод на консоль
        double_num num1(number1); if (info) num1.print('a');
        double_num num2(number2); if (info) num2.print('b');

        // Создание третьего экземпляра состоящего из перемноженных, двух прошлых экземпляров и вывод на консоль
        double_num num3(mul(num1.numBin, num2.numBin)); if (info) num3.print('r');

        // Проверка результата с Сишным умножением (работает только для fp32 и fp64)
        if (info and manLength == 23 and expLength == 8) checker_32(num1.num, num2.num, num3.num);
        else if (info and manLength == 52 and expLength == 11) checker_64(num1.num, num2.num, num3.num);

        cout << "Нажмите ESC чтобы закончить" << endl
             << "Нажмите ПРОБЕЛ чтобы продолжить" << endl;

        // Проверка нажатой клавиши
        char ch;
        ch = _getch();
        i = static_cast<int>(ch);
        cout << endl << endl;

        
    }
    return 0;
}

