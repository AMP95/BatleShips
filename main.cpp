#include <iostream>
#include <ctime>
#include <string>
#include <Windows.h>
#define size 10
//#define TEST
//#define SMALL
using namespace std;
enum class color {
	green = 2,
	red = 4,
};
enum class exitopt {
	endgame,
	pause,
	exit,
};
enum class direction {
	down,
	right,
	up,
	left,
	unknown
};
enum class cellFilling {
	//значения целого и убитого корабля различаются на 7
	empty,
#ifdef SMALL
	scho = 1,
	brig = 2,
	freg = 3,
	gall = 4,
	hitscho = 8,
	hitbrig = 9,
	hitfreg = 10,
	hitgall = 11,
#else
	scho = 2,
	brig = 3,
	freg = 4,
	gall = 6,
	hitscho = 9,
	hitbrig = 10,
	hitfreg = 11,
	hitgall = 13,
#endif // SMALL
	miss,
	hit
};
enum class shipType {
	OnField = 0,//корабль поставлен на поле
#ifdef SMALL
	Schooner = 1,
	Brig = 2,
	Fregata = 3,
	Galleon = 4
#else
	Schooner = 2,
	Brig = 3,
	Fregata = 4,
	Galleon = 6
#endif // SMALL	
};
enum class turn {
	player1,
	player2,
};

struct ship {
	shipType name;
	void info() {
		switch (name)
		{
		case shipType::Schooner: cout << "Schooner (size - " << (int)shipType::Schooner << ")";
			break;
		case shipType::Brig:cout << "Brig (size - " << (int)shipType::Brig << ")";
			break;
		case shipType::Fregata:cout << "Fregata (size - " << (int)shipType::Fregata << ")";
			break;
		case shipType::Galleon:cout << "Galleon (size - " << (int)shipType::Galleon << ")";
			break;
		default:
			break;
		}
	}
};
struct player {
	string name = "Player";
	int savedI = -1; //сохраненные координаты последнего попадания
	int savedJ = -1;
	direction savedDir = direction::unknown; // сохраненное направление
	int killedCnt = 0; // количество убитых кораблей противника
	//регенерация начальных значений переменных
	void valsRegen() {
		savedI = -1;
		savedJ = -1;
		killedCnt = 0;
		savedDir = direction::unknown;
	}
};
struct statist
{
	int allGame = 0;
	int userWins = 0;
	int userLoss = 0;
	int comp1Wins = 0;
	int comp2Wins = 0;
};

exitopt exopt = exitopt::endgame;
statist score;

player player1;
cellFilling** secretFieldPl1 = nullptr;//скрытое поле для кораблей
cellFilling** viewFieldPl1 = nullptr; // поле для отображения

player player2;
cellFilling** secretFieldPl2 = nullptr;
cellFilling** viewFieldPl2 = nullptr;

HANDLE cons;

void errorMenu() {
	cout << "Error, incorrect input!\n";
}
bool acceptMenu() {
	while (1) {
		cout << "Are you sure?(1-0)\n-> ";
		string res;
		getline(cin, res);
		if (res.length() != 1)
			errorMenu();
		else if (res[0] == '0')
			return false;
		else if (res[0] == '1')
			return true;
		else
			errorMenu();
	}
}
void inputCheck(string input, int& i, int& j) {
	if (input.length() > 3) {
		errorMenu();
		i = j = -1;
		return;
	}
	else {
		if ('1' <= input[0] && input[0] <= '9') {
			i = (int)input[0] - 49;
			if ('0' <= input[1] && input[1] <= '9') {
				i++;
				i = i * 10 + input[1] - 49;
				if ('a' <= input[2] && input[2] <= 'z')
					j = (int)input[2] - 97;
				else {
					errorMenu();
					i = j = -1;
					return;
				}
			}
			else if ('a' <= input[1] && input[1] <= 'z' && input[2] == '\0') {
				j = (int)input[1] - 97;
			}
			else {
				errorMenu();
				i = j = -1;
				return;
			}
		}
		else if ('a' <= input[0] && input[0] <= 'z') {
			j = (int)input[0] - 97;
			if ('1' <= input[1] && input[1] <= '9') {
				i = (int)input[1] - 49;
				if ('0' <= input[2] && input[2] <= '9') {
					i++;
					i = i * 10 + input[2] - 49;
				}
				else if (input[2] == '\0')
					return;
				else {
					errorMenu();
					i = j = -1;
					return;
				}
			}
			else {
				errorMenu();
				i = j = -1;
				return;
			}
		}
		else {
			errorMenu();
			i = j = -1;
			return;
		}
		if (i >= size || j >= size) {
			i = j = -1;
			errorMenu();
			return;
		}
	}
}

void fieldsClean(cellFilling** field1, cellFilling** field2 = nullptr, cellFilling** field3 = nullptr, cellFilling** field4 = nullptr) {
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			field1[i][j] = cellFilling::empty;
			if (field2 != nullptr) {
				field2[i][j] = cellFilling::empty;
				if (field3 != nullptr) {
					field3[i][j] = cellFilling::empty;
					if (field4 != nullptr)
						field4[i][j] = cellFilling::empty;
				}
			}
		}
	}
}
void fieldsCreate() {
	secretFieldPl1 = new cellFilling * [size];
	viewFieldPl1 = new cellFilling * [size];
	secretFieldPl2 = new cellFilling * [size];
	viewFieldPl2 = new cellFilling * [size];
	for (int i = 0; i < size; i++)
	{
		secretFieldPl1[i] = new cellFilling[size];
		viewFieldPl1[i] = new cellFilling[size];
		secretFieldPl2[i] = new cellFilling[size];
		viewFieldPl2[i] = new cellFilling[size];
		for (int j = 0; j < size; j++)
		{
			secretFieldPl1[i][j] = cellFilling::empty;
			viewFieldPl1[i][j] = cellFilling::empty;
			secretFieldPl2[i][j] = cellFilling::empty;
			viewFieldPl2[i][j] = cellFilling::empty;
		}
	}
}
void fieldsDelete() {
	if (secretFieldPl1 != nullptr) {
		for (int i = 0; i < size; i++)
		{

			delete[]secretFieldPl1[i];
			delete[]viewFieldPl1[i];
			delete[]secretFieldPl2[i];
			delete[]viewFieldPl2[i];
		}
		delete[]secretFieldPl1;
		delete[]viewFieldPl1;
		delete[]secretFieldPl2;
		delete[]viewFieldPl2;
		secretFieldPl1 = nullptr;
		viewFieldPl1 = nullptr;
		secretFieldPl2 = nullptr;
		viewFieldPl2 = nullptr;
	}
}

