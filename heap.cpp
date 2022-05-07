#include <windows.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <fstream>

#define __HEAP_MAX (24 * 1024)

using namespace std;


//To use it you need any argv argument
void step_mode(int is_step_mode){
    if (is_step_mode > 1){
        char button;

        do{
            button = cin.get();
        }while (button != '\n');
    }
}



int main(int argc, char *argv[]){

    setlocale(LC_ALL, "Russian");



    // Point 1. Heap creating.
    auto heap = HeapCreate(HEAP_NO_SERIALIZE, 0, __HEAP_MAX);

    if (heap != NULL){
        cout << "1) Куча создана\n";
    }
    else{
       return EXIT_FAILURE;
    }

    step_mode(argc);



    // Point 2. Using heap
    uniform_int_distribution<size_t> block_size(32, 1024);
    mt19937 generator(random_device{}());

    do{
        size_t sz = block_size(generator);
        if (!HeapAlloc(heap, 0, sz)){
            break;
        }
    }while (true);
    cout << "2) Сейчас в куче потрачена вся память" << "\n";

    step_mode(argc);



    //Point 3. Measurements
    PROCESS_HEAP_ENTRY entry{};

    size_t summury = 0;
    size_t over_summury = 0;

    while (HeapWalk(heap, &entry)){
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)){
            over_summury += entry.cbOverhead;
            summury += entry.cbData;
        }
    }
    auto in_total = summury + over_summury;

    cout << "3) Параметры кучи" << "\n";
    cout << "    " << "1. Cуммарный объем хранимых данных: " << summury << "\n" <<
            "    " << "2. Cуммарные накладные расходы: " << over_summury << "\n" <<
            "    " << "3. Все расходы: " << in_total << "\n" <<
            "    " << "4. Разность между размером кучи и суммой величин пунктов '3.1' и '3.2': " <<
                                                                 __HEAP_MAX - in_total << "\n";

    step_mode(argc);



    //Point 4. Blocks become free
    uniform_int_distribution<size_t> free(1, 3);

    LPVOID free_block = nullptr;
    entry.lpData = nullptr;

    while (HeapWalk(heap, &entry)){
        if (free_block){
            HeapFree(heap, 0, free_block);
            free_block = nullptr;
        }
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)){
            if (free(generator) == 1){
                free_block = entry.lpData;
            }
            else{
                free_block = nullptr;
            }
        }
    }

    if (free_block){
        HeapFree(heap, 0, free_block);
    }

    cout << "4) Освобождение блоков с вероятностью 1/3" << "\n\n";

    step_mode(argc);


    //Point 5. Measurements again
    summury = 0;
    over_summury = 0;
    size_t summury_free = 0;
    size_t biggest_free = 0;
    entry.lpData = nullptr;

    while (HeapWalk(heap, &entry)){
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)){
            summury += entry.cbData;
            over_summury += entry.cbOverhead;
        }
        else if (!entry.wFlags){
            summury_free += entry.cbData;
            if (biggest_free < entry.cbData){
                biggest_free = entry.cbData;
            }
        }
    }

    in_total = summury + over_summury;

    cout << "5) Повторное измерение параметров кучи" << "\n";
    cout << "    " << "1. Cуммарный объем хранимых данных: " << summury << "\n" <<
            "    " << "2. Cуммарные накладные расходы: " << over_summury << "\n" <<
            "    " << "3. Все расходы: " << in_total << "\n" <<
            "    " << "4. Разность между размером кучи и суммой величин пунктов '5.1' и '5.2': " << __HEAP_MAX - in_total << "\n" <<
            "    " << "5. Суммарный свободный объем" << summury_free << "\n" <<
            "    " << "6. Наибольший объем свободного блока: " << biggest_free << "\n\n";

    step_mode(argc);



    //Point 6. Try to allocate memory blocks
    cout << "6) Попытки выделить блоки памяти" << "\n";


    LPVOID p = nullptr;
    size_t alloc_size_memory;


    if (biggest_free - 16 > 0){
        alloc_size_memory = biggest_free - 16;
    }
    else{
        alloc_size_memory = biggest_free;
    }
    cout << "Выделение " << alloc_size_memory << " байт: ";


    if ((p = HeapAlloc(heap, 0, alloc_size_memory))){
        cout << "Успешно" << "\n";
        HeapFree(heap, 0, p);
    }
    else{
        cout << "Не успешно" << "\n";
    }



    if (summury_free - 16 > 0){
        alloc_size_memory = summury_free - 16;
    }
    else{
        alloc_size_memory = summury_free;
    }
    cout << "Выделение " << alloc_size_memory << " байт: ";

    if ((p = HeapAlloc(heap, 0, alloc_size_memory))){
        cout << "Успешно" << "\n";
        HeapFree(heap, 0, p);
    }
    else{
        cout << "Не успешно" << "\n";
    }

    cout << "\n";

    step_mode(argc);



    // Point 7. Destroying the heap
    HeapDestroy(heap);
    cout << "7) Уничтожение кучи" << "\n\n";

    step_mode(argc);



    //Point 8. New heap
    heap = HeapCreate(HEAP_NO_SERIALIZE, 0, __HEAP_MAX);

    if (heap == NULL){
        return EXIT_FAILURE;
    }

    alloc_size_memory = 1;

    while (HeapAlloc(heap, 0, alloc_size_memory++));
    cout << "8) Распределение расходов" << "\n\n";
    cout.setf(ios::left);
    cout << setw(16) << "Размер блока" << "Расход" << "\n";

    entry.lpData = nullptr;

    fstream fout;
    fout.open("data.txt");

    while (HeapWalk(heap, &entry)){
        if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)){
            cout << setw(16) << entry.cbData << (unsigned)entry.cbOverhead << "\n";
            fout << entry.cbData << " " << (unsigned)entry.cbOverhead << "\n";
        }
    }
    fout.close();

    HeapDestroy(heap);

    step_mode(argc);

return 0;
}
