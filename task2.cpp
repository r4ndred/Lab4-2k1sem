#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>

using namespace std;

// Структура для хранения даты
struct Date {
    int day;
    int month;
    int year;

    // Метод для проверки, находится ли дата в диапазоне [start, end]
    bool isInRange(const Date& start, const Date& end) const {
        if (year < start.year || year > end.year) return false;
        if (year == start.year && (month < start.month || (month == start.month && day < start.day))) return false;
        if (year == end.year && (month > end.month || (month == end.month && day > end.day))) return false;
        return true;
    }
};

// Глобальные переменные для хранения результатов и мьютекса для синхронизации
vector<Date> results; // Вектор для хранения найденных дат
mutex resultsMutex; // Мьютекс для синхронизации доступа к вектору результатов

// Функция для обработки части массива данных
void processDates(const vector<Date>& dates, const Date& start, const Date& end, int startIdx, int endIdx, vector<Date>& localResults) {
    for (int i = startIdx; i < endIdx; ++i) { // Перебираем заданную часть массива дат
        if (dates[i].isInRange(start, end)) { // Проверяем, находится ли дата в заданном диапазоне
            localResults.push_back(dates[i]); // Добавляем дату в локальные результаты
        }
    }
}

// Функция для обработки данных без многопоточности
void processWithoutThreading(const vector<Date>& dates, const Date& start, const Date& end) {
    results.clear(); // Очищаем результаты
    processDates(dates, start, end, 0, dates.size(), results); // Обрабатываем все данные в одном потоке
}

// Функция для обработки данных с использованием многопоточности
void processWithThreading(const vector<Date>& dates, const Date& start, const Date& end, int numThreads) {
    results.clear(); // Очищаем результаты
    vector<thread> threads; // Вектор для хранения потоков
    int chunkSize = dates.size() / numThreads; // Размер части данных для каждого потока
    vector<vector<Date>> localResults(numThreads); // Вектор для локальных результатов

    for (int i = 0; i < numThreads; ++i) { // Создаем указанное количество потоков
        int startIdx = i * chunkSize; // Начальный индекс для текущего потока
        int endIdx = (i == numThreads - 1) ? dates.size() : startIdx + chunkSize; // Конечный индекс для текущего потока
        threads.emplace_back(processDates, ref(dates), ref(start), ref(end), startIdx, endIdx, ref(localResults[i])); // Создаем поток и запускаем обработку части данных
    }

    for (auto& t : threads) {
        t.join(); // Ожидаем завершения всех потоков
    }

    // Слияние локальных результатов
    for (const auto& localResult : localResults) {
        results.insert(results.end(), localResult.begin(), localResult.end());
    }
}

// Функция для генерации случайных дат
vector<Date> generateRandomDates(int count) {
    vector<Date> dates; // Вектор для хранения сгенерированных дат
    random_device rd; // Устройство для генерации случайных чисел
    mt19937 gen(rd()); // Генератор случайных чисел
    uniform_int_distribution<int> dayDist(1, 31);
    uniform_int_distribution<int> monthDist(1, 12); 
    uniform_int_distribution<int> yearDist(2000, 2025); 
    
    for (int i = 0; i < count; ++i) { // Генерируем заданное количество случайных дат
        dates.push_back({dayDist(gen), monthDist(gen), yearDist(gen)}); // Добавляем случайную дату в вектор
    }
    return dates; // Возвращаем вектор случайных дат
}

int main() {
    // Генерация случайных данных
    int numDates = 100000; // Количество случайных дат
    vector<Date> dates = generateRandomDates(numDates); // Генерируем случайные даты
    
    Date start = {1, 1, 2010}; // Начальная дата диапазона
    Date end = {31, 12, 2025};  // Конечная дата диапазона

    int numThreads = 4; // Количество потоков

    // Обработка без многопоточности
    auto startTime = chrono::high_resolution_clock::now(); // Засекаем время начала
    processWithoutThreading(dates, start, end); // Обрабатываем данные
    auto endTime = chrono::high_resolution_clock::now(); // Засекаем время окончания
    chrono::duration<double> elapsed = endTime - startTime; // Вычисляем время выполнения
    cout << "Время без многопоточности: " << elapsed.count() << " секунд\n"; // Выводим время выполнения
    cout << "Найдено дат: " << results.size() << "\n"; // Выводим количество найденных дат

    // Обработка с многопоточностью
    startTime = chrono::high_resolution_clock::now(); // Засекаем время начала
    processWithThreading(dates, start, end, numThreads); // Обрабатываем данные в многопоточном режиме
    endTime = chrono::high_resolution_clock::now(); // Засекаем время окончания
    elapsed = endTime - startTime; // Вычисляем время выполнения
    cout << "Время с многопоточностью: " << elapsed.count() << " секунд\n"; // Выводим время выполнения
    cout << "Найдено дат: " << results.size() << "\n"; // Выводим количество найденных дат

    return 0; // Завершаем выполнение программы
}