void cellColor(color background) {
	SetConsoleTextAttribute(cons, ((int)background << 4) | 0);
	cout << "   ";
	SetConsoleTextAttribute(cons, (0 << 4) | 7);
}
void printCell(cellFilling val) {
	switch (val)
	{
	case cellFilling::empty:cout << "   ";
		break;
	case cellFilling::hit: cellColor(color::red);
		break;
	case cellFilling::miss: cout << " * ";
		break;
#ifdef TEST
	case cellFilling::scho:
		cout << " " << (int)cellFilling::scho << " ";
		break;
	case cellFilling::brig:
		cout << " " << (int)cellFilling::brig << " ";
		break;
	case cellFilling::freg:
		cout << " " << (int)cellFilling::freg << " ";
		break;
	case cellFilling::gall:
		cout << " " << (int)cellFilling::gall << " ";
		break;
	case cellFilling::hitscho:cout << " S ";
		break;
	case cellFilling::hitbrig:cout << " B ";
		break;
	case cellFilling::hitfreg:cout << " F ";
		break;
	case cellFilling::hitgall:cout << " G ";
		break;
#else
	case cellFilling::scho: cellColor(color::green);
		break;
	case cellFilling::brig: cellColor(color::green);
		break;
	case cellFilling::freg: cellColor(color::green);
		break;
	case cellFilling::gall: cellColor(color::green);
		break;
	case cellFilling::hitscho: cellColor(color::red);
		break;
	case cellFilling::hitbrig: cellColor(color::red);
		break;
	case cellFilling::hitfreg: cellColor(color::red);
		break;
	case cellFilling::hitgall: cellColor(color::red);
		break;
#endif // TEST
	}
}
void printFields(bool show = false) {
	system("cls");
	cout << "Pause -> \"pause\"\nExit -> \"exit\"\n\n";
	int space = 4 + size * 4;
	//при любой длине имени игрока№1 имя игрока№2 будет на том же месте
	cout << "\t" << player1.name;
	for (int i = 0; i < space - (player1.name.length()); i++)
	{
		cout << " ";
	}
	cout << "\t" << player2.name << endl;
	for (int i = 0; i < 2; i++)
	{
		cout << "\t     ";
		for (int i = 0; i < size; i++)
		{
			cout << char(i + 65) << "   ";
		}
		"\t     ";
	}
	cout << endl;
	//вывод верхних линий полей
	for (int i = 0; i < 2; i++)
	{
		cout << "\t   " << char(218);
		for (int i = 0; i < size - 1; i++)
		{
			cout << char(196) << char(196) << char(196) << char(194);
		}
		cout << char(196) << char(196) << char(196) << char(191);
	}
	cout << endl;
	//вывод полей
	for (int i = 0; i < size; i++)
	{
		if (i < 9)
			cout << "\t" << i + 1 << "  " << char(179);
		else
			cout << "\t" << i + 1 << " " << char(179);
		for (int j = 0; j < size; j++)
		{
			//у первого игрока всегда отображается поле с кораблями
			printCell(secretFieldPl1[i][j]);
			cout << char(179);
		}
		if (i < 9)
			cout << "\t" << i + 1 << "  " << char(179);
		else
			cout << "\t" << i + 1 << " " << char(179);
		for (int j = 0; j < size; j++)
		{
#ifdef TEST
			printCell(secretFieldPl2[i][j]);
#else
			if (show)//опция для отображения поля в кастомной растановке кораблей в игре комп vs комп
				printCell(secretFieldPl2[i][j]);
			else
				printCell(viewFieldPl2[i][j]);
#endif // TEST
			cout << char(179);
		}
		//вывод средних линий
		if (i < size - 1) {
			cout << endl;
			for (int j = 0; j < 2; j++)
			{
				cout << "\t   " << char(195) << char(196) << char(196) << char(196);
				for (int i = 0; i < size - 1; i++)
				{
					cout << char(197) << char(196) << char(196) << char(196);
				}
				cout << char(180);
			}
			cout << endl;
		}
		//вывод последних линий
		else {
			cout << endl;
			for (int j = 0; j < 2; j++)
			{
				cout << "\t   " << char(192) << char(196) << char(196) << char(196);
				for (int i = 0; i < size - 1; i++)
				{
					cout << char(193) << char(196) << char(196) << char(196);
				}
				cout << char(217);
			}
			cout << endl;
		}
	}
}

void erase(direction*& arr, int& len, int ind) {
	direction* buf = new direction[len - 1];
	for (size_t i = 0; i < len - 1; i++)
	{
		if (i < ind)
			buf[i] = arr[i];
		else
			buf[i] = arr[i + 1];
	}
	delete[]arr;
	len--;
	arr = buf;
}
//окружение убитого корабля промахами
void shipCover(cellFilling** field, int i, int j, int length, direction d) {
	if (d == direction::right) // корабль горизонтальный
	{
		for (int k = j; k < j + length; k++)
		{
			if (i < (size - 1))
				field[i + 1][k] = cellFilling::miss; // сверху по всей длине
			if (i > 0)
				field[i - 1][k] = cellFilling::miss; //снизу
		}
		if (((j + length) - 1) < (size - 1))
			field[i][j + length] = cellFilling::miss; // крайний правый
		if (j > 0)
			field[i][j - 1] = cellFilling::miss; // крайний левый
		if (j > 0 && i > 0)
			field[i - 1][j - 1] = cellFilling::miss; // левый верхний
		if (j > 0 && i < (size - 1))
			field[i + 1][j - 1] = cellFilling::miss; // левый нижний
		if (i > 0 && (j + length - 1) < (size - 1))
			field[i - 1][j + length] = cellFilling::miss; // правый верхний
		if (i < (size - 1) && (j + length - 1) < (size - 1))
			field[i + 1][j + length] = cellFilling::miss; // правый нижний

	}
	else if (d == direction::down) // вертикальный
	{
		for (int k = i; k < i + length; k++)
		{
			if (j < (size - 1))
				field[k][j + 1] = cellFilling::miss; // справа по всей длине
			if (j > 0)
				field[k][j - 1] = cellFilling::miss; // слева
		}
		if ((i + length - 1) < (size - 1))
			field[i + length][j] = cellFilling::miss; // крайний нижний
		if (i > 0)
			field[i - 1][j] = cellFilling::miss; // крайний верхний
		if (j > 0 && i > 0)
			field[i - 1][j - 1] = cellFilling::miss; // левый верхний 
		if (j > 0 && (i + length - 1) < (size - 1))
			field[i + length][j - 1] = cellFilling::miss; // левый нижний
		if (i > 0 && j < (size - 1))
			field[i - 1][j + 1] = cellFilling::miss; // правый верхний
		if ((i + length - 1) < (size - 1) && j < (size - 1))
			field[i + length][j + 1] = cellFilling::miss; // правый нижний
	}
}
// расположение корабля по выбранный координатам и направлению
bool placeShip(cellFilling** field, ship* fleet, direction direct, int i, int j, int shipNum) {
	int shSize = (int)(fleet[shipNum].name);
	// проверка возможности расположить корабль
	if (((j + shSize) > size) && direct == direction::right ||
		((j - shSize) < 0) && direct == direction::left ||
		((i + shSize) > size) && direct == direction::down ||
		((i - shSize) < 0) && direct == direction::up)
		return false; // корабль выходит за край поля
	else {
		// корабль всегда ставится слева-направо, сверху-вниз
		// если выбраны обратные направления берется крайняя левая/верхняя точка
		// заданные координаты - размер корабля = новые координаты
		if (direct == direction::left)
			j = j - shSize + 1;
		if (direct == direction::up)
			i = i - shSize + 1;
		//проверка все ли клетки свободны там где будет корабль
		for (int k = 0; k < shSize; k++)
		{
			if (direct == direction::right || direct == direction::left) {
				if (field[i][j + k] != cellFilling::empty)
					return false;
			}
			else {
				if (field[i + k][j] != cellFilling::empty)
					return false;
			}
		}
	}
	//ставим корабль
	//горизонтально
	if (direct == direction::left || direct == direction::right) {
		for (int k = 0; k < shSize; k++)
		{
			field[i][j + k] = cellFilling(fleet[shipNum].name);
		}
		//для исключения возможности ставить корабли рядом они окружаются промахами
		shipCover(field, i, j, (int)fleet[shipNum].name, direction::right);
		fleet[shipNum].name = shipType::OnField;
		return true;
	}
	//вертикально
	else {
		for (int k = 0; k < shSize; k++)
		{
			field[i + k][j] = cellFilling(fleet[shipNum].name);
		}
		shipCover(field, i, j, (int)fleet[shipNum].name, direction::down);
		fleet[shipNum].name = shipType::OnField;
		return true;
	}
}

void customLocation(turn t = turn::player1) {
	cellFilling** sfield = nullptr;
	// по-умолчанию всегда работает для поля первого игрока (пользвателя)
	if (t == turn::player1) {
		sfield = secretFieldPl1;
	}
	// поле второго игрока в игре комп vs комп, если выбран пункт: Custom fleet location(both)
	else {
		sfield = secretFieldPl2;
	}
	ship* fleet = new ship[]{ shipType::Galleon,
								shipType::Fregata, shipType::Fregata,
								shipType::Brig, shipType::Brig, shipType::Brig,
								shipType::Schooner, shipType::Schooner,shipType::Schooner,shipType::Schooner };
	for (int k = 0; k < 10; k++)
	{
		system("cls");
		if (t == turn::player1)
			printFields();
		else
			printFields(1); // 1 - при отображении видно скрытое поле второго игрока
		while (fleet[k].name != shipType::OnField) {
			//если значение корабля не стало OnField, значит его нельзя поставить по выбранным координатам, запрашиваются новые
			string input;
			int i = -1, j = -1;
			string direct;
			while (1) {
				while (i == -1 || j == -1) {
					cout << "Enter start point for "; fleet[k].info(); cout << " -> ";
					getline(cin, input);
					if (input == "pause" || input == "exit")
					{
						//выход до окончания расстановки
						if (acceptMenu()) {
							if (input == "pause") // можно вернуться и продолжить с конечной точки
								exopt = exitopt::pause;
							else if (input == "exit") 
								exopt = exitopt::exit;
							return;
						}
					}
					else if (input == "")
						continue;
					else
						inputCheck(input, i, j);
				}
				if (sfield[i][j] != cellFilling::empty)
				{
					cout << "Already ocupied!\n";
					i = j = -1;
				}
				else
					break;
			}
			cout << "Enter dirrection (h - horizontal, v - vertical) -> ";//(1 - h, 0 - v)
			cin >> direct;
			cin.ignore();
			if (direct.length() != 1) {
				errorMenu();
			}
			else {
				if (direct[0] == 'h' || direct[0] == 'H') {
					if (!placeShip(sfield, fleet, direction::right, i, j, k)) {
						cout << "Unable to place!\n";
					}
				}
				else if (direct[0] == 'v' || direct[0] == 'V') {
					if (!placeShip(sfield, fleet, direction::down, i, j, k)) {
						cout << "Unable to place!\n";
					}
				}
				else
					errorMenu();
			}
		}
	}
	if (t == turn::player1)
		printFields();
	else
		printFields(1);
	//все 10 корабле на поле, удаляются окружающие корабли клетки miss
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (sfield[i][j] == cellFilling::miss)
				sfield[i][j] = cellFilling::empty;
		}
	}
	delete[]fleet;
}
bool randomLocation(turn t = turn::player2) {
	cellFilling** sfield = nullptr;
	// по-умолчанию всегда работает для поля второго игрока (компьютер)
	if (t == turn::player2)
		sfield = secretFieldPl2;
	//если выбран пункт меню:  Random fleet location или Random fleet location(both), работает для первого игоа
	else
		sfield = secretFieldPl1;
	ship* fleet = new ship[]{ shipType::Galleon,
							shipType::Fregata, shipType::Fregata,
							shipType::Brig, shipType::Brig, shipType::Brig,
							shipType::Schooner, shipType::Schooner, shipType::Schooner, shipType::Schooner };
	for (int n = 0; n < 10; n++)
	{
		int cnt = 0;
		int i, j;
		while (fleet[n].name != shipType::OnField)
		{
			cnt++;
			if (cnt == 10) {
				// если один корабль не смог расположится за 10 циклов, поле очищается, все начинается заново
				delete[]fleet;
				fieldsClean(sfield);
				return false;
			}
			else {
				int dirind;
				int cnt2 = 0;
				int leng = 4;
				direction* dir = new direction[leng]{ direction::down,
													direction::up,
													direction::right,
													direction::left };
				do
				{
					//генерирует координаты пока не найдет пустую точку
					if (cnt2 == size / 2) {
						// если за size / 2 цикла не найдена пустая клетка - все начинается заново
						delete[]fleet;
						delete[]dir;
						fieldsClean(sfield);
						return false;
					}
					i = rand() % size;
					j = rand() % size;
					cnt2++;
				} while (sfield[i][j] != cellFilling::empty);
				do{
					//генерирует направление пока корабль не поставится
					dirind = rand() % leng;
					if (!placeShip(sfield, fleet, dir[dirind], i, j, n)) {
						if (leng == 1)
							//если по всем направлениям корабль поставить невозможно, генерируются новые координаты
							break;
						erase(dir, leng, dirind);
					}
					else
						break;
				} while (1);
				delete[]dir;
			}
		}
	}
	delete[]fleet;
	//все 10 корабле на поле, удаляются окружающие корабли клетки miss
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (sfield[i][j] == cellFilling::miss)
				sfield[i][j] = cellFilling::empty;
		}
	}
	return true;
}
// убит ли корабль
bool isKilled(int i, int j, turn t = turn::player1) {
	cellFilling** defendViewField = nullptr;
	cellFilling** defendSecrField = nullptr;
	if (t == turn::player1) {
		//очередь 1-го игрока, под обстрелом поле 2-го игрока
		defendViewField = viewFieldPl2;
		defendSecrField = secretFieldPl2;
	}
	else {
		//очередь 2-го игрока, под обстрелом поле 1-го игрока
		defendViewField = viewFieldPl1;
		defendSecrField = secretFieldPl1;
	}
	bool flag = false;
	int mark;
	int cnt = 0;
	//найти крайнюю левую подбитую область корабля (на поле-отображения)
	for (mark = j; mark >= 0; mark--)
	{
		if (defendViewField[i][mark] == cellFilling::empty ||
			defendViewField[i][mark] == cellFilling::miss)
		{
			mark++; break;
		}
	}
	if (mark == -1)
		mark++;
	//выясняем сколько клеток данного корабля подбито (на поле-отображения)
	for (int k = mark; k < size; k++)
	{
		if (defendViewField[i][k] == cellFilling::empty ||
			defendViewField[i][k] == cellFilling::miss)
			break;
		else if (defendViewField[i][k] == cellFilling::hit)
			cnt++;
	}
	// сравниваем количество попаданий с размером корабля (на скрытом поле)
	// на поле отображения все подбитые клетки имеют значение hit
	// на скрытом поле зависит от корабля который расположен (если было brig то подбитое поле будет hitbrig)
	// разница между корабль и hit-корабль всегда 7
	if (cnt == (int)(defendSecrField[i][j]) - 7)
		flag = true;
	// если false, возможно это вертикальный корабль
	// проверка для вертикального корабля
	if (!flag) {
		mark = 0;
		cnt = 0;
		for (mark = i; mark >= 0; mark--)
		{
			if (defendViewField[mark][j] == cellFilling::empty ||
				defendViewField[mark][j] == cellFilling::miss)
			{
				mark++; break;
			}
		}
		if (mark == -1)
			mark++;
		//выясняем сколько клеток данного корабля подбито
		for (int k = mark; k < size; k++)
		{
			if (defendViewField[k][j] == cellFilling::empty ||
				defendViewField[k][j] == cellFilling::miss)
				break;
			else if (defendViewField[k][j] == cellFilling::hit)
				cnt++;
		}
		//сравниваем количество попаданий с размером корабля
		if (cnt == (int)(defendSecrField[i][j]) - 7)
			flag = true;
		//если равно (вертикальный) -> потоплен, окружается клетками miss, возвращает true
		if (flag) {
			shipCover(defendViewField, mark, j, cnt, direction::down);
			for (int k = 0; k < size; k++)
			{
				for (int l = 0; l < size; l++)
				{
					if (defendViewField[k][l] == cellFilling::miss)
						defendSecrField[k][l] = defendViewField[k][l];
				}
			}
			return true;
		}
		//если не равно ->  простое попадание, возвращает false
		else
			return false;
	}
	//если равно (для горизотального корабля) -> потоплен, окружается клетками miss, возвращает true
	else {
		shipCover(defendViewField, i, mark, cnt, direction::right);
		for (int k = 0; k < size; k++)
		{
			for (int l = 0; l < size; l++)
			{
				if (defendViewField[k][l] == cellFilling::miss)
					defendSecrField[k][l] = defendViewField[k][l];
			}
		}
		return true;
	}

}
// проверка попадания
bool isHit(int i, int j, turn t = turn::player1) {
	cellFilling** defendViewField = nullptr;
	cellFilling** defendSecrField = nullptr;
	player* attaker = nullptr;
	if (t == turn::player1) {
		//очередь 1-го игрока, под обстрелом поле 2-го игрока
		attaker = &player1;
		defendViewField = viewFieldPl2;
		defendSecrField = secretFieldPl2;
	}
	else {
		//очередь 2-го игрока, под обстрелом поле 1-го игрока
		attaker = &player2;
		defendViewField = viewFieldPl1;
		defendSecrField = secretFieldPl1;
	}
	// если клетка поля-отображения совпадает с кораблем на скрытом поле - попадание
	if (defendSecrField[i][j] == cellFilling::scho ||
		defendSecrField[i][j] == cellFilling::brig ||
		defendSecrField[i][j] == cellFilling::freg ||
		defendSecrField[i][j] == cellFilling::gall)
	{
		// значение клетки на поле отображения меняется на hit
		defendViewField[i][j] = cellFilling::hit;
		// на скрытом поле на hit-корабль (+7 к значению корабля)
		defendSecrField[i][j] = cellFilling((int)(defendSecrField[i][j]) + 7);
		bool killing = isKilled(i, j, t); 
		printFields();
		cout << attaker->name << " turn: ";
		if (killing)
		{
			switch (defendSecrField[i][j])
			{
			case cellFilling::hitscho: cout << "Schooner";
				break;
			case cellFilling::hitbrig: cout << "Brig";
				break;
			case cellFilling::hitfreg: cout << "Fregata";
				break;
			case cellFilling::hitgall: cout << "Galleon";
				break;
			default:
				break;
			}
			cout << " is killed!\n";
			attaker->killedCnt++; // количество убитых кораблей противника увеличивается
		}
		else {
			cout << i + 1 << char(j + 65) << "\nHit!\n";
		}
		system("pause");
		return true;
	}
	else // промах
	{
		defendViewField[i][j] = cellFilling::miss;
		defendSecrField[i][j] = cellFilling::miss;
		printFields();
		cout << attaker->name << " turn: ";
		cout << i + 1 << char(j + 65) << "\nMiss!\n";
		system("pause");
		return false;
	}
}
// умный выстрел
bool compIntellectShot(turn t = turn::player2) {
	cellFilling** defendViewField = nullptr;
	player* attaker = nullptr;
	if (t == turn::player1) {
		//очередь 1-го игрока, под обстрелом поле 2-го игрока
		attaker = &player1;
		defendViewField = viewFieldPl2;
	}
	else {
		//очередь 2-го игрока, под обстрелом поле 1-го игрока
		attaker = &player2;
		defendViewField = viewFieldPl1;
	}
	int bufKillcnt = attaker->killedCnt;
	int i = attaker->savedI;
	int j = attaker->savedJ;
	// направление корабля неизвестно
	if (attaker->savedDir == direction::unknown) {
		while (1) {
			direction direct = (direction)(rand() % 4);
			if (direct == direction::down && i != (size - 1)) {
				if (defendViewField[i + 1][j] == cellFilling::miss)
					continue;//если снизу клетка промаха, продолжает искать направление
				else {
					// клетка пустая
					i++;
					if (isHit(i, j, t)) // стреляет по выбранному направлению
					{
						if (bufKillcnt != attaker->killedCnt) {
							// если сохраненное значение убитых кораблей не равно нынешнему, значит был убит корабль, координаты обнуляются
							attaker->savedI = -1;
							attaker->savedJ = -1;
						}
						else
						{
							// простое попадание, следующий выстрел будет в обратном направлении
							attaker->savedI = --i;
							attaker->savedDir = direction::up;
						}
						return true;
					}
					else // промах, координаты сохраняются
						return false;
				}
			}
			else if (direct == direction::up && i != 0) {
				if (defendViewField[i - 1][j] == cellFilling::miss)
					continue; // если сверху клетка промаха, продолжает искать направление
				else {
					i--;
					if (isHit(i, j, t))
					{
						if (bufKillcnt != attaker->killedCnt) {
							attaker->savedI = -1;
							attaker->savedJ = -1;
							attaker->savedDir = direction::unknown;
						}
						else {
							attaker->savedI = ++i;
							attaker->savedDir = direction::down;
						}
						return true;
					}
					else
						return false;
				}
			}
			else if (direct == direction::right && j != (size - 1)) {
				if (defendViewField[i][j + 1] == cellFilling::miss)
					continue; //если справа клетка промаха, продолжает искать направление
				else {
					j++;
					if (isHit(i, j, t))
					{
						if (bufKillcnt != attaker->killedCnt) {
							attaker->savedI = -1;
							attaker->savedJ = -1;
							attaker->savedDir = direction::unknown;
						}
						else {
							attaker->savedJ = --j;
							attaker->savedDir = direction::left;
						}
						return true;
					}
					else
						return false;
				}
			}
			else if (direct == direction::left && j != 0) {
				if (defendViewField[i][j - 1] == cellFilling::miss)
					continue; //если слева клетка промаха, продолжает искать направление
				else {
					j--;
					if (isHit(i, j, t))
					{
						if (bufKillcnt != attaker->killedCnt) {
							attaker->savedI = -1;
							attaker->savedJ = -1;
							attaker->savedDir = direction::unknown;
						}
						else {
							attaker->savedJ = ++j;
							attaker->savedDir = direction::right;
						}
						return true;
					}
					else
						return false;
				}
			}
		}
	}
	//направление корабля известно
	else {
		if (attaker->savedDir == direction::down) {
			if (i == (size - 1) || defendViewField[i + 1][j] == cellFilling::miss) {
				//если следующая клетка - промах, или вне поля, берется противоположное направление
				while (defendViewField[i][j] != cellFilling::empty) {
					i--;
				}
				attaker->savedI = ++i;
				attaker->savedDir = direction::up;
				return true;
			}
			else {
				i++;
				if (isHit(i, j, t)) {
					if (bufKillcnt != attaker->killedCnt) {
						attaker->savedI = -1;
						attaker->savedJ = -1;
						attaker->savedDir = direction::unknown;
					}
					else {
						//следующий выстрел в противоположном направлении
						while (i >= 0 && defendViewField[i][j] == cellFilling::hit) {
							i--;
						}
						attaker->savedI = ++i;
						attaker->savedDir = direction::up;
					}
					return true;
				}
				else {
					while (defendViewField[i][j] != cellFilling::empty) {
						i--;
					}
					attaker->savedI = ++i;
					attaker->savedDir = direction::up;
					return false;
				}
			}
		}
		else if (attaker->savedDir == direction::up) {
			if (i == 0 || defendViewField[i - 1][j] == cellFilling::miss) {
				while (defendViewField[i][j] != cellFilling::empty) {
					i++;
				}
				attaker->savedI = --i;
				attaker->savedDir = direction::down;
				return true;
			}
			else {
				i--;
				if (isHit(i, j, t)) {
					if (bufKillcnt != attaker->killedCnt) {
						attaker->savedI = -1;
						attaker->savedJ = -1;
						attaker->savedDir = direction::unknown;
					}
					else
					{
						while (i < size && defendViewField[i][j] == cellFilling::hit) {
							i++;
						}
						attaker->savedI = --i;
						attaker->savedDir = direction::down;
					}
					return true;
				}
				else {
					while (defendViewField[i][j] != cellFilling::empty) {
						i++;
					}
					attaker->savedI = --i;
					attaker->savedDir = direction::down;
					return false;
				}
			}
		}
		else if (attaker->savedDir == direction::right) {
			if (j == (size - 1) || defendViewField[i][j + 1] == cellFilling::miss) {
				while (defendViewField[i][j] != cellFilling::empty) {
					j--;
				}
				attaker->savedJ = ++j;
				attaker->savedDir = direction::left;
				return true;
			}
			else {
				j++;
				if (isHit(i, j, t)) {
					if (bufKillcnt != attaker->killedCnt) {
						attaker->savedI = -1;
						attaker->savedJ = -1;
						attaker->savedDir = direction::unknown;
					}
					else
					{
						while (j >= 0 && defendViewField[i][j] == cellFilling::hit) {
							j--;
						}
						attaker->savedJ = ++j;
						attaker->savedDir = direction::left;
					}
					return true;
				}
				else {
					while (defendViewField[i][j] != cellFilling::empty) {
						j--;
					}
					attaker->savedJ = ++j;
					attaker->savedDir = direction::left;
					return false;
				}
			}
		}
		else if (attaker->savedDir == direction::left) {
			if (j == 0 || defendViewField[i][j - 1] == cellFilling::miss) {
				while (defendViewField[i][j] != cellFilling::empty) {
					j++;
				}
				attaker->savedJ = --j;
				attaker->savedDir = direction::right;
				return true;
			}
			else {
				j--;
				if (isHit(i, j, t)) {
					if (bufKillcnt != attaker->killedCnt) {
						attaker->savedI = -1;
						attaker->savedJ = -1;
						attaker->savedDir = direction::unknown;
					}
					else
					{
						while (j < size && defendViewField[i][j] == cellFilling::hit) {
							j++;
						}
						attaker->savedJ = --j;
						attaker->savedDir = direction::right;
					}
					return true;
				}
				else {
					while (defendViewField[i][j] != cellFilling::empty) {
						j++;
					}
					attaker->savedJ = --j;
					attaker->savedDir = direction::right;
					return false;
				}
			}
		}
	}
}
// рандомный выстрел
bool compRandomShot(turn t = turn::player2) {
	cellFilling** defendViewField = nullptr;
	player* attaker = nullptr;
	if (t == turn::player1) {
		//очередь 1-го игрока, под обстрелом поле 2-го игрока (для игры комп vs комп)
		attaker = &player1;
		defendViewField = viewFieldPl2;
	}
	else {
		//очередь 2-го игрока, под обстрелом поле 1-го игрока
		attaker = &player2;
		defendViewField = viewFieldPl1;
	}
	int i, j;
	while (1) {
		//генерирует координаты пока не найдет пустую клетку
		i = rand() % size;
		j = rand() % size;
		if (defendViewField[i][j] != cellFilling::empty)
			continue;
#ifndef SMALL
		//если минимальный размер корабля 2, то игнорируются все одиночные пустые клетки, так как с них точно не может быть попадания
		if (i == 0 && j == 0 && defendViewField[i + 1][j] == cellFilling::miss && defendViewField[i][j + 1] == cellFilling::miss ||
			i == 0 && j == (size - 1) && defendViewField[i + 1][j] == cellFilling::miss && defendViewField[i][j - 1] == cellFilling::miss ||
			i == (size - 1) && j == (size - 1) && defendViewField[i - 1][j] == cellFilling::miss && defendViewField[i][j - 1] == cellFilling::miss ||
			i == (size - 1) && j == 0 && defendViewField[i - 1][j] == cellFilling::miss && defendViewField[i][j + 1] == cellFilling::miss ||
			i == 0 && 0 < j && j < (size - 1) && defendViewField[i + 1][j] == cellFilling::miss && defendViewField[i][j + 1] == cellFilling::miss && defendViewField[i][j - 1] == cellFilling::miss ||
			j == (size - 1) && 0 < i && i < (size - 1) && defendViewField[i + 1][j] == cellFilling::miss && defendViewField[i - 1][j] == cellFilling::miss && defendViewField[i][j - 1] == cellFilling::miss ||
			i == (size - 1) && 0 < j && j < (size - 1) && defendViewField[i][j + 1] == cellFilling::miss && defendViewField[i - 1][j] == cellFilling::miss && defendViewField[i][j - 1] == cellFilling::miss ||
			j == 0 && 0 < i && i < (size - 1) && defendViewField[i + 1][j] == cellFilling::miss && defendViewField[i - 1][j] == cellFilling::miss && defendViewField[i][j + 1] == cellFilling::miss)
			continue;
#endif // !SMALL
		break;
	}
	if (isHit(i, j, t)) { // при попадании сохраняются координаты
		attaker->savedI = i;
		attaker->savedJ = j;
		return true;
	}
	else
		return false;
}
bool userShot() {
	string input;
	int j = -1, i = -1;
	while (1) {
		printFields();
		cout << player1.name << " turn:\n";
		while (i == -1 || j == -1) {
			cout << "Enter your target -> ";
			getline(cin, input);
			if (input == "pause" || input == "exit")
			{
				if (acceptMenu()) {
					if (input == "pause")
						exopt = exitopt::pause;
					else if (input == "exit") {
						exopt = exitopt::exit;
						score.allGame++;
						score.userLoss++;
					}
					return false;
				}
			}
			else if (input == "")
				continue;
			else
				inputCheck(input, i, j);
		}
		if (viewFieldPl2[i][j] == cellFilling::empty)
			break;
		else
		{
			cout << "Already fired!\n";
			i = j = -1;
		}
	}
	return isHit(i, j);
}
// компьютер против компьютера
void compVsCompGame() {
	int cnt = 0;
	printFields();
	system("pause");
	while (1) {
		while (1) {
			// если нет сохраненных координат, стреляет рандомно
			if (player1.savedI == -1 && player1.savedJ == -1) {
				if (compRandomShot(turn::player1)) {
					if (compIntellectShot(turn::player1)) {}
					else
						break;
				}
				else
					break;
			}
			//координаты сохранены - интеллектуальный выстрел
			else {
				if (compIntellectShot(turn::player1)) {}
				else
					break;
			}
			if (player1.killedCnt == 10) {
				cout << "All " << player2.name << " ships killed.\n";
				cout << player1.name << " is a winner!!!\n";
				exopt = exitopt::endgame;
				score.allGame++;
				score.comp1Wins++;
				return;
			}
		}
		system("cls");
		while (1) {
			if (player2.savedI == -1 && player2.savedJ == -1) {
				if (compRandomShot()) {
					if (compIntellectShot()) {}
					else
						break;
				}
				else
					break;
			}
			else {
				if (compIntellectShot()) {}
				else
					break;
			}
			if (player2.killedCnt == 10) {
				cout << "All " << player1.name << " ships killed.\n";
				cout << player2.name << " is a winner!!!\n";
				exopt = exitopt::endgame;
				score.allGame++;
				score.comp2Wins++;
				return;
			}
		}
		if (cnt % 5 == 0 && cnt != 0) // через каждые 5 кругов предлагает покинуть игру
		{
			while (1)
			{
				cout << "\nPause/Exit?\n";
				cout << "Enter \"pause\" or \"exit\" or \"no\"\n->";
				string input;
				getline(cin, input);
				if (input == "no") {
					break;
				}
				else if (input == "pause" || input == "exit") {
					if (acceptMenu()) {
						if (input == "pause")
							exopt = exitopt::pause;
						else
							exopt = exitopt::exit;
						return;
					}
				}
				else
					errorMenu();
			}
		}
		cnt++;
		system("cls");
	}
}
//пользователь против компьютера
void userVsCompGame() {
	while (1) {
		//ход пользователя
		while (1) {
			if (!(userShot())) { 
				if (exopt == exitopt::pause ||
					exopt == exitopt::exit)
					return; // пользователь хочет выйти
				else
					break; // промах
			}
			if (player1.killedCnt == 10) {
				cout << "All " << player2.name << " ships killed.\n";
				cout << player1.name << " is a winner!!!\n";
				exopt = exitopt::endgame;
				score.allGame++;
				score.userWins++;
				return;
			}
		}
		system("cls");
		//ход компьютера
		while (1) {
			// нет сохраненных координа - рандомный выстрел
			if (player2.savedI == -1 && player2.savedJ == -1) {
				if (compRandomShot())
					if (compIntellectShot()) {}
					else
						break;
				else
					break;
			}
			//координаты сохранены - интеллектуальный выстрел
			else {
				if (compIntellectShot()) {}
				else
					break;
			}
			if (player2.killedCnt == 10) {
				cout << "All " << player1.name << " ships killed.\n";
				cout << player2.name << " is a winner!!!\n";
				exopt = exitopt::endgame;
				score.allGame++;
				score.userLoss++;
				return;
			}
		}
		system("cls");
	}
}

void compVsCompMenu() {
	//fieldFlag - есть ли на поле корабли, gameFlag - начата ли игра
	bool fieldFlag = false, gameFlag = false;
	player1.name = "Computer #1";
	player2.name = "Computer #2";
	while (1) {
		system("cls");
		cout << "\t***" << player1.name << " VS " << player2.name << "***\n\n";
		cout << "1. Random fleet location(both);\n";
		cout << "2. Custom fleet location(both);\n";
		cout << "3. Custom fleet location(one player);\n";
		cout << "4. Clean fields (new game);\n";
		cout << "5. Change players name;\n";
		cout << "6. Start/Continue;\n";
		cout << "0. Exit.\n";
		cout << "-> ";
		int ch;
		cin >> ch;
		cin.ignore();
		switch (ch)
		{
		case 1:
			if (gameFlag) {//если уже сть начатая игра, нельзя заново поставить корабли
				cout << "Game already began! Ships can not be replaced!\n";
			}
			else {
				if (!fieldFlag) {
					while (1) {
						if (randomLocation(turn::player1))
							break;
					}
					while (1) {
						if (randomLocation())
							break;
					}
					cout << "Done!\n";
					fieldFlag = true;
				}
				else // если корабли уже расставлены, заново их расставить нельзя
					cout << "\nFleet already placed!\n";
			}
			break;
		case 2:
			if (gameFlag) {
				cout << "Game already began! Ships can not be replaced!\n";
			}
			else {
				if (!fieldFlag) {
					customLocation();
					if (exopt == exitopt::exit) {
						// если пользователь вышел не окончив расстановку, поле очищается
						fieldsClean(secretFieldPl1);
						exopt = exitopt::endgame;
					}
					else {
						cout << "First done!\n";
						system("pause");
						customLocation(turn::player2);
						if (exopt == exitopt::exit) {
							// если пользователь вышел не окончив расстановку, оба поля очищаются
							fieldsClean(secretFieldPl1, secretFieldPl2);
							exopt = exitopt::endgame;
						}
						else {
							fieldFlag = true; // корабли расставлены на обоих полях
							cout << "Second done!";
						}
					}
				}
				else
					cout << "Fleet already placed!\n";
			}
			break;
		case 3:
			if (gameFlag) {
				cout << "Game already begun! Ships can not be replaced!\n";
			}
			else {
				if (!fieldFlag) {
					customLocation();
					if (exopt == exitopt::exit) {
						// выход до окончания расстановки всех кораблей 
						fieldsClean(secretFieldPl1);
						exopt = exitopt::endgame;
					}
					else {
						while (1) {
							if (randomLocation())
								break;
						}
						fieldFlag = true;
					}
				}
				else
					cout << "Fleet already placed!\n";
			}
			break;
		case 4:
			if (!fieldFlag) // на полях нет кораблей
				cout << "No ships on field!\n";
			else
			{
				if (gameFlag) { // очистка полей приведет к окончанию текущей игры
					cout << "Game has started! Cleaning fields leads to game over!\n";
					if (acceptMenu()) {
						fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
						gameFlag = false;
						fieldFlag = false;
						cout << "Done!\n";
					}
				}
				else {
					fieldsClean(secretFieldPl1, secretFieldPl2);
					fieldFlag = false;
					cout << "Done!\n";
				}
			}
			break;
		case 5:
			cout << "Enter first player name -> ";
			getline(cin, player1.name);
			cout << "Enter second player name -> ";
			getline(cin, player2.name);
			cout << "Done!\n";
			break;
		case 6:
			if (fieldFlag) {
				exopt = exitopt::endgame;
				compVsCompGame();
				if (exopt == exitopt::pause) {
					// прогресс текущей игры сохраняется
					gameFlag = true;
					exopt = exitopt::endgame;
				}
				else if (exopt == exitopt::exit) {
					// выход из игры до окончания
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					exopt = exitopt::endgame;
					return;
				}
				else {
					// игра окончена, есть победитель
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					player1.valsRegen();
					player2.valsRegen();
					gameFlag = false;
					fieldFlag = false;
				}
			}
			else // если на полях нет кораблей игру начать нельзя
				cout << "No ships on fields!\n";
			break;
		case 0:
			if (gameFlag) {
				// выход в лавное меню приведет к окончанию текущей игры
				cout << "Game has started! Exit leads to game over!\n";
				if (acceptMenu())
				{
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					return;
				}
			}
			else if (fieldFlag) {
				fieldsClean(secretFieldPl1, secretFieldPl2);
				return;
			}
			else
				return;
			break;
		default:
			errorMenu();
			break;
		}
		system("pause");
	}
}
void userVsCompMenu() {
	//fieldFlag - есть ли на поле корабли, gameFlag - начата ли игра
	bool fieldFlag = false, gameFlag = false;
	player1.name = "User";
	player2.name = "Computer";
	while (1) {
		system("cls");
		cout << "\t***" << player1.name << " VS " << player2.name << "***\n\n";
		cout << "1. Random fleet location;\n";
		cout << "2. Custom fleet location;\n";
		cout << "3. Clean fields (new game);\n";
		cout << "4. Change user name;\n";
		cout << "5. Start / Continue;\n";
		cout << "0. Exit.\n";
		cout << "-> ";
		int ch;
		cin >> ch;
		cin.ignore();
		switch (ch)
		{
		case 1:
			
			if (gameFlag) { // уже начата игра
				cout << "Game already began! Ships can not be replaced!\n";
			}
			else { // поля пустые
				if (!fieldFlag) {
					while (1) {
						if (randomLocation(turn::player1))
							break;
					}
					while (1) {
						if (randomLocation())
							break;
					}
					cout << "Done!\n";
					fieldFlag = true;
				}
				else // на полях уже расставлены корабли
					cout << "\nFleet already placed!\n";
			}
			break;
		case 2:
			if (gameFlag) {
				cout << "Game already began! Ships can not be replaced!\n";
			}
			else {
				if (!fieldFlag) {
					customLocation();
					if (exopt == exitopt::exit || exopt == exitopt::pause) {
						// пользователь вышел до окончательной расстановки кораблей
						fieldsClean(secretFieldPl1);
						exopt = exitopt::endgame;
					}
					else {
						while (1) {
							if (randomLocation())
								break;
						}
						fieldFlag = true;
					}
				}
				else
					cout << "Fleet already placed!\n";
			}
			break;
		case 3:
			if (!fieldFlag) // поля пустые
				cout << "No ships on fields!\n";
			else
			{
				if (gameFlag) { // игра начата
					cout << "Game has started! Cleaning fields leads to game over!\n";
					if (acceptMenu()) {
						// очистка ведет к автоматическому проигрышу пользователя
						score.allGame++;
						score.userLoss++;
						fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
						player1.valsRegen();
						player2.valsRegen();
						gameFlag = false;
						fieldFlag = false;
						cout << "Done!\n";
					}
				}
				else {
					fieldsClean(secretFieldPl1, secretFieldPl2);
					fieldFlag = false;
					cout << "Done!\n";
				}
			}
			break;
		case 4:
			cout << "Enter your name -> ";
			getline(cin, player1.name);
			cout << "Done!\n";
			break;
		case 5:
			if (fieldFlag) { // корабли расставлены, игра начинается
				exopt = exitopt::endgame;
				userVsCompGame();
				if (exopt == exitopt::pause) {
					// можно вернутся в текущую игру
					gameFlag = true;
					exopt = exitopt::endgame;
				}
				else if (exopt == exitopt::exit) {
					// пользователь вышел до окончания игры, автоматический проигрыш
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					score.allGame++;
					score.userLoss++;
					exopt = exitopt::endgame;
					return;
				}
				else {
					// игра окончена, есть победитель
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					player1.valsRegen();
					player2.valsRegen();
					gameFlag = false;
					fieldFlag = false;
				}
			}
			else // на полях нет кораблей, игру начать нельзя
				cout << "No ships on field!\n";
			break;
		case 0:
			if (gameFlag) {
				cout << "Game has started! Exit leads to game over!\n";
				if (acceptMenu())
				{
					//текущая игра заканчивается, проигрыш пользователя
					fieldsClean(secretFieldPl1, viewFieldPl1, secretFieldPl2, viewFieldPl2);
					score.allGame++;
					score.userLoss++;
					return;
				}
			}
			else if (fieldFlag) {
				fieldsClean(secretFieldPl1, secretFieldPl2);
				return;
			}
			else
				return;
			break;
		default:
			errorMenu();
			break;
		}
		system("pause");
	}
}

void mainMenu() {
	fieldsCreate();
	while (1) {
		system("cls");
		cout << "\t***Battleship***\n\n";
		cout << "1. User VS Computer;\n";
		cout << "2. Computer #1 VS Computer #2;\n";
		cout << "3. Score;\n";
		cout << "0. Exit.\n";
		cout << "-> ";
		int ch;
		cin >> ch;
		cin.ignore();
		switch (ch)
		{
		case 1:
			userVsCompMenu();
			break;
		case 2:
			compVsCompMenu();
			break;
		case 3:
			system("cls");
			cout << "\t***Score***\n\n";
			cout << "Games played: " << score.allGame << endl;
			cout << "Users wins: " << score.userWins << endl;
			cout << "Users losses: " << score.userLoss << endl;
			cout << "Computer #1 wins: " << score.comp1Wins << endl;
			cout << "Computer #2 wins: " << score.comp2Wins << endl;
			break;
		case 0:
			if (acceptMenu()) {
				fieldsDelete();
				return;
			}
			break;
		default:
			errorMenu();
			break;
		}
		system("pause");
	}
}
void main() {
	cons = GetStdHandle(STD_OUTPUT_HANDLE);
	srand(time(0));
	mainMenu();
}